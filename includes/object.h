#ifndef OBJECT_H
#define OBJECT_H


#include "value.h"
#include "chunk.h"

typedef enum
{
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
} ObjType;

struct Obj
{
    ObjType type;
    Obj *next;
};

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define ALLOCATE_OBJ(type, objectType) \
    (type *)object_allocate(sizeof(type), objectType)

Obj *object_allocate(size_t size, ObjType type);
bool is_obj_type(Value value, ObjType type);
void object_print(Value value);

typedef struct VM x;
typedef Value (*NativeFn)(x *vm, Value *args);
typedef struct ObjString ObjString;

struct ObjString
{
    Obj obj;
    size_t length;
    uint32_t hash;
    char chars[];
};
typedef struct
{
    Obj obj;
    int arity;
    Chunk *chunk;
    Values *values;
    ObjString *name;
} ObjFunction;

typedef struct
{
    Obj obj;
    NativeFn function;
    int arity;
    ObjString *name;
} ObjNative;

#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_NATIVE(value) ((ObjNative *)AS_OBJ(value))

#define IS_STRING(value) is_obj_type(value, OBJ_STRING)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value) is_obj_type(value, OBJ_NATIVE)

ObjString *cstr_to_objstr(char *chars);
ObjString *new_string(char *chars, size_t length);
ObjString *allocate_string(size_t length, uint32_t hash);

ObjFunction *new_function();

void objects_free();

char *value_typeof(Value value);
ObjNative *new_native(NativeFn function, int arity);
#endif