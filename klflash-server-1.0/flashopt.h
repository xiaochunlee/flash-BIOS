#ifndef _FLASHOPT_H

#define _FLASHOPT_H

#include "data.h"
#pragma pack(1)

#define KL_SIGNATURE_MAGIC_SIZE		    16
typedef struct {
  uint8_t   Magic[KL_SIGNATURE_MAGIC_SIZE];   // Magic Code: "_KEF".    UINT32  Offset;
  uint32_t  HdrLen;                           // Length of this header
  uint32_t  BiosLen;                          // Length of the BIOS
  uint32_t  SignLen;                          // Length of the signature
  uint32_t  PubKeyLen;                        // Length of the publick key
} KUNLUN_BIOS_SIGNATURE_HEADER;


typedef struct  FBTS_FLASH_DEVICE{
  uint8_t  Size;
  char VendorName [31];
  char DeviceName [32];
  uint32_t Id;
  uint32_t SpecifiedSize;
}PartInfo ;

typedef struct  FD_BLOCK_MAP{
 uint16_t      BlockSize;
 uint16_t      Mutiple;
 uint16_t      EOS;
}PartBlock ;

typedef enum {
  FbtsRomMapPei = 0,
  FbtsRomMapCpuMicrocode,  //EFI_FLASH_AREA_CPU_MICROCODE
  FbtsRomMapNVRam,         //EFI_FLASH_AREA_EFI_VARIABLES
  FbtsRomMapDxe,           //DXE EFI_FLASH_AREA_MAIN_BIOS
  FbtsRomMapEc,            //EFI_FLASH_AREA_FV_EC
  FbtsLogo,                //
  FbtsRomMapNvStorage,     //EFI_FLASH_AREA_GUID_DEFINED
  FbtsRomMapFtwBackup,     //EFI_FLASH_AREA_FTW_BACKUP
  FbtsRomMapFtwState,      //EFI_FLASH_AREA_FTW_STATE
  FbtsRomMapSmbiosLog,     //EFI_FLASH_AREA_SMBIOS_LOG
  FbtsRomMapOemData,       //EFI_FLASH_AREA_OEM_BINARY
  FbtsRomMapGpnv,          //EFI_FLASH_AREA_GPNV
  FbtsRomMapDmiFru,        //EFI_FLASH_AREA_DMI_FRU
  FbtsRomMapPalB,          //EFI_FLASH_AREA_PAL_B
  FbtsRomMapMcaLog,        //EFI_FLASH_AREA_MCA_LOG
  FbtsRomMapPassword,      //EFI_FLASH_AREA_RESERVED_03
  FbtsRomMapOemNvs,        //EFI_FLASH_AREA_RESERVED_04
  FbtsRomMapReserved07,    //EFI_FLASH_AREA_RESERVED_07
  FbtsRomMapReserved08,    //EFI_FLASH_AREA_RESERVED_08
  FbtsRomMapReserved09,    //
  FbtsRomMapReserved0A,    //EFI_FLASH_AREA_RESERVED_0A
  FbtsRomMapUnused,        //EFI_FLASH_AREA_UNUSED
  FbtsRomMapFactoryCopy,   //EFI_FLASH_AREA_RESERVED_09
  FbtsRomMapUndefined,     //
  FbtsRomMapEos = 0xff,    //

} FBST_ROM_MAP_CODE;

typedef struct  {
  uint8_t                                 Type;
  uint32_t                                Address;
  uint32_t                                Length;
} FBTS_PLATFORM_ROM_MAP;

typedef struct FBTS_PLATFORM_ROM_MAP_BUFFER {
  FBTS_PLATFORM_ROM_MAP                 PlatFormRomMap [40];
}PlatformRomMapBuffer ;

typedef struct {
		uint32_t                                LinearAddress;
		uint32_t                                Size;
} FBTS_PLATFORM_PRIVATE_ROM;

typedef struct FBTS_PLATFORM_PRIVATE_ROM_BUFFER{
  FBTS_PLATFORM_PRIVATE_ROM             PlatFormRomMap [40];
} PrivateRomMapBuffer;
#pragma pack()

uint8_t getFlashPartInfo(PartInfo*, PartBlock*, uint8_t,uint16_t);

//uint8_t writeFlash(uint8_t *buffer, uint32_t writeSzie, uint32_t falshAddress, uint16_t smiPort);

//uint8_t readFlash(uint8_t *buffer, uint32_t readSzie, uint32_t falshAddress, uint16_t smiPort);
uint8_t writeFlash(unsigned long buffer, uint32_t writeSzie, uint32_t falshAddress, uint16_t smiPort);
uint8_t readFlash(unsigned long buffer, uint32_t readSzie, uint32_t falshAddress, uint16_t smiPort);

uint8_t completeFalsh(uint32_t command, uint16_t smiPort);

uint8_t getFlashRomMap(PlatformRomMapBuffer*, PrivateRomMapBuffer*,  uint16_t smiPort );

uint8_t checkSecureFlashBB ( uint8_t* flashRomInMem, uint16_t smiPort, uint32_t BbRecovery );

uint8_t checkSecureFlashALL ( uint8_t* flashRomInMem, uint16_t smiPort);

uint8_t GetOA3Status (  uint16_t smiPort );
uint8_t GetSecureBIOSStatus (  uint16_t smiPort );
/**
 * @brief 处理/O
 * @return
 */
int readFlashToFile(const char *);

/**
 * @brief  处理 /ALL 以及其他的OPTION，如/R的组合
 * @return
 */
int writeFileToFlash(const char*, int r, uint8_t *buffer);

/**
 * @brief 处理/P
 * @return
 */
int  writeFileToMainBlock(const char*, uint8_t *buffer);

/**
 * @brief  处理/B
 * @return
 */
int writeFileToBootBlock(const char*,  int r , uint8_t *buffer);

int writeFileToRomHoles(const char*,  int r , uint8_t *buffer);
/**
 * @brief 处理/N
 * @return
 */
int writeFileToNVROMBlock(const char*, uint8_t *buffer);

/**
 * @brief 处理/CLNEVENLOG
 * @return
 */
int clearEventLog();

/**
 * @brief 完成后的操作
 * @param type =1 关机；=2 重启
 * @return 
 */
int  executeAfterFalsh(int type);;

int writeFileToOA2Block(const char*fileName, uint8_t *buffer);

#endif
