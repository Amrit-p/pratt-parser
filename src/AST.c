AST *init_ast(AST_Type type)
{
    AST *ast = arena_alloc(&default_arena, sizeof(*ast));
    ast->type = type;
    return ast;
}
AST *init_primary_ast(AST_Type type, char *string)
{
    AST *ast = init_ast(type);
    ast->as.string = string;
    return ast;
}
AST *init_postfix_ast(TokenType type)
{
    AST *postfix = init_ast(AST_POSTFIX);
    switch (type)
    {
    case TOKEN_INCREMENT:
        postfix->as.postfixExpr.type = POSTFIX_INCR;
        break;
    case TOKEN_DECREMENT:
        postfix->as.postfixExpr.type = POSTFIX_DECR;
        break;
    default:
    {
        fprintf(stderr, "[ERROR] invalid postfix operator\n");
        exit(1);
    }
    }
    return postfix;
}
AST *init_unary_ast(TokenType type)
{
    AST *unary = init_ast(AST_UNARY);
    switch (type)
    {
    case TOKEN_MINUS:
        unary->as.unaryExpr.type = UNARY_MINUS;
        break;
    case TOKEN_NOT:
        unary->as.unaryExpr.type = UNARY_NOT;
        break;
    case TOKEN_INCREMENT:
        unary->as.unaryExpr.type = UNARY_INCREMENT;
        break;
    case TOKEN_DECREMENT:
        unary->as.unaryExpr.type = UNARY_DECREMENT;
        break;
    default:
    {
        fprintf(stderr, "[ERROR] invalid unary operator\n");
        exit(1);
    }
    }
    return unary;
}
AST *init_binary_ast(TokenType type)
{
    AST *bin = init_ast(AST_BINARY);
    switch (type)
    {
    case TOKEN_PLUS:
        bin->as.binaryExpr.type = BINARY_PLUS;
        break;
    case TOKEN_MINUS:
        bin->as.binaryExpr.type = BINARY_MINUS;
        break;
    case TOKEN_MUL:
        bin->as.binaryExpr.type = BINARY_MULTIPLY;
        break;
    case TOKEN_DIV:
        bin->as.binaryExpr.type = BINARY_DIVIDE;
        break;
    case TOKEN_MOD:
        bin->as.binaryExpr.type = BINARY_MOD;
        break;
    case TOKEN_OR:
        bin->as.binaryExpr.type = BINARY_OR;
        break;
    case TOKEN_AND:
        bin->as.binaryExpr.type = BINARY_AND;
        break;
    case TOKEN_BITWISE_AND:
        bin->as.binaryExpr.type = BINARY_BITWISE_AND;
        break;
    case TOKEN_BITWISE_OR:
        bin->as.binaryExpr.type = BINARY_BITWISE_OR;
        break;
    case TOKEN_BITWISE_XOR:
        bin->as.binaryExpr.type = BINARY_BITWISE_XOR;
        break;
    case TOKEN_LTE:
        bin->as.binaryExpr.type = BINARY_LTE;
        break;
    case TOKEN_LT:
        bin->as.binaryExpr.type = BINARY_LT;
        break;
    case TOKEN_GTE:
        bin->as.binaryExpr.type = BINARY_GTE;
        break;
    case TOKEN_LEFT_SHIFT:
        bin->as.binaryExpr.type = BINARY_LSH;
        break;
    case TOKEN_RIGHT_SHIFT:
        bin->as.binaryExpr.type = BINARY_RSH;
        break;
    case TOKEN_EQUALS:
        bin->as.binaryExpr.type = BINARY_EQUALS;
        break;
    case TOKEN_NOT_EQUALS:
        bin->as.binaryExpr.type = BINARY_NOT_EQUALS;
        break;
    case TOKEN_ASSIGNMENT:
        bin->as.binaryExpr.type = BINARY_ASSIGNMENT;
        break;
    default:
    {
        fprintf(stderr, "[ERROR] invalid binary operator\n");
        exit(1);
    }
    }
    return bin;
}
AST *init_print_stmt()
{
    AST *ast = init_ast(AST_STMT);
    ast->as.statment.type = STMT_PRINT;
    return ast;
}
AST *init_compound_ast()
{
    AST *ast = init_ast(AST_STMT);
    ast->as.statment.type = STMT_COMPOUND;
    return ast;
}
AST *init_if_ast()
{
    AST *ast = init_ast(AST_STMT);
    ast->as.statment.type = STMT_IF;
    return ast;
}
AST *init_stmt_ast(StatmentType type)
{
    switch (type)
    {
    case STMT_PRINT:
        return init_print_stmt();
    case STMT_COMPOUND:
        return init_compound_ast();
    case STMT_IF:
        return init_if_ast();
    default:
    {
        fprintf(stderr, "[ERROR] invalid statmentType at %s.\n", __func__);
        exit(1);
    }
    }
}

size_t ast_push(AST *ast, AST *child)
{
    if (ast->type == AST_SEQUENCE)
    {
        size_t i = ast->as.sequenceExpr.count;
        nob_da_append(&ast->as.sequenceExpr, child);
        return i;
    }
    if (ast->type == AST_STMT)
    {
        switch (ast->as.statment.type)
        {
        case STMT_COMPOUND:
        {
            size_t i = ast->as.statment.as.compound.count;
            nob_da_append(&ast->as.statment.as.compound, child);
            return i;
        }
        default:
            break;
        }
    }
    fprintf(stderr, "[ERROR] unknown statementType in %s\n", __func__);
    exit(1);
}

char *ast_type_to_str(AST_Type type)
{
    switch (type)
    {
    case AST_NUMBER:
        return "NUMBER";
    case AST_BINARY:
        return "BINARYEXPR";
    case AST_UNARY:
        return "UNARYEXPR";
    case AST_STMT:
        return "STATEMENT";
    case AST_TERNARY:
        return "TERNARY";
    case AST_NULL:
        return "NULL";
    case AST_FALSE:
        return "FALSE";
    case AST_TRUE:
        return "TRUE";
    case AST_ID:
        return "ID";
    case AST_STRING:
        return "STRING";
    case AST_POSTFIX:
        return "POSTFIX";
    case AST_FUNCTION_CALL:
        return "FUNCTION_CALL";
    case AST_SEQUENCE:
        return "SEQUENCE";
    default:
        return "UNKNOWN";
    }
}