const char *chunk_byte_to_str(byte instruction)
{
    switch (instruction)
    {
    case OP_RETURN:
        return "OP_RETURN";
    case OP_CONSTANT:
        return "OP_CONSTANT";
    case OP_NEGATE:
        return "OP_NEGATE";
    case OP_ADD:
        return "OP_ADD";
    case OP_SUBTRACT:
        return "OP_SUBTRACT";
    case OP_MULTIPLY:
        return "OP_MULTIPLY";
    case OP_DIVIDE:
        return "OP_DIVIDE";
    case OP_MOD:
        return "OP_MOD";
    case OP_NOT:
        return "OP_NOT";
    case OP_EQUAL:
        return "OP_EQUAL";
    case OP_GT:
        return "OP_GT";
    case OP_LT:
        return "OP_LT";
    case OP_GTE:
        return "OP_GTE";
    case OP_LTE:
        return "OP_LTE";
    case OP_NOT_EQUAL:
        return "OP_NOT_EQUAL";
    case OP_BITWISE_AND:
        return "OP_BITWISE_AND";
    case OP_BITWISE_OR:
        return "OP_BITWISE_OR";
    case OP_BITWISE_NOT:
        return "OP_BITWISE_NOT";
    case OP_PRINT:
        return "OP_PRINT";
    case OP_DEFINE_GLOBAL:
        return "OP_DEFINE_GLOBAL";
    case OP_GET_GLOBAL:
        return "OP_GET_GLOBAL";
    case OP_SET_GLOBAL:
        return "OP_SET_GLOBAL";
    case OP_SET_LOCAL:
        return "OP_SET_LOCAL";
    case OP_POP:
        return "OP_POP";
    case OP_GET_LOCAL:
        return "OP_GET_LOCAL";
    case OP_JMP_IF_FALSE:
        return "OP_JMP_IF_FALSE";
    case OP_JMP:
        return "OP_JMP";
    case OP_LOOP:
        return "OP_LOOP";
    case OP_LEFT_SHIFT:
        return "OP_LEFT_SHIFT";
    case OP_RIGHT_SHIFT:
        return "OP_RIGHT_SHIFT";
    case OP_DUP:
        return "OP_DUP";
    case OP_CALL:
        return "OP_CALL";
    default:
        return "UNKNOWN";
    }
}
Chunk *init_chunk()
{
    Chunk *chunk = arena_alloc(&default_arena, sizeof(*chunk));
    return chunk;
}
size_t chunk_push(Chunk *chunk, byte instruction)
{
    size_t i = chunk->count;
    nob_da_append(chunk, instruction);
    return i;
}
byte chunk_pop(Chunk *chunk)
{
    return nob_da_pop(chunk);
}
byte chunk_instruction_at(Chunk *chunk, size_t offset)
{
    return array_at(chunk, offset);
}
void chunk_print_opcode(byte instruction, FILE *stream)
{
    fprintf(stream, "%s  ", chunk_byte_to_str(instruction));
}
char *chunk_operand_type_to_str(OperandType type)
{
    switch (type)
    {
    case OPERAND_IMMEDIATE:
        return "%";
    case OPERAND_MEMORY:
        return "#";
    default:
        NOB_UNREACHABLE("chunk_operand_type_to_str");
    }
}
void chunk_print_operand(OperandType type, byte index, FILE *stream)
{
    fprintf(stream, "%s%u", chunk_operand_type_to_str(type), index);
}
size_t chunk_print_instruction(Chunk *chunk, size_t offset, FILE *stream)
{
    fprintf(stream, "    &%04zu  ", offset);
    byte instruction = chunk_instruction_at(chunk, offset);
    chunk_print_opcode(instruction, stream);
    switch (instruction)
    {
    case OP_GET_LOCAL:
    {
        chunk_print_operand(OPERAND_IMMEDIATE, chunk_instruction_at(chunk, ++offset), stream);
        chunk_print_operand(OPERAND_MEMORY, chunk_instruction_at(chunk, ++offset), stream);
        break;
    }
    case OP_LOOP:
    case OP_JMP:
    case OP_JMP_IF_FALSE:
    {
        byte high_byte = chunk_instruction_at(chunk, ++offset);
        byte low_byte = chunk_instruction_at(chunk, ++offset);
        uint16_t jmpOffset = ((uint16_t)((high_byte << 8) | low_byte));
        fprintf(stream, "&%04u", jmpOffset);
        break;
    }
    case OP_CALL:
    case OP_SET_LOCAL:
    case OP_SET_GLOBAL:
    case OP_GET_GLOBAL:
    case OP_DEFINE_GLOBAL:
    case OP_CONSTANT:
    {
        chunk_print_operand(OPERAND_MEMORY, chunk_instruction_at(chunk, ++offset), stream);
        break;
    }
    default:
        break;
    }
    fprintf(stream, "\n");
    return ++offset;
}

void chunk_dump(Chunk *chunk, FILE *stream)
{
    for (size_t offset = 0; offset < (chunk->count);)
    {
        offset = chunk_print_instruction(chunk, offset, stream);
    }
}

void chunk_free(Chunk *chunk)
{
    free(chunk->items);
    free(chunk);
}