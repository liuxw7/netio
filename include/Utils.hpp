
#define ARRAY_SIZE(n)  (sizeof(n) / sizeof(n[0]))

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)  
