#ifndef PARSER_H
#define PARSER_H
#include "token.h"
#include "AST.h"
#include "lexer.h"

typedef struct
{
    Token current_token;
    int had_error;
    int panic_mode;
    int parsing_call;
    Lexer *lexer;
} Parser;

typedef enum
{
    PREC_NONE,
    PREC_COMMA,       // ,
    PREC_ASSIGNMENT,  // =
    PREC_TERNARY,     // ?:
    PREC_LOGICAL_OR,  // ||
    PREC_LOGICAL_AND, // &&
    PREC_BITWISE_OR,  // |
    PREC_BITWISE_XOR, // ^
    PREC_BITWISE_AND, // &
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_SHIFT,       // >> <<
    PREC_TERM,        // + -
    PREC_FACTOR,      // * / *
    PREC_UNARY,       // ! - ++ --
    PREC_POSTIFX,     // . () ++ -- []
} Precedence;
char *parser_prec_to_str(Precedence prec);
Parser *init_parser(Lexer *lexer);
Token parser_advance(Parser *parser);
AST *parser_parse(Parser *parser);
AST *parser_parse_decl(Parser *parser);
AST *parser_parse_stmt(Parser *parser);
AST *parser_parse_expr(Parser *parser);
AST *parser_parse_group(Parser *parser);
AST *parser_parse_prefix(Parser *parser);
AST *parser_parse_infix(Parser *parser, AST *prefix);
AST *parser_parse_number(Parser *parser);
AST *parser_parse_ternary(Parser *parser, AST *condition);
AST *parser_parse_primary(Parser *parser);
AST *parser_parse_string(Parser *parser);
AST *parser_parse_postfix(Parser *parser, AST *operand);
AST *parser_parse_call(Parser *parser, AST *callee);
AST *parser_parse_comma(Parser *parser, AST *prefix);
AST *parser_parse_compound(Parser *parser);
void parser_state(Parser *parser);
void parser_error(Parser *parser, char *message);
typedef AST *(*ParsePrefixFn)(Parser *parser);
typedef AST *(*ParseInfixFn)(Parser *parser, AST *prefix);
typedef struct
{
    ParsePrefixFn prefix;
    ParseInfixFn infix;
    Precedence precedence;
} ParseRule;
void parser_free(Parser *parser);
#endif