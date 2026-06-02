Table *init_table()
{
    Table *table = arena_alloc(&default_arena, sizeof(*table));
    table->count = table->capacity = 0;
    return table;
}
/*FNV-a1 hash function 32 bit*/
uint32_t table_hash_string(char *key, size_t length)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

Entry *table_find_entry(Table *table, ObjString *key)
{
    uint32_t capacity = (uint32_t)table->capacity;
    uint32_t index = key->hash % capacity;
    Entry *tombstone = NULL;
    for (;;)
    {
        Entry *entry = &array_at(table, index);
        if (entry->key == NULL)
        {
            if (IS_NULL(entry->value))
            {
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                if (tombstone == NULL)
                    tombstone = entry;
            }
        }
        else if (entry->key == key)
        {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}
bool table_remove(Table *table, ObjString *key)
{
    if (table->count == 0)
        return false;
    Entry *entry = table_find_entry(table, key);
    if (entry->key == NULL)
        return false;
    entry->key = NULL;
    entry->value = BOOL_VAL(true, 0, 0);
    return true;
}
bool table_get(Table *table, ObjString *key, Value *value)
{
    if (table->count == 0)
        return false;

    Entry *entry = table_find_entry(table, key);
    if (entry->key == NULL)
        return false;
    *value = entry->value;

    return true;
}
bool table_set(Table *table, ObjString *key, Value value)
{
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        table->capacity = table->capacity == 0 ? 256 : table->capacity * 2;
        Entry *previous_items = table->items;
        Entry *current_items = arena_alloc(&default_arena, table->capacity * sizeof(*table->items));
        assert(current_items != NULL && "failed to allocate hash table");
        for (size_t i = 0; i < table->capacity; i++)
        {
            current_items[i].key = NULL;
            current_items[i].value = NULL_VAL(0, 0);
        }
        // re-hashing
        table->count = 0;
        for (size_t i = 0; i < table->capacity && table->items != NULL; i++)
        {
            Entry *entry = &array_at(table, i);
            if (entry->key == NULL)
                continue;

            table->items = current_items;
            Entry *dest = table_find_entry(table, entry->key);
            dest->key = entry->key;
            dest->value = entry->value;
            table->items = previous_items;
            table->count++;
        }
        free(table->items);
        table->items = current_items;
    }
    Entry *entry = table_find_entry(table, key);
    bool is_new_key = entry->key == NULL;

    if (is_new_key && IS_NULL(entry->value))
        table->count++;

    entry->key = key;
    entry->value = value;

    return is_new_key;
}
ObjString *table_find_string(Table *table, char *chars)
{
    if (table->count == 0)
        return NULL;

    size_t length = strlen(chars);
    uint32_t hash = table_hash_string(chars, length);
    uint32_t index = hash % table->capacity;

    for (;;)
    {
        Entry *entry = &array_at(table, index);

        if (entry->key == NULL)
        {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NULL(entry->value))
                return NULL;
        }
        else if (entry->key->length == length &&
                 entry->key->hash == hash &&
                 memcmp(entry->key->chars, chars, length) == 0)
        {
            // We found it.
            return entry->key;
        }
        index = (index + 1) % table->capacity;
    }
}
void table_free(Table *table)
{
    nob_da_free(*table);
    free(table);
}