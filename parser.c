#include "lexer.h"
#include "parser.h"
#include "token.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define parser_error_prefix()                                            \
    do                                                                   \
    {                                                                    \
        fprintf(stderr, "ParserError at %s:", parser->lexer->file_path); \
    } while (0)

#define parser_token_location(token)                           \
    do                                                         \
    {                                                          \
        fprintf(stderr, "%zu:%zu ", (token).row, (token).col); \
    } while (0)
#define parser_token_error(token, message) \
    do                                     \
    {                                      \
        parser->had_error = 1;             \
        parser_error_prefix();             \
        parser_token_location(token);      \
        fprintf(stderr, "%s\n", message);  \
    } while (0)

char *parser_unexpected_token(Token token, char *message)
{
    char *template = "unexpected '%s'";
    size_t message_len = 0;
    if (message)
    {
        message_len = strlen(message) + 2;
    }
    char *token_value = token_text(token);
    char *buffer = calloc(strlen(template) + strlen(token_value) + message_len + 1, sizeof(char));
    sprintf(buffer, template, token_value);
    if (message)
    {
        strcat(buffer, ", ");
        strcat(buffer, message);
    }

    free(token_value);
    return buffer;
}

static ParseRule rules[] = {
    [TOKEN_LPAREN] = {parser_parse_group, parser_parse_call, PREC_POSTIFX},
    [TOKEN_RPAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] = {parser_parse_prefix, parser_parse_infix, PREC_TERM},
    [TOKEN_PLUS] = {NULL, parser_parse_infix, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_DIV] = {NULL, parser_parse_infix, PREC_FACTOR},
    [TOKEN_MUL] = {NULL, parser_parse_infix, PREC_FACTOR},
    [TOKEN_NUMBER] = {parser_parse_number, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, parser_parse_comma, PREC_COMMA},
    [TOKEN_QUESTION] = {NULL, parser_parse_ternary, PREC_TERNARY},
    [TOKEN_TRUE] = {parser_parse_primary, NULL, PREC_NONE},
    [TOKEN_FALSE] = {parser_parse_primary, NULL, PREC_NONE},
    [TOKEN_NULL] = {parser_parse_primary, NULL, PREC_NONE},
    [TOKEN_ID] = {parser_parse_primary, NULL, PREC_NONE},
    [TOKEN_STRING] = {parser_parse_string, NULL, PREC_NONE},
    [TOKEN_EQUALS] = {NULL, parser_parse_infix, PREC_EQUALITY},
    [TOKEN_ASSIGNMENT] = {NULL, parser_parse_infix, PREC_ASSIGNMENT},
    [TOKEN_NOT] = {parser_parse_prefix, NULL, PREC_UNARY},
    [TOKEN_NOT_EQUALS] = {NULL, parser_parse_infix, PREC_EQUALITY},
    [TOKEN_MOD] = {NULL, parser_parse_infix, PREC_FACTOR},
    [TOKEN_RIGHT_SHIFT] = {NULL, parser_parse_infix, PREC_SHIFT},
    [TOKEN_LEFT_SHIFT] = {NULL, parser_parse_infix, PREC_SHIFT},
    [TOKEN_AND] = {NULL, parser_parse_infix, PREC_LOGICAL_AND},
    [TOKEN_BITWISE_AND] = {NULL, parser_parse_infix, PREC_BITWISE_AND},
    [TOKEN_OR] = {NULL, parser_parse_infix, PREC_LOGICAL_OR},
    [TOKEN_BITWISE_OR] = {NULL, parser_parse_infix, PREC_BITWISE_OR},
    [TOKEN_LTE] = {NULL, parser_parse_infix, PREC_COMPARISON},
    [TOKEN_LT] = {NULL, parser_parse_infix, PREC_COMPARISON},
    [TOKEN_GTE] = {NULL, parser_parse_infix, PREC_COMPARISON},
    [TOKEN_GT] = {NULL, parser_parse_infix, PREC_COMPARISON},
    [TOKEN_BITWISE_NOT] = {parser_parse_prefix, NULL, PREC_UNARY},
    [TOKEN_INCREMENT] = {parser_parse_prefix, parser_parse_postfix, PREC_UNARY},
    [TOKEN_DECREMENT] = {parser_parse_prefix, parser_parse_postfix, PREC_UNARY},
    [TOKEN_LCURLY] = {NULL, NULL, PREC_NONE},
    [TOKEN_RCURLY] = {NULL, NULL, PREC_NONE},
};
Parser *init_parser(Lexer *lexer)
{
    Parser *parser = calloc(1, sizeof(Parser));
    parser->current_token = lexer_next_token(lexer);
    parser->had_error = 0;
    parser->panic_mode = 0;
    parser->lexer = lexer;
    parser->parsing_call = 0;
    return parser;
}
char *parser_prec_to_str(Precedence prec)
{
    switch (prec)
    {
    case PREC_NONE:
        return "PREC_NONE";
    case PREC_ASSIGNMENT:
        return "PREC_ASSIGNMENT";
    case PREC_BITWISE_AND:
        return "PREC_BITWISE_AND";
    case PREC_BITWISE_OR:
        return "PREC_BITWISE_XOR";
    case PREC_POSTIFX:
        return "PREC_POSTIFX";
    case PREC_UNARY:
        return "PREC_UNARY";
    case PREC_TERNARY:
        return "PREC_TERNNARY";
    case PREC_COMMA:
        return "PREC_COMMA";
    case PREC_COMPARISON:
        return "PREC_COMPARISON";
    case PREC_EQUALITY:
        return "PREC_EQUALITY";
    case PREC_FACTOR:
        return "PREC_FACTOR";
    case PREC_TERM:
        return "PREC_TERM";
    case PREC_LOGICAL_AND:
        return "PREC_LOGICAL_AND";
    case PREC_LOGICAL_OR:
        return "PREC_LOGICAL_OR";
    case PREC_SHIFT:
        return "PREC_SHIFT";
    default:
        return "PREC_UNKNOWN";
    }
}
Token parser_advance(Parser *parser)
{
    parser->current_token = lexer_next_token(parser->lexer);
    return parser->current_token;
}
Token parser_eat(Parser *parser, TokenType type, char *message)
{
    if ((type != parser->current_token.type || type == TOKEN_ERROR) && parser->panic_mode == 0)
    {
        parser_error(parser, parser_unexpected_token(parser->current_token, message));
        return parser->current_token;
    }
    else
    {
        parser->panic_mode = 0;
        Token token = parser->current_token;
        parser_advance(parser);
        return token;
    }
}
ParseRule *parser_production(TokenType type)
{
    return &rules[type];
}
void parser_current_token(Parser *parser)
{
    printf("current token %s\n", token_to_str(parser->current_token));
}
void parser_error(Parser *parser, char *message)
{
    if (parser->panic_mode)
        return;

    char *_message = message;
    if (parser->current_token.message)
    {
        _message = parser->current_token.message;
    }
    parser->had_error = 1;
    parser->panic_mode = 1;

    parser_error_prefix();
    parser_token_location(parser->current_token);
    fprintf(stderr, "%s\n", _message);
}
AST *parser_parse_string(Parser *parser)
{
    AST *string = init_primary_ast(AST_STRING, token_text(parser->current_token));
    string->token = parser->current_token;
    parser_eat(parser, TOKEN_STRING, 0);
    return string;
}
AST *parser_parse_primary(Parser *parser)
{
    switch (parser->current_token.type)
    {
    case TOKEN_TRUE:
    {

        AST *ast_true = init_primary_ast(AST_TRUE, token_text(parser->current_token));
        ast_true->token = parser->current_token;
        parser_eat(parser, TOKEN_TRUE, 0);
        return ast_true;
    }
    case TOKEN_FALSE:
    {
        AST *ast_false = init_primary_ast(AST_FALSE, token_text(parser->current_token));
        ast_false->token = parser->current_token;
        parser_eat(parser, TOKEN_FALSE, 0);
        return ast_false;
    }
    case TOKEN_NULL:
    {

        AST *ast_null = init_primary_ast(AST_NULL, token_text(parser->current_token));
        ast_null->token = parser->current_token;
        parser_eat(parser, TOKEN_NULL, 0);
        return ast_null;
    }
    default:
    {
        AST *primary = init_primary_ast(AST_ID, token_text(parser->current_token));
        primary->token = parser->current_token;
        parser_eat(parser, TOKEN_ID, 0);
        return primary;
    }
    }
}
AST *parser_parse_precendence(Parser *parser, Precedence precedence)
{
    ParsePrefixFn prefix_handler = parser_production(parser->current_token.type)->prefix;
    if (prefix_handler == NULL)
    {
        parser_error(parser, parser_unexpected_token(parser->current_token, "expected an expr."));
        if (parser->current_token.type == TOKEN_EOF)
            return NULL;
        parser_advance(parser);
        return NULL;
    }
    AST *prefix = prefix_handler(parser);
    while (precedence <= parser_production(parser->current_token.type)->precedence)
    {
        ParseInfixFn infix_handler = parser_production(parser->current_token.type)->infix;
        if (infix_handler == NULL)
        {
            printf("infix_handler is null for token %s\n", token_to_str(parser->current_token));
            return prefix;
        }
        else
        {
            prefix = infix_handler(parser, prefix);
        }
    }

    return prefix;
}
AST *parser_parse_number(Parser *parser)
{
    AST *number = init_ast(AST_NUMBER);
    number->token = parser->current_token;
    char buffer[parser->current_token.length + 1];
    sprintf(buffer, "%.*s", (int)parser->current_token.length, parser->current_token.start);
    parser_eat(parser, TOKEN_NUMBER, 0);
    number->as.number = atof(buffer);
    return number;
}

AST *parser_parse_group(Parser *parser)
{
    parser_eat(parser, TOKEN_LPAREN, "expected '('.");
    if (parser->current_token.type == TOKEN_RPAREN && parser->parsing_call == 1)
    {
        parser_eat(parser, TOKEN_RPAREN, 0);
        return NULL;
    }
    AST *group = parser_parse_expr(parser);
    if (group && parser->current_token.type != TOKEN_RPAREN && parser->current_token.type != TOKEN_EOF)
    {
        parser_error(parser, parser_unexpected_token(parser->current_token, "expected ',' or ')' after expression."));

        while (parser->current_token.type != TOKEN_RPAREN)
        {
            parser_advance(parser);
            if (parser->current_token.type == TOKEN_EOF)
                break;
        }
    }
    if (group)
        parser_eat(parser, TOKEN_RPAREN, "expected ')'.");
    return group;
}
AST *parser_parse_call(Parser *parser, AST *callee)
{
    if (callee->type != AST_ID)
    {
        parser_token_error(callee->token, parser_unexpected_token(callee->token, "callee should be a identifier."));
    }
    AST *call = init_ast(AST_FUNCTION_CALL);
    call->as.functionCall.callee = callee;
    parser->parsing_call = 1;
    call->as.functionCall.args = parser_parse_group(parser);
    parser->parsing_call = 0;
    return call;
}
AST *parser_parse_prefix(Parser *parser)
{
    AST *unary = init_unary_ast(parser->current_token.type);
    unary->token = parser->current_token;
    Token token = parser_eat(parser, parser->current_token.type, 0);
    AST *operand = parser_parse_precendence(parser, PREC_UNARY);
    if ((token.type == TOKEN_INCREMENT || token.type == TOKEN_DECREMENT) && operand && operand->type != AST_ID)
    {
        parser_token_error(operand->token, "invalid expr in prefix operation.");
    }
    if (!operand)
        return NULL;
    unary->as.unaryExpr.value = operand;
    return unary;
}
AST *parser_parse_comma(Parser *parser, AST *prefix)
{
    AST *exprs = init_ast(AST_SEQUENCE);
    ast_push(exprs, prefix);
    while (parser->current_token.type == TOKEN_COMMA)
    {
        parser_eat(parser, TOKEN_COMMA, 0);
        AST *child = parser_parse_expr(parser);
        if (child == NULL)
        {
            parser->panic_mode = 1;
            return NULL;
        }
        ast_push(exprs, child);
    }
    return exprs;
}
AST *parser_parse_infix(Parser *parser, AST *prefix)
{
    AST *bin = init_binary_ast(parser->current_token.type);
    bin->token = parser->current_token;
    Token token = parser_eat(parser, parser->current_token.type, 0);

    if (token.type == TOKEN_ASSIGNMENT && prefix && prefix->type != AST_ID && prefix->token.type != TOKEN_ASSIGNMENT)
    {
        parser_token_error(prefix->token, "lvalue cannot be a constant.");
        return NULL;
    }

    ParseRule *rule = parser_production(token.type);
    Precedence precedence = token.type == TOKEN_ASSIGNMENT ? PREC_ASSIGNMENT : rule->precedence + 1;
    AST *right = parser_parse_precendence(parser, precedence);

    if (right == NULL)
        return NULL;
    bin->as.binaryExpr.left = prefix;
    bin->as.binaryExpr.right = right;

    return bin;
}
AST *parser_parse_ternary(Parser *parser, AST *condition)
{
    AST *ternary = init_ast(AST_TERNARY);
    ternary->token = parser->current_token;
    ternary->as.ternaryExpr.condition = condition;
    TokenType operatorType = parser->current_token.type;
    parser_eat(parser, operatorType, 0);
    AST *then = parser_parse_expr(parser);
    if (parser->current_token.type != TOKEN_COLON)
    {
        parser_error(parser, "expect ':' after expr.");
        return NULL;
    }
    then->token = parser->current_token;
    parser_eat(parser, TOKEN_COLON, 0);
    AST *other_wise = parser_parse_expr(parser);
    if (!other_wise)
        return NULL;
    ternary->as.ternaryExpr.then = then;
    ternary->as.ternaryExpr.other_wise = other_wise;
    return ternary;
}
AST *parser_parse_postfix(Parser *parser, AST *oprand)
{
    AST *postfix = init_postfix_ast(parser->current_token.type);
    postfix->token = parser->current_token;
    postfix->as.postfixExpr.operand = oprand;
    parser_eat(parser, parser->current_token.type, 0);
    return postfix;
}
void parser_state(Parser *parser)
{
    parser_current_token(parser);
    printf("had_error %d\npanic_mode %d\n", parser->had_error, parser->panic_mode);
}
AST *parser_parse_expr(Parser *parser)
{
    AST *expr = parser_parse_precendence(parser, PREC_COMMA);
    return expr;
}
AST *parser_parse_print(Parser *parser)
{
    AST *print = init_stmt_ast(STMT_PRINT);
    parser_eat(parser, TOKEN_PRINT, 0);
    AST *expr = parser_parse_expr(parser);
    if (!expr)
        return NULL;
    print->as.statment.as.print.expr = expr;
    return print;
}
AST *parser_parse_stmt(Parser *parser)
{
    TokenType token_type = parser->current_token.type;
    AST *stmt = NULL;
    switch (token_type)
    {
    case TOKEN_LCURLY:
    {
        parser_eat(parser, TOKEN_LCURLY, 0);
        if (parser->current_token.type == TOKEN_RCURLY)
        {
            parser_eat(parser, TOKEN_RCURLY, 0);
            return init_stmt_ast(STMT_COMPOUND);
        }
        stmt = parser_parse_compound(parser);
        parser_eat(parser, TOKEN_RCURLY, "expected '}'");
        return stmt;
    }
    case TOKEN_PRINT:
        stmt = parser_parse_print(parser);
        break;
    case TOKEN_RETURN:
        UNIMPLEMENTED;
        break;
    default:
        stmt = parser_parse_expr(parser);
    }
    if (stmt)
        parser_eat(parser, TOKEN_SEMICOLON, "statement should be ended with semicolon.");
    return stmt;
}
AST *parser_parse_if(Parser *parser)
{
    parser_eat(parser, TOKEN_IF, 0);
    AST *_if = init_stmt_ast(STMT_IF);
    _if->as.statment.as._if.condition = parser_parse_group(parser);

    if (parser->current_token.type == TOKEN_SEMICOLON)
        parser_advance(parser);
    else
    {
        _if->as.statment.as._if.then = parser_parse_decl(parser);
        if (parser->current_token.type == TOKEN_ELSE)
        {
            parser_eat(parser, TOKEN_ELSE, 0);
            _if->as.statment.as._if.other_wise = parser_parse_decl(parser);
        }
    }

    return _if;
}
AST *parser_parse_decl(Parser *parser)
{
    TokenType token_type = parser->current_token.type;
    switch (token_type)
    {
    case TOKEN_VAR:
        token_print(parser->current_token);
        UNIMPLEMENTED;
    case TOKEN_FUNCTION:
        token_print(parser->current_token);
        UNIMPLEMENTED;
    case TOKEN_FOR:
        token_print(parser->current_token);
        UNIMPLEMENTED;
    case TOKEN_WHILE:
        token_print(parser->current_token);
        UNIMPLEMENTED;
    case TOKEN_IF:
        return parser_parse_if(parser);
    case TOKEN_ELSE:
    {
        parser_error(parser, "cannot have 'else' without 'if'.");
        parser_eat(parser, TOKEN_ELSE, 0);
        return NULL;
    }
    default:
        return parser_parse_stmt(parser);
    }
}
AST *parser_parse_compound(Parser *parser)
{
    AST *compound = init_stmt_ast(STMT_COMPOUND);
    while (parser->current_token.type != TOKEN_EOF)
    {
        AST *child = parser_parse_decl(parser);
        if (child)
            ast_push(compound, child);
        if (parser->current_token.type == TOKEN_RCURLY)
            break;
    }
    return compound;
}
AST *parser_parse(Parser *parser)
{
    return parser_parse_compound(parser);
}
