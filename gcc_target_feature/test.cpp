#include <iostream>
#include <cassert>

#include <immintrin.h>

__attribute__ ((target ("default")))
void check()
{
    assert(!__builtin_cpu_supports ("avx2"));

    printf("avx2 is not supported.\n");
}

__attribute__ ((target("avx2")))
void check()
{
    assert(!!__builtin_cpu_supports ("avx2"));

    printf("avx2 supported. %d\n", sizeof _mm256_set1_epi64x(0));
}

int main()
{
    check();

    return 0;
}
