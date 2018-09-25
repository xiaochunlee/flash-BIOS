#ifndef _DATADEFINED_H

#define _DATADEFINED_H

#include <stdint.h>
//#define __DEBUG__  0 
//#define debug(format,...) do{if(__DEBUG__) fprintf(stdout, format, ##__VA_ARGS__);}while(0)
#ifdef __DEBUG__
#define debug(format,...) do{ fprintf(stdout, format, ##__VA_ARGS__);}while(0)
#else
#define debug(format, ...) 
#endif

#ifdef __DEBUG__ 
#define __INDEX_DEBUG__
//#define __DEBUG_1__  
#endif

#define __AUTHENTICATE__ 1

#ifndef __DEBUG_1__
#define __NEED_WRITE2__ 1
#endif
#define PAGE_SIZE 0x1000
#define FLASH_INFO_FLAG			1
#define FLASH_BLOCK_FLAG		2
#define FLASH_READ_FLAG			3
#define FLASH_WRITE_FLAG		4
#define FLASH_ALL_FLAG			5		
#define FLASH_PLATFORM_ROMMAP_FLAG	6
#define FLASH_END_FLAG			7	

#define RATE1	1
#define RATE2	2
#define RATE3   3	
#define RATENOCHANGE	4

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

int mem_addr_vir2phy(unsigned long vir, unsigned long *phy);
int get_content(int, int, void*, unsigned long);
int get_phy_addr(int, unsigned long* );
int send_data_to_module(unsigned long);
int get_module_fd();
int write_data_to_module(int fd, unsigned char *buffer, unsigned long size);
#endif
