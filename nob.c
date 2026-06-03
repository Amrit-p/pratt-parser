#include "thirdparty/nob.c"

#define INCLUDES_DIR "includes/"
#define THIRDPARTY_DIR "thirdparty/"
#define SRC_DIR "src/"
#define BUILD_DIR ".build/"

bool build_the_interpreter(char *output_path);

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;

    if (!build_the_interpreter("interpreter"))
        return 1;

    return 0;
}

bool build_the_interpreter(char *output_path)
{

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd,
                   "cc",
                   "-Wall",
                   "-Wextra",
                   "-Wno-missing-braces",
                   "-fno-strict-aliasing",
                   "-I" INCLUDES_DIR,
                   "-I" THIRDPARTY_DIR,
                   "-o",
                   nob_temp_sprintf("%s%s", BUILD_DIR, output_path),
                   SRC_DIR"one.c");
    return nob_cmd_run(&cmd);
}
