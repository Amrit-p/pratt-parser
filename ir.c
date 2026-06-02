void ast_to_byte(AST *ast, Compiler *compiler);
void ast_patch_jump(Compiler *compiler, size_t offset);
size_t ast_emit_jump(Compiler *compiler, OpCode instruction);

char *ast_stmt_type_to_str(StatmentType type)
{
    switch (type)
    {
    case STMT_COMPOUND:
        return "STMT_COMPOUND";
    case STMT_FOR:
        return "STMT_FOR";
    case STMT_IF:
        return "STMT_IF";
    case STMT_WHILE:
        return "STMT_WHILE";
    case STMT_PRINT:
        return "STMT_PRINT";
    case STMT_RETURN:
        return "STMT_RETURN";
    default:
        return "UNKNOWN_STMT";
    }
}
void ast_stmt_to_byte(AST *ast, Compiler *compiler)
{
    if (!ast)
        return;
    StatmentType stmt_type = ast->as.statment.type;
    switch (stmt_type)
    {
    case STMT_COMPOUND:
    {
        StatmentCompound stmts = ast->as.statment.as.compound;
        for (size_t i = 0; i < (stmts.count); i++)
        {
            AST *stmt = array_at(&stmts, i);
            ast_to_byte(stmt, compiler);
        }
    }
    break;
    case STMT_PRINT:
    {
        StatmentPrint print_stmt = ast->as.statment.as.print;
        ast_to_byte(print_stmt.expr, compiler);
        chunk_push(compiler->function->chunk, OP_PRINT);
    }
    break;
    default:
    {
        char *type = ast_stmt_type_to_str(stmt_type);
        NOB_TODO(type);
    }
    }
}
void ast_binary_to_byte(AST *ast, Compiler *compiler)
{
    BinaryExpr expr = ast->as.binaryExpr;
    if (expr.type != BINARY_ASSIGNMENT &&
        expr.type != BINARY_AND &&
        expr.type != BINARY_OR)
    {
        ast_to_byte(expr.left, compiler);
        ast_to_byte(expr.right, compiler);
    }
    switch (expr.type)
    {
    case BINARY_PLUS:
        chunk_push(compiler->function->chunk, OP_ADD);
        break;
    case BINARY_MULTIPLY:
        chunk_push(compiler->function->chunk, OP_MULTIPLY);
        break;
    case BINARY_DIVIDE:
        chunk_push(compiler->function->chunk, OP_DIVIDE);
        break;
    case BINARY_MINUS:
        chunk_push(compiler->function->chunk, OP_SUBTRACT);
        break;
    case BINARY_EQUALS:
        chunk_push(compiler->function->chunk, OP_EQUAL);
        break;
    case BINARY_GT:
        chunk_push(compiler->function->chunk, OP_GT);
        break;
    case BINARY_GTE:
        chunk_push(compiler->function->chunk, OP_GTE);
        break;
    case BINARY_LT:
        chunk_push(compiler->function->chunk, OP_LT);
        break;
    case BINARY_LTE:
        chunk_push(compiler->function->chunk, OP_LTE);
        break;
    case BINARY_MOD:
        chunk_push(compiler->function->chunk, OP_MOD);
        break;
    case BINARY_NOT_EQUALS:
        chunk_push(compiler->function->chunk, OP_NOT_EQUAL);
        break;
    case BINARY_RSH:
        chunk_push(compiler->function->chunk, OP_RIGHT_SHIFT);
        break;
    case BINARY_LSH:
        chunk_push(compiler->function->chunk, OP_LEFT_SHIFT);
        break;
    case BINARY_BITWISE_AND:
        chunk_push(compiler->function->chunk, OP_BITWISE_AND);
        break;
    case BINARY_BITWISE_OR:
        chunk_push(compiler->function->chunk, OP_BITWISE_OR);
        break;
    case BINARY_BITWISE_XOR:
        chunk_push(compiler->function->chunk, OP_BITWISE_OR);
        break;
    case BINARY_AND:
        ast_to_byte(expr.left, compiler);
        size_t offset = ast_emit_jump(compiler, OP_JMP_IF_FALSE);
        ast_to_byte(expr.right, compiler);
        ast_patch_jump(compiler, offset);
        break;
    case BINARY_OR:
        ast_to_byte(expr.left, compiler);
        size_t else_offset = ast_emit_jump(compiler, OP_JMP_IF_FALSE);
        size_t end_offset = ast_emit_jump(compiler, OP_JMP);
        ast_patch_jump(compiler, else_offset);
        ast_to_byte(expr.right, compiler);
        ast_patch_jump(compiler, end_offset);
        break;
    default:
        NOB_TODO(ast_type_to_str(ast->type));
        break;
    }
}
void ast_to_byte(AST *ast, Compiler *compiler)
{
    if (!ast)
        return;
    switch (ast->type)
    {
    case AST_STMT:
        ast_stmt_to_byte(ast, compiler);
        break;
    case AST_NUMBER:
    {
        Value number = NUMBER_VAL(ast->as.number, ast->token.row, ast->token.col);
        Chunk *chunk = compiler->function->chunk;
        Values *values = compiler->function->values;
        chunk_push(chunk, OP_CONSTANT);
        chunk_push(chunk, value_push(values, number));
    }
    break;
    case AST_STRING:
    case AST_UNARY:
    case AST_BINARY:
    {
        ast_binary_to_byte(ast, compiler);
        break;
    }
    case AST_TERNARY:
    case AST_SEQUENCE:
    case AST_FUNCTION_CALL:
    case AST_POSTFIX:
    case AST_ID:
    case AST_FALSE:
    case AST_TRUE:
    case AST_NULL:
    {
        char *type = ast_type_to_str(ast->type);
        NOB_TODO(type);
    }
    default:
    {
        char *type = ast_type_to_str(ast->type);
        NOB_TODO(type);
    }
    }
}

size_t ast_emit_jump(Compiler *compiler, OpCode instruction)
{
    chunk_push(compiler->function->chunk, instruction);
    chunk_push(compiler->function->chunk, 0xff);
    chunk_push(compiler->function->chunk, 0xff);
    return (compiler->function->chunk->count - 2);
}
void ast_emit_loop(Compiler *compiler, size_t loop_start)
{
    chunk_push(compiler->function->chunk, OP_LOOP);
    size_t offset = (compiler->function->chunk->count - loop_start + 2);
    if (offset > UINT16_MAX)
    {
        compiler->had_error = true;
        fprintf(stderr, "Loop body too large.\n");
    }
    chunk_push(compiler->function->chunk, (offset >> 8) & 0xff);
    chunk_push(compiler->function->chunk, offset & 0xff);
}
void ast_patch_jump(Compiler *compiler, size_t offset)
{
    size_t jmp = (compiler->function->chunk->count);
    if (jmp > UINT16_MAX)
    {
        compiler->had_error = true;
        fprintf(stderr, "Too much code to jump over.\n");
    }
    array_at(compiler->function->chunk, offset) = (jmp >> 8) & 0xff;
    array_at(compiler->function->chunk, offset + 1) = jmp & 0xff;
}
void ast_patch_jump_with(Compiler *compiler, size_t offset, size_t with)
{
    if (with > UINT16_MAX)
    {
        compiler->had_error = true;
        fprintf(stderr, "Too much code to jump over.\n");
    }
    array_at(compiler->function->chunk, offset) = (with >> 8) & 0xff;
    array_at(compiler->function->chunk, offset + 1) = with & 0xff;
}
void ast_begin_scope(Compiler *compiler)
{
    compiler->scope_depth++;
}
void ast_end_scope(Compiler *compiler)
{
    compiler->scope_depth--;
    while (compiler->local_count > 0 && compiler->locals[compiler->local_count - 1].depth > compiler->scope_depth)
    {
        chunk_push(compiler->function->chunk, OP_POP);
        compiler->local_count--;
    }
}
void ast_resolve_breaks(Compiler *compiler)
{
    for (byte i = 0; i < compiler->break_stack_count; i++)
    {
        byte offset = compiler->break_stack[i];
        ast_patch_jump(compiler, offset);
    }
    compiler->break_stack_count = 0;
}
void ast_resolve_continues(Compiler *compiler, size_t loop_start)
{
    for (byte i = 0; i < compiler->continue_stack_count; i++)
    {
        byte offset = compiler->continue_stack[i];
        ast_patch_jump_with(compiler, offset, loop_start);
    }
    compiler->continue_stack_count = 0;
}

void ast_unary_to_byte(AST *ast, Compiler *compiler)
{
    ast_to_byte(ast->as.unaryExpr.value, compiler);
    switch (ast->token.type)
    {
    case TOKEN_MINUS:
        chunk_push(compiler->function->chunk, OP_NEGATE);
        break;
    case TOKEN_NOT:
        chunk_push(compiler->function->chunk, OP_NOT);
        break;
    case TOKEN_BITWISE_NOT:
        chunk_push(compiler->function->chunk, OP_BITWISE_NOT);
        break;
    case TOKEN_DECREMENT:
    case TOKEN_INCREMENT:
    {
        NOB_TODO("TOKEN_INCREMENT and TOKEN_DECREMENT");
        break;
    }
    default:
        NOB_UNREACHABLE("ast_unary_to_byte");
    }
}

Compiler *ir_gen(AST *ast)
{
    Compiler *compiler = arena_alloc(&default_arena, sizeof(*compiler));
    init_compiler(compiler, TYPE_SCRIPT);
    ast_to_byte(ast, compiler);
    chunk_push(compiler->function->chunk, OP_RETURN);
    return compiler;
}