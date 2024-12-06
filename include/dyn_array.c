#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "./data.h"
#include "./dyn_array.h"

dyn_array* init_dyn_array(data_type_t type) {
  // Allocate the entire struct.
  dyn_array* result = calloc(1, sizeof(dyn_array));

  // Assign properties.
  result->data_type = type;
  result->occupied  = 0;
  result->allocated = DYN_ARRAY_INIT_SIZE;

  // Allocate appropriately sized data array for the data type.
  result->data = calloc(result->allocated, size_of_data_type(type));

  return result;
}

void set_data_array_element(void* data, const void* el, size_t index,
                            data_type_t type) {
  switch (type) {
  case UINT64:
    ((uint64_t*)data)[index] = (uint64_t)el;
  case DYN_ARRAY:
    ((dyn_array**)data)[index] = (dyn_array*)el;
  }
}

dyn_array* copy_dyn_array(dyn_array* arr) {
  // Allocate new empty array.
  dyn_array* copy = init_dyn_array(arr->data_type);

  // Copy properties over.
  copy->occupied  = arr->occupied;
  copy->allocated = arr->allocated;

  // Allocated the contents array of the needed size. Free what was initialized.
  free(copy->data);
  copy->data = calloc(arr->allocated, size_of_data_type(arr->data_type));

  // Copy the data in - raw access to the underlying array without
  // needing the full push procedure.
  void* (*copier)(const void* v) = copier_for_data_type(arr->data_type);
  for (size_t i = 0; i < arr->allocated; i += 1) {
    void* el = get_element_of_dyn_array(arr, i);
    if (el != NULL) {
      void* el_copy = copier(el);
      set_data_array_element(copy->data, el_copy, i, arr->data_type);
    }
  }

  return copy;
}

void free_dyn_array(dyn_array* arr) {
  void (*freer)(const void* v) = freer_for_data_type(arr->data_type);

  for (size_t i = 0; i < arr->allocated; i += 1) {
    void* el = get_element_of_dyn_array(arr, i);
    if (el != NULL) {
      freer(el);
    }
  }

  // Free the constituent data array, which was allocated.
  free(arr->data);

  // Free the allocated struct itself.
  free(arr);
}

void* get_element_of_dyn_array(dyn_array* arr, size_t idx) {
  if (idx >= arr->occupied) {
    return NULL;
  } else {
    switch (arr->data_type) {
    case UINT64:
      return (void*)(((uint64_t*)arr->data)[idx]);
    case DYN_ARRAY:
      return (void*)(((dyn_array**)arr->data)[idx]);
    }
    printf("The C type system has been defeated.");
    exit(-1);
  }
}

void push_onto_dyn_array(dyn_array* arr, const void* el) {
  if (arr->occupied + 1 > (arr->allocated / 2)) {
    // Re-allocate a larger data buffer.
    size_t new_allocation_size = 2 * arr->allocated;
    void* new_data             = realloc(arr->data, new_allocation_size *
                                                        size_of_data_type(arr->data_type));

    arr->data      = new_data;
    arr->allocated = new_allocation_size;
  }

  void* (*copier)(const void* v) = copier_for_data_type(arr->data_type);
  void* el_copy                  = copier(el);
  set_data_array_element(arr->data, el_copy, arr->occupied, arr->data_type);

  arr->occupied += 1;
  return;
}

bool remove_element_of_dyn_array(dyn_array* arr, size_t idx) {
  if (idx >= arr->occupied) {
    return false;
  } else {
    size_t end = arr->occupied;
    for (size_t i = idx; i < end; i += 1) {
      switch (arr->data_type) {
      case UINT64:
        ((uint64_t*)arr->data)[i] = ((uint64_t*)arr->data)[i + 1];
      case DYN_ARRAY:
        ((dyn_array**)arr->data)[i] = ((dyn_array**)arr->data)[i + 1];
      }
    }
    arr->occupied -= 1;
    return true;
  }
}

void sort_dyn_array(dyn_array* arr) {
  // Simply sorting the contents as they are now.
  void* to_be_sorted = arr->data;

  // For qsort purposes pretend the data is only 'occupied' long,
  // instead of its actual 'allocated' presence.
  size_t num_things = arr->occupied;

  // The memory size of the individual elements.
  size_t thing_size = size_of_data_type(arr->data_type);

  // Different datatypes have different comparators.
  int (*comparator)(const void*, const void*) =
      comparator_for_data_type(arr->data_type);

  qsort(to_be_sorted, num_things, thing_size, comparator);
}

dyn_array* sorted_dyn_array(dyn_array* arr) {
  dyn_array* sorted = copy_dyn_array(arr);
  sort_dyn_array(sorted);
  return sorted;
}

char* pp_dyn_array(dyn_array* arr) {
  if (arr->occupied == 0) {
    // Base case is easier to do manually with none of the other
    // assumptions.
    char* pp_string = malloc(3 * sizeof(char));
    pp_string[0]    = '[';
    pp_string[1]    = ']';
    pp_string[2]    = '\0';
    return pp_string;
  } else {
    // The pretty printer for the elements.
    char* (*p_printer)(const void*) = pp_for_data_type(arr->data_type);

    // Initialize with enough space for the static initial character
    // and terminal - to make a well-behaved string. It will be
    // re-allocated for elements.
    char* pp_string = malloc(2 * sizeof(char));
    pp_string[0]    = '[';
    pp_string[1]    = '\0';

    for (int i = 0; i < arr->occupied; i += 1) {
      // A string just containing this element's printed representation.
      char* pp_el = p_printer(get_element_of_dyn_array(arr, i));

      // Length of that representation.
      size_t pp_el_len = strlen(pp_el);

      // Allocate space for this element and its ", " separator (plus
      // the \0 terminator).
      size_t new_length = (strlen(pp_string) + pp_el_len + 2 + 1);
      pp_string         = realloc(pp_string, new_length * sizeof(char));

      // Copy the representation into the buffer, free the source.
      pp_string = strcat(pp_string, pp_el);
      free(pp_el);

      // Add element separator into the unused space at the end of the
      // allocation, preserving the string terminal.
      pp_string[new_length - 3] = ',';
      pp_string[new_length - 2] = ' ';
      pp_string[new_length - 1] = '\0';
    }

    // Terminate the array, removing the extraneous element separator,
    // and preserve the string terminal.
    size_t pre_cut_length         = strlen(pp_string);
    pp_string[pre_cut_length - 2] = ']';
    pp_string[pre_cut_length - 1] = '\0';

    size_t final_length = strlen(pp_string);

    // Shrink the allocated string down to the required size.
    pp_string = realloc(pp_string, (final_length + 1) * sizeof(char));

    return pp_string;
  }
}

void print_dyn_array(dyn_array* arr) {
  char* pp = pp_dyn_array((void*)arr);
  printf("arr: %s\n", pp);
  free(pp);
}
