#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "data.h"

#define HASH_TABLE_INIT_SIZE 16

typedef struct {
  const void* key; // If this entry is unassigned then key is NULL.
  void* value;
} hash_table_entry;

typedef struct {
  data_type_t key_type;   // Type of keys (pre-hashing) in this table.
  data_type_t value_type; // Type of values in this table.

  hash_table_entry* entries; // Array
  size_t occupied;
  size_t allocated;
} hash_table;

// Initialize a hash table with KEY_TYPE and VALUE TYPE.
hash_table* init_hash_table(data_type_t key_type, data_type_t value_type);

// Free the given hash table TABLE.
void free_hash_table(hash_table* table);

// Return the value associated with the given key in TABLE.
void* get_entry_in_hash_table(hash_table* table, const void* key);

// Set VALUE to be associated with KEY in TABLE.
void set_entry_in_hash_table(hash_table* table, const void* key, void* value);

// Return a string representing the given table TABLE.
char* pp_hash_table(hash_table* table);

// Print the given hash table TABLE to stdout.
void print_hash_table(hash_table* table);

#endif
