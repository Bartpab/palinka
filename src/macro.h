#ifndef __MACRO_H__
#define __MACRO_H__

#define __COUT_VA_ARGS__(...) __VA_VALS__(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define __VA_VALS__(N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15, N16, N, ...) N
#define MAP_NAME(N) __MAP_NAME__(N)
#define __MAP_NAME__(N) MAP_ ## N

#define MAP(F, ...) MAP_NAME(__COUT_VA_ARGS__(__VA_ARGS__))(F, __VA_ARGS__)

#define MAP_1(F, A) F(A);
#define MAP_2(F, A, ...) F(A); MAP_1(F, __VA_ARGS__)
#define MAP_3(F, A, ...) F(A); MAP_2(F, __VA_ARGS__)
#define MAP_4(F, A, ...) F(A); MAP_3(F, __VA_ARGS__)
#define MAP_5(F, A, ...) F(A); MAP_4(F, __VA_ARGS__)
#define MAP_6(F, A, ...) F(A); MAP_5(F, __VA_ARGS__)
#define MAP_7(F, A, ...) F(A); MAP_6(F, __VA_ARGS__)
#define MAP_8(F, A, ...) F(A); MAP_7(F, __VA_ARGS__)
#define MAP_9(F, A, ...) F(A); MAP_8(F, __VA_ARGS__)
#define MAP_10(F, A, ...) F(A); MAP_9(F, __VA_ARGS__)
#define MAP_11(F, A, ...) F(A); MAP_10(F, __VA_ARGS__)
#define MAP_12(F, A, ...) F(A); MAP_11(F, __VA_ARGS__)
#define MAP_13(F, A, ...) F(A); MAP_12(F, __VA_ARGS__)
#define MAP_14(F, A, ...) F(A); MAP_13(F, __VA_ARGS__)
#define MAP_15(F, A, ...) F(A); MAP_14(F, __VA_ARGS__)
#define MAP_16(F, A, ...) F(A); MAP_15(F, __VA_ARGS__)

#endif