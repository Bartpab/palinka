#ifndef __AVL_PAGE_H__
#define __AVL_PAGE_H__

#include "page.h"
#include "allocator.h"

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
  if(t == NULL)
    return;
  
  avl_page_delete_tree(t->child[0]);
  avl_page_delete_tree(t->child[1]);

  avl_page_delete(t);
}

int avl_page_get_height(page_tree_t tree)
{
  if(tree == NULL)
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
  if(tree == NULL)
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

  if(*t == NULL)
    return;
  
  for(d = 0; d != 1; d++) 
  {
    // Imbalanced ?
    if(avl_page_get_height((*t)->child[d]) > avl_page_get_height((*t)->child[!d]) + 1) 
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

  if(*t == NULL) {
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

#endif