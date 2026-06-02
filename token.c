Token init_token(char *start, TokenType type, size_t length, size_t row, size_t col)
{
    Token token = {
        .start = start,
        .type = type,
        .row = row,
        .col = col,
        .length = length,
    };
    return token;
}
const char *token_type_str(TokenType type)
{
    switch (type)
    {
    case TOKEN_ID:
        return "TOKEN_ID";
    case TOKEN_NUMBER:
        return "TOKEN_NUMBER";
    case TOKEN_PLUS:
        return "TOKEN_PLUS";
    case TOKEN_MINUS:
        return "TOKEN_MINUS";
    case TOKEN_MUL:
        return "TOKEN_MUL";
    case TOKEN_DIV:
        return "TOKEN_DIV";
    case TOKEN_EOF:
        return "TOKEN_EOF";
    case TOKEN_SEMICOLON:
        return "TOKEN_SEMICOLON";
    case TOKEN_IF:
        return "TOKEN_IF";
    case TOKEN_ELSE:
        return "TOKEN_ELSE";
    case TOKEN_FUNCTION:
        return "TOKEN_FUNCTION";
    case TOKEN_FOR:
        return "TOKEN_FOR";
    case TOKEN_WHILE:
        return "TOKEN_WHILE";
    case TOKEN_PRINT:
        return "TOKEN_PRINT";
    case TOKEN_RETURN:
        return "TOKEN_RETURN";
    case TOKEN_VAR:
        return "TOKEN_VAR";
    case TOKEN_LPAREN:
        return "TOKEN_LPAREN";
    case TOKEN_RPAREN:
        return "TOKEN_RPAREN";
    case TOKEN_ERROR:
        return "TOKEN_ERROR";
    case TOKEN_COMMA:
        return "TOKEN_COMMA";
    case TOKEN_NULL:
        return "TOKEN_NULL";
    case TOKEN_FALSE:
        return "TOKEN_FALSE";
    case TOKEN_TRUE:
        return "TOKEN_TRUE";
    case TOKEN_STRING:
        return "TOKEN_STRING";
    case TOKEN_ASSIGNMENT:
        return "TOKEN_ASSIGNMENT";
    case TOKEN_NOT_EQUALS:
        return "TOKEN_NOT_EQUALS";
    case TOKEN_NOT:
        return "TOKEN_NOT";
    case TOKEN_MOD:
        return "TOKEN_MOD";
    case TOKEN_RIGHT_SHIFT:
        return "TOKEN_RIGHT_SHIFT";
    case TOKEN_LEFT_SHIFT:
        return "TOKEN_LEFT_SHIFT";
    case TOKEN_LTE:
        return "TOKEN_LTE";
    case TOKEN_LT:
        return "TOKEN_LT";
    case TOKEN_GTE:
        return "TOKEN_GTE";
    case TOKEN_GT:
        return "TOKEN_GT";
    case TOKEN_AND:
        return "TOKEN_AND";
    case TOKEN_BITWISE_AND:
        return "TOKEN_BITWISE_AND";
    case TOKEN_OR:
        return "TOKEN_OR";
    case TOKEN_BITWISE_OR:
        return "TOKEN_BITWISE_OR";
    case TOKEN_INCREMENT:
        return "TOKEN_INCREMENT";
    case TOKEN_DECREMENT:
        return "TOKEN_DECREMENT";
    case TOKEN_BITWISE_NOT:
        return "TOKEN_BITWISE_NOT";
    case TOKEN_LCURLY:
        return "TOKEN_LCURLY";
    case TOKEN_RCURLY:
        return "TOKEN_RCURLY";
    case TOKEN_DOT:
        return "TOKEN_DOT";
    default:
        return "UNKNOWN";
    }
}
char *token_to_str(Token token)
{
    const char *token_type = token_type_str(token.type);
    return arena_sprintf(&default_arena, 
        "<type='%s' value='%.*s' row='%d' col='%zu'>",
        token_type, token.length, token.start, token.row, token.col);
}

void token_print(Token token)
{
    printf("[Token] %s\n", token_to_str(token));
}
char *token_text(Token token)
{
    if (token.type == TOKEN_EOF)
    {
        return arena_sprintf(&default_arena, "end of file");
    }
    return arena_sprintf(&default_arena, "%.*s", (int)token.length, token.start);
}

bool token_equal(Token t1, Token t2)
{
    return t1.type == t2.type &&
           t1.length == t2.length &&
           strncmp(t1.start, t2.start, t2.length) == 0;
}