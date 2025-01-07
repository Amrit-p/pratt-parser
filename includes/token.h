#ifndef TOKEN_H
#define TOKEN_H
#include <stddef.h>
typedef enum
{
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_EOF,
    TOKEN_VAR,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_ERROR,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_QUESTION,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_STRING,
    TOKEN_EQUALS,
    TOKEN_ASSIGNMENT,
    TOKEN_NOT,
    TOKEN_NOT_EQUALS,
    TOKEN_MOD,
    TOKEN_RIGHT_SHIFT,
    TOKEN_LTE,
    TOKEN_LT,
    TOKEN_LEFT_SHIFT,
    TOKEN_GTE,
    TOKEN_GT,
    TOKEN_AND,
    TOKEN_BITWISE_AND,
    TOKEN_OR,
    TOKEN_BITWISE_OR,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_BITWISE_NOT,
    TOKEN_LCURLY,
    TOKEN_RCURLY,
} TokenType;
typedef struct
{
    char *start;
    size_t row;
    size_t col;
    size_t length;
    TokenType type;
    char *message;
} Token;
Token init_token(char *start, TokenType type, size_t length, size_t row, size_t col);
const char *token_type_str(TokenType type);
char *token_to_str(Token token);
void token_print(Token token);
char *token_text(Token token);
#endif