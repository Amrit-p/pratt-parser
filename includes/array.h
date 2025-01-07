#ifndef FU_ARRAY_H
#define FU_ARRAY_H
#include <stddef.h>
#include <assert.h>

#define define_array(name, type) \
    struct                       \
    {                            \
        type *items;             \
        size_t count;            \
        size_t capacity;         \
    } name
#define init_array(name)                      \
    do                                        \
    {                                         \
        (name)->count = (name)->capacity = 0; \
        (name)->items = NULL;                 \
    } while (0)

#define array_push(array, item)                                                                    \
    do                                                                                             \
    {                                                                                              \
        if ((array)->count >= (array)->capacity)                                                   \
        {                                                                                          \
            (array)->capacity = (array)->capacity == 0 ? 256 : (array)->capacity * 2;              \
            (array)->items = realloc((array)->items, (array)->capacity * sizeof(*(array)->items)); \
            assert((array)->items != NULL && "cannot allocate memory");                            \
        }                                                                                          \
                                                                                                   \
        (array)->items[(array)->count++] = (item);                                                 \
    } while (0)

#define array_pop(array) \
    (array)->items[--(array)->count]

#define array_peek(array) \
    (array)->items[(array)->count]

#define array_free(array) \
    free((array)->items)

#define array_at(array, offset) \
    (array)->items[offset]

#define array_size(array) \
    (array)->count

#endif