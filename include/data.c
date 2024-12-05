#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "./data.h"

size_t size_of_data_type(data_type_t type) {
  switch (type) {
  case UINT64:
    return sizeof(uint64_t);
  default:
    printf("The C type system has been defeated.");
    exit(-1);
  }
}

int compare_int(const void* v1, const void* v2) {
  return *(uint64_t*)v1 - *(uint64_t*)v2;
}

int (*comparator_for_data_type(data_type_t type))(const void* a, const void*) {
  switch (type) {
  case UINT64:
    return compare_int;
  default:
    printf("The C type system has been defeated.");
    exit(-1);
  }
}

#define PP_LENGTH 2

char* pp_uint64(const void* v) {
  // Initally the buffer has space for one character and \0.
  char* pp_buf = malloc(PP_LENGTH * sizeof(char));

  // Attempt to put the entire integer into pp_buf. This result is the
  // number of characters snprintf tried to put into the buffer, not
  // including the \0.
  size_t pp_length = snprintf(pp_buf, PP_LENGTH, "%ld", (uint64_t)v);

  // If the length of the given integer PLUS the \0 is bigger than the
  // size of pp_buf, reallocate it.
  if ((pp_length + 1) >= PP_LENGTH) {
    pp_buf = realloc(pp_buf, (pp_length + 1) * sizeof(char));
    snprintf(pp_buf, (pp_length + 1), "%ld", (uint64_t)v);
  }

  return pp_buf;
}

char* (*pp_for_data_type(data_type_t type))(const void*) {
  switch (type) {
  case UINT64:
    return pp_uint64;
  default:
    printf("The C type system has been defeated.");
    exit(-1);
  }
}

uint64_t hash_uint64(const void* v, data_type_t type) {
  return (uint64_t)v;
}

/*
  NOTE: Commenting out until it is used.
//// Taken from the internet - I am not a cryptographer
// TODO: May eventually need to hash non-primitive memory location.
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for KEY, which is a TYPE.
// See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_of_key(const void* key, data_type_t type) {
  // For every byte of data in the raw representation of KEY, do the
  // thing.
  uint64_t hash = FNV_OFFSET;
  for (int i = 0; i < size_of_data_type(type); i += 1) {
    hash *= FNV_PRIME;
    hash ^= (((u_int8_t*)key)[i]);
  }
  return hash;
}

////
*/

uint64_t (*hasher_for_data_type(data_type_t type))(const void*,
                                                   data_type_t type) {
  switch (type) {
  case UINT64:
    return hash_uint64;
  default:
    printf("The C type system has been defeated.");
    exit(-1);
  }
}
