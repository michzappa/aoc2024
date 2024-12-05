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

void free_hash_table(hash_table* table) {
  free(table->entries);
  free(table);
}

void* get_entry_in_hash_table(hash_table* table, const void* key) {
  uint64_t (*hasher)(const void*, data_type_t type) =
      hasher_for_data_type(table->key_type);

  uint64_t hash = hasher(key, table->key_type);

  // Scaling the hash value into the range of table indices.
  size_t table_index = (size_t)(hash & (int64_t)(table->allocated - 1));

  // Look for a matching entry until an empty entry is found. If we
  // find an empty entry that means the desired entry is not present.
  // Despite the wrap-around, this will terminate because the entries
  // list is never full.
  while (table->entries[table_index].key != NULL) {
    if (table->entries[table_index].key == key) {
      return table->entries[table_index].value;
    }

    // Linear probing resolves conflicts - move to the next entry.
    table_index += 1;
    if (table_index >= table->allocated) {
      // Wrap around.
      table_index = 0;
    }
  }

  return NULL;
}

void set_entry(hash_table_entry* entries, size_t num_entries,
               size_t start_index, const void* key, void* value,
               size_t* occupied) {

  size_t table_index = start_index;

  while (entries[table_index].key != NULL) {
    if (entries[table_index].key == key) {
      // Found an already existing entry for this key, update the
      // value.
      entries[table_index].value = value;
      return;
    }

    // Linear probing resolves conflicts - move to the next entry.
    table_index += 1;
    if (table_index >= num_entries) {
      // Wrap around.
      table_index = 0;
    }
  }

  // table_index is the first eligible empty entry. Set it so.
  entries[table_index].key   = key;
  entries[table_index].value = value;

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
  uint64_t (*hasher)(const void*, data_type_t type) =
      hasher_for_data_type(table->key_type);

  if (table->occupied + 1 > (table->allocated / 2)) {
    size_t new_allocation_size = 2 * table->allocated;
    hash_table_entry* new_entries =
        calloc(new_allocation_size, sizeof(hash_table_entry));

    for (size_t i = 0; i < table->allocated; i += 1) {
      hash_table_entry entry = table->entries[i];
      // If this entry is set, then transfer it to the new place.
      if (entry.key != NULL) {
        uint64_t hash = hasher(entry.key, table->key_type);
        size_t table_index =
            (size_t)(hash & (uint64_t)(new_allocation_size - 1));
        set_entry(new_entries, new_allocation_size, table_index, entry.key,
                  entry.value, NULL);
      }
    }

    free(table->entries);
    table->entries   = new_entries;
    table->allocated = new_allocation_size;
  }

  uint64_t hash = hasher(key, table->key_type);

  // Scaling the hash value into the range of table indices.
  size_t table_index = (size_t)(hash & (uint64_t)(table->allocated - 1));

  set_entry(table->entries, table->allocated, table_index, key, value,
            &table->occupied);
  return;
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

        puts(pp_string);
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
    pp_string[pre_cut_length - 2] = ']';
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
