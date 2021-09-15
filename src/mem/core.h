#ifndef __MEM_H__
#define __MEM_H__

#include <math.h>

#include "../../lib/common/include/allocator.h"
#include "../../lib/common/include/macro.h"
#include "../../lib/common/include/types.h"

#include "./avl_page.h"

#define PAGE_SIZE 4096

#define DIR_ID(vaddr) ((uintptr_t)(vaddr) >> 38)
#define PAGE_ID(vaddr) ((uintptr_t )(vaddr) >> 12)
#define PAGE_OFFSET(vaddr) ((uintptr_t)(vaddr) & 0xFFF)

#define PAGE_NOT_PRESENT 1;

/**
* Paging works \w 64-bits address

* 20 bits for directory id (1MiB)
* 20 bits for table id (1MiB)
* 12 bits for offset (page_size = 4KiB)
*/

/**
* \brief Managed real memory
*/
typedef struct managed_memory_t
{
  void* vaddr;

  size_t len;
  allocator_t allocator;

  struct managed_memory_t* next;
} managed_memory_t;

typedef struct {
  page_tree_t pages;
  managed_memory_t* managed;

  allocator_t allocator;
  allocator_t page_node_allocator;
} memory_t;


/**
 * \brief Create a memory device
 * 
 * \return A new memory device
 */
memory_t* mem_create(allocator_t* allocator);

/**
 * \brief Init the memory device
 */
void __mem_init(memory_t* mem, allocator_t* allocator, allocator_t* page_node_allocator);

/**
 * \brief Delete the memory device
 */
void mem_delete(memory_t* mem);

/**
 * \brief Returns a memory-aligned length.
 */ 
size_t mem_align(size_t len);

/**
 * \brief Memory translation from virtual to real addresses
 */
bool mem_tl(memory_t* mem, void* vaddr, void** out, char* exceptions);

/**
 * \brief Map virtual memory to a real memory block.
 */
void* mem_map(memory_t* mem, void* vaddr, void* paddr, size_t len);

/**
 * \brief Allocate real memory block and map it to a virtual memory block
 * 
 * The allocated memory is fully managed by the memory device.
 */
void* mem_alloc_managed(memory_t* mem, allocator_t* allocator, void* vaddr, size_t len);

/**
 * \brief Free all managed allocated memory
 */
static void __managed_mem_delete_all(managed_memory_t* managed);

void __mem_init(memory_t* mem, allocator_t* allocator, allocator_t* page_node_allocator)
{
  mem->pages = 0;
  mem->managed = 0;

  mem->allocator = allocator == NULL ? NO_ALLOCATOR: allocator_copy(allocator);
  mem->page_node_allocator = page_node_allocator == NULL ? NO_ALLOCATOR: allocator_copy(page_node_allocator);
}

memory_t* mem_create(allocator_t* allocator)
{
  memory_t* mem = (memory_t*) pmalloc(allocator, sizeof(memory_t));
  __mem_init(mem, allocator, allocator);
  return mem;
}

size_t mem_align(size_t len)
{
  size_t rem = len % PAGE_SIZE;

  len -= rem;
  len /= PAGE_SIZE;
  len += rem ? 1 : 0;
  len *= PAGE_SIZE;

  return len;
}

bool mem_tl(memory_t* mem, void* vaddr, void** out, char* exceptions)
{
  unsigned long pid = PAGE_ID(vaddr);
  unsigned long offset = PAGE_OFFSET(vaddr);

  page_node_t* pnode;
  page_t* page;

  if(!avl_page_search(mem->pages, pid, &pnode))
  {
    *exceptions |= PAGE_NOT_PRESENT;
    return false;    
  }

  page = &pnode->page;

  if(!page->present)
  {
    *exceptions |= PAGE_NOT_PRESENT;
    return false;
  }
  
  *out = page->paddr + offset;
  
  return 1;
}

void* mem_map(memory_t* mem, void* vaddr, void* paddr, size_t len)
{
  len = mem_align(len);
  uintptr_t pid = PAGE_ID(vaddr);
  void* vbase = (void*) (pid << 12); // Realign the memory
  vaddr = vbase;

  page_node_t* pnode;

  do
  {
    pid = PAGE_ID(vaddr);

    // The page is not created, we create it.
    if(!avl_page_search(mem->pages, pid, &pnode)) 
    {
      pnode = avl_page_insert(&mem->pages, pid, &mem->page_node_allocator);
      
      if(!pnode) 
        return NULL;
    }

    page_t* page = &pnode->page;

    page->paddr = paddr;
    page->present = 1;

    if (len > PAGE_SIZE)
      len -= PAGE_SIZE;
    else  
      len = 0;

    vaddr += PAGE_SIZE;
    paddr += PAGE_SIZE;

  } while (len > PAGE_SIZE);

  return vbase;
}

void* mem_alloc_managed(memory_t* mem, allocator_t* allocator, void* vaddr, size_t len)
{
  size_t offset = PAGE_OFFSET(vaddr);
  len = mem_align(offset + len)  + sizeof(struct managed_memory_t); // Align the memory
  struct managed_memory_t* header = (struct managed_memory_t*) pmalloc(allocator, len);

  header->vaddr = vaddr;
  header->len = len;
  header->allocator = allocator_copy(allocator);

  void* paddr = (void*)(header + 1);  
  
  mem_map(mem, vaddr, paddr, len); // Map the memory.

  header->next = mem->managed;
  mem->managed = header;

  return paddr;
}

static void __managed_mem_delete_all(managed_memory_t* managed)
{
  struct managed_memory_t* it = managed;

  while(it != NULL) 
  {
    struct managed_memory_t* nxt = it->next;

    allocator_t allocator = allocator_copy(&it->allocator);
    allocator_delete(&it->allocator);

    pfree(&allocator, it);
    
    allocator_delete(&allocator);

    it = nxt;
  }
}

void mem_delete(memory_t* mem)
{
  // Delete all page nodes.
  avl_page_delete_tree(mem->pages);
  mem->pages = 0;

  // Delete all managed memory.
  __managed_mem_delete_all(mem->managed);
  mem->managed = 0;

  // Free the rest.
  allocator_t allocator = allocator_copy(&mem->allocator);
  allocator_delete(&mem->allocator);
  pfree(&allocator, mem);
  allocator_delete(&allocator);
}

#endif