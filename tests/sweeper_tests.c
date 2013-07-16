#include "minunit.h"
#include <sweeper/sweeper.h>

static SWPHeader *foo = NULL;
static SWPHeader *bar = NULL;
static SWPHeader *baz = NULL;
static SWPHeader *quux = NULL;

typedef struct Object_s {
  SWPHeader gc;
  int a;
  int b;
  int c;
  int d;
} Object;

void test_release(SWPHeader *obj) {
  free(obj);
  obj = NULL;
}

void test_add_roots(void *state, SWPArray *roots) {
  SWPArray_push(roots, foo);
  SWPArray_push(roots, quux);
}

void test_add_children(SWPHeader *obj, SWPArray *children) {
  if(obj == foo) {
    SWPArray_push(children, bar);
  }
}

SWPHeap *new_heap(unsigned int size) {
  return SWPHeap_new(
    size,
    size * 2,
    1.8,
    (void*)0x0, // state
    sizeof(Object),
    test_release,
    test_add_roots,
    test_add_children
  );
}

char *test_heap_new() {
  SWPHeap *heap = new_heap(2);
  mu_assert(heap->size == 2, "Wrong heap size.");

  SWPHeap_destroy(heap);
  return NULL;
}

char *test_heap_allocate() {
  SWPHeap *heap = new_heap(2);

  SWPHeader *obj = swp_allocate(heap);
  mu_assert(obj != NULL, "Couldn't allocate object");

  SWPHeap_destroy(heap);
  return NULL;
}

char *test_heap_mark_from_roots() {
  SWPHeap *heap = new_heap(4);

  foo = swp_allocate(heap);
  bar = swp_allocate(heap);
  baz = swp_allocate(heap);
  quux = swp_allocate(heap);

  swp_mark_from_roots(heap);

  mu_assert(SWPHeader_is_marked(foo), "Foo should be marked");
  mu_assert(SWPHeader_is_marked(bar), "Bar should be marked as a child of Foo");
  mu_assert(!SWPHeader_is_marked(baz), "Baz should not be marked");
  mu_assert(SWPHeader_is_marked(quux), "Quux should be marked");

  SWPHeap_destroy(heap);
  return NULL;
}

char *test_heap_grow() {
  SWPHeap *heap = new_heap(3); // max_size == 6
  SWPHeap_grow(heap);
  mu_assert(heap->size == 5, "Wrong heap size");
  SWPHeap_destroy(heap);
  return NULL;
}

char *test_heap_collect() {
  SWPHeap *heap = new_heap(4);

  foo = swp_allocate(heap);
  bar = swp_allocate(heap);
  baz = swp_allocate(heap);
  quux = swp_allocate(heap);

  SWPHeader *x = swp_allocate(heap); // triggers a collection
  mu_assert(x != NULL, "X couldn't be allocated");

  mu_assert(heap->collections == 1, "Collection wasn't triggered");
  mu_assert(heap->size == 7, "Heap didn't grow");

  SWPHeap_destroy(heap);
  return NULL;
}

char *all_tests() {
  mu_suite_start();

  mu_run_test(test_heap_new);
  mu_run_test(test_heap_allocate);
  mu_run_test(test_heap_grow);
  mu_run_test(test_heap_mark_from_roots);
  mu_run_test(test_heap_collect);

  return NULL;
}

RUN_TESTS(all_tests);