#ifndef VALUE_H
#define VALUE_H

typedef struct Value Value;
typedef struct Obj Obj;
typedef uint8_t byte;

typedef enum
{
    VAL_BOOL,
    VAL_NUMBER,
    VAL_NULL,
    VAL_OBJ,
    VAL_UNDEF,
} ValueType;

struct Value
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        Obj *obj;
    } as;

    uint32_t row;
    uint32_t col;
};

#define BOOL_VAL(value, row, col) ((Value){VAL_BOOL, {.boolean = value}, row, col})
#define NULL_VAL(row, col) ((Value){VAL_NULL, {.number = 0}, row, col})
#define NUMBER_VAL(value, row, col) ((Value){VAL_NUMBER, {.number = value}, row, col})
#define OBJ_VAL(object, row, col) ((Value){VAL_OBJ, {.obj = (Obj *)object}, row, col})
#define UNDEF_VAL(row, col) ((Value){VAL_UNDEF, {.number = 0}, row, col})

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_INTEGRAL(value) ((int)(value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NULL(value) ((value).type == VAL_NULL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)
#define IS_UNDEF(value) ((value).type == VAL_UNDEF)

typedef struct
{
    Value *items;
    size_t count;
    size_t capacity;
} Values;

Values *init_values();
byte value_push(Values *values, Value data); /*returns index of the data */
Value value_pop(Values *values);
void values_dump(Values *values, FILE *stream);
void values_free(Values *values);
void value_print(Value value,uint8_t new_ln);
#endif