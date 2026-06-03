Arena default_arena = {0};

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "AST.h"
#include "ir.h"
#include "VM.h"
#include "common.h"

typedef enum
{
    OUTPUT_END,
    OUTPUT_TOKENS,
    OUTPUT_AST,
    OUTPUT_IR,
} Output_Type;

typedef struct
{
    int argc;
    char **argv;
    char *input_file;
    char *source;
    Output_Type output;
    bool help;
} Options;

void usage(FILE *stream)
{
    fprintf(stream, "Usage: %s [OPTIONS] [--] [INPUT FILE]\n", flag_program_name());
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
}

bool parse_options(Options *options)
{
    flag_bool_var(&options->help, "help", false, "Print this help to stdout and exit with 0");
    flag_str_var(&options->source, "s", NULL, "Used to give source code text not path");
    char *output = NULL;
    flag_str_var(&output, "o", NULL, "Used to set output from {TOKENS|AST|IR}");

    if (!flag_parse(options->argc, options->argv))
    {
        usage(stderr);
        flag_print_error(stderr);
        return false;
    }

    options->argc = flag_rest_argc();
    options->argv = flag_rest_argv();
    options->input_file = options->argv[0];
    if (output)
    {
        if (strcmp(output, "TOKENS") == 0)
        {
            options->output = OUTPUT_TOKENS;
        }
        else if (strcmp(output, "AST") == 0)
        {
            options->output = OUTPUT_AST;
        }
        else if (strcmp(output, "IR") == 0)
        {
            options->output = OUTPUT_IR;
        }
        else
        {
            usage(stderr);
            fprintf(stderr, "Error: -o flag is invalid should be in {TOKENS|AST|IR} but given \"%s\"\n", output);
            return false;
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    Options options = {0};
    options.argc = argc;
    options.argv = argv;

    if (!parse_options(&options))
        return 2;

    if (options.help)
    {
        usage(stdout);
        return 0;
    }

    if (!options.input_file && !options.source)
    {
        usage(stderr);
        return 1;
    }

    Nob_String_Builder sb = {0};
    if (!options.source && !nob_read_entire_file(options.input_file, &sb))
        return 3;

    if (options.source)
        nob_sb_append_cstr(&sb, options.source);

    Lexer *lexer = init_lexer(sb, options.input_file);
    if (options.output == OUTPUT_TOKENS)
    {
        lexer_dump(lexer, stdout);
        goto success;
    }

    Parser *parser = init_parser(lexer);
    AST *ast = parser_parse(parser);
    if (options.output == OUTPUT_AST)
    {
        NOB_TODO("impl ast_dump function");
    }

    Compiler *compiler = ir_gen(ast);
    if (options.output == OUTPUT_IR)
    {
        compiler_dump(compiler, stdout);
        goto success;
    }

    VM *vm = init_vm(compiler);
    VM_Error err = vm_interpret(vm);
    vm_print_stack_trace(vm);
    vm_print_error(vm, err);

success:
    arena_free(&default_arena);
    return 0;
}
