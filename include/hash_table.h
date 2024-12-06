#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdbool.h>

#include "data.h"

#define HASH_TABLE_INIT_SIZE 16

// XXX: At the moment using NULL/0L as a key is unsupported. Such
// entries will not be retrievable or free-able. A key scheme change
// is required.

typedef struct {
  const void* key;     // If this entry is unassigned then key is NULL.
  void* value;         // Copy-in, reference-out.
  size_t displacement; // This entry's distance from its hash-ideal index.
} hash_table_entry;

typedef struct {
  data_type_t key_type;   // Type of keys (pre-hashing) in this table.
  data_type_t value_type; // Type of values in this table.

  hash_table_entry* entries;
  size_t occupied;
  size_t allocated;
} hash_table;

// Initialize a hash table with KEY_TYPE and VALUE TYPE.
hash_table* init_hash_table(data_type_t key_type, data_type_t value_type);

// Free the given hash table TABLE.
void free_hash_table(hash_table* table);

// Return the value associated with the given key in TABLE.
void* get_entry_in_hash_table(hash_table* table, const void* key);

// Set a copy of VALUE to be associated with KEY in TABLE.
void set_entry_in_hash_table(hash_table* table, const void* key, void* value);

// Remove the value associated with the given key in TABLE - return
// whether operation succeeded.
bool remove_entry_in_hash_table(hash_table* table, const void* key);

// Return a string representing the given table TABLE.
char* pp_hash_table(hash_table* table);

// Print the given hash table TABLE to stdout.
void print_hash_table(hash_table* table);

#endif
