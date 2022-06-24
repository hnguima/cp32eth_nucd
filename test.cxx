#ifndef __STRUCTS__H
#define __STRUCTS__H

#define NARG(...) NARG_(__VA_ARGS__, RSEQ_N())
#define NARG_(...) ARG_N(__VA_ARGS__)
#define ARG_N(                               \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
    _11, _12, _13, _14, _15, _16, N, ...) N
#define RSEQ_N() 16, 15, 14, 13, 12, 11, 10, \
                 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define __NL__

#define ITEM1_(first) first;
#define ITEM2_(first, ...) \
    first;                 \
    ITEM1_(__VA_ARGS__)
#define ITEM3_(first, ...) \
    first;                 \
    ITEM2_(__VA_ARGS__)
#define ITEM4_(first, ...) \
    first;                 \
    ITEM3_(__VA_ARGS__)
#define ITEM5_(first, ...) \
    first;                 \
    ITEM4_(__VA_ARGS__)
#define ITEM6_(first, ...) \
    first;                 \
    ITEM5_(__VA_ARGS__)
#define ITEM7_(first, ...) \
    first;                 \
    ITEM6_(__VA_ARGS__)
#define ITEM8_(first, ...) \
    first;                 \
    ITEM7_(__VA_ARGS__)
#define ITEM9_(first, ...) \
    first;                 \
    ITEM8_(__VA_ARGS__)
#define ITEM10_(first, ...) \
    first;                  \
    ITEM9_(__VA_ARGS__)
#define ITEM11_(first, ...) \
    first;                  \
    ITEM10_(__VA_ARGS__)
#define ITEM12_(first, ...) \
    first;                  \
    ITEM11_(__VA_ARGS__)
#define ITEM13_(first, ...) \
    first;                  \
    ITEM12_(__VA_ARGS__)
#define ITEM14_(first, ...) \
    first;                  \
    ITEM13_(__VA_ARGS__)
#define ITEM15_(first, ...) \
    first;                  \
    ITEM14_(__VA_ARGS__)
#define ITEM16_(first, ...) \
    first;                  \
    ITEM15_(__VA_ARGS__)

#define PASTE(a, b, c) PASTE_(a, b, c)
#define PASTE_(a, b, c) a##b##c

#define CREATE_STRUCT(name, ...)          \
    typedef struct                        \
    {                                     \
        PASTE(ITEM, NARG(__VA_ARGS__), _) \
        (__VA_ARGS__)                     \
    } name;

CREATE_STRUCT(my_struct, int a);
#endif //<-- __STRUCTS__H -->