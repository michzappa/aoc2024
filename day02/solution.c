#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/data.h"
#include "../include/dyn_array.h"
#include "../include/handler.h"

bool is_safe(dyn_array* report) {
  assert(report->data_type == UINT64);
  // Assume a report is safe unless proven otherwise.
  bool is_safe = true;

  // Guards for the trackers.
  bool have_last_level    = false;
  bool have_is_increasing = false;

  // Uninitialized, but also not used until initialized.
  uint64_t last_level;
  bool is_increasing;
  for (size_t j = 0; j < report->occupied; j += 1) {
    uint64_t cur_level = (uint64_t)get_element_of_dyn_array(report, j);

    // At the first element, have nothing to compare it with -
    // continue.
    if (!have_last_level) {
      last_level      = cur_level;
      have_last_level = true;
      continue;
    }

    // At second element. Record whether we have an increase or not
    // (only time that is done) and move to the main comparison.
    if (!have_is_increasing) {
      is_increasing      = cur_level > last_level;
      have_is_increasing = true;
    }

    // Check whether the two values in the current frame meet the
    // trend and the bounds.
    if (have_last_level && have_is_increasing) {
      if (cur_level == last_level) {
        // unsafe report, adjacent levels are the same.
        is_safe = false;
        break;
      } else {
        if (is_increasing && (cur_level > last_level)) {
          if ((cur_level - last_level) > 3) {
            is_safe = false;
            break;
          }
        } else if (!is_increasing && (cur_level < last_level)) {
          if ((last_level - cur_level) > 3) {
            is_safe = false;
            break;
          }
        } else {
          // unsafe report, moving from increase to decrease
          is_safe = false;
          break;
        }
      }
      last_level = cur_level;
    }
  }

  return is_safe;
}

int solve(FILE* input_file) {
  //// Read input into list-of-lists.
  // If an input requires more space, increase the line size.
  char line[100];
  dyn_array* reports = init_dyn_array(DYN_ARRAY);
  while (fscanf(input_file, "%99[^\n]\n", line) == 1) {
    dyn_array* line_arr = init_dyn_array(UINT64);

    char* token;
    char* rest_of_line = line;
    while ((token = strtok_r(rest_of_line, " ", &rest_of_line))) {
      uint64_t num = atoi(token);
      push_onto_dyn_array(line_arr, (void*)num);
    }
    push_onto_dyn_array(reports, (void*)line_arr);

    free_dyn_array(line_arr);
  }

  //// Part 1.
  size_t safe_reports_1 = 0;
  for (size_t i = 0; i < reports->occupied; i += 1) {
    dyn_array* report = get_element_of_dyn_array(reports, i);
    assert(report->occupied != 0);

    if (is_safe(report)) {
      safe_reports_1 += 1;
    }
  }
  printf("Answer 1: %ld\n", safe_reports_1);

  //// Part 2.
  size_t safe_reports_2 = 0;
  for (size_t i = 0; i < reports->occupied; i += 1) {
    dyn_array* report = get_element_of_dyn_array(reports, i);

    bool dampened_is_safe = false;
    for (size_t j = 0; j < report->occupied; j += 1) {
      dyn_array* copy = copy_dyn_array(report);
      remove_element_of_dyn_array(copy, j);
      dampened_is_safe |= is_safe(copy);
      free_dyn_array(copy);
    }
    if (dampened_is_safe) {
      safe_reports_2 += 1;
    }
  }

  printf("Answer 2: %ld\n", safe_reports_2);

  //// Cleanup.
  free_dyn_array(reports);
  fclose(input_file);
  return 0;
}

int main(int argc, char** argv) {
  return input_file_handler(argv[1], *solve);
}
