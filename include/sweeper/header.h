#ifndef _SWEEPER_HEADER_H_
#define _SWEEPER_HEADER_H_

#include <sweeper/array.h>

typedef struct SWPHeader_s {
  char marked;
} SWPHeader;

#define SWPHeader_is_marked(A) ((A)->marked == 1)
#define SWPHeader_mark(A) ((A)->marked = 1)
#define SWPHeader_unmark(A) ((A)->marked = 0)

#endif
