// UCLA CS 111 Lab 1 storage allocation

#include "alloc.h"

#include <error.h>
#include <errno.h>
#include <stdlib.h>

static void
memory_exhausted (int errnum)
{
  error (1, errnum, "memory exhausted");
}

static void *
check_nonnull (void *p)
{
  if (! p)
    memory_exhausted (errno);
  return p;
}

void *
checked_malloc (size_t size)
{
  return check_nonnull (malloc (size ? size : 1));
}

void *
checked_realloc (void *ptr, size_t size)
{
  return check_nonnull (realloc (ptr, size ? size : 1));
}

void *
checked_grow_alloc (void *ptr, size_t *size)
{
  size_t max = -1;
  if (*size == max)
    memory_exhausted (0);
  *size = *size < max / 2 ? 2 * *size : max;
  return checked_realloc (ptr, *size);
}
