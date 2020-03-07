// 32 bit arm tons of instructions, 64 bit arm few (something to do with 32 bit mull overflow)
// better with -O3
#include <stdint.h>

typedef int32_t fixed

#define FIX_FRAC 16

#define fix2float(a) (a / (float)(1<<FX_FRAC))
#define fix_make(a)  ((fixed)(a * (1<<FX_FRAC)))
#define fix_add(a,b) (a + b)
#define fix_sub(a,b) (a - b)
#define fix_mul(a,b) ((fixed)(((int64_t)a * b) >> FX_FRAC))
#define fix_div(a,b) ((fixed)(((int64_t)a << FX_FRAC) / b))
