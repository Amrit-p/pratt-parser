
#define ARENA_IMPLEMENTATION
#include "arena.h"
Arena default_arena = {0};
#define NOB_IMPLEMENTATION
#include "lexer.h"

typedef struct
{
    String_View name;
    String_View args;
    String_View source;
    String_View expected;
    String_View file_path;
} Test;

bool test_run(Test test)
{
    fprintf(stdout, "[RUNNING]: " SV_Fmt " :: " SV_Fmt "\n", SV_Arg(test.file_path), SV_Arg(test.name));

    Nob_Cmd cmd = {0};
    const char *source = nob_temp_sv_to_cstr(test.source);

    nob_cmd_append(&cmd, "../bin/fu");
    // TODO use "test.args"
    nob_cmd_append(&cmd, "-o", "TOKENS");
    nob_cmd_append(&cmd,
                   "-s",
                   source);

    char *output = "stdout.txt";
    Nob_Cmd_Opt opt = {
        .stdout_path = output,
        .stderr_path = output,
    };
    nob_cmd_run_opt(&cmd, opt);

    Nob_String_Builder output_sb = {0};
    if (!nob_read_entire_file(output, &output_sb))
        return false;

    Nob_String_View output_sv = nob_sv_trim(nob_sb_to_sv(output_sb));
    // printf("output_sv = " SV_Fmt "\n", SV_Arg(output_sv));
    // printf("test.expected = " SV_Fmt "\n", SV_Arg(test.expected));
    bool success = nob_sv_eq(output_sv, test.expected);
    // nob_delete_file(output);
    return success;
}

void test_dump(Test test)
{
    fprintf(stdout, "TEST:\n\t \"" SV_Fmt "\"\n", SV_Arg(test.name));
    fprintf(stdout, "ARGS:\n\t\"" SV_Fmt "\"\n", SV_Arg(test.args));
    fprintf(stdout, "SOURCE:\n\t \"" SV_Fmt "\"\n", SV_Arg(test.source));
    fprintf(stdout, "EXPECT:\n\t \"" SV_Fmt "\"\n", SV_Arg(test.expected));
}

void usage(char *argv[])
{
    fprintf(stderr, "[ERROR] %s <filename>\n", argv[0]);
}

Nob_String_View test_parse_section(Nob_String_Builder content, char *section_name)
{
    Lexer *lexer = init_lexer(content, "");
    Token token = lexer_next_token(lexer);
    Token given_token = init_token(section_name, TOKEN_ID, strlen(section_name), 0, 0);
    Nob_String_View section = {0};
    bool anchor = false;
    while (token.type != TOKEN_EOF)
    {
        if (token.type == TOKEN_DECREMENT)
        {
            Token id_token = lexer_next_token(lexer);
            if (id_token.type == TOKEN_ID)
            {
                Token decr = lexer_next_token(lexer);
                if (decr.type == TOKEN_DECREMENT && token_equal(id_token, given_token))
                {
                    Token current_token = lexer_next_token(lexer);
                    section.data = current_token.start;
                    anchor = true;
                }
                else if (token.type == TOKEN_DECREMENT && anchor)
                {
                    section.count = token.start - section.data;
                    section = nob_sv_trim(section);
                    anchor = false;
                    break;
                }
            }
        }
        token = lexer_next_token(lexer);
    }
    if (anchor)
    {
        section.count = token.start - section.data;
        section = nob_sv_trim(section);
        anchor = false;
    }
    return section;
}

Test test_parse(Nob_String_Builder content, char *file_path)
{
    Test test = {0};
    test.file_path = (Nob_String_View){
        .data = file_path,
        .count = strlen(file_path),
    };
    test.name = test_parse_section(content, "NAME");
    test.args = test_parse_section(content, "ARGS");
    test.source = test_parse_section(content, "SOURCE");
    test.expected = test_parse_section(content, "EXPECT");
    return test;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        usage(argv);
        return 1;
    }

    Nob_String_Builder content = {0};
    if (!nob_read_entire_file(argv[1], &content))
    {
        return 1;
    }

    Test test = test_parse(content, argv[1]);
    // test_dump(test);
    printf("%s\n", test_run(test) ? "[PASSED]" : "[FAILED]");

    arena_free(&default_arena);
    return 0;
}