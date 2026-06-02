#ifndef TABLE_H
#define TABLE_H

#include "object.h"

#define TABLE_MAX_LOAD 0.75

typedef struct
{
    ObjString *key;
    Value value;
} Entry;

typedef struct
{
    Entry *items;
    size_t count;
    size_t capacity;
} Table;

Table *init_table();
void table_free(Table *table);
uint32_t table_hash_string(char *key, size_t length);
Entry *table_find_entry(Table *table, ObjString *key);
bool table_set(Table *table, ObjString *key, Value value);
bool table_get(Table *table, ObjString *key, Value *value);
bool table_remove(Table *table, ObjString *key);
ObjString *table_find_string(Table *table, char *chars);
#endif