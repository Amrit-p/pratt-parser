Obj *objects = {0};
Obj *object_allocate(size_t size, ObjType type)
{
    Obj *object = arena_alloc(&default_arena, size);
    object->type = type;
    object->next = objects;
    objects = object;
    return object;
}

bool is_obj_type(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}
void object_print(Value value)
{
    switch (OBJ_TYPE(value))
    {
    case OBJ_STRING:
        fprintf(stdout, "%s", AS_CSTRING(value));
        break;
    case OBJ_FUNCTION:
        fprintf(stdout, "<fn .%s>", AS_FUNCTION(value)->name == NULL ? "entry" : AS_FUNCTION(value)->name->chars);
        break;
    case OBJ_NATIVE:
        fprintf(stdout, "<native fn>");
        break;
    default:
        NOB_UNREACHABLE("object_print");
    }
}
ObjString *allocate_string(size_t length, uint32_t hash)
{
    ObjString *string = (ObjString *)object_allocate(sizeof(ObjString) + (length + 1) * sizeof(char), OBJ_STRING);
    string->length = length;
    string->hash = hash;
    return string;
}
ObjString *new_string(char *chars, size_t length)
{
    uint32_t hash = 0;
    if (chars != NULL)
        hash = table_hash_string(chars, length);

    ObjString *string = allocate_string(length, hash);
    string->length = length;
    if (chars)
        strcpy(string->chars, chars);
    return string;
}

ObjString *cstr_to_objstr(char *chars)
{
    size_t chars_len = strlen(chars);
    ObjString *string = new_string(chars, chars_len);
    return string;
}

ObjFunction *new_function()
{
    ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->chunk = init_chunk();
    function->values = init_values();
    return function;
}
void object_free(Obj *object)
{
    switch (object->type)
    {
    case OBJ_STRING:
    {
        ObjString *string = (ObjString *)object;
        free(string);
        break;
    }
    case OBJ_FUNCTION:
    {
        ObjFunction *function = (ObjFunction *)object;
        values_free(function->values);
        chunk_free(function->chunk);
        free(function);
        break;
    }
    case OBJ_NATIVE:
    {
        free(object);
        break;
    }
    default:
        NOB_UNREACHABLE("object_free");
    }
}

void objects_free()
{
    Obj *temp = objects;
    while (temp != NULL)
    {
        Obj *next = temp->next;
        object_free(temp);
        temp = next;
    }
}

ObjNative *new_native(NativeFn function, int arity)
{
    ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    native->arity = arity;
    return native;
}

char *value_typeof(Value value)
{
    switch (value.type)
    {
    case VAL_BOOL:
        return "Boolean";
    case VAL_NULL:
        return "null";
    case VAL_NUMBER:
        return "Number";
    case VAL_OBJ:
    {
        Obj *obj = AS_OBJ(value);
        switch (obj->type)
        {
        case OBJ_STRING:
            return "String";
        case OBJ_FUNCTION:
            return "Function";
        default:
            return "Object";
        }
    }
    default:
        NOB_UNREACHABLE("value_typeof");
    }
}