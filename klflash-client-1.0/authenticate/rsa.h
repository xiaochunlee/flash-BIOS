#ifndef __RSA_H___
#define __RSA_H___

#pragma warning(disable:4244)

char *encrypt(char *pubN, char *priKey, char *plain);
char *decrypt(char *pubN, char *pubKey, char *cipher);
int InverseMod2power(int n, int s);
void modmul(unsigned char *X, unsigned char *Y, unsigned char *N, int size);
void modpow(unsigned char *base, unsigned char *exponent, unsigned char *modulus, unsigned char *result, int size);
void HEX_to_UINT(char *phex, unsigned char *puint);
void UINT_to_HEX(unsigned char *puint, int size, char *phex);
int compare(unsigned char *X, unsigned char *Y, int size);

//
// Macros that directly map functions to BaseLib, BaseMemoryLib, and DebugLib functions
//

#define strlen(str)                       EfiAsciiStrLen(str)

#if 0
#define malloc(size)                      EfiLibAllocatePool(size)
#define memcpy(dest,source,count)         gBS->CopyMem (dest, source, (UINTN)(count))
#define memset(dest,ch,count)             gBS->SetMem (dest,(UINTN)(count),(UINT8)(ch))
#define strlen(str)                       EfiAsciiStrLen(str)
#define free(buf)                         gBS->FreePool (buf)
#define exit(n)                           return
#define printf(str)                       DEBUG ((EFI_D_ERROR, str))
#endif

#endif

