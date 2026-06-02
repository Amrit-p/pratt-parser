#define NOB_IMPLEMENTATION
#include "../includes/nob.h"

#define CFLAGS "-Wall", "-Wextra", "-I../includes", "-ggdb"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Nob_Cmd cmd = {0};
    nob_cc(&cmd);
    nob_cmd_append(&cmd,
                   CFLAGS,
                   "../lexer.c", "main.c", "../token.c",
                   "-o", "main");

    if (!nob_cmd_run(&cmd))
        return 1;
}