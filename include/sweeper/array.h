#ifndef _SWEEPER_ARRAY_H_
#define _SWEEPER_ARRAY_H_
#include <stdlib.h>
#include <assert.h>
#include <sweeper/dbg.h>

#define SWPARRAY_DEFAULT_SIZE 100

typedef struct SWPArray {
  int end;
  int max;
  size_t element_size;
  size_t expand_rate;
  void **contents;
} SWPArray;

SWPArray *SWPArray_create(size_t element_size, size_t initial_max);

void SWPArray_destroy(SWPArray *array);
void SWPArray_clear(SWPArray *array);
int SWPArray_expand(SWPArray *array);
int SWPArray_contract(SWPArray *array);
int SWPArray_push(SWPArray *array, void *el);
void *SWPArray_pop(SWPArray *array);
void SWPArray_clear_destroy(SWPArray *array);

#define SWPArray_last(A) ((A)->contents[(A)->end - 1])
#define SWPArray_first(A) ((A)->contents[0])
#define SWPArray_at(A, I) ((A)->contents[(I)])
#define SWPArray_end(A) ((A)->end)
#define SWPArray_count(A) SWPArray_end(A)
#define SWPArray_max(A) ((A)->max)
#define SWPArray_is_empty(A) (SWPArray_count(A) == 0)

#define DEFAULT_EXPAND_RATE 300

static inline void SWPArray_set(SWPArray *array, int i, void *el)
{
  check(i < array->max, "darray attempt to set past max");
  array->contents[i] = el;
error:
  return;
}

static inline void *SWPArray_get(SWPArray *array, int i)
{
  check(i < array->max, "darray attempt to get past max");
  return array->contents[i];
error:
  return NULL;
}

static inline void *SWPArray_remove(SWPArray *array, int i)
{
  void *el = array->contents[i];

  array->contents[i] = NULL;

  return el;
}

static inline void *SWPArray_new(SWPArray *array)
{
  check(array->element_size > 0, "Can't use SWPArray_new on 0 size darrays.");

  return calloc(1, array->element_size);

error:
  return NULL;
}

#define SWPArray_free(E) free((E))

#endif
