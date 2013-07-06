#include <stdlib.h>
#include <sweeper/header.h>
#include <sweeper/sweeper.h>

#define INITIAL_SIZE 20
#define GROWTH_FACTOR 1.8

static inline void SWPHeaders_grow(SWPHeaders* list) {
  size_t cur = sizeof(list->objects);
  list->objects = realloc(list->objects, cur * GROWTH_FACTOR);
  list->capacity *= GROWTH_FACTOR;
}

SWPHeaders* SWPHeaders_new() {
  SWPHeaders *headers = calloc(1, sizeof(SWPHeaders));
  headers->objects = calloc(INITIAL_SIZE, sizeof(SWPHeader*));
  headers->__start__ = headers->objects;
  headers->capacity = INITIAL_SIZE;
  headers->size = 0;
  return headers;
}

void SWPHeaders_destroy(SWPHeaders *list) {
  free(list->__start__);
  free(list);
}

void SWPHeaders_push(SWPHeaders* list, SWPHeader* elem) {
  if(SWPHeaders_full(list)) SWPHeaders_grow(list);
  list->objects[list->size] = elem;
  list->size++;
}

SWPHeader* SWPHeaders_pop(SWPHeaders* list) {
  SWPHeader *obj = list->objects[0];
  list->objects++;
  list->size--;
  return obj;
}
