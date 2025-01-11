#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "token.h"
#include "helper.h"
#include "AST.h"
#include "parser.h"
#include "lexer.h"

static char *readFile(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "[ERROR] could not open file \"%s\".\n", path);
        exit(1);
    }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = (size_t)ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "[ERROR] not enough memory to read \"%s\".\n", path);
        exit(1);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
        fprintf(stderr, "[ERROR] could not read file \"%s\".\n", path);
        fclose(file);
        free(buffer);
        exit(0);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}
void usage(char *argv[])
{
    fprintf(stderr, "[ERROR] %s <filename>\n", argv[0]);
}
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        usage(argv);
        return 1;
    }
    char *path = argv[1];
    char *source = readFile(path);
    if (strlen(source) == 0)
    {
        fprintf(stderr, "[ERROR] cannot parse '%s' empty file.\n", path);
        return 0;
    }
    Lexer *lexer = init_lexer(source, path);
    Parser *parser = init_parser(lexer);
    AST *ast = parser_parse(parser);
    free(source);
    if (parser->had_error == 0)
    {
        ast_print(ast);
        ast_free(ast);
    }
    parser_free(parser);
    lexer_free(lexer);
}