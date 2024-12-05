#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/data.h"
#include "../include/dyn_array.h"
#include "../include/handler.h"
#include "../include/hash_table.h"

int solve(FILE* input_file) {
  //// Parse input file into meaningful data.
  dyn_array* lefts  = init_dyn_array(UINT64);
  dyn_array* rights = init_dyn_array(UINT64);
  uint64_t left;
  uint64_t right;
  while (fscanf(input_file, "%ld   %ld\n", &left, &right) == 2) {
    push_onto_dyn_array(lefts, (void*)left);
    push_onto_dyn_array(rights, (void*)right);
  }
  assert(lefts->occupied == rights->occupied);

  //// Part 1.
  dyn_array* sorted_lefts  = sorted_dyn_array(lefts);
  dyn_array* sorted_rights = sorted_dyn_array(rights);

  uint64_t distance = 0;
  for (int i = 0; i < lefts->occupied; i += 1) {
    uint64_t l = (uint64_t)get_element_of_dyn_array(sorted_lefts, i);
    uint64_t r = (uint64_t)get_element_of_dyn_array(sorted_rights, i);
    distance += (l > r) ? (l - r) : (r - l);
  }
  printf("Answer 1: %ld\n", distance);

  //// Part 2.
  int similarity = 0;

  hash_table* right_counts = init_hash_table(UINT64, UINT64);

  // Go through rights getting a lookup table of element counts.
  for (int i = 0; i < rights->occupied; i += 1) {
    void* v_el  = get_element_of_dyn_array(rights, i);
    uint64_t el = (uint64_t)v_el;

    void* current_entry = get_entry_in_hash_table(right_counts, (void*)el);
    if (current_entry == NULL) {
      set_entry_in_hash_table(right_counts, (void*)el, (void*)1);
    } else {
      uint64_t new_count = (uint64_t)current_entry + 1;
      set_entry_in_hash_table(right_counts, (void*)el, (void*)new_count);
    }
  }

  // Go through lefts checking the similarities and adding them up.
  for (int i = 0; i < lefts->occupied; i += 1) {
    void* v_el  = get_element_of_dyn_array(lefts, i);
    uint64_t el = (uint64_t)v_el;

    void* right_count = get_entry_in_hash_table(right_counts, (void*)el);
    if (right_count != NULL) {
      uint64_t count = (uint64_t)right_count;
      similarity += count * el;
    }
  }
  printf("Answer 2: %d\n", similarity);

  //// Cleanup.
  free_hash_table(right_counts);
  free_dyn_array(sorted_lefts);
  free_dyn_array(sorted_rights);
  free_dyn_array(lefts);
  free_dyn_array(rights);
  fclose(input_file);

  return 0;
}

int main(int argc, char** argv) {
  return input_file_handler(argv[1], *solve);
}
