#ifndef JOS_MACHINE_TYPES_H
#define JOS_MACHINE_TYPES_H

#ifndef NULL
#define NULL (0)
#endif

#ifndef inline
#define inline __inline__
#endif

#define asm __asm
#define typeof __typeof
#define volatile __volatile
// Represents true-or-false values
typedef int bool_t;
typedef int boot;


// Explicitly-sized versions of integer types
typedef __signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#if __LONG_MAX__==9223372036854775807L
typedef long int64_t;
typedef unsigned long uint64_t;
typedef __int128_t int128_t;
typedef __uint128_t uint128_t;
#elif __LONG_LONG_MAX__==9223372036854775807LL
typedef long long int64_t;
typedef unsigned long long uint64_t;
#else
#error Missing 64-bit type
#endif
typedef uint64_t __uint64_t;

typedef  uint8_t  __u8;
typedef uint8_t _u8;
typedef uint8_t u8;

typedef uint16_t __u16;
typedef uint16_t  _u16;
typedef uint16_t  u16;

typedef uint32_t __u32;
typedef uint32_t _u32;
typedef uint32_t  u32;

typedef uint64_t __u64;
typedef uint64_t _u64;
typedef uint64_t u64;



// Pointers and addresses are 64 bits long.
// We use pointer types to represent virtual addresses,
// uintptr_t to represent the numerical values of virtual addresses,
// and physaddr_t to represent physical addresses.
// Use __PTRDIFF_TYPE__ so that -m32 works out properly.
typedef __PTRDIFF_TYPE__ intptr_t;
typedef unsigned __PTRDIFF_TYPE__ uintptr_t;
typedef unsigned __PTRDIFF_TYPE__ physaddr_t;
typedef unsigned __PTRDIFF_TYPE__ guestaddr_t;

// Page numbers are 64 bits long.
typedef uint64_t ppn_t;

#include <stddef.h>	// gcc header file

#define PRIu64 "ld"
#define PRIx64 "lx"

/* use gcc builtin offset */
//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *) 0)->MEMBER)

#endif /* !JOS_MACHINE_TYPES_H */
