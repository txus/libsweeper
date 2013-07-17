#include <stdlib.h>
#include <sweeper/dbg.h>
#include <sweeper/sweeper.h>

void SWPHeap_print(SWPHeap *heap) {
  printf("SIZE (%i objects, %ld bytes); COLLECTIONS %i\n",
    heap->size,
    sizeof(SWPHeader*) * heap->size,
    heap->collections
    );

  printf("HEAP\n");
  for(int i=0; i < heap->size; i++) {
    printf("\t[%i: %p] %p\n", i, &heap->objects[i], heap->objects[i]);
  }
}

void SWPHeap_grow(SWPHeap *heap) {
  int new_objs = heap->size * heap->growth_factor;

  check(new_objs <= heap->max_size, "Heap max size exhausted.");

  size_t before = sizeof(SWPHeap*) * heap->size;

  SWPHeader **new_space = realloc(heap->objects, new_objs * sizeof(SWPHeader*));
  check_mem(new_space);

  // Set the newly grown region to zeros
  size_t offset = ((new_objs + 1) * sizeof(SWPHeader*)) - before;
  SWPHeader **start = new_space;
  int count = heap->size;
  while(--count) start++;
  memset(start, 0, offset);

  heap->objects = new_space;
  heap->size = new_objs;

  return;

error:
  fprintf(stderr, "Out of memory.");
  exit(1);
}

SWPHeap* SWPHeap_new(
  unsigned int amount,
  unsigned int max_size,
  double growth_factor,
  void *state,
  size_t object_size,
  SWPReleaseFn release_fn,
  SWPAddRootsFn add_roots_fn,
  SWPAddChildrenFn add_children_fn
  ) {
  SWPHeap *heap = calloc(1, sizeof(SWPHeap));
  heap->objects = calloc(amount, sizeof(SWPHeader*));

  heap->state = state;
  heap->object_size = object_size;
  heap->size = amount;
  heap->max_size = max_size;
  heap->growth_factor = growth_factor;
  heap->release = release_fn;
  heap->add_roots = add_roots_fn;
  heap->add_children = add_children_fn;

  SWPHeap_enable(heap);
  return heap;
}

void SWPHeap_destroy(SWPHeap *heap) {
  if(heap->objects) free(heap->objects);
  free(heap);
}

void swp_mark(SWPHeap *heap, SWPArray *worklist) {
  while(!SWPArray_is_empty(worklist)) {
    SWPHeader *obj = SWPArray_pop(worklist);

    SWPArray *children = SWPArray_create(sizeof(SWPHeader*), SWPARRAY_DEFAULT_SIZE);
    heap->add_children(obj, children);
    for(int i = 0; i < children->end; i++) {
      SWPHeader *child = (SWPHeader*)SWPArray_at(children, i);
      if(child && !SWPHeader_is_marked(child)) {
        SWPHeader_mark(child);
        SWPArray_push(worklist, child);
      }
    }
    SWPArray_destroy(children);
  }
}

void swp_mark_from_roots(SWPHeap *heap) {
  SWPArray *worklist = SWPArray_create(sizeof(SWPHeader*), SWPARRAY_DEFAULT_SIZE);
  SWPArray *roots = SWPArray_create(sizeof(SWPHeader*), SWPARRAY_DEFAULT_SIZE);
  heap->add_roots(heap->state, roots);

  for(int i=0; i < SWPArray_count(roots); i++) {
    SWPHeader *obj = (SWPHeader*)SWPArray_at(roots, i);
    if(obj && !SWPHeader_is_marked(obj)) {
      SWPHeader_mark(obj);
      SWPArray_push(worklist, obj);
      swp_mark(heap, worklist);
    }
  }
  SWPArray_destroy(worklist);
  SWPArray_destroy(roots);
}

void swp_sweep(SWPHeap *heap) {
  for(int i=0; i < heap->size; i++) {
    SWPHeader *obj = heap->objects[i];
    if(obj != NULL) {
      if(SWPHeader_is_marked(obj)) {
        SWPHeader_unmark(obj);
      } else {
        heap->objects[i] = NULL;
        heap->release(obj);
      }
    }
  }
}

void swp_collect(SWPHeap *heap) {
  swp_mark_from_roots(heap);
  swp_sweep(heap);
  heap->collections++;
}

SWPHeader* swp_allocate(SWPHeap *heap) {
  for(int i=0; i < heap->size; i++) {
    if(!heap->objects[i]) {
      SWPHeader *obj = calloc(1, heap->object_size);
      heap->objects[i] = obj;
      return obj;
    }
  }
  // no free slots, collect!
  if(heap->enabled) {
    swp_collect(heap);
  }
  SWPHeap_grow(heap);
  return swp_allocate(heap);
}