#include "MathUtils.h"

#include <cmath>

bool is_prime(int32_t n)
{
    if (n < 2)
        return false;
    if (n == 2)
        return true;
    if (n % 2 == 0)
        return false;
    int32_t lim = static_cast<int32_t>(std::sqrt(static_cast<double>(n)));
    for (int32_t i = 3; i <= lim; i += 2)
        if (n % i == 0)
            return false;
    return true;
}

int32_t extended_evklid(int32_t a, int32_t b, int32_t &x, int32_t &y)
{
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    int32_t x1, y1;
    int32_t d = extended_evklid(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return d;
}

int32_t mod_inverse(int32_t a, int32_t m)
{
    int32_t x, y;
    int32_t g = extended_evklid(a, m, x, y);
    if (g != 1)
        return -1;
    return ((x % m) + m) % m;
}

uint32_t mod_pow(uint32_t base, uint32_t exp, uint32_t mod)
{
    if (mod == 1)
        return 0;
    uint64_t result = 1;
    uint64_t b = base % mod;
    uint32_t e = exp;
    while (e > 0) {
        if (e & 1)
            result = (result * b) % mod;
        b = (b * b) % mod;
        e >>= 1;
    }
    return static_cast<uint32_t>(result);
}
