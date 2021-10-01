#ifndef __RISCV_CSR_H__
#define __RISCV_CSR_H__

typedef enum {
    // Machine info registers
    MVENDORID = 0xF11, // Vendor ID
    MARCHID = 0xF12, // Architecture ID
    MIMPID = 0xF13, // Implementation ID
    MHARTID = 0xF14, // Hardware thread ID
    // Machine trap setup
    MSTATUS = 0x300, // Machine status register
    MISA = 0x301, // ISA and extensions
    MEDELEG = 0x302, // Machine exception delegation register
    MIDELEG = 0x303, // Machine interrupt delegation register
    MIE = 0x304, // Machine interrupt-enable register
    MTVEC = 0x305, // Machine trap-handler base address
    MTCOUNTEREN = 0x306, // Machine counter enable
    MSTATUSH = 0x310, // Additional machine status register, RV32 only
    // Machine trap handling
    MSCRATCH = 0x340, // Machine status register
    MEPC = 0x341, // Machine exception program counter
    MCAUSE = 0x342, // Machine trap cause
    MTVAL = 0x343, // Machine bad address or instruction
    MIP = 0x344, // Machine interrupt pending
    MTINST = 0x34A, // Machine trap instruction (transformed)
    MTVAL2 = 0x34B, // Machine bad guest physical address
    // Machine memory protection
    PMPCFG0 = 0x3A0, // Physical memory protection config.
    PMPCFG1 = 0x3A1, // Physical memory protection config, RV32 only
    PMPCFG2 = 0x3A2, // Physical memory protection config.    
    PMPCFG3 = 0x3A3, // Physical memory protection config, RV32 only
    PMPCFG4 = 0x3A4, // Physical memory protection config.   
    PMPCFG5 = 0x3A5, // Physical memory protection config, RV32 only
    PMPCFG6 = 0x3A6, // Physical memory protection config.
    PMPCFG7 = 0x3A7, // Physical memory protection config, RV32 only
    PMPCFG8 = 0x3A8, // Physical memory protection config.    
    PMPCFG9 = 0x3A9, // Physical memory protection config, RV32 only
    PMPCFG10 = 0x3AA, // Physical memory protection config.   
    PMPCFG11 = 0x3AB, // Physical memory protection config, RV32 only
    PMPCFG12 = 0x3AC, // Physical memory protection config.
    PMPCFG13 = 0x3AD, // Physical memory protection config, RV32 only
    PMPCFG14 = 0x3AE, // Physical memory protection config.    
    PMPCFG15 = 0x3AF, // Physical memory protection config, RV32 only
    PMPADDR0 = 0x3B0, // Physical memory protection address register
    PMPADDR1 = 0x3B1, // Physical memory protection address register
    PMPADDR2 = 0x3B2, // Physical memory protection address register
    PMPADDR3 = 0x3B3, // Physical memory protection address register
    PMPADDR4 = 0x3B4, // Physical memory protection address register
    PMPADDR5 = 0x3B5, // Physical memory protection address register
    PMPADDR6 = 0x3B6, // Physical memory protection address register
    PMPADDR7 = 0x3B7, // Physical memory protection address register
    PMPADDR8 = 0x3B8, // Physical memory protection address register
    PMPADDR9 = 0x3B9, // Physical memory protection address register
    PMPADDR10 = 0x3BA, // Physical memory protection address register
    PMPADDR11 = 0x3BB, // Physical memory protection address register
    PMPADDR12 = 0x3BC, // Physical memory protection address register
    PMPADDR13 = 0x3BD, // Physical memory protection address register
    PMPADDR14 = 0x3BE, // Physical memory protection address register
    PMPADDR15 = 0x3BF, // Physical memory protection address register
    PMPADDR16 = 0x3C0, // Physical memory protection address register
    PMPADDR17 = 0x3C1, // Physical memory protection address register
    PMPADDR18 = 0x3C2, // Physical memory protection address register
    PMPADDR19 = 0x3C3, // Physical memory protection address register
    PMPADDR20 = 0x3C4, // Physical memory protection address register
    PMPADDR21 = 0x3C5, // Physical memory protection address register
    PMPADDR22 = 0x3C6, // Physical memory protection address register
    PMPADDR23 = 0x3C7, // Physical memory protection address register
    PMPADDR24 = 0x3C8, // Physical memory protection address register
    PMPADDR25 = 0x3C9, // Physical memory protection address register
    PMPADDR26 = 0x3CA, // Physical memory protection address register
    PMPADDR27 = 0x3CB, // Physical memory protection address register
    PMPADDR28 = 0x3CC, // Physical memory protection address register
    PMPADDR29 = 0x3CD, // Physical memory protection address register
    PMPADDR30 = 0x3CE, // Physical memory protection address register
    PMPADDR31 = 0x3CF, // Physical memory protection address register
    PMPADDR32 = 0x3D0, // Physical memory protection address register
    PMPADDR33 = 0x3D1, // Physical memory protection address register
    PMPADDR34 = 0x3D2, // Physical memory protection address register
    PMPADDR35 = 0x3D3, // Physical memory protection address register
    PMPADDR36 = 0x3D4, // Physical memory protection address register
    PMPADDR37 = 0x3D5, // Physical memory protection address register
    PMPADDR38 = 0x3D6, // Physical memory protection address register
    PMPADDR39 = 0x3D7, // Physical memory protection address register
    PMPADDR40 = 0x3D8, // Physical memory protection address register
    PMPADDR41 = 0x3D9, // Physical memory protection address register
    PMPADDR42 = 0x3DA, // Physical memory protection address register
    PMPADDR43 = 0x3DB, // Physical memory protection address register
    PMPADDR44 = 0x3DC, // Physical memory protection address register
    PMPADDR45 = 0x3DE, // Physical memory protection address register
    PMPADDR46 = 0x3DF, // Physical memory protection address register
    PMPADDR47 = 0x3E0, // Physical memory protection address register
    PMPADDR48 = 0x3E1, // Physical memory protection address register
    PMPADDR49 = 0x3E2, // Physical memory protection address register
    PMPADDR50 = 0x3E3, // Physical memory protection address register
    PMPADDR51 = 0x3E4, // Physical memory protection address register
    PMPADDR52 = 0x3E5, // Physical memory protection address register
    PMPADDR53 = 0x3E6, // Physical memory protection address register
    PMPADDR54 = 0x3E7, // Physical memory protection address register
    PMPADDR55 = 0x3E8, // Physical memory protection address register
    PMPADDR56 = 0x3E9, // Physical memory protection address register
    PMPADDR57 = 0x3EA, // Physical memory protection address register
    PMPADDR58 = 0x3EB, // Physical memory protection address register
    PMPADDR59 = 0x3EC, // Physical memory protection address register
    PMPADDR60 = 0x3ED, // Physical memory protection address register
    PMPADDR61 = 0x3EF, // Physical memory protection address register
    PMPADDR62 = 0x3F0, // Physical memory protection address register
    PMPADDR63 = 0x3F1, // Physical memory protection address register
    // Machine counter and timers
    MCYCLE = 0xB00, // Machine cycle counter
    MINSTRET = 0xB02, // Machine instructions-retired counter
    MHPMCOUNTER3 = 0xB03, // Machine performance monitoring counter
    MHPMCOUNTER4 = 0xB04, 
    MHPMCOUNTER5 = 0xB05,
    MHPMCOUNTER6 = 0xB06,
    MHPMCOUNTER7 = 0xB07,
    MHPMCOUNTER8 = 0xB08,
    MHPMCOUNTER9 = 0xB09,
    MHPMCOUNTER10 = 0xB0A, 
    MHPMCOUNTER11 = 0xB0B, 
    MHPMCOUNTER12 = 0xB0C,
    MHPMCOUNTER13 = 0xB0D,
    MHPMCOUNTER14 = 0xB0E,
    MHPMCOUNTER15 = 0xB0F,
    MHPMCOUNTER16 = 0xB10,
    MHPMCOUNTER17 = 0xB11, // Machine performance monitoring counter
    MHPMCOUNTER18 = 0xB12, 
    MHPMCOUNTER19 = 0xB13,
    MHPMCOUNTER20 = 0xB14,
    MHPMCOUNTER21 = 0xB15,
    MHPMCOUNTER22 = 0xB16,
    MHPMCOUNTER23 = 0xB17,
    MHPMCOUNTER24 = 0xB18, 
    MHPMCOUNTER25 = 0xB19, 
    MHPMCOUNTER26 = 0xB1A,
    MHPMCOUNTER27 = 0xB1B,
    MHPMCOUNTER28 = 0xB1C,
    MHPMCOUNTER29 = 0xB1D,
    MHPMCOUNTER30 = 0xB1E,
    MHPMCOUNTER31 = 0xB1F,
    MCYCLEH = 0xB80, // Upper 32 bits of mycle, RV32 only
    MINSTRETH = 0xB82, // Upper 32 bits of minstret, RV32 only
    MHPMCOUNTER3H = 0xB83, // Machine performance monitoring counter
    MHPMCOUNTER4H = 0xB84, 
    MHPMCOUNTER5H = 0xB85,
    MHPMCOUNTER6H = 0xB86,
    MHPMCOUNTER7H = 0xB87,
    MHPMCOUNTER8H = 0xB88,
    MHPMCOUNTER9H = 0xB89,
    MHPMCOUNTER10H = 0xB8A, 
    MHPMCOUNTER11H = 0xB8B, 
    MHPMCOUNTER12H = 0xB8C,
    MHPMCOUNTER13H = 0xB8D,
    MHPMCOUNTER14H = 0xB8E,
    MHPMCOUNTER15H = 0xB8F,
    MHPMCOUNTER16H = 0xB90,
    MHPMCOUNTER17H = 0xB91, // Machine performance monitoring counter
    MHPMCOUNTER18H = 0xB92, 
    MHPMCOUNTER19H = 0xB93,
    MHPMCOUNTER20H = 0xB94,
    MHPMCOUNTER21H = 0xB95,
    MHPMCOUNTER22H = 0xB96,
    MHPMCOUNTER23H = 0xB97,
    MHPMCOUNTER24H = 0xB98, 
    MHPMCOUNTER25H = 0xB99, 
    MHPMCOUNTER26H = 0xB9A,
    MHPMCOUNTER27H = 0xB9B,
    MHPMCOUNTER28H = 0xB9C,
    MHPMCOUNTER29H = 0xB9D,
    MHPMCOUNTER30H = 0xB9E,
    MHPMCOUNTER31H = 0xB9F,
    // Machine counter setup
    MCOUNTINHIBIT = 0x320,
    MHPMEVENT3 = 0x323, // Machine performance monitoring event selector
    MHPMEVENT4 = 0x324, 
    MHPMEVENT5 = 0x325,
    MHPMEVENT6 = 0x326,
    MHPMEVENT7 = 0x327,
    MHPMEVENT8 = 0x328,
    MHPMEVENT9 = 0x329,
    MHPMEVENT10 = 0x32A, 
    MHPMEVENT11 = 0x32B, 
    MHPMEVENT12 = 0x32C,
    MHPMEVENT13 = 0x32D,
    MHPMEVENT14 = 0x32E,
    MHPMEVENT15 = 0x32F,
    MHPMEVENT16 = 0x330,
    MHPMEVENT17 = 0x331, // Machine performance monitoring counter
    MHPMEVENT18 = 0x332, 
    MHPMEVENT19 = 0x333,
    MHPMEVENT20 = 0x334,
    MHPMEVENT21 = 0x335,
    MHPMEVENT22 = 0x336,
    MHPMEVENT23 = 0x337,
    MHPMEVENT24 = 0x338, 
    MHPMEVENT25 = 0x339, 
    MHPMEVENT26 = 0x33A,
    MHPMEVENT27 = 0x33B,
    MHPMEVENT28 = 0x33C,
    MHPMEVENT29 = 0x33D,
    MHPMEVENT30 = 0x33E,
    MHPMEVENT31 = 0x34F,
    // Debug/Trace registers
    TSELECT = 0X7A0,
    TDATA1 = 0x7A1,
    TDATA2 = 0x7A2,
    TDATA3 = 0x7A3,
    // Debug mode registers
    DCSR = 0x7B0,
    DPC = 0x7B1,
    DSCRATCH0 = 0x7B2,
    DSCRATCH1 = 0x7B3,
    // Supervisor trap setup
    SSTATUS = 0x100, // Supervisor status register
    SEDELEG = 0x102, // Supervisor exception delegation register
    SIDELEG = 0x103, // Supervisor interrupt delegation register
    SIE = 0x104, // Supervisor interrupt-enable register
    STVEC = 0x105, // Supervisor trap handler base address
    SCOUNTEREN = 0x106, // Supervisor counter enable
    // Supervisor trap handling
    SSCRATCH = 0x140, // Scratch register for supervisor trap handlers
    SEPC = 0x141, // Supervisor exception program counter
    SCAUSE = 0x142, // Supervisor trap cause,
    STVAL = 0x143, // Supervisor bad address or instruction
    SIP = 0x144, // Supervisor interrupt pending
    // Supervisor protection and translation
    SATP = 0x180 // Supervisor protection and translation
} riscv_csrs_t;


#endif