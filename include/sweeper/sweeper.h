#ifndef _SWEEPER_SWEEPER_H_
#define _SWEEPER_SWEEPER_H_

#include <sweeper/header.h>
#include <sweeper/array.h>

typedef void (*SWPReleaseFn)(SWPHeader*);
typedef void (*SWPAddRootsFn)(void*, SWPArray*);
typedef void (*SWPAddChildrenFn)(SWPHeader*, SWPArray*);

typedef struct SWPHeap_s {
  void *state;
  SWPHeader **objects;
  size_t size;
  size_t expand_rate;
  unsigned int collections;
  SWPReleaseFn release;
  SWPAddRootsFn add_roots;
  SWPAddChildrenFn add_children;
  unsigned char enabled;
  size_t object_size;
} SWPHeap;

SWPHeap* SWPHeap_new(
  size_t size,
  size_t expand_rate,
  void *state,
  size_t object_size,
  SWPReleaseFn release,
  SWPAddRootsFn add_roots,
  SWPAddChildrenFn add_children
  );

#define SWPHeap_disable(A) (A)->enabled = 0
#define SWPHeap_enable(A) (A)->enabled = 1
void SWPHeap_print(SWPHeap*);
int SWPHeap_expand(SWPHeap*);
void SWPHeap_destroy(SWPHeap*);
SWPHeader* swp_allocate(SWPHeap*);
void swp_mark_from_roots(SWPHeap *heap);
void swp_collect(SWPHeap *heap);

#endif