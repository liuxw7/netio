
#include <assert.h>

#define ARRAY_SIZE(n)  (sizeof(n) / sizeof(n[0]))

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)  

#define SIZE_B(n)    (n)
#define SIZE_K(n)    (n << 10)
#define SIZE_M(n)    (n << 20)
#define SIZE_G(n)    (n << 30)

#define ASSERT(x)    assert(x)

#define CHKRET(x)  (ASSERT(((x)) >= 0))

#define NIP_QUAD(addr)\
  ((unsigned char *)&addr)[0],\
  ((unsigned char *)&addr)[1],\
  ((unsigned char *)&addr)[2],\
  ((unsigned char *)&addr)[3]
#define HIP_QUAD(addr)\
  ((unsigned char *)&addr)[3],\
  ((unsigned char *)&addr)[2],\
  ((unsigned char *)&addr)[1],\
  ((unsigned char *)&addr)[0]
#define IPQUAD_FMT "%u.%u.%u.%u" 
