#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "./data.h"
#include "./hash_table.h"

hash_table* init_hash_table(data_type_t key_type, data_type_t value_type) {
  hash_table* table = malloc(sizeof(hash_table));

  table->key_type   = key_type;
  table->value_type = value_type;
  table->occupied   = 0;
  table->allocated  = HASH_TABLE_INIT_SIZE;

  // Zero out the entries array - important.
  table->entries = calloc(table->allocated, sizeof(hash_table_entry));

  return table;
}

void free_table_entries(hash_table_entry* entries, size_t num_entries,
                        data_type_t value_type) {
  void (*freer)(const void* v) = freer_for_data_type(value_type);

  // The values themselves may be allocated and need freeing.
  for (int i = 0; i < num_entries; i += 1) {
    hash_table_entry entry = entries[i];

    if (entry.key != NULL) {
      freer((void*)entry.value);
    }
  }

  free(entries);
}

void free_hash_table(hash_table* table) {
  free_table_entries(table->entries, table->allocated, table->value_type);
  free(table);
}

void* get_entry_in_hash_table(hash_table* table, const void* key) {
  if (key == NULL) {
    return NULL;
  }

  uint64_t (*hasher)(const void*) = hasher_for_data_type(table->key_type);
  uint64_t hash                   = hasher(key);

  // Scaling the hash value into the range of table indices.
  size_t ideal_index = (size_t)(hash & (int64_t)(table->allocated - 1));

  // Look for a matching entry until an empty entry is found. If we
  // find an empty entry that means the desired entry is not present.
  // Despite the wrap-around, this will terminate because the entries
  // list is never full.
  size_t idx = ideal_index;
  while (table->entries[idx].key != NULL) {
    if (table->entries[idx].key == key) {
      return table->entries[idx].value;
    }

    // If the entry we are search for is "farther from home" than the
    // current entry, we know what we are looking for is not in the
    // table.
    if ((idx - ideal_index) > table->entries[idx].displacement) {
      return NULL;
    }

    // Linear probing resolves conflicts - move to the next entry.
    idx += 1;
    if (idx >= table->allocated) {
      // Wrap around.
      idx = 0;
    }
  }

  return NULL;
}

void set_entry(hash_table_entry* entries, size_t num_entries, const void* key,
               void* value, size_t* occupied, data_type_t key_type,
               data_type_t value_type) {
  if (key == NULL) {
    return;
  }

  uint64_t (*hasher)(const void*) = hasher_for_data_type(key_type);
  uint64_t hash                   = hasher(key);

  void* (*copier)(const void* v) = copier_for_data_type(value_type);
  void* val_copy                 = copier(value);

  // The ideal index of this entry based on the key's hash.
  size_t ideal_index = (size_t)(hash & (uint64_t)(num_entries - 1));

  // Tracker var for processing.
  size_t idx = ideal_index;

  // Key and value needing a home. Changes upon displacement.
  const void* homeless_key     = key;
  void* homeless_val           = val_copy;
  size_t homeless_displacement = 0;

  while (entries[idx].key != NULL) {
    if (entries[idx].key == key) {
      // Found an already existing entry for this key, update the
      // value.

      // First, free the existing value.
      void (*freer)(const void* v) = freer_for_data_type(value_type);
      freer(entries[idx].value);

      entries[idx].value = val_copy;
      return;
    }

    // How far the entry currently at idx is from its hash-ideal index.
    size_t cur_disp = entries[idx].displacement;

    // A candidate to displace! We are farther from home than they
    // are.
    if (homeless_displacement > cur_disp) {
      // If our current insertion is 'further from home' than what
      // currently resides at this location, displace what is there
      // and replace with the new entry.

      // Record what needs to be put elsewhere.
      const void* tmp_k = entries[idx].key;
      void* tmp_v       = entries[idx].value;
      size_t tmp_d      = entries[idx].displacement;

      // Displace.
      entries[idx].key          = homeless_key;
      entries[idx].value        = homeless_val;
      entries[idx].displacement = homeless_displacement;

      // Assign the new homeless entry and move on.
      homeless_key          = tmp_k;
      homeless_val          = tmp_v;
      homeless_displacement = tmp_d;
    }

    // Linear probing resolves conflicts - move to the next entry.
    idx += 1;
    homeless_displacement += 1;
    if (idx >= num_entries) {
      // Wrap around.
      idx = 0;
    }
  }

  // If we make it here, there was no better place for this entry
  // in the robin hood system. Put it here.
  entries[idx].key          = homeless_key;
  entries[idx].value        = homeless_val;
  entries[idx].displacement = idx - ideal_index;

  // 'occupied' is a passed-down pointer to a hash table's occupied
  // counter. If it is NULL then we are not meant to modify anything.
  // If it is not NULL, increment it to represent the additional
  // entry.
  if (occupied != NULL) {
    (*occupied)++;
  }

  return;
}

void set_entry_in_hash_table(hash_table* table, const void* key, void* value) {

  if (table->occupied + 1 > (table->allocated / 2)) {
    size_t new_allocation_size = 2 * table->allocated;
    hash_table_entry* new_entries =
        calloc(new_allocation_size, sizeof(hash_table_entry));

    for (size_t i = 0; i < table->allocated; i += 1) {
      hash_table_entry entry = table->entries[i];
      // If this entry is set, then transfer it to the new place.
      if (entry.key != NULL) {
        set_entry(new_entries, new_allocation_size, entry.key, entry.value,
                  NULL, table->key_type, table->value_type);
      }
    }

    // Free the old entries array.
    free_table_entries(table->entries, table->allocated, table->value_type);

    // Update the table's metadata.
    table->entries   = new_entries;
    table->allocated = new_allocation_size;
  }

  set_entry(table->entries, table->allocated, key, value, &table->occupied,
            table->key_type, table->value_type);
  return;
}

bool remove_entry_in_hash_table(hash_table* table, const void* key) {
  // TODO could re-allocate to be smaller.
  if (key == NULL) {
    return false;
  }

  uint64_t (*hasher)(const void*) = hasher_for_data_type(table->key_type);
  uint64_t hash                   = hasher(key);

  // Scaling the hash value into the range of table indices.
  size_t ideal_index = (size_t)(hash & (int64_t)(table->allocated - 1));

  // Look for a matching entry until an empty entry is found. If we
  // find an empty entry that means the desired entry is not present.
  // Despite the wrap-around, this will terminate because the entries
  // list is never full.
  size_t idx    = ideal_index;
  bool found_it = false;
  while (table->entries[idx].key != NULL) {
    if (table->entries[idx].key == key) {
      // Delete entry by marking NULL and freeing the value.
      table->entries[idx].key      = NULL;
      void (*freer)(const void* v) = freer_for_data_type(table->value_type);
      freer(table->entries[idx].value);

      // Decrement the number of occupied entries.
      table->occupied -= 1;

      // Signal to move into backwards shifting.
      found_it = true;
      break;
    }

    // Linear probing resolves conflicts - move to the next entry.
    idx += 1;
    if (idx >= table->allocated) {
      // Wrap around.
      idx = 0;
    }
  }

  // Backwards-shift to preserve continuity of entries with the same
  // ideal index. There is currently no entry at idx.
  if (found_it) {
    size_t empty_space_idx = idx;
    size_t backshift_candidate_idx;
    if (idx >= table->allocated) {
      backshift_candidate_idx = 0;
    } else {
      backshift_candidate_idx = idx + 1;
    }

    while (table->entries[backshift_candidate_idx].key != NULL &&
           table->entries[backshift_candidate_idx].displacement > 0) {
      table->entries[empty_space_idx].key =
          table->entries[backshift_candidate_idx].key;
      table->entries[empty_space_idx].value =
          table->entries[backshift_candidate_idx].value;
      table->entries[empty_space_idx].displacement =
          table->entries[backshift_candidate_idx].displacement - 1;

      // The backshifted entry has moved so the key field at that index is
      // nulled out - but don't free the value since its still live.
      table->entries[backshift_candidate_idx].key = NULL;

      empty_space_idx += 1;
      if (empty_space_idx >= table->allocated) {
        // Wrap around.
        empty_space_idx = 0;
      }

      backshift_candidate_idx += 1;
      if (backshift_candidate_idx >= table->allocated) {
        // Wrap around.
        backshift_candidate_idx = 0;
      }
    }

    return true;
  } else {

    return false;
  }
}

char* pp_hash_table(hash_table* table) {
  if (table->occupied == 0) {
    char* pp_string = malloc(4 * sizeof(char));
    pp_string[0]    = '{';
    pp_string[1]    = ' ';
    pp_string[2]    = '}';
    pp_string[3]    = '\0';

    return pp_string;
  } else {
    // The pretty printer for the elements.
    char* (*key_pp_printer)(const void*) = pp_for_data_type(table->key_type);
    char* (*value_pp_printer)(const void*) =
        pp_for_data_type(table->value_type);

    // Initialize with enough space for the static initial character
    // and terminal - to make a well-behaved string. It will be
    // re-allocated for elements.
    char* pp_string = malloc(2 * sizeof(char));
    pp_string[0]    = '{';
    pp_string[1]    = '\0';

    for (int i = 0; i < table->allocated; i += 1) {
      hash_table_entry entry = table->entries[i];
      // If this entry is set, then transfer it to the new place.

      if (entry.key != NULL) {
        char* pp_key   = key_pp_printer(entry.key);
        char* pp_value = value_pp_printer(entry.value);

        size_t pp_key_len   = strlen(pp_key);
        size_t pp_value_len = strlen(pp_value);

        // Allocate space for the key and the ": " separator (plus the
        // \0 terminator).
        size_t new_length = (strlen(pp_string) + pp_key_len + 2 + 1);
        pp_string         = realloc(pp_string, new_length * sizeof(char));

        // Copy the representation into the buffer, free the source.
        pp_string = strcat(pp_string, pp_key);
        free(pp_key);

        pp_string[new_length - 3] = ':';
        pp_string[new_length - 2] = ' ';
        pp_string[new_length - 1] = '\0';

        new_length = (strlen(pp_string) + pp_value_len + 2 + 1);
        pp_string  = realloc(pp_string, new_length * sizeof(char));

        pp_string = strcat(pp_string, pp_value);
        free(pp_value);

        pp_string[new_length - 3] = ',';
        pp_string[new_length - 2] = ' ';
        pp_string[new_length - 1] = '\0';
      }
    }

    // Terminate the array, removing the extraneous element separator,
    // and preserve the string terminal.
    size_t pre_cut_length         = strlen(pp_string);
    pp_string[pre_cut_length - 2] = '}';
    pp_string[pre_cut_length - 1] = '\0';

    size_t final_length = strlen(pp_string);

    // Shrink the allocated string down to the required size.
    pp_string = realloc(pp_string, (final_length + 1) * sizeof(char));

    return pp_string;
  }
}

// Print the given hash table TABLE to stdout.
void print_hash_table(hash_table* table) {
  char* pp = pp_hash_table(table);
  printf("table: %s\n", pp);
  free(pp);
}
