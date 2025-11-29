#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include "hashmap.h"
#include "../defs.h"

#define INITIAL_CAPACITY 2
#define IS_POWER_OF_TWO 1
#define INITIAL_BUCKET_CAPACITY 1
#define RESIZE_RATIO 0.75

char* put_label_internal(symbol_table_ptr_t table, char *label, u32 address, bool overwrite, bool new_alloc);

#define PUT_LABEL_ENTRY(table, entry, overwrite, new_alloc) (char*)put_label_internal(table, (entry).label, (entry).address, overwrite, new_alloc);

typedef unsigned int uint;

typedef struct {
  char *label;
  u32 address;
} entry;

typedef struct {
  entry *entries;
  uint capacity;
  uint size;
} bucket_t;

struct symbol_table_t {
  bucket_t *buckets;
  uint capacity;
  uint size;
}; 

symbol_table_ptr_t create_table_ADT( void ) {
  symbol_table_ptr_t table = malloc(sizeof(struct symbol_table_t));
  if (table == NULL) {
    return NULL;
  }

  table->capacity = INITIAL_CAPACITY;
  table->size = 0;
  table->buckets = malloc(INITIAL_CAPACITY * sizeof(bucket_t));
  if (table->buckets == NULL) {
    free(table);
    return NULL;
  }
  for (uint i = 0; i < INITIAL_CAPACITY; ++i) {
    table->buckets[i].entries = calloc(INITIAL_BUCKET_CAPACITY, sizeof(entry));
    if (table->buckets[i].entries == NULL) {
      for (uint j = 0; j < i; ++j) {
        free(table->buckets[j].entries);
      }
      free(table->buckets);
      free(table);
      return NULL;
    }
    table->buckets[i].capacity = INITIAL_BUCKET_CAPACITY;
    table->buckets[i].size = 0;
  }

  return table;
}

// resizing table, will *NOT* check if it is a useful resize
static void resize(symbol_table_ptr_t table) {
  struct symbol_table_t new_table = {
    .capacity = table->capacity << 1,
    .size     = 0,
    .buckets  = malloc(2 * table->capacity * sizeof(bucket_t))
  };
  if (new_table.buckets == NULL) {
    fprintf(stderr, "FAILED HASHMAP RESIZE\n");
    free_table(table);
    exit(1);
  }
  for (uint i = 0; i < new_table.capacity; ++i) {
    new_table.buckets[i].entries = calloc(INITIAL_BUCKET_CAPACITY, sizeof(entry));
    if (new_table.buckets[i].entries == NULL) {
      for (uint j = 0; j < i; ++j) {
        free(new_table.buckets[j].entries);
      }
      free(new_table.buckets);
      free_table(table);
      fprintf(stderr, "FAILED HASHMAP RESIZE\n");
      exit(1);
    }
    new_table.buckets[i].capacity = INITIAL_BUCKET_CAPACITY;
    new_table.buckets[i].size = 0;
  }

  // copying elements in new table, while freeing entries
  for (uint i = 0; i < table->capacity; ++i) {
    for (uint j = 0; j < table->buckets[i].size; ++j) {
      PUT_LABEL_ENTRY(&new_table, table->buckets[i].entries[j], true, false);
    }
    free(table->buckets[i].entries);
  }
  free(table->buckets);
  *table = new_table;
}

static u32 get_hash(char *label) {
  u32 hash = 2166136261u;
  for (unsigned char p; (p = *label) != '\0'; label++) {
    hash ^= p;
    hash *= 16777619u;
  }
  return hash;
}

static uint get_bucket_index(uint capacity, char *label) {
  if (IS_POWER_OF_TWO) {
    return (get_hash(label) & (capacity - 1));
  } else {
    return (get_hash(label) % capacity);
  }
}

// return NULL if it doesnt' have it, pointer to entry if it does
static entry* bucket_contains(bucket_t *bucket, char* label) {
  entry *end = bucket->entries + bucket->size;
  for (entry* it = bucket->entries; it != end; it++) {
    if (!strcmp(it->label, label)) {
      return it;
    }
  }
  return NULL;
}

//throws error if lable already exists
char* put_label_internal(symbol_table_ptr_t table, char *label, u32 address, bool overwrite, bool new_alloc) {
  uint bucket_index = get_bucket_index(table->capacity, label);
  entry *existing_entry = NULL;
  if ((existing_entry = bucket_contains(&table->buckets[bucket_index], label)) != NULL) {
    if (overwrite) {
      existing_entry->address = address;
    } else {
      return existing_entry->label;
    }
  }
  if (table->size >= table->capacity * RESIZE_RATIO) {
    resize(table);
    return put_label_internal(table, label, address, overwrite, new_alloc);
  } else {
    table->size++;
    if (table->buckets[bucket_index].size == table->buckets[bucket_index].capacity) {
      table->buckets[bucket_index].capacity <<= 1;      
      entry *tmp = realloc(table->buckets[bucket_index].entries, 
        table->buckets[bucket_index].capacity * sizeof(entry));
      if (tmp == NULL) {
        free_table(table);
        fprintf(stderr, "Malloc failed for resizing bucket");
        exit(1);
      }
      table->buckets[bucket_index].entries = tmp;
    }
    entry new_entry = {
      .label = NULL,
      .address = address
    };
    if (new_alloc) {
      new_entry.label = malloc((strlen(label) + 1) * sizeof(char));
      if (new_entry.label == NULL) {
        free_table(table);
        fprintf(stderr, "Malloc failed for adding entry");
        exit(1);
      }
      strcpy(new_entry.label, label); 
    } else {
      new_entry.label = label;
    }
    table->buckets[bucket_index].entries[table->buckets[bucket_index].size++] = new_entry;
    return new_entry.label;
  }
}

void free_table(symbol_table_ptr_t table) {
  for (uint i = 0; i < table->capacity; ++i) {
    for (uint j = 0; j < table->buckets[i].size; ++j) {
      free(table->buckets[i].entries[j].label);
    }
    free(table->buckets[i].entries);
  }
  free(table->buckets);
  free(table);
}

//returns -1 if it doesn't exist
u32 get_label_address(symbol_table_ptr_t table, char *label) {
  uint bucket_index = get_bucket_index(table->capacity, label);
  for (uint i = 0; i < table->buckets[bucket_index].size; i++) {
    if (!strcmp(table->buckets[bucket_index].entries[i].label, label)) {
      return table->buckets[bucket_index].entries[i].address;
    }
  }
  return UINT32_MAX;
}

char* put_label(symbol_table_ptr_t table, char *label, u32 address, bool overwrite) {
  return put_label_internal(table, label, address, overwrite, true);
}
