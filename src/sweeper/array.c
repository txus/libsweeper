#include <sweeper/array.h>
#include <assert.h>

SWPArray *SWPArray_create(size_t element_size, size_t initial_max)
{
  SWPArray *array = malloc(sizeof(SWPArray));
  check_mem(array);
  array->max = initial_max;
  check(array->max > 0, "You must set an initial_max > 0.");

  array->contents = calloc(initial_max, sizeof(void *));
  check_mem(array->contents);

  array->end = 0;
  array->element_size = element_size;
  array->expand_rate = DEFAULT_EXPAND_RATE;

  return array;

error:
  if(array) free(array);
  return NULL;
}

void SWPArray_clear(SWPArray *array)
{
  int i = 0;
  if(array->element_size > 0) {
    for(i = 0; i < array->max; i++) {
      if(array->contents[i] != NULL) {
        free(array->contents[i]);
      }
    }
  }
}

static inline int SWPArray_resize(SWPArray *array, size_t newsize)
{
  array->max = newsize;
  check(array->max > 0, "The newsize must be > 0.");

  void *contents = realloc(array->contents, array->max * sizeof(void *));
  // check contents and assume realloc doesn't harm the original on error

  check_mem(contents);

  array->contents = contents;

  return 0;
error:
  return -1;
}

int SWPArray_expand(SWPArray *array)
{
  size_t old_max = array->max;
  check(SWPArray_resize(array, array->max + array->expand_rate) == 0,
    "Failed to expand array to new size: %d",
    array->max + (int)array->expand_rate);

  memset(array->contents + old_max, 0, array->expand_rate + 1);
  return 0;

error:
  return -1;
}

int SWPArray_contract(SWPArray *array)
{
  int new_size = array->end < (int)array->expand_rate ? (int)array->expand_rate : array->end;

  return SWPArray_resize(array, new_size + 1);
}

void SWPArray_destroy(SWPArray *array)
{
  if(array) {
    if(array->contents) free(array->contents);
    free(array);
  }
}

void SWPArray_clear_destroy(SWPArray *array)
{
  SWPArray_clear(array);
  SWPArray_destroy(array);
}

int SWPArray_push(SWPArray *array, void *el)
{
  array->contents[array->end] = el;
  array->end++;

  if(SWPArray_end(array) >= SWPArray_max(array)) {
    return SWPArray_expand(array);
  } else {
    return 0;
  }
}

void *SWPArray_pop(SWPArray *array)
{
  check(array->end - 1 >= 0, "Attempt to pop from empty array.");

  void *el = SWPArray_remove(array, array->end - 1);
  array->end--;

  if (SWPArray_end(array) > (int)array->expand_rate && SWPArray_end(array) % array->expand_rate) {
    SWPArray_contract(array);
  }

  return el;
error:
  return NULL;
}
