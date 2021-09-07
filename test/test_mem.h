#include "utils.h"
#include "../src/mem.h"
#include "../src/allocator.h"

memory_t mem_boostrap()
{
  memory_t mem;
  allocator_t node_page_allocator = GLOBAL_ALLOCATOR;
  allocator_t allocator = NO_ALLOCATOR;
  __mem_init(&mem, &allocator, &node_page_allocator);
  return mem;
}

define_test(
  page_node, test_print("Page node basics")
) {
  allocator_t allocator = GLOBAL_ALLOCATOR;

  page_tree_t tree = NULL;
  page_node_t* pnode;

  test_check(
    test_print("If no page node are set, we do get a negative search result."),
    !avl_page_search(tree, 10, &pnode),
    test_failure("Should be false")
  );

  pnode = avl_page_insert(&tree, 10, &allocator);

  test_check(
    test_print("Insert #10th page."),
    pnode != NULL,
    test_failure("Should get an inserted page node.")
  );

  test_check(
    test_print("If #10th page exists, we do get a positive search result."),
    avl_page_search(tree, 10, &pnode),
    test_failure("Should be true")
  );

  pnode = avl_page_insert(&tree, 5, &allocator);

  test_check(
    test_print("Insert #5th page."),
    pnode != NULL,
    test_failure("Should get an inserted page node.")
  );

  avl_page_remove(&tree, 10);

  test_check(
    test_print("Check when the #10th page is removed, we do get a negative search result."),
    !avl_page_search(tree, 10, &pnode),
    test_failure("Should be false")
  );

  test_success;
  
  test_teardown {
    avl_page_delete_tree(tree); 
  }

  test_end;
}

define_test(
  mem_tl,
  test_print("Memory translation")
) {
  
  char exceptions = 0;
  void *out;

  memory_t mem = mem_boostrap();

  test_check(
    test_print("Check when no page are set, that we get a page fault."), 
    !mem_tl(&mem, (void*) 0x0, &out, &exceptions),
    test_failure("Should be a page fault");
  );

  test_success;
  
  test_teardown {
    mem_delete(&mem);
  }

  test_end;
}

define_test(
  mem_map, 
  test_print("Memory map")
) {

  char exceptions = 0;
  void* out;
  void *pblock = (void*) 0xB0;
  void *vbase = (void*) 0x8000000000000000;

  memory_t mem = mem_boostrap();
  
  vbase = mem_map(&mem, vbase, pblock, 1);

  test_check(
    test_print("Map physical %#08lx to virtual %#08lx, the returned address should not be NULL.",  (octa) pblock, (octa) vbase),
    vbase,
    test_failure("Should not be NULL")
  );

  test_check(
    test_print("We translate virtual %#lx, it must not cause a page fault.", (octa) vbase),
    mem_tl(&mem, vbase, &out, &exceptions),
    test_failure("Should not have a page fault.")
  );

  test_check(
    test_print("The translated address %#lx must be %#lx.", (octa) vbase, (octa) pblock),
    pblock == out,
    test_failure("Got %#lx.", (octa) out)
  )

  test_success;
  
  test_teardown {
    mem_delete(&mem);
  }

  test_end;
}

define_test(
  mem_alloc_managed, 
  test_print("Memory allocate managed physical memory")
) {
  memory_t mem = mem_boostrap();
  allocator_t allocator = GLOBAL_ALLOCATOR;

  void* vaddr = (void*) 0x8000000000000000;
  void* pblock = mem_alloc_managed(&mem, &allocator, vaddr, sizeof(char));
  void* out;
  
  test_check(
    test_print("Managed physical memory is allocated by the memory unit and mapped to the closest page's lower bound near %#lx.", (octa) vaddr),
    pblock != NULL,
    test_failure("Should not be NULL")
  );

  test_success;
  
  test_teardown {
    mem_delete(&mem);
  }

  test_end;
}


define_test_chapter(
  mem, test_print("Memory"), 
  page_node, mem_tl, mem_map, mem_alloc_managed
)
