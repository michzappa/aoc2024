#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <stdbool.h>
#include <stdlib.h>

#include "./data.h"

#define DYN_ARRAY_INIT_SIZE 10

typedef struct {
  data_type_t data_type; // Type of data elements.
  void* data;            // Actual content.
  size_t occupied;       // How much of the data array is populated.
  size_t allocated;      // Actual size of the data array.
} dyn_array;

// Initialize a dynamic array of the given TYPE.
dyn_array* init_dyn_array(data_type_t type);

// Return a newly allocated copy of the given dynamic array ARR.
dyn_array* copy_dyn_array(dyn_array* arr);

// Free the given dynamic array ARR.
void free_dyn_array(dyn_array* arr);

// Return the element at the given IDX in ARR.
void* get_element_of_dyn_array(dyn_array* arr, size_t idx);

// Insert a copy of the value EL onto the end of the dynamic array ARR.
// NOTE: EL must be of the same data type as underlies ARR.
void push_onto_dyn_array(dyn_array* arr, const void* el);

// Remove the element at the given IDX from the given ARR.
bool remove_element_of_dyn_array(dyn_array* arr, size_t idx);

// Sort the contents of the given dynamic array ARR in place.
void sort_dyn_array(dyn_array* arr);

// Return a newly-alloced dynamic array with the contents of the given
// ARR, but sorted.
dyn_array* sorted_dyn_array(dyn_array* arr);

// Return a string representing the given dynamic array ARR.
char* pp_dyn_array(dyn_array* arr);

// Print the given dynamic array ARR to stdout.
void print_dyn_array(dyn_array* arr);

#endif
