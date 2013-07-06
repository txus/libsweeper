#ifndef _SWEEPER_SWEEPER_H_
#define _SWEEPER_SWEEPER_H_

#include <sweeper/header.h>

typedef void (*SWPReleaseFn)(SWPHeader*);
typedef void (*SWPAddRootsFn)(void*, SWPHeaders*);
typedef void (*SWPAddChildrenFn)(SWPHeader*, SWPHeaders*);

typedef struct SWPHeap_s {
  void *state;
  SWPHeader **objects;
  unsigned int size;
  unsigned int max_size;
  double growth_factor;
  unsigned int collections;
  SWPReleaseFn release;
  SWPAddRootsFn add_roots;
  SWPAddChildrenFn add_children;
  unsigned char enabled;
  size_t object_size;
} SWPHeap;

SWPHeap* SWPHeap_new(
  unsigned int size,
  unsigned int max_size,
  double growth_factor,
  void *state,
  size_t object_size,
  SWPReleaseFn release,
  SWPAddRootsFn add_roots,
  SWPAddChildrenFn add_children
  );

#define SWPHeap_disable(A) (A)->enabled = 0
#define SWPHeap_enable(A) (A)->enabled = 1
void SWPHeap_print(SWPHeap*);
void SWPHeap_grow(SWPHeap*);
void SWPHeap_destroy(SWPHeap*);
SWPHeader* swp_allocate(SWPHeap*);
void swp_mark_from_roots(SWPHeap *heap);
void swp_collect(SWPHeap *heap);

#endif