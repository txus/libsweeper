#include "minunit.h"
#include <sweeper/header.h>
#include <sweeper/sweeper.h>

static SWPHeader *foo = NULL;
static SWPHeader *bar = NULL;

void setup() {
  foo = calloc(1, sizeof(SWPHeader));
  bar = calloc(1, sizeof(SWPHeader));
}

void teardown() {
  free(foo);
  free(bar);
}

char *test_push() {
  SWPHeaders *list = SWPHeaders_new();
  SWPHeaders_push(list, foo);

  mu_assert(!SWPHeaders_is_empty(list), "SWPHeaders is empty");

  SWPHeaders_destroy(list);
  return NULL;
}

char *test_pop() {
  SWPHeaders *list = SWPHeaders_new();
  SWPHeaders_push(list, foo);
  SWPHeaders_push(list, bar);

  mu_assert(SWPHeaders_pop(list) == foo, "SWPHeaders_pop didn't return the first element");

  mu_assert(SWPHeaders_size(list) == 1, "SWPHeaders didn't remove the popped element");

  SWPHeaders_destroy(list);
  return NULL;
}

char *all_tests() {
  mu_suite_start();
  setup();

  mu_run_test(test_push);
  mu_run_test(test_pop);

  teardown();
  return NULL;
}

RUN_TESTS(all_tests);
