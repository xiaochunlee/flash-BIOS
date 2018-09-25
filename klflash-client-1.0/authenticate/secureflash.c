#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "secureflash.h"
#define __DEBUG__ 1
#define debug(format,...) do{if(__DEBUG__) fprintf(stdout, format, ##__VA_ARGS__);}while(0)
#define SHA1_HASH_VALUE_SIZE    20
#define EFI_UINTN_ALIGN_MASK    (sizeof (size_t) - 1)
#define EFI_UINTN_ALIGNED(ptr)  (((size_t) (ptr)) & EFI_UINTN_ALIGN_MASK)

#define KL_SIGNATURE_MAGIC_SIZE		    16
typedef struct {
  uint8_t   Magic[KL_SIGNATURE_MAGIC_SIZE];   // Magic Code: "_KEF".    UINT32  Offset;
  uint32_t  HdrLen;                           // Length of this header
  uint32_t  BiosLen;                          // Length of the BIOS
  uint32_t  SignLen;                          // Length of the signature
  uint32_t  PubKeyLen;                        // Length of the publick key
} KUNLUN_BIOS_SIGNATURE_HEADER;



typedef struct  {
  uint32_t  Size;               // The size of the signature, in bytes
  uint32_t	Reserve;
} SIGNATURE_DESCRIPTOR;

typedef struct _SIGNATURE_PUBLIC_KEY {
  uint32_t RsaNSize;           // The size of RSAN, in bytes
  uint32_t  RsaESize;           // The size of RSAE, in bytes
  uint8_t *RsaN;
  uint8_t *RsaE;
} SIGNATURE_PUBLIC_KEY;

typedef struct _RSA_PUBLIC_KEY_DESCRIPTOR {
  uint32_t  RsaNSize;           // The size of RSAN, in bytes
  uint32_t  RsaESize;           // The size of RSAE, in bytes
  uint32_t  Reserve1;
  uint32_t  Reserve2;
} RSA_PUBLIC_KEY_DESCRIPTOR;

static int GetSignature(char *sign_area_addr, uint8_t **sign_buffer, uint32_t *sign_size)
{
	int status = 0;
	SIGNATURE_DESCRIPTOR *signature_desc = NULL;
	if (sign_area_addr == NULL || sign_size == 0) return 1;

	
	signature_desc = (SIGNATURE_DESCRIPTOR*)sign_area_addr;
	*sign_size = signature_desc->Size;
	
	*sign_buffer = (char *)(calloc)(1, *sign_size + 1);

	memcpy(*sign_buffer, (void*)(sign_area_addr + sizeof(SIGNATURE_DESCRIPTOR)), *sign_size);
	
	return 0;
	
}

static int GetSignPublicKey(char *pubkey_addr, SIGNATURE_PUBLIC_KEY *publickey)
{
	char * ptrbuf;
	RSA_PUBLIC_KEY_DESCRIPTOR *rsa_publicKey;

	if (pubkey_addr == NULL || publickey == NULL)
		return 1;

	rsa_publicKey = (RSA_PUBLIC_KEY_DESCRIPTOR *)pubkey_addr;
	publickey->RsaNSize = rsa_publicKey->RsaNSize;
	publickey->RsaESize = rsa_publicKey->RsaESize;

	publickey->RsaN = (char *)calloc(1, publickey->RsaNSize + 1);
	publickey->RsaE = (char *)calloc(1, publickey->RsaESize + 1);

	

	ptrbuf = (char *)(pubkey_addr + sizeof(RSA_PUBLIC_KEY_DESCRIPTOR));
	//memset(publickey->RsaN, 0,  publickey->RsaNSize + 1);
	memcpy(publickey->RsaN, ptrbuf, publickey->RsaNSize);

	ptrbuf = (void *)(pubkey_addr + sizeof(RSA_PUBLIC_KEY_DESCRIPTOR) + publickey->RsaNSize);
	//memset(publickey->RsaE, publickey->RsaESize + 1);
	memcpy(publickey->RsaE, ptrbuf, publickey->RsaESize);

	return 0;
	
}

static int GetPublicKey(char *pubkey_area_addr, SIGNATURE_PUBLIC_KEY *publickey)
{
	uint32_t status = 0;

  	status = GetSignPublicKey(pubkey_area_addr, publickey);

    return status;
}



static int DecodeSignDigest (
  SIGNATURE_PUBLIC_KEY     *SignPublicKey,
  uint8_t                    *StandDigest,
  uint32_t                   DigestSize,
  uint8_t                    **DecodeDigest,
  uint32_t                  *DecodeSize)
{

  *DecodeDigest = decrypt(SignPublicKey->RsaN, SignPublicKey->RsaE, StandDigest);
  *DecodeSize = EfiAsciiStrLen(*DecodeDigest);

  return 0;
}

static int DecodeSignature (SIGNATURE_PUBLIC_KEY *SignPublicKey,
  								uint8_t                    *Signature,
  								uint32_t                   SignatureSize,
 								uint8_t                    **DecodeDigest,
  								uint32_t                   *DecodeSize)
{
  size_t Status = 0;

  if (SignPublicKey == NULL || Signature == NULL || 
      DecodeDigest == NULL || DecodeSize == NULL) {
    return 1;
  }

  Status = DecodeSignDigest (SignPublicKey, Signature, SignatureSize, DecodeDigest, DecodeSize);

  return Status;
}

static int CalcHash (
  uint8_t                             *Data,
  uint32_t                            DataSize,
  uint8_t                             **HashValue,
  uint32_t                            *HashSize
  )
{
  size_t          Status;

  *HashSize = SHA1_HASH_VALUE_SIZE;

  *HashValue = (char *)calloc(1, SHA1_HASH_VALUE_SIZE + 1);

  sha1_csum(Data, DataSize, *HashValue);

  return 0;
}

/*++
EFI_STATUS
EFIAPI
RsaPkcsVerify (
  IN RSA_CONTEXT             *Ctx,
  IN INT                     Alg,
  IN UCHAR                   *Buf,
  IN UINT                    BufLen,
  IN UCHAR                   *SigBuf,
  IN UINT                    SigLen
  )
{

  if (rsa_pkcs1_verify( Ctx, Alg, Buf, BufLen, SigBuf, SigLen )) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;

}
--*/


static int Hash (
  uint8_t                             *ImageData,
  uint32_t                            DataSize,
  uint8_t                             **HashValue,
  uint32_t                            *HashSize  
  )
/*++
  
  Routine Description:
    Calc the hash valude of the ImageData using OEM's hash algorithm.
    
  Arguments:
    ImageData               - A pointer to the data to be hashed.
    DataSize                - Data size of the data to be hashed.
    HashValue               - A pointer to the hash value.
    HashSize                - Data size of the hash value.

  Returns:
    EFI_SUCCESS             - Hash operation is successful.
    
--*/
{
  size_t                   Status;

  Status = CalcHash (ImageData, DataSize, HashValue, HashSize);

  return Status;
}



static long EfiCompareMem (
  void     *MemOne,
  void     *MemTwo,
  size_t    Length
  )
/*++

Routine Description:

  Compares two memory buffers of a given length.

Arguments:

  MemOne - First memory buffer

  MemTwo - Second memory buffer

  Len    - Length of Mem1 and Mem2 memory regions to compare

Returns:

  = 0     if MemOne == MemTwo

--*/
{
  long  ReturnValue;

  if (!(EFI_UINTN_ALIGNED (MemOne) || EFI_UINTN_ALIGNED (MemTwo) || EFI_UINTN_ALIGNED (Length))) {
    //
    // If Destination/Source/Length are aligned do UINTN conpare
    //
    for (; Length > 0; Length -= sizeof (long), MemOne = (void *)((size_t)MemOne + sizeof (long)), MemTwo = (void *)((size_t)MemTwo + sizeof (long))) {
      if (*(long*)MemOne != *(long *)MemTwo) {
        break;
      }
    }
  }

  //
  // If Destination/Source/Length not aligned do byte compare
  //
  for (; Length > 0; Length--, MemOne = (void *)((size_t)MemOne + 1), MemTwo = (void *)((size_t)MemTwo + 1)) {
    ReturnValue = (long)(*(char *)MemOne - *(char *)MemTwo);
    if (ReturnValue != 0) {
      return ReturnValue;
    }
  }

  return 0;
}


static int ReleaseSignPublicKey (
  SIGNATURE_PUBLIC_KEY  *PublicKey
  )
/*++
  
  Routine Description:
    Get the Public Key of RSA algorithm

  Arguments:
    PubKeyAddr            - The start address of Public Key data in the FLASH.
    PublicKey             - A pointer to the RSA Public Key.

  Returns:
    EFI_SUCCESS             - Authenticate successfully.
    EFI_INVALID_PARAMETER   - PubKeyAddr or PublicKey pointer is invalid.
    
--*/
{

  if (PublicKey == NULL) {
    return 1;
  }

  if (PublicKey->RsaN) {
    free(PublicKey->RsaN);
  }
  
  if (PublicKey->RsaE) {
    free(PublicKey->RsaE);
  }

  return 1;
}


static int ReleasePublicKey (
  SIGNATURE_PUBLIC_KEY  *PublicKey
  )
/*++
  
  Routine Description:
    Release the memory allocated to the member of the SIGNATURE_PUBLIC_KEY struct, 
    which contains the Public Key of the signature algorithm.

  Arguments:
    PublicKey               - A pointer to the Public Key.

  Returns:
    EFI_SUCCESS             - Release successfully.
    EFI_INVALID_PARAMETER   - PublicKey parameter is invalid.
    
--*/
{
  size_t                Status;
  
  Status = ReleaseSignPublicKey (PublicKey);
  
  return Status;
}

size_t AuthenticateImage (uint8_t *ImageData,uint8_t *Valid)
{
  size_t	                        Status;
  SIGNATURE_PUBLIC_KEY              SignPublicKey;
  KUNLUN_BIOS_SIGNATURE_HEADER      *KlSignHeader;
  uint8_t                             *BiosAreaAddr;
  uint8_t                             *SignAreaAddr;
  uint8_t                             *PubKeyAreaAddr;
  uint8_t                             *SignatureBuf;
  uint32_t							  SignatureSize;
  uint8_t                             *DecodeHash;
  uint32_t                            DecodeHashSize;
  uint8_t                             *HashValue;
  uint32_t                            HashSize; 
  
  SignatureBuf = NULL;
  DecodeHash   = NULL;
  HashValue    = NULL;

 
  KlSignHeader   = (KUNLUN_BIOS_SIGNATURE_HEADER *)ImageData;
  BiosAreaAddr   = ImageData + KlSignHeader->HdrLen;
  SignAreaAddr   = BiosAreaAddr + KlSignHeader->BiosLen;
  PubKeyAreaAddr = SignAreaAddr + KlSignHeader->SignLen;

  if(KlSignHeader->HdrLen == 0 || KlSignHeader->HdrLen == 0) {
	debug("Not a  Authorized image\n");
	return 1;
  	}

  debug("HdrLen:0x%x,BiosLen:0x%x,SignLen:0x%x,PublicKeyLen:0x%x\n",KlSignHeader->HdrLen,KlSignHeader->BiosLen,
  	KlSignHeader->SignLen,KlSignHeader->PubKeyLen);
  // 
  // Step 1: Get signature from the signed bios image.
  // 
  Status = GetSignature (SignAreaAddr, &SignatureBuf, &SignatureSize);
  if (Status) {
    goto Error1;
  }
 
  // 
  // Step 2: Get the RSA public key from the signed bios image.
  // 
  Status = GetPublicKey(PubKeyAreaAddr, &SignPublicKey);


  if (Status) {
    goto Error;
  }

  // 
  // Step 3: Decode signature using Public Key
  // 
  Status = DecodeSignature(&SignPublicKey, SignatureBuf, SignatureSize, &DecodeHash, &DecodeHashSize);
  
  if (Status) {
    goto Error;
  }

  //
  // Step 4: Calc the SHA1 hash of BIOS
  //
  Status = Hash (BiosAreaAddr, KlSignHeader->BiosLen, &HashValue, &HashSize);
 
  if (Status) {
    goto Error;
  }

  //
  // Step 5: Compare the two hash values
  //
  *Valid = (char)(EfiCompareMem (DecodeHash, HashValue, HashSize) == 0);

Error:
  
  ReleasePublicKey (&SignPublicKey);
Error1:  
  if (SignatureBuf)
    free(SignatureBuf);
    SignatureBuf = 0;
 
  if (DecodeHash)
    free(DecodeHash);
  
  if (HashValue)
    free(HashValue);

  return Status;
}


size_t SecureFlashAuthentication(uint8_t *ImageData)
{
  size_t                	Status;
  uint8_t                   Valid = 0;


     Status=AuthenticateImage(ImageData,&Valid);

    if((!Status) &&((Valid)==1)){
		printf("Authorize Passed!\n");
        return 0;
    }else {
    	debug("Authorize Failed!\n");
      return Status;
    }  

}

int checkSecureFlashApp(FILE *infile)
{
	assert(infile);
	fseek(infile, 0, SEEK_END);
	uint32_t fsize = ftell(infile);
	printf("SechurFlash File size:%x\n",fsize);
	rewind(infile);

	char * buffer = (char *)calloc(1, fsize);
	int ret = fread(buffer, 1, fsize, infile);
	if (ret != fsize) 
		return 1;
	
	if(SecureFlashAuthentication(buffer)) {
		free(buffer);
		buffer = NULL;
		return 1;
	}	

	free(buffer);
	buffer = NULL;

	return 0;
}

#if 0 
int main (int argc, char **argv)
{
	if (argc < 2) { 
		fprintf(stderr,"Usage:%s fileimage.\n",argv[0]);
		return 1;
		}		
	FILE *infile = fopen(argv[1], "rb");
	fseek(infile, 0, SEEK_END);
	uint32_t fsize = ftell(infile);
	printf("file size:%x\n",fsize);
	rewind(infile);

	char * buffer = (char *)calloc(1, fsize);
	int ret = fread(buffer, 1, fsize, infile);
	if (ret != fsize) 
		return 1;
	
	SecureFlashAuthentication(buffer);

	free(buffer);

	return 0;
}
#endif
