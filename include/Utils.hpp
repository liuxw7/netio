
#include <assert.h>

#define ARRAY_SIZE(n)  (sizeof(n) / sizeof(n[0]))

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)  

#define SIZE_K(n)    (n << 10)
#define SIZE_M(n)    (n << 20)
#define SIZE_G(n)    (n << 30)

#define ASSERT(x)    assert(x)
