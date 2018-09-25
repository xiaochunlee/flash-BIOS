#ifndef _DATADEFINED_H

#define _DATADEFINED_H

#define __DEBUG__ 1 
#define debug(format,...) do{if(__DEBUG__) fprintf(stdout, format, ##__VA_ARGS__);}while(0)

typedef unsigned char           uint8_t;  
typedef unsigned short          uint16_t;  
typedef unsigned int            uint32_t; 

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;

typedef signed char	s8;
typedef signed short	s16;
typedef signed int	s32;

#define FLASH_SIZE_128K     0x01
#define FLASH_SIZE_256K     0x02
#define FLASH_SIZE_512K     0x03
#define FLASH_SIZE_1024K    0x04
#define FLASH_SIZE_2048K    0x05
#define FLASH_SIZE_4096K    0x06
#define FLASH_SIZE_8192K    0x07
#define FLASH_SIZE_16384K   0x08

#define  DEFAULT_FLASH_DEVICE_TYPE   0

#define IHISI_DO_NOTHING     0x00
#define IHISI_SHOUTDOWN      0x01
#define IHISI_REBOOT         0x02

#define IHISI_DO_NOTHING     0x00
#define IHISI_SHOUTDOWN      0x01
#define IHISI_REBOOT         0x02

#define IHISI_SIGNATURE 0x02448324F

extern char errorInfo[100];

int mem_addr_vir2phy(unsigned long vir, unsigned long long *phy);
#endif
