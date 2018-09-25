
//#include "Tiano.h"
//#include "EfiDriverLib.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "rsa.h"

#define BETA 0x100 

char * encrypt(char *pubN, char *priKey, char *plain)
{
	char *cipher;
	unsigned char *N, *e, *n, *res; /* res = n^e mod N */
	int i, MaxSize = strlen(pubN)/2+2;

	N = (unsigned char *)malloc(sizeof(unsigned char)*MaxSize);
	e = (unsigned char *)malloc(sizeof(unsigned char)*MaxSize);
	n = (unsigned char *)malloc(sizeof(unsigned char)*MaxSize);
	res = (unsigned char *)malloc(sizeof(unsigned char)*MaxSize);
	cipher = (char *)malloc(sizeof(char)*MaxSize*2+1);

	memset(N, 0, MaxSize);
	memset(e, 0, MaxSize);
	memset(n, 0, MaxSize);
	memset(res, 0, MaxSize);
	memset(cipher, 0, MaxSize*2);

	HEX_to_UINT(pubN, N);
	HEX_to_UINT(priKey, e);

	n[0] = strlen(plain);
	for (i=0; i<(int)strlen(plain); ++i) {
		n[i+1] = (unsigned char)plain[i];
	}

	modpow(n, e, N, res, MaxSize); /* res = n^e mod N */

	UINT_to_HEX(res, MaxSize, cipher);
	
	free(N);
	free(e);
	free(n);
	free(res);

	return cipher;
}

char * decrypt(char *pubN, char *pubKey, char *cipher)
{
	char *plain;
	unsigned char *N, *d, *c, *res; /* res = n^e mod N */
	int i, MaxSize = (strlen(pubN))/2+3;

	N = (unsigned char *)malloc(sizeof(unsigned char)*MaxSize);
	d = (unsigned char *)malloc(sizeof(unsigned char)*MaxSize);
	c = (unsigned char *)malloc(sizeof(unsigned char)*MaxSize);
	res = (unsigned char *)malloc(sizeof(unsigned char)*MaxSize);

	memset(N, 0, MaxSize);
	memset(d, 0, MaxSize);
	memset(c, 0, MaxSize);

	HEX_to_UINT(pubN, N);
	HEX_to_UINT(pubKey, d);
	HEX_to_UINT(cipher, c);

	modpow(c, d, N, res, MaxSize); /* res = c^d mod N */

	plain = (char *)malloc(sizeof(char)*(res[0]+1));
	for (i=0; i<res[0]; ++i) {
		plain[i] = (char)res[i+1];
	}
	plain[i] = 0;

	free(N);
	free(d);
	free(c);
	free(res);

	return plain;
}

void modpow(unsigned char *base, unsigned char *exponent, unsigned char *modulus,unsigned char *result, int size)
/* result = base ^ exponent mod modulus */
{

	int i, j, true_size;
	unsigned int temp;
  true_size = 0;
	for (i=size-1; i>=0; --i) {
		if (exponent[i] > 0) {
			true_size = i+1;
			break;
		}
	}
	memset(result, 0, size);
	result[0] = 1;
	for (i=0; i<true_size; ++i) {
		temp = exponent[i];
		for (j=0; j<8; ++j) {
			if ((temp & 1) == 1) {
				modmul(result, base, modulus, size);
			}
			temp >>= 1;
			modmul(base, base, modulus, size);
		}
	}
}

void modmul(unsigned char *X, unsigned char *Y, unsigned char *N, int size)
/* X = X*Y mod N */
{
	unsigned int q, temp, carrier;
	int inv, k, i, true_size, w;
	unsigned char *A, *N_array, *Y_origin;
  
  true_size = 0;
  temp = 0;
  
	Y_origin = (unsigned char *)malloc(sizeof(unsigned char)*size);
	N_array = (unsigned char *)malloc(sizeof(unsigned char)*size);
	memset(Y_origin, 0, sizeof(unsigned char)*size);

	for (i=0; i<size; ++i) {
		Y_origin[i] = Y[i];
	}

	for (i=size-1; i>=0; --i) {
		if (N[i] != 0) {
			true_size = i+1;
			break;
		}
	}
	if (true_size >= size-1) {
		printf("buffer too small!\n");
		exit(1);
	}
	// Compute X' = X * BETA^true_size mod N
	//		Step1: T_array = X * BETA^true_size
	for (k=0; k<true_size; ++k) {
		// Compute X' = X * BETA
 		for (i=true_size-1; i>=0; --i) {
			X[i+1] = X[i];
		}
		X[0] = 0;
		memset(N_array, 0, sizeof(unsigned char)*size);
		for (i=0; i<size; ++i) {
			N_array[i] = N[i];
		}
		//		Step2: X' = X' mod N 
		
		while (compare(X, N_array, sizeof(unsigned char)*size) == 1) {
			// N_array << 1
			carrier = 0;
			for (i=0; i<size; ++i) {
				temp = N_array[i];
				N_array[i] = ((temp<<1) + carrier)%BETA;
				carrier = (temp>>7);
			}
		}
		while (compare(X, N, sizeof(unsigned char)*size) != -1) {
			while (compare(X, N_array, sizeof(unsigned char)*size) == -1) {
				// N_array >> 1
				carrier = 0;
				for (i=size-1; i>=0; --i) {
					temp = N_array[i];
					N_array[i] = ((temp>>1) + carrier)%BETA;
					carrier = (temp<<7)%BETA;
				}
			}
			// X' -= N_array
			carrier = 0;
			for (i=0; i<size; ++i) {
				w = (int)X[i] - (int)N_array[i] - carrier;
				if (w < 0) {
					carrier = 1;
					X[i] = (unsigned char)(w + BETA);
				}
				else {
					carrier = 0;
					X[i] = (unsigned char)w;
				}
			}
		}
	}

	// Compute A = X'*Y/(BETA^true_size) mod N = X*Y mod N
	A = (unsigned char *)malloc(sizeof(unsigned char)*size);
	memset(A, 0, sizeof(unsigned char)*size);
	inv = InverseMod2power(BETA-N[0], 8);
	
	for (k=0; k<true_size; ++k) {
		q = ( (A[0]+X[k]*Y_origin[0])*inv % BETA );
		// A = A + xk * Y +q * N
		carrier = 0;
		for (i=0; i<size; ++i) {
			temp = A[i] + X[k]*Y_origin[i] + q*N[i] + carrier;
			if (temp > 0xff) {
				carrier = temp/BETA;
				A[i] = (unsigned char)(temp%BETA);
			}
			else {
				carrier = 0;
				A[i] = (unsigned char)temp;
			}
		}
		// A = A / BETA
		if (A[0] != 0) {
			printf("Error!\n");
			exit(1);
		}
		for (i=0; i<size-1; ++i) {
			A[i] = A[i+1];
		}
		A[size-1] = 0;
		//  0 =< A < N
//		while (1) {
			for (i=size-1; i>=0; --i) {
				if (A[i] > N[i]) {
					temp = 1;
					break;
				}
				if (N[i] > A[i]) {
					temp = 0;
					break;
				}
			}
			if (i < 0) {
				memset(A, 0, size);
//				break;
			}
			carrier = 0;
			if (temp == 1) {
				for (i=0; i<size; ++i) {
					w = (int)A[i] - (int)N[i] - carrier;
					if (w < 0) {
						carrier = 1;
						A[i] = (unsigned char)(w + BETA);
					}
					else {
						carrier = 0;
						A[i] = (unsigned char)w;
					}
				}
			}
//			else
//				break;
//		}
	}

	for (k=0; k<size; ++k) {
		X[k] = A[k];
	}
	free(A);
	free(N_array);
}

void UINT_to_HEX(unsigned char *uint, int size, char *hex)
/* uint : in
   size : in ( size of uint )
   hex  : out && in ( characters: 0~9 A~F ) */
{
	unsigned char ch;
	int i, j;
	for (i=size-1; i>=0; --i) {
		if (uint[i] > 0)
			break;
	}
	j = 0;
	ch = uint[i];
	if (ch < 0x0010) {
		if (ch<10 && ch>=0)
			hex[j] = (char)(ch+48);
		else if (ch<16 && ch>=10)
			hex[j] = (char)(ch+65-10);
		++j;
		--i;
	}
	for (; i>=0; --i) {
		ch = (unsigned char)(uint[i]>>4);
		if (ch<10 && ch>=0)
			hex[j] = (char)(ch+48);
		else if (ch<16 && ch>=10)
			hex[j] = (char)(ch+65-10);
		else {
			printf("UINT_to_HEX error!\n");
			exit(1);
		}
		ch = (unsigned char)(uint[i]%(1<<4));
		if (ch<10 && ch>=0)
			hex[++j] = (char)(ch+48);
		else if (ch<16 && ch>=10)
			hex[++j] = (char)(ch+65-10);
		else {
			printf("UINT_to_HEX error!\n");
			exit(1);
		}
		++j;
	}
	hex[j] = 0;
}

void HEX_to_UINT(char *hex, unsigned char *uint)
/* hex  : in
   uint : out && in (  uint = 0 ) */
{
	int i, mult, len = strlen(hex), j;
	for (i=0; i<len; ++i) {
		if (i%2 == 0) {
			mult = 1;
			uint[i/2] = 0;
		}
		else 
			mult = (1<<4);
		j = len-1-i;
		if (hex[j]<58 && hex[j]>=48)
			uint[i/2] += (hex[j]-48)*mult;
		else if (hex[j]<71 && hex[j]>=65) 
			uint[i/2] += (hex[j]-65+10)*mult;
		else {
			printf("HEX_to_UINT error!\n");
			exit(1);
		}
	}
}

int InverseMod2power(int n, int s) // return_value = n^(-1) mod 2^s
{
	int result = 1, i;
	for (i=1; i<s; i++) {
		if ( (1<<i) < (n*result)%(1<<(i+1)) ) {
			result += (1<<i);
		}
	}
	return result;
}

int compare(unsigned char *X, unsigned char *Y, int size) 
/* X > Y : 1
   X = Y : 0
   X < Y : -1 */
{
	int i;
	for (i=size-1; i>=0; --i) {
		if (X[i] > Y[i])
			return 1;
		if (X[i] < Y[i])
			return -1;
	}
	return 0;
}
