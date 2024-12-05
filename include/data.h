/*
  An 'abstraction' for pseudo-generic datastructures using dynamic
  lookups.
 */

#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
  UINT64 // raw uint64_t - NOT A POINTER/REFERENCE TO ONE!!!
} data_type_t;

// Return the memory size which underlies the given TYPE.
size_t size_of_data_type(data_type_t type);

// Return a comparator function for the given data TYPE.
int (*comparator_for_data_type(data_type_t type))(const void*, const void*);

// Return a pretty printing function for the given data TYPE.
char* (*pp_for_data_type(data_type_t type))(const void*);

uint64_t (*hasher_for_data_type(data_type_t type))(const void*,
                                                   data_type_t type);

#endif
