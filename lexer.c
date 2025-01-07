#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define MIN(a, b) \
    a < b ? a : b

Token lexer_parse_number(Lexer *lexer)
{
    size_t start = lexer->index;
    size_t col = lexer->col;

    while (isdigit(lexer->current_char) || lexer->current_char == '.')
    {
        lexer_advance(lexer);
    }

    Token token = init_token(&lexer->src[start], TOKEN_NUMBER, lexer->index - start,
                             lexer->row, col);
    return token;
}
TokenType lexer_check_keyword(Lexer *lexer, size_t length,
                              const char *rest, TokenType type)
{
    if (memcmp(&lexer->src[lexer->index - length], rest, length) == 0)
    {
        return type;
    }

    return TOKEN_ID;
}
Token lexer_parse_id(Lexer *lexer)
{
    size_t start = lexer->index;
    size_t col = lexer->col;
    while (isalpha(lexer->current_char) || lexer->current_char == '_' || isdigit(lexer->current_char))
    {
        lexer_advance(lexer);
    }
    TokenType token_type = TOKEN_ID;
    switch (lexer->src[start])
    {
    case 'e':
        token_type = lexer_check_keyword(lexer, 3, "lse", TOKEN_ELSE);
        break;
    case 'i':
        token_type = lexer_check_keyword(lexer, 1, "f", TOKEN_IF);
        break;
    case 'p':
        token_type = lexer_check_keyword(lexer, 4, "rint", TOKEN_PRINT);
        break;
    case 'r':
        token_type = lexer_check_keyword(lexer, 5, "eturn", TOKEN_RETURN);
        break;
    case 'v':
        token_type = lexer_check_keyword(lexer, 2, "ar", TOKEN_VAR);
        break;
    case 'w':
        token_type = lexer_check_keyword(lexer, 4, "hile", TOKEN_WHILE);
        break;
    case 't':
        token_type = lexer_check_keyword(lexer, 3, "rue", TOKEN_TRUE);
        break;
    case 'n':
        token_type = lexer_check_keyword(lexer, 3, "ull", TOKEN_NULL);
        break;
    case 'f':
        if (&lexer->src[lexer->index] - lexer->src > 1)
        {
            switch (lexer->src[start + 1])
            {
            case 'o':
                token_type = lexer_check_keyword(lexer, 1, "r", TOKEN_FOR);
                break;
            case 'u':
                token_type = lexer_check_keyword(lexer, 7, "unction", TOKEN_FUNCTION);
                break;
            case 'a':
                token_type = lexer_check_keyword(lexer, 4, "alse", TOKEN_FALSE);
                break;
            default:
                break;
            }
        }

    default:
        break;
    }
    Token token = init_token(&lexer->src[start], token_type, lexer->index - start,
                             lexer->row, col);
    return token;
}
void lexer_advance(Lexer *lexer)
{

    if (lexer->index < lexer->src_size && lexer->current_char == '\n')
    {
        lexer->index += 1;
        lexer->row += 1;
        lexer->col = 1;
        lexer->current_char = lexer->src[lexer->index];
    }
    else if (lexer->index < lexer->src_size && lexer->current_char != '\0')
    {
        lexer->index += 1;
        lexer->col += 1;
        lexer->current_char = lexer->src[lexer->index];
    }
    else
    {
        lexer->index += 1;
        lexer->col += 1;
        lexer->current_char = '\0';
    }
}
void lexer_skip_space(Lexer *lexer)
{
    while (isspace(lexer->current_char))
    {
        lexer_advance(lexer);
    }
}
Token lexer_advance_with(Lexer *lexer, Token token)
{
    lexer_advance(lexer);
    return token;
}
Token lexer_parse_string(Lexer *lexer)
{
    size_t col = lexer->col;
    size_t row = lexer->row;
    lexer_advance(lexer);
    size_t start = lexer->index;
    while (lexer->current_char != '"')
    {
        lexer_advance(lexer);
        if (lexer->current_char == '\n' || lexer->current_char == '\0')
            break;
    }
    if (lexer->current_char == '"')
    {
        return lexer_advance_with(lexer, init_token(&lexer->src[start], TOKEN_STRING, lexer->index - start, row, col));
    }

    Token string = init_token(&lexer->src[start - 1], TOKEN_ERROR, 1, row, col);
    lexer_advance(lexer);
    string.message = "non-terminated string";
    return string;
}
char lexer_peek(Lexer *lexer, size_t offset)
{
    return lexer->src[MIN(lexer->index + offset, lexer->src_size)];
}
Token lexer_next_token(Lexer *lexer)
{
    while (lexer->current_char != '\0')
    {
        lexer_skip_space(lexer);
        if (isalpha(lexer->current_char) || lexer->current_char == '_')
            return lexer_parse_id(lexer);

        if (
            isdigit(lexer->current_char) ||
            lexer->current_char == '_' ||
            lexer->current_char == '.')
        {
            return lexer_parse_number(lexer);
        }
        if (lexer->current_char == '"')
        {
            return lexer_parse_string(lexer);
        }

        switch (lexer->current_char)
        {
        case '+':
            if (lexer_peek(lexer, 1) == '+')
            {
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_INCREMENT, 2, lexer->row, lexer->col)));
            }
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_PLUS, 1,
                                                        lexer->row, lexer->col));
        case '-':
            if (lexer_peek(lexer, 1) == '-')
            {
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_DECREMENT, 2, lexer->row, lexer->col)));
            }
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_MINUS, 1,
                                                        lexer->row, lexer->col));
        case '*':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_MUL, 1,
                                                        lexer->row, lexer->col));
        case '/':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_DIV, 1,
                                                        lexer->row, lexer->col));
        case ';':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_SEMICOLON, 1,
                                                        lexer->row, lexer->col));
        case '(':
            return lexer_advance_with(lexer,
                                      init_token(&lexer->src[lexer->index], TOKEN_LPAREN, 1,
                                                 lexer->row, lexer->col));
        case ')':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_RPAREN, 1,
                                                        lexer->row, lexer->col));
        case ',':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_COMMA, 1, lexer->row, lexer->col));
        case '?':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_QUESTION, 1, lexer->row, lexer->col));
        case ':':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_COLON, 1, lexer->row, lexer->col));
        case '=':
            if (lexer_peek(lexer, 1) == '=')
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_EQUALS, 2, lexer->row, lexer->col)));
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_ASSIGNMENT, 1, lexer->row, lexer->col));
        case '!':
            if (lexer_peek(lexer, 1) == '=')
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_NOT_EQUALS, 2, lexer->row, lexer->col)));
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_NOT, 1, lexer->row, lexer->col));
        case '%':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_MOD, 1, lexer->row, lexer->col));
        case '<':
            if (lexer_peek(lexer, 1) == '<')
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_LEFT_SHIFT, 2, lexer->row, lexer->col)));
            if (lexer_peek(lexer, 1) == '=')
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_LTE, 2, lexer->row, lexer->col)));
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_LT, 1, lexer->row, lexer->col));
        case '>':
            if (lexer_peek(lexer, 1) == '>')
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_RIGHT_SHIFT, 2, lexer->row, lexer->col)));
            if (lexer_peek(lexer, 1) == '=')
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_GTE, 2, lexer->row, lexer->col)));
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_GT, 1, lexer->row, lexer->col));
        case '&':
            if (lexer_peek(lexer, 1) == '&')
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_AND, 2, lexer->row, lexer->col)));
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_BITWISE_AND, 1, lexer->row, lexer->col));
        case '|':
            if (lexer_peek(lexer, 1) == '|')
                return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_OR, 2, lexer->row, lexer->col)));
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_BITWISE_OR, 1, lexer->row, lexer->col));
        case '~':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_BITWISE_NOT, 1, lexer->row, lexer->col));
        case '{':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_LCURLY, 1, lexer->row, lexer->col));
        case '}':
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_RCURLY, 1, lexer->row, lexer->col));
        case '\0':
            break;
        default:
            return lexer_advance_with(lexer, init_token(&lexer->src[lexer->index], TOKEN_ERROR,
                                                        1, lexer->row, lexer->col));
        }
    }
    return init_token(&lexer->src[lexer->index], TOKEN_EOF, 0,
                      lexer->row, lexer->col);
}
Lexer *init_lexer(char *source, char *path)
{
    Lexer *lexer = calloc(1, sizeof(Lexer));
    lexer->col = 1;
    lexer->row = 1;
    lexer->index = 0;
    lexer->src = source;
    lexer->src_size = strlen(source);
    lexer->current_char = source[0];
    lexer->file_path = path;
    return lexer;
}
