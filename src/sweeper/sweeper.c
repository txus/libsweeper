#include <stdlib.h>
#include <sweeper/dbg.h>
#include <sweeper/sweeper.h>

void SWPHeap_print(SWPHeap *heap) {
  printf("SIZE (%li objects, %ld bytes); COLLECTIONS %i\n",
    heap->size,
    heap->size * heap->object_size,
    heap->collections
    );

  printf("HEAP\n");
  for(int i=0; i < heap->size; i++) {
    printf("\t[%i: %p] %p\n", i, &heap->objects[i], heap->objects[i]);
  }
}

static inline int SWPHeap_resize(SWPHeap *heap, size_t newsize)
{
  heap->size = newsize;
  check(heap->size > 0, "The newsize must be > 0.");

  SWPHeader **objects = realloc(heap->objects, heap->size * sizeof(SWPHeader*));
  // check contents and assume realloc doesn't harm the original on error

  check_mem(objects);

  heap->objects = objects;

  return 0;
error:
  return -1;
}

int SWPHeap_expand(SWPHeap *heap)
{
  size_t old_size = heap->size;
  check(SWPHeap_resize(heap, heap->size + heap->expand_rate) == 0,
    "Failed to expand heap to new size: %ld",
    heap->size + (int)heap->expand_rate);

  SWPHeader **ptr = heap->objects;
  while(old_size--) ptr++;
  memset(ptr, 0, (heap->expand_rate) * sizeof(SWPHeader*));
  return 0;

error:
  return -1;
}

SWPHeap* SWPHeap_new(
  size_t size,
  size_t expand_rate,
  void *state,
  size_t object_size,
  SWPReleaseFn release_fn,
  SWPAddRootsFn add_roots_fn,
  SWPAddChildrenFn add_children_fn
  ) {
  SWPHeap *heap = calloc(1, sizeof(SWPHeap));

  heap->objects = calloc(size, sizeof(SWPHeader*));

  heap->state = state;
  heap->object_size = object_size;
  heap->size = size;
  heap->expand_rate = expand_rate;
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
  SWPHeap_expand(heap);
  return swp_allocate(heap);
}