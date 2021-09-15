#ifndef __PAGE_H__
#define __PAGE_H__

typedef struct {
  char present;
  void* paddr;
} page_t;

#endif