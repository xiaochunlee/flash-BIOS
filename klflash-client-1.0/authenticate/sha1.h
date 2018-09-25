#ifndef _SHA1_H
#define _SHA1_H

#include <string.h>
#ifndef _STD_TYPES
#define _STD_TYPES

#define uchar   unsigned char
#define uint    unsigned int
#define ulong   unsigned long int

#endif

typedef struct
{
    ulong total[2];
    ulong state[5];
    uchar buffer[64];
} sha1_context;

/*
 * Output SHA-1(buf)
 */
void sha1_csum( uchar *buf, uint buflen, uchar digest[20] );

#endif /* sha1.h */
