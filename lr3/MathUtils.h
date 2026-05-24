#ifndef MathUtilsH
#define MathUtilsH

#include <cstdint>

bool    is_prime(int32_t n);
int32_t extended_evklid(int32_t a, int32_t b, int32_t &x, int32_t &y);
int32_t mod_inverse(int32_t a, int32_t m);
uint32_t mod_pow(uint32_t base, uint32_t exp, uint32_t mod);

#endif
