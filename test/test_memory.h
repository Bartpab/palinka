#include "../lib/common/include/testing/utils.h"
#include "../lib/common/include/allocator.h"

#include "../src/memory/core.h"

memory_t mem_boostrap()
{
  memory_t mem;
  allocator_t node_page_allocator = GLOBAL_ALLOCATOR;
  __mem_init(&mem, &node_page_allocator);
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

  avl_page_remove(&tree, 10, &allocator);

  test_check(
    test_print("Check when the #10th page is removed, we do get a negative search result."),
    !avl_page_search(tree, 10, &pnode),
    test_failure("Should be false")
  );

  test_success;
  
  test_teardown {
    avl_page_delete_tree(tree, &allocator); 
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
    mem_destroy(&mem);
  }

  test_end;
}

define_test(
  mem_map, 
  test_print("Memory map")
) {
  char s[3][32];
  octa o[3];

  char exceptions = 0;
  void* out;
  void *pblock = (void*) 0xB0;
  void *vbase = (void*) 0x8000000000000000;

  memory_t mem = mem_boostrap();
  
  vbase = mem_map(&mem, vbase, pblock, 1);

  o[0] = voidp_to_octa(pblock);
  o[1] = voidp_to_octa(vbase);

  octa_str(o[0], s[0], 32), octa_str(o[1], s[1], 32);

  test_check(
    test_print("Map physical %s to virtual %s, the returned address should not be NULL.",  s[0], s[1]),
    vbase,
    test_failure("Should not be NULL")
  );

  test_check(
    test_print("We translate virtual %s, it must not cause a page fault.", s[1]),
    mem_tl(&mem, vbase, &out, &exceptions),
    test_failure("Should not have a page fault.")
  );

  o[3] = voidp_to_octa(out);
  octa_str(o[3], s[3], 32);

  test_check(
    test_print("The translated address %s must be %s.", s[1], s[0]),
    pblock == out,
    test_failure("Got %s.", s[3])
  )

  test_success;
  
  test_teardown {
    mem_destroy(&mem);
  }

  test_end;
}

define_test(
  mem_alloc_managed, 
  test_print("Memory allocate managed physical memory")
) {
  octa o[1];
  char s[1][32];

  memory_t mem = mem_boostrap();
  allocator_t allocator = GLOBAL_ALLOCATOR;

  void* vaddr = (void*) 0x8000000000000000;
  void* pblock = mem_alloc_managed(&mem, &allocator, vaddr, sizeof(char));

  o[1] = voidp_to_octa(vaddr);
  octa_str(o[1], s[1], 32);

  test_check(
    test_print("Managed physical memory is allocated by the memory unit and mapped to the closest page's lower bound near %s.", s[1]),
    pblock != NULL,
    test_failure("Should not be NULL")
  );

  test_success;
  
  test_teardown {
    mem_destroy(&mem);
  }

  test_end;
}


define_test_chapter(
  memory, test_print("Memory"), 
  page_node, mem_tl, mem_map, mem_alloc_managed
)
