VM *init_vm(Compiler *compiler)
{
    VM *vm = arena_alloc(&default_arena, sizeof(*vm));

    vm->sp = vm->stack.items;

    vm->file_path = compiler->file_path;
    vm->globals = compiler->globals;
    vm->strings = compiler->strings;
    vm->variables = compiler->variables;

    *vm->sp = OBJ_VAL(compiler->function, __LINE__, 0);
    vm->sp++;

    CallFrame *frame = &vm->frames[vm->frame_count++];
    frame->function = compiler->function;
    frame->ip = compiler->function->chunk->items;
    frame->slots = vm->stack.items;
    return vm;
}
void vm_free(VM *vm)
{
    free(vm);
}

bool is_falsey(Value value)
{
    return IS_NULL(value) ||
           (IS_NUMBER(value) && AS_NUMBER(value) == 0) ||
           (IS_STRING(value) && AS_STRING(value)->length == 0) ||
           (IS_BOOL(value) && !AS_BOOL(value));
}
VM_Error vm_interpret(VM *vm)
{
    CallFrame *frame = &vm->frames[vm->frame_count - 1];

#define VM_CURRENT_CHUNK (frame->function->chunk)
#define VM_CURRENT_CHUNK_BASE (frame->function->chunk->items)

#define VM_STACK_PUSH(value) \
    do                       \
    {                        \
        *vm->sp = (value);   \
        vm->sp++;            \
        vm->stack.count++;   \
    } while (0)

#define VM_STACK_POP(ident) \
    do                      \
    {                       \
        vm->sp--;           \
        ident = *vm->sp;    \
        vm->stack.count--;  \
    } while (0)

#define VM_STACK_PEEK(ident, offset) \
    do                               \
    {                                \
        ident = vm->sp[-1 - offset]; \
    } while (0)

#define VM_READ_BYTE() (*frame->ip++)
#define VM_READ_CONSTANT() (frame->function->values->items[VM_READ_BYTE()])
#define VM_READ_STRING() AS_STRING(VM_READ_CONSTANT())
#define VM_READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define VM_TYPE_ERROR(must, given, _row, _col)                                           \
    do                                                                                   \
    {                                                                                    \
        char *template = "Operand must be a type of " must ". But given type is \"%s\""; \
        char *message = arena_sprintf(&default_arena, template, given);                  \
        vm->message = message;                                                           \
        vm->row = (_row);                                                                \
        vm->col = (_col);                                                                \
        return VM_TYPE_ERROR;                                                            \
    } while (0)

#define VM_REF_ERROR(obj_str, value)                                               \
    do                                                                             \
    {                                                                              \
        char *template = "'%s' is not defined";                                    \
        char *message = arena_sprintf(&default_arena, template, (obj_str)->chars); \
        vm->row = (value).row;                                                     \
        vm->col = (value).col;                                                     \
        vm->message = message;                                                     \
        return VM_REFERENCE_ERROR;                                                 \
    } while (0);

#define LOGICAL_OP(op)                                                       \
    do                                                                       \
    {                                                                        \
        if (vm->stack.count < 2)                                             \
        {                                                                    \
            vm->message = arena_sprintf(&default_arena, "stack underflow");  \
            return VM_STACK_UNDERFLOW;                                       \
        }                                                                    \
        Value b = {0};                                                       \
        VM_STACK_POP(b);                                                     \
        Value a = {0};                                                       \
        VM_STACK_POP(a);                                                     \
        if (b.type != a.type)                                                \
        {                                                                    \
            VM_STACK_PUSH(BOOL_VAL(false, 0, 0));                            \
        }                                                                    \
        else                                                                 \
            switch (b.type)                                                  \
            {                                                                \
            case VAL_BOOL:                                                   \
                VM_STACK_PUSH(BOOL_VAL(AS_BOOL(a) op AS_BOOL(b), 0, 0));     \
                break;                                                       \
            case VAL_NUMBER:                                                 \
                VM_STACK_PUSH(BOOL_VAL(AS_NUMBER(a) op AS_NUMBER(b), 0, 0)); \
                break;                                                       \
            case VAL_NULL:                                                   \
                VM_STACK_PUSH(BOOL_VAL(true, 0, 0));                         \
                break;                                                       \
            case VAL_OBJ:                                                    \
                VM_STACK_PUSH(BOOL_VAL(AS_OBJ(a) op AS_OBJ(b), 0, 0));       \
                break;                                                       \
            default:                                                         \
                NOB_UNREACHABLE(__func__);                                   \
            }                                                                \
    } while (0)

#define ARITHMETIC_OP(op, AS)                                               \
    do                                                                      \
    {                                                                       \
        if (vm->stack.count < 2)                                            \
        {                                                                   \
            vm->message = arena_sprintf(&default_arena, "stack underflow"); \
            return VM_STACK_UNDERFLOW;                                      \
        }                                                                   \
        Value b = {0};                                                      \
        VM_STACK_POP(b);                                                    \
        Value a = {0};                                                      \
        VM_STACK_POP(a);                                                    \
        if (!IS_NUMBER(b) || !IS_NUMBER(a))                                 \
        {                                                                   \
            int row = !IS_NUMBER(a) ? a.row : b.row;                        \
            int col = !IS_NUMBER(a) ? a.col : b.col;                        \
            char *given_type = value_typeof(!IS_NUMBER(a) ? a : b);         \
            VM_TYPE_ERROR("\"Number\"", given_type, row, col);              \
        }                                                                   \
        VM_STACK_PUSH(NUMBER_VAL(AS(a) op AS(b), b.row, b.col));            \
    } while (0)

#define UNARY_ARITHMETIC_OP(op, AS)                                    \
    do                                                                 \
    {                                                                  \
        Value value = {0};                                             \
        VM_STACK_POP(value);                                           \
        if (!IS_NUMBER(value))                                         \
        {                                                              \
            char *given_type = value_typeof(value);                    \
            VM_TYPE_ERROR("Number", given_type, value.row, value.row); \
        }                                                              \
        VM_STACK_PUSH(NUMBER_VAL(op AS(value), value.row, value.col)); \
    } while (0)

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        chunk_print_instruction(VM_CURRENT_CHUNK, (size_t)(frame->ip - VM_CURRENT_CHUNK_BASE), stdout);
        Value *slot = vm->stack.items;
        printf("          ");
        for (; slot < vm->sp; slot++)
        {
            printf("[");
            value_print(*slot, 0);
            printf("]");
        }
        printf("\n");
        getc(stdin);
#endif
        byte instruction = VM_READ_BYTE();
        switch (instruction)
        {
        case OP_CALL:
        {
            if (vm->frame_count == FRAMES_MAX)
            {
                vm->message = arena_sprintf(&default_arena, "maximum call depth reached");
                return VM_STACK_OVERFLOW;
            }
            uint8_t agr_count = VM_READ_BYTE();
            Value value = {0};
            VM_STACK_PEEK(value, agr_count);
            if (IS_OBJ(value))
            {
                switch (OBJ_TYPE(value))
                {
                case OBJ_FUNCTION:
                {
                    ObjFunction *function = AS_FUNCTION(value);
                    if (agr_count != function->arity)
                    {
                        char *template = "Function '%s' expects %d parameters, but %d was provided.";
                        char *message = arena_sprintf(&default_arena, template, function->name->chars, function->arity, agr_count);
                        vm->message = message;
                        vm->row = value.row;
                        vm->col = value.col;
                    }
                    if (agr_count < function->arity)
                    {
                        return VM_TOO_FEW_ARGUMENTS;
                    }
                    if (agr_count > function->arity)
                    {
                        return VM_TOO_MANY_ARGUMENTS;
                    }
                    CallFrame *call_frame = &vm->frames[vm->frame_count++];
                    call_frame->function = function;
                    call_frame->ip = function->chunk->items;
                    call_frame->slots = vm->sp - agr_count - 1;
                    frame = call_frame;
                    continue;
                }
                case OBJ_NATIVE:
                {
                    ObjNative *native = AS_NATIVE(value);
                    Value result = native->function(vm, vm->sp - agr_count);
                    if (vm->message)
                    {
                        return VM_TYPE_ERROR;
                    }
                    vm->sp -= agr_count + 1;
                    result.row = value.row;
                    result.col = value.col;
                    VM_STACK_PUSH(result);
                    continue;
                }
                default:
                {
                }
                }
            }
            char *given = value_typeof(value);
            VM_TYPE_ERROR("\"Function\"", given, value.row, value.col);
            break;
        }
        case OP_DUP:
        {
            Value value = {0};
            VM_STACK_PEEK(value, 0);
            VM_STACK_PUSH(value);
            break;
        }
        case OP_LOOP:
        {
            uint16_t offset = VM_READ_SHORT();
            frame->ip -= offset;
            break;
        }
        case OP_JMP:
        {
            uint16_t offset = VM_READ_SHORT();
            frame->ip = VM_CURRENT_CHUNK_BASE + offset;
            break;
        }
        case OP_JMP_IF_FALSE:
        {
            uint16_t offset = VM_READ_SHORT();
            Value value;
            VM_STACK_PEEK(value, 0);
            bool fval = is_falsey(value);
            if (fval)
                frame->ip = VM_CURRENT_CHUNK_BASE + offset;
            break;
        }
        case OP_POP:
        {
            Value value;
            VM_STACK_POP(value);
            (void)value;
            break;
        }
        case OP_SET_LOCAL:
        {
            byte slot = VM_READ_BYTE();
            Value value;
            VM_STACK_PEEK(value, 0);
            frame->slots[slot] = value;
            break;
        }
        case OP_GET_LOCAL:
        {
            byte slot = VM_READ_BYTE();
            Value name = VM_READ_CONSTANT();
            Value value = frame->slots[slot];
            value.row = name.row;
            value.col = name.col;
            VM_STACK_PUSH(value);
            break;
        }
        case OP_SET_GLOBAL:
        {
            Value name = VM_READ_CONSTANT();
            ObjString *str = AS_STRING(name);
            Value value = {0};
            VM_STACK_PEEK(value, 0);
            if (table_set(vm->globals, str, value))
            {
                table_remove(vm->globals, str);
                VM_REF_ERROR(str, value);
            }
            break;
        }
        case OP_DEFINE_GLOBAL:
        {
            ObjString *name = VM_READ_STRING();
            Value value = {0};
            VM_STACK_PEEK(value, 0);
            table_set(vm->globals, name, value);
            VM_STACK_POP(value);
            break;
        }
        case OP_GET_GLOBAL:
        {
            Value name = VM_READ_CONSTANT();
            ObjString *str = AS_STRING(name);
            Value value = {0};
            if (!table_get(vm->globals, str, &value))
            {
                VM_REF_ERROR(str, name);
            }
            value.row = name.row;
            value.col = name.col;
            VM_STACK_PUSH(value);
            break;
        }
        case OP_PRINT:
        {
            Value value = {0};
            VM_STACK_POP(value);
            value_print(value, 1);
            break;
        }
        case OP_RETURN:
        {
            Value value = {0};
            vm->frame_count--;
            if (vm->frame_count == 0)
            {
                VM_STACK_POP(value);
                return VM_OK;
            }
            VM_STACK_POP(value);
            vm->sp = frame->slots;
            VM_STACK_PUSH(value);
            frame = &vm->frames[vm->frame_count - 1];
            break;
        }
        case OP_CONSTANT:
        {
            Value constant = VM_READ_CONSTANT();
            VM_STACK_PUSH(constant);
            break;
        }
        case OP_ADD:
        {
            Value b = {0};
            Value a = {0};
            VM_STACK_POP(b);
            VM_STACK_POP(a);
            if (IS_STRING(b) && IS_STRING(a))
            {
                ObjString *bString = AS_STRING(b);
                ObjString *aString = AS_STRING(a);
                ObjString *result = new_string(NULL, aString->length + bString->length);
                strcpy(result->chars, aString->chars);
                strcat(result->chars, bString->chars);
                result->hash = table_hash_string(result->chars, result->length);
                VM_STACK_PUSH(OBJ_VAL(result, 0, 0));
            }
            else if (IS_NUMBER(b) && IS_NUMBER(a))
            {
                VM_STACK_PUSH(NUMBER_VAL(AS_NUMBER(a) + AS_NUMBER(b), 0, 0));
            }
            else
            {
                uint32_t row = !IS_NUMBER(a) ? a.row : b.row;
                uint32_t col = !IS_NUMBER(a) ? a.col : b.col;
                char *given_type = value_typeof(!IS_NUMBER(a) ? a : b);
                VM_TYPE_ERROR("\"Number\"", given_type, row, col);
            }
            break;
        }
        case OP_SUBTRACT:
            ARITHMETIC_OP(-, AS_NUMBER);
            break;
        case OP_MULTIPLY:
            ARITHMETIC_OP(*, AS_NUMBER);
            break;
        case OP_DIVIDE:
            ARITHMETIC_OP(/, AS_NUMBER);
            break;
        case OP_MOD:
            ARITHMETIC_OP(%, AS_INTEGRAL);
            break;
        case OP_BITWISE_AND:
            ARITHMETIC_OP(&, AS_INTEGRAL);
            break;
        case OP_BITWISE_OR:
            ARITHMETIC_OP(|, AS_INTEGRAL);
            break;
        case OP_BITWISE_NOT:
            UNARY_ARITHMETIC_OP(~, AS_INTEGRAL);
            break;
        case OP_LEFT_SHIFT:
            ARITHMETIC_OP(<<, AS_INTEGRAL);
            break;
        case OP_RIGHT_SHIFT:
            ARITHMETIC_OP(>>, AS_INTEGRAL);
            break;
        case OP_EQUAL:
            LOGICAL_OP(==);
            break;
        case OP_NOT_EQUAL:
            LOGICAL_OP(!=);
            break;
        case OP_LT:
            LOGICAL_OP(<);
            break;
        case OP_LTE:
            LOGICAL_OP(<=);
            break;
        case OP_GT:
            LOGICAL_OP(>);
            break;
        case OP_GTE:
            LOGICAL_OP(>=);
            break;
        case OP_NOT:
        {
            Value val = {0};
            VM_STACK_POP(val);
            VM_STACK_PUSH(BOOL_VAL(is_falsey(val), val.row, val.col));
            break;
        }
        case OP_NEGATE:
        {
            UNARY_ARITHMETIC_OP(-, AS_NUMBER);
            break;
        }
        default:
            fprintf(stderr, "[ERROR] illegal instruction pointer(%zu)\n", (size_t)(frame->ip - VM_CURRENT_CHUNK_BASE) - 1);
            return VM_ILLEGAL_INSTRUCTION;
        }
    }
#undef VM_READ_BYTE
#undef VM_READ_CONSTANT
#undef VM_READ_SHORT
#undef VM_READ_STRING
#undef VM_STACK_PUSH
#undef VM_STACK_POP
#undef VM_TYPE_ERROR
#undef VM_CURRENT_CHUNK_BASE
#undef VM_CURRENT_CHUNK
}

void vm_print_stack_trace(VM *vm)
{
    for (int i = vm->frame_count - 1; i >= 0; --i)
    {
        CallFrame *frame = &vm->frames[i];
        ObjFunction *function = frame->function;
        fprintf(stderr, "#%d ", i);
        if (function->name == NULL)
        {
            fprintf(stderr, "%s\n", vm->file_path);
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
}

void vm_print_error(VM *vm, VM_Error error)
{
    if (error == VM_OK)
        return;

    vm_print_stack_trace(vm);

    switch (error)
    {
    case VM_TYPE_ERROR:
        fprintf(stderr, "TypeError at %s:%d:%d %s\n", vm->file_path, vm->row, vm->col, vm->message);
        break;
    case VM_STACK_OVERFLOW:
    case VM_STACK_UNDERFLOW:
        fprintf(stderr, "%s InternalError %s\n", vm->file_path, vm->message);
        break;
    case VM_REFERENCE_ERROR:
        fprintf(stderr, "ReferenceError at %s:%d:%d %s\n", vm->file_path, vm->row, vm->col, vm->message);
        break;
    case VM_TOO_FEW_ARGUMENTS:
        fprintf(stderr, "TooFewArguments at %s:%d:%d %s\n", vm->file_path, vm->row, vm->col, vm->message);
        break;
    case VM_TOO_MANY_ARGUMENTS:
        fprintf(stderr, "TooManyArguments at %s:%d:%d %s\n", vm->file_path, vm->row, vm->col, vm->message);
        break;
    default:
        break;
    }
}
