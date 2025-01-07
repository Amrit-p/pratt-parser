#include "AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
AST *init_ast(AST_Type type)
{
    AST *ast = calloc(1, sizeof(AST));
    if (type == AST_COMPOUND || type == AST_SEQUENCEEXPR)
    {
        init_array(&ast->childs);
    }
    ast->type = type;
    return ast;
}
size_t ast_push(AST *ast, AST *child)
{
    size_t i = array_size(&ast->childs);
    array_push(&ast->childs, child);
    return i;
}
char *ast_to_str(AST *ast)
{
    return ast_type_to_str(ast->type);
}
char *ast_type_to_str(int type)
{
    switch (type)
    {
    case AST_NUMBER:
        return "NUMBER";
    case AST_BINARY:
        return "BINARYEXPR";
    case AST_UNARY:
        return "UNARYEXPR";
    case AST_GROUP:
        return "GROUP";
    case AST_COMPOUND:
        return "COMPOUND";
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
    case AST_SEQUENCEEXPR:
        return "SEQUENCEEXPR";
    case AST_IF:
        return "IF";
    default:
        return "UNKNOWN";
    }
}
char *ast_to_json(AST *ast)
{
    if (ast == NULL)
        return NULL;
    char *c = calloc(2, sizeof(char));
    strcpy(c, "{");
    char *type_key = "\"type\": \"AST_%s\"";
    char *type = calloc(strlen(type_key) + strlen(ast_type_to_str(ast->type)) + 1, sizeof(char));
    sprintf(type, type_key, ast_type_to_str(ast->type));
    c = realloc(c, strlen(c) + strlen(type) + 1 * sizeof(char));
    strcat(c, type);
    free(type);

    if (ast->name)
    {
        char *name_key = ",\"name\": \"%s\"";
        char *name = calloc(strlen(name_key) + strlen(ast->name) + 1, sizeof(char));
        sprintf(name, name_key, ast->name);
        c = realloc(c, strlen(c) + strlen(name) + 1 * sizeof(char));
        strcat(c, name);
        free(name);
    }
    if ((ast->type == AST_NUMBER))
    {
        char str[50];
        sprintf(str, "%f", ast->number);
        size_t digits = strlen(str);
        char *int_value_key = ",\"number\": \"%f\"";
        char *number = calloc(strlen(int_value_key) + digits + 1, sizeof(char));
        sprintf(number, int_value_key, ast->number);
        c = (char *)realloc(c, strlen(c) + strlen(number) + 1 * sizeof(char));
        strcat(c, number);
        free(number);
    }

    if (ast->left != NULL)
    {
        char *left_key = ",\"left\": ";
        c = realloc(c, strlen(c) + strlen(left_key) + 1 * sizeof(char));
        strcat(c, left_key);
        AST *left = ast->left;
        char *temp = ast_to_json(left);
        if (temp)
        {
            c = realloc(c, strlen(c) + strlen(temp) + 1 * sizeof(char));
            strcat(c, temp);
            free(temp);
        }
    }
    if (ast->right != NULL)
    {
        char *right_key = ",\"right\": ";
        c = realloc(c, strlen(c) + strlen(right_key) + 1 * sizeof(char));
        strcat(c, right_key);
        AST *right = ast->right;
        char *temp = ast_to_json(right);
        if (temp)
        {
            c = realloc(c, strlen(c) + strlen(temp) + 1 * sizeof(char));
            strcat(c, temp);
            free(temp);
        }
    }
    if (ast->value != NULL)
    {
        char *value = ",\"value\": ";
        c = realloc(c, strlen(c) + strlen(value) + 1 * sizeof(char));
        strcat(c, value);
        char *temp = ast_to_json(ast->value);
        if (temp)
        {
            c = realloc(c, strlen(c) + strlen(temp) + 1 * sizeof(char));
            strcat(c, temp);
            free(temp);
        }
    }
    if (array_size(&ast->childs) != 0)
    {
        char *children = ",\"children\": [";
        c = realloc(c, strlen(c) + strlen(children) + 1 * sizeof(char));
        strcat(c, children);
        for (size_t i = 0; i < array_size(&ast->childs); i++)
        {
            AST *child = array_at(&ast->childs, i);
            if (child)
            {
                char *temp = ast_to_json(child);
                if (temp)
                {
                    c = realloc(c, strlen(c) + strlen(temp) + 1 * sizeof(char));
                    strcat(c, temp);
                    free(temp);
                }
                if (i != array_size(&ast->childs) - 1)
                {
                    c = realloc(c, strlen(c) + 2 * sizeof(char));
                    strcat(c, ",");
                }
            }
        }
        c = realloc(c, strlen(c) + 2 * sizeof(char));
        strcat(c, "]");
    }
    c = realloc(c, strlen(c) + 2 * sizeof(char));
    strcat(c, "}");
    return c;
}
void ast_print(AST *root)
{
    fprintf(stdout, "%s\n", ast_to_json(root));
}