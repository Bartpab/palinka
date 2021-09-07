#ifndef __MEM_H__
#define __MEM_H__

#include <math.h>

#include "allocator.h"
#include "types.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define PAGE_SIZE 4096

#define DIR_ID(vaddr) ((unsigned long)vaddr >> 38)
#define PAGE_ID(vaddr) ((unsigned long)vaddr >> 12)
#define PAGE_OFFSET(vaddr) ((unsigned long)vaddr & 0xFFF)

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
  char present;
  void* paddr;
} page_t;

typedef struct page_node_t {
  struct page_node_t* child[2];
  int height;
  unsigned long pid;
  page_t page;
  allocator_t allocator;
} page_node_t;

typedef page_node_t* page_tree_t;

void avl_page_delete(page_node_t* pnode)
{
  if(!pnode)
    return;

  allocator_t alloc = allocator_copy(&pnode->allocator);
  allocator_delete(&pnode->allocator);
  pfree(&alloc, pnode);
  allocator_delete(&alloc); 
}

void avl_page_delete_tree(page_tree_t t)
{
  if(!t)
    return;
  
  avl_page_delete_tree(t->child[0]);
  avl_page_delete_tree(t->child[1]);

  avl_page_delete(t);
}

int avl_page_get_height(page_tree_t tree)
{
  if(!tree)
    return 0;
  
  return tree->height;
}

bool avl_page_search(page_tree_t tree, unsigned long pid, page_node_t** out)
{
  if (tree == NULL) {
    return 0;
  } else if(tree->pid == pid) {
    *out = tree;
    return 1;
  } else {
    return avl_page_search(tree->child[pid > tree->pid], pid, out);
  }
}

void avl_page_fix_height(page_tree_t tree)
{
  if(!tree)
    return;

  tree->height = 1 + MAX(avl_page_get_height(tree->child[0]), avl_page_get_height(tree->child[1]));  
}

void avl_page_rotate(page_tree_t* root, unsigned char d)
{
  d = d > 1 ? 1 : d;

  page_tree_t old_root, new_root, old_middle;

  old_root = *root;
  new_root = old_root->child[d];
  old_middle = old_root->child[!d];

  old_root->child[d] = old_middle;
  new_root->child[!d] = old_root;

  *root = new_root;

  avl_page_fix_height((*root)->child[!d]);
  avl_page_fix_height(*root);
}

void avl_page_rebalance(page_tree_t* t)
{
  unsigned char d;

  if(!*t)
    return;
  
  for(d = 0; d != 1; d++) 
  {
    // Imbalanced ?
    if(avl_page_get_height((*t)->child[d]) > avl_page_get_height((*t)->child[!d] + 1)) 
    {
      if(avl_page_get_height((*t)->child[d]->child[d]) > avl_page_get_height((*t)->child[d]->child[!d])) {
        avl_page_rotate(t, d);
      } else {
        avl_page_rotate(&(*t)->child[d], !d);
        avl_page_rotate(t, d);
      }
    }

    avl_page_fix_height(*t);
  }
}

page_node_t* avl_page_insert(page_tree_t* t, unsigned long pid, allocator_t* allocator)
{
  page_node_t* pnode;

  // It already exists !
  if(avl_page_search(*t, pid, &pnode)) 
    return pnode;

  if(!*t) {
    *t = (page_node_t*) pmalloc(allocator, sizeof(page_node_t));

    if(*t == NULL)
      return NULL;

    (*t)->child[0] = NULL;
    (*t)->child[1] = NULL;
    (*t)->pid = pid;
    (*t)->height = 1;
    (*t)->allocator = allocator_copy(allocator);

    return *t;
  } else {
    pnode = avl_page_insert(&(*t)->child[pid > (*t)->pid], pid, allocator);
    avl_page_rebalance(t);
    return pnode;
  }
}

page_node_t avl_page_remove_min(page_tree_t* t)
{
  page_node_t pnode;
  
  if((*t)->child[0] == NULL) 
  {
    page_tree_t old_root;

    old_root = *t;
    pnode = *old_root;
    *t = old_root->child[1];

    avl_page_delete(old_root);
  } else {
    pnode = avl_page_remove_min(&(*t)->child[0]);
  }

  avl_page_rebalance(t);

  return pnode;
}

void avl_page_remove(page_tree_t* t, unsigned long pid)
{
  page_tree_t old_root;

  if(*t == NULL)
    return;
  
  if((*t)->pid == pid) {
    page_node_t** right = &(*t)->child[1];
    page_node_t* left = (*t)->child[0];

    if(*right != NULL) {
      page_node_t tmp = avl_page_remove_min(right);
      (*t)->page = tmp.page;
      (*t)->pid = tmp.pid; 
    } else {
      old_root = *t;
      *t = left;
      avl_page_delete(old_root);
    }
  } else {
    avl_page_remove(&(*t)->child[pid > (*t)->pid], pid);
  }

  avl_page_rebalance(t);
}

typedef struct {
  page_tree_t pages;
  managed_memory_t* managed;

  allocator_t allocator;
  allocator_t page_node_allocator;
} memory_t;


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
  unsigned long pid = PAGE_ID(vaddr);
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
  
  size_t _len = sizeof(managed_memory_t);

  header->vaddr = vaddr;
  header->len = len;
  header->allocator = allocator_copy(allocator);

  void* paddr = (void*)(header + 1);  
  
  mem_map(mem, vaddr, paddr, len); // Map the memory.

  header->next = mem->managed;
  mem->managed = header;

  return paddr;
}

void managed_mem_delete_all(managed_memory_t* managed)
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
  managed_mem_delete_all(mem->managed);
  mem->managed = 0;

  // Free the rest.
  allocator_t allocator = allocator_copy(&mem->allocator);
  allocator_delete(&mem->allocator);
  pfree(&allocator, mem);
  allocator_delete(&allocator);
}

#endif