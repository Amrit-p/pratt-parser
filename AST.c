#include "AST.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
AST *init_ast(AST_Type type)
{
    AST *ast = calloc(1, sizeof(AST));
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
        size_t i = array_size(&ast->as.sequenceExpr.exprs);
        array_push(&ast->as.sequenceExpr.exprs, child);
        return i;
    }
    if (ast->type == AST_STMT)
    {
        switch (ast->as.statment.type)
        {
        case STMT_COMPOUND:
        {
            size_t i = array_size(&ast->as.statment.as.compound.statments);
            array_push(&ast->as.statment.as.compound.statments, child);
            return i;
        }
        default:
            break;
        }
    }
    fprintf(stderr, "[ERROR] unknown statementType in %s\n", __func__);
    exit(1);
}
char *ast_to_str(AST *ast)
{
    return ast_type_to_str(ast->type);
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
char *ast_type_to_str(AST *ast)
{
    char *template = "\"type\":\"AST_%s\"";
    char *type = ast_type_str(ast->type);
    char *buffer = calloc(strlen(template) + strlen(type) + 1, sizeof(char));
    sprintf(buffer, template, type);
    return buffer;
}
char *ast_number_to_json(AST *ast)
{
    char *template = "{%s,\"number\":\"%f\"}";
    char *type = ast_type_to_str(ast);
    char *buffer = calloc(strlen(template) + strlen(type) + sizeof(double) + 1, sizeof(char));
    sprintf(buffer, template, type, ast->as.number);
    free(type);
    return buffer;
}
char *ast_stmt_to_json(AST *ast)
{
    switch (ast->as.statment.type)
    {
    case STMT_COMPOUND:
    {
        char *template = "{\"type\":\"Compound\",\"statments\":[%s]}";
        char *statments = calloc(1, sizeof(char));
        for (size_t i = 0; i < array_size(&ast->as.statment.as.compound.statments); i++)
        {
            AST *child = array_at(&ast->as.statment.as.compound.statments, i);
            char *expr = ast_to_json(child);
            if (expr)
            {
                statments = realloc(statments, strlen(statments) + strlen(expr) + 2 * sizeof(char));
                strcat(statments, expr);
                if (i != array_size(&ast->as.statment.as.compound.statments) - 1)
                    strcat(statments, ",");
            }
        }
        char *buffer = calloc(strlen(template) + strlen(statments) + 1, sizeof(char));
        sprintf(buffer, template, statments);
        return buffer;
    }
    case STMT_PRINT:
    {
        char *template = "{\"type\":\"Print\",\"expression\":%s}";
        char *expr = ast_to_json(ast->as.statment.as.print.expr);
        char *print = calloc(1, sizeof(char));
        sprintf(print, template, expr);
        return print;
    }
    case STMT_IF:
    {
        char *template = "{\"type\":\"if\",\"condition\":%s,\"then\":%s";
        char *condition = ast_to_json(ast->as.statment.as._if.condition);
        char *then = ast_to_json(ast->as.statment.as._if.then);
        char *buffer = calloc(strlen(template) + strlen(condition) + strlen(then) + 1, sizeof(char));
        sprintf(buffer, template, condition, then);
        if (ast->as.statment.as._if.other_wise)
        {
            char *template = ",\"otherwise\":%s";
            char *otherwise = ast_to_json(ast->as.statment.as._if.other_wise);
            char *buffer_otherwise = calloc((strlen(otherwise) + strlen(template) + 1), sizeof(char));
            sprintf(buffer_otherwise, template, otherwise);
            buffer = realloc(buffer, (strlen(buffer_otherwise) + strlen(buffer) + 1) * sizeof(char));
            strcat(buffer, buffer_otherwise);
            free(buffer_otherwise);
        }
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strcat(buffer, "}");
        return buffer;
    }
    break;
    default:
        printf("[ERROR] %s\n", ast_type_to_str(ast->type));
    }
}
char *ast_string_to_json(AST *ast)
{
    char *template = "{\"type\":\"%s\",\"value\":\"%s\"}";
    char *buffer = (strlen(template) + strlen(ast->as.string) + 1, sizeof(char));
    sprintf(buffer, template, ast_type_to_str(ast->type), ast->as.string);
    return buffer;
}
char *ast_to_json(AST *ast)
{
    if (!ast)
        return NULL;
    switch (ast->type)
    {
    case AST_STMT:
        return ast_stmt_to_json(ast);
    case AST_NUMBER:
        return ast_number_to_json(ast);
    case AST_STRING:
        return ast_string_to_json(ast);
    default:
        printf("[ERROR] unreachanle %s\n", ast_type_to_str(ast->type));
        exit(1);
        break;
    }
}
void ast_print(AST *ast)
{
    char *json = ast_to_json(ast);
    fprintf(stderr, "%s\n", json);
    free(json);
}
