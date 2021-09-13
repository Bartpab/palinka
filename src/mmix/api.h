#ifndef __MMIX_API_H__
#define __MMIX_API_H__

#include <time.h>

#include "../sys.h"
#include "../utils.h"

#include "mem.h"
#include "processor.h"
#include "instr.h"
#include "dispatch.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

typedef void(*mmix_ivt_hdlr)(system_t* sys, instr_t* instr);

system_t* mmix_create(allocator_t* allocator, mmix_cfg_t* cfg);
void mmix_alloc_sim_time(system_t* s, unsigned int ms);
void mmix_step(system_t* sys);
void mmix_restart(system_t* sys);
void mmix_set_interrupt_handler(system_t* sys, byte ircode, mmix_ivt_hdlr hdlr);

static void __mmix_init(system_t* sys, mmix_cfg_t* cfg);

static inline void __fetch_next_instr(system_t* sys, mmix_processor_t* proc, instr_t* instr);
static inline void __convert_rel_to_abs_addr(mmix_processor_t* proc, instr_t* instr);
static inline void __install_operands(system_t* sys, mmix_processor_t* proc, instr_t* instr);

////////////
/// IMPL ///
////////////

static void __mmix_init(system_t* sys, mmix_cfg_t* cfg)
{
  sys->step = mmix_step;
  sys->alloc_sim_time = mmix_alloc_sim_time;
  
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->instr_ptr = (octa*) MMIX_START_ADDR;
  proc->frequency = cfg->frequency;
  proc->lmask     = cfg->lmask;
  proc->lsize     = cfg->lsize;
  proc->l = __get_mmix_lregs(sys);
  
  for(unsigned char i = 0; i != 0xFF; i++) 
  {
    proc->g[i] = 0x00;
    proc->ivt[i].hdlr = NULL;
  }
  
  for(unsigned int i = 0; i < proc->lsize; i++) 
  {
    proc->l[i] = 0x00;
  }

  proc->g[rK] = int_to_octa(-1);
  proc->g[rN] = tetra_to_octa(
    VERSION_MAJOR << 24 
    | VERSION_MINOR << 16 
    | VERSION_PATCH << 8, 
    time(NULL)
  );
  
  proc->g[rT]   = tetra_to_octa(0x80000005, 0);
  proc->g[rTT]  = tetra_to_octa(0x80000006, 0);
  proc->g[rV]   = tetra_to_octa(0x369c2004, 0);
  proc->rounding_mode = ROUND_NEAR;
 
  proc->rop = 0;

  proc->S = proc->L = proc->O = 0;
  proc->G = 32;

  proc->state = 0;
  proc->sclock = octa_zero;
}

void mmix_alloc_sim_time(system_t* sys, unsigned int ms)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);

  double s = ms / 1000.0;
  int nb_cycle = (int)(s * proc->frequency);

  proc->g[rI] = int_to_octa(nb_cycle);
}

void mmix_restart(system_t* sys)
{
  mmix_processor_t* proc = __get_mmix_proc(sys);

  proc->instr_ptr = (octa*) MMIX_START_ADDR;

  for(unsigned char i = 0; i != 0xFF; i++) 
  {
    proc->g[i] = 0x00;
    proc->ivt[i].hdlr = NULL;
  }
  
  for(unsigned int i = 0; i < proc->lsize; i++) 
  {
    proc->l[i] = 0x00;
  }

  proc->g[rK] = int_to_octa(-1);
  proc->g[rN] = tetra_to_octa(
    VERSION_MAJOR << 24 
    | VERSION_MINOR << 16 
    | VERSION_PATCH << 8, 
    time(NULL)
  );
  
  proc->g[rT]   = tetra_to_octa(0x80000005, 0);
  proc->g[rTT]  = tetra_to_octa(0x80000006, 0);
  proc->g[rV]   = tetra_to_octa(0x369c2004, 0);
  proc->rounding_mode = ROUND_NEAR;
 
  proc->rop = 0;

  proc->S = proc->L = proc->O = 0;
  proc->G = 32;

  proc->state = 0; 
  proc->sclock = octa_zero;
}

system_t* mmix_create(allocator_t* allocator, mmix_cfg_t* cfg)
{
  assert(cfg->lsize % 2 == 0);

  if(cfg->lsize < 256) cfg->lsize = 256;
  cfg->lmask = cfg->lsize - 1;
  
  system_t* sys = pmalloc(
    allocator, 
    sizeof(system_t) 
      + sizeof(mmix_processor_t) 
      + sizeof(octa) * cfg->lsize
  );
  
  if(!sys)
    return NULL;

  __sys_init(sys, allocator);
  __mmix_init(sys, cfg);
 
  return sys;
}

static inline void __fetch_next_instr(system_t* sys, mmix_processor_t* proc, instr_t* instr) {
  tetra raw_instr;

  // Resuming from TRIP/TRAP
  if(HAS_FLAG(MMIX_RESUMING, proc->state)) 
  {
    raw_instr = proc->g[rX];
    *instr = mmix_read_instr(raw_instr);

    proc->instr_ptr--;
    instr->loc = (octa) proc->instr_ptr;
  } else {
    MMIX_MEM_READ(sys, proc->instr_ptr, tetra, raw_instr);
    // Read the instruction
    *instr = mmix_read_instr(raw_instr);
    instr->loc = (octa) proc->instr_ptr;
    proc->instr_ptr++;
  }
}

static inline void __convert_rel_to_abs_addr(mmix_processor_t* proc, instr_t* instr) {
  if(instr->f & rel_addr_bit) {

    if((instr->op & 0xFE) == JMP) 
      instr->yz = instr->bin & 0xFFFFFF;

    if(instr->op & 1) 
      instr->yz -= (instr->op == JMPB ? 0x1000000 : 0x10000);

    instr->y = (octa) proc->instr_ptr;
    instr->z = (octa) (instr->loc + (instr->yz << 2));
  }
}

static inline void __install_operands(system_t* sys, mmix_processor_t* proc, instr_t* instr)
{
  if(HAS_FLAG(MMIX_RESUMING, proc->state) && proc->rop != MMIX_RESUME_AGAIN) {
    if(proc->rop == MMIX_RESUME_SET) 
    {
      instr->op = ORI;
      instr->y = proc->g[rZ];
      instr->z = tetra_to_octa(0, 0);
      instr->exc = ((proc->g[rX] >> 32) & 0xFF00);
      instr->f = X_is_dest_bit;
    } else {
      instr->y = proc->g[rY];
      instr->z = proc->g[rZ];
    }
  } else {
    if(instr->f & X_is_source_bit) {
      // Set b from register xx
      mmix_read_regv(proc, &instr->b, instr->xx);
    }
    
    // Set b from special register
    if(instr->info->third_operand) {
      instr->b = proc->g[instr->info->third_operand];
    }

    if(instr->f & Z_is_immed_bit) {
      instr->z = tetra_to_octa(0, instr->zz);
    } else if(instr->f & Z_is_source_bit) {
      mmix_read_regv(proc, &instr->z, instr->zz);
    } else if((instr->op & 0xF0) == SETH) {
      switch(instr->op & 0x3) {
        case 0: instr->z |= tetra_to_octa(instr->yz << 16, 0); 
        break;
        case 1: instr->z |= tetra_to_octa(instr->yz, 0); 
        break;
        case 2: instr->z |= tetra_to_octa(0, instr->yz << 16);
        break;
        case 3: instr->z |= tetra_to_octa(0, instr->yz);
        break;
      }
      instr->y = instr->b;
    }

    if(instr->f & Y_is_immed_bit) instr->y = instr->yy;
    else if(instr->f & Y_is_source_bit) {
      mmix_read_regv(proc, &instr->y, instr->yy);
    }
  }

  // Install register X as the dest, adjusting the reg stack if required. if L < x < G, then L = x + 1
  if(instr->f & X_is_dest_bit) 
  {
    if(instr->xx > proc->G) {
      instr->x_ptr = &proc->g[instr->xx];
    } else {
      while(instr->xx > proc->L) __mmix_lring_push_local(sys, proc);
      instr->x_ptr = &proc->l[(proc->O + instr->xx) & proc->lmask];
    }
  }
}

static bool __check_sys_timed_interrupt(system_t* sys, mmix_processor_t* proc)
{
  if(octa_eq(proc->g[rI], octa_zero)) 
  {
    sys_halt(sys);
    return true;
  }

  return false;
}

void mmix_step(system_t* sys)
{
  mmix_processor_t* proc;

  instr_t instr;

  proc = __get_mmix_proc(sys);

  // Resuming from TRIP/TRAP
  __fetch_next_instr(sys, proc, &instr);
  
  // Convert rel addr to abs addr
  __convert_rel_to_abs_addr(proc, &instr);

  // Install operand fields
  __install_operands(sys, proc, &instr);

  bool overflow = false;
  instr.w = octa_plus(instr.y, instr.z, &overflow);

  // Dispatch the instruction.
  mmix_dispatch(sys, proc, &instr);
  
  // Set the exception
  proc->g[rA] = instr.exc;

  if(!__check_sys_timed_interrupt(sys, proc)) 
  {

    bool overflow;
    proc->sclock = octa_plus(
      proc->sclock, 
      octa_left_shift(
        int_to_octa(instr.info->mems), 
        32
      ),
      &overflow
    );

    proc->g[rU] = octa_incr(proc->g[rU], 1);
    __mmix_sclock_incr(proc, instr.info->oops);
    __check_sys_timed_interrupt(sys, proc);
  }


  // Check for RESUME
  if(instr.op != RESUME && HAS_FLAG(MMIX_RESUMING, proc->state)) {
      FLAG_OFF(MMIX_RESUMING, proc->state);
    }

}

void mmix_set_interrupt_handler(system_t* sys, byte ircode, void (*hdlr)(system_t* sys, instr_t* instr))
{
  mmix_processor_t* proc = __get_mmix_proc(sys);
  proc->ivt[ircode].hdlr = hdlr;
}

#endif