#ifndef HELPER_H
#define HELPER_H
int helper_num_places(int n);
#define UNIMPLEMENTED                                                            \
    do                                                                           \
    {                                                                            \
        fprintf(stderr, "unimplementd: %s:%d at %s\n", __FILE__,__LINE__,__func__); \
        exit(1);                                                                 \
    }                                                                            \
    while (0)
#endif