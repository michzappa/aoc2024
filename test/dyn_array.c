#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "../include/dyn_array.h"

#define BIG_ARRAY_SIZE 1000

void run_test(char* name, int (*test)()) {
  printf("- %s\n", name);
  int res = test();
  printf(" - result: %d\n", res);
}

int make_a_big_one() {
  dyn_array* arr = init_dyn_array(UINT64);

  for (size_t i = 0; i < BIG_ARRAY_SIZE; i += 1) {
    push_onto_dyn_array(arr, (void*)i);
  }

  for (size_t i = 0; i < BIG_ARRAY_SIZE; i += 1) {
    void* el = get_element_of_dyn_array(arr, i);
    assert((uint64_t)el == i);
  }

  free_dyn_array(arr);

  return 0;
}

int make_a_big_one_and_remove_everything() {
  dyn_array* arr = init_dyn_array(UINT64);

  for (size_t i = 0; i < BIG_ARRAY_SIZE; i += 1) {
    push_onto_dyn_array(arr, (void*)i);
  }

  assert(arr->occupied == BIG_ARRAY_SIZE);
  char* pp = pp_dyn_array(arr);
  free(pp);

  for (size_t i = 0; i < BIG_ARRAY_SIZE; i += 1) {
    void* el = get_element_of_dyn_array(arr, i);
    if ((uint64_t)el != i) {
      return -1;
    }
  }

  for (size_t i = 0; i < BIG_ARRAY_SIZE; i += 1) {
    remove_element_of_dyn_array(arr, 0);
  }

  assert(arr->occupied == 0);

  pp = pp_dyn_array(arr);
  free(pp);

  free_dyn_array(arr);
  return 0;
}

int array_of_arrays() {
  dyn_array* arr = init_dyn_array(DYN_ARRAY);
  for (size_t i = 0; i < BIG_ARRAY_SIZE; i += 1) {
    dyn_array* arr = init_dyn_array(UINT64);
    for (size_t j = 0; j < BIG_ARRAY_SIZE; j += 1) {
      push_onto_dyn_array(arr, (void*)j);
    }

    char* pp = pp_dyn_array(arr);
    free(pp);

    free_dyn_array(arr);
  }

  char* pp = pp_dyn_array(arr);
  free(pp);

  free_dyn_array(arr);
  return 0;
}

int main(int argc, char** argv) {
  printf("Running Tests\n");
  printf("-------------\n");
  run_test("make_a_big_one", make_a_big_one);
  run_test("make_a_big_one_and_remove_everything",
           make_a_big_one_and_remove_everything);
  run_test("array_of_arrays", array_of_arrays);

  return 0;
}
