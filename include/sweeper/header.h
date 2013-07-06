#ifndef _SWEEPER_HEADER_H_
#define _SWEEPER_HEADER_H_

typedef struct SWPHeader_s {
  char marked;
} SWPHeader;

typedef struct SWPHeaders_s {
  SWPHeader **objects;
  SWPHeader **__start__; // retain a reference to free it
  unsigned int size;
  unsigned int capacity;
} SWPHeaders;

SWPHeaders* SWPHeaders_new();
void SWPHeaders_destroy(SWPHeaders*);

void SWPHeaders_push(SWPHeaders*, SWPHeader*);
SWPHeader* SWPHeaders_pop(SWPHeaders*);

#define SWPHeader_is_marked(A) ((A)->marked == 1)
#define SWPHeader_mark(A) ((A)->marked = 1)
#define SWPHeader_unmark(A) ((A)->marked = 0)
#define SWPHeaders_size(A) ((A)->size)
#define SWPHeaders_full(A) ((A)->size == (A)->capacity)
#define SWPHeaders_is_empty(A) ((A)->size == 0)

#endif
