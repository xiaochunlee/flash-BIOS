#include "flashopt.h"
#include "base.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include "data.h"
#include "smbios.h"

extern unsigned long phy_partinfo;
extern unsigned long phy_partblock;
extern unsigned long buffer_phy_rdwr;
extern unsigned long platform_rommap_phyaddr; 
extern unsigned long private_rommap_phyaddr; 

extern int module_fd;
extern uint8_t SecureFlash;
extern uint8_t SecureBios;
extern uint32_t flashSize;
static int rate_flag = 0;
#ifdef __DOS__
//#define infile "/opt/mm_addr-master/dram"
//#define outfile "/opt/mm_addr-master/aa"
int myopen(int *fd, char **buffer)
{
	fd[0] = open("/opt/mm_addr-master/dram", O_RDONLY);
	if (fd[0] < 0) {
		perror("open");
		return 1;
	}
	fd[1] = open("./bb", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd[1] < 0) {
		perror("open");
		close(fd[0]);
		return 1;
	}
	*buffer = (char *) malloc(PAGE_SIZE);
	fprintf(stdout, "The cpy_content buffer is:%p\n", *buffer);

	return 0;
}

void myclose(int *fd, char *buffer)
{
	close(fd[0]);
	close(fd[1]);
	free(buffer);
	buffer = 0;

}

int cpy_content(int *fd, char *buffer, unsigned long long offset)
{
	unsigned long long location = 0;
	//memset(buffer, 0, sizeof(PAGE_SIZE));
	location = lseek64(fd[0], offset, SEEK_SET);
	read(fd[0], buffer, PAGE_SIZE);
	write(fd[1], buffer, PAGE_SIZE);
	return 0;

}
#endif

void setcur(uint16_t STATUS)
{

#ifdef __DOS__
	__asm {
	PUSH ECX PUSH EAX MOV CX, STATUS MOV AH,
		    1 INT 10 H POP EAX POP ECX}
#elif  defined __i386__
	if (STATUS) {
		system("clear");
		system("stty icanon");
		system("stty echo");

		//exit(1);
	} else {
		system("clear");
		system("stty -icanon");
		system("stty -echo");

		//exit(1);
	}

#endif				/*  */
}

uint8_t getFlashPartInfo(PartInfo * partInfo, PartBlock * partBlock,
			 uint8_t flashTypeSelect, uint16_t smiPort)
{

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov cl, flashTypeSelect
		    mov edi, partInfo
		    mov esi, partBlock
		    mov ebx, IHISI_SIGNATURE
		    mov ax, 13EF h
		    mov dx, smiPort
		    out dx, al pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__)|| defined(__x86_64__)
/*32位系统下面物理地址有可能大于32位，所以要用long long 型，但是下面到寄存器是32位的，所以要和下层SMI接口商量这个问题，
 * 为了测试，现在先用long型*/
//      unsigned long long phy_partinfo = 0;
//      unsigned long long phy_partblock = 0;
//	unsigned long phy_partinfo = 0;
//	unsigned long phy_partblock = 0;
/*    
    if (mem_addr_vir2phy((unsigned long) partInfo, &phy_partinfo)
	|| mem_addr_vir2phy((unsigned long) partBlock, &phy_partblock)) {
	fprintf(stderr,
		"Error:Not find the physical address of partinfo or partblock\n");
	return 1;
    }
 */
/*

	if (get_phy_addr(FLASH_BLOCK_FLAG, &phy_partblock)) {
		fprintf(stderr, "Get Phyaddr Failed\n");
		return 1;
	}
*/
/*	
	phy_partinfo = phy_partblock + 8;
    debug("The partinfo phy_addr is:%p\n"
	  "the partinfo vir-addr is:%p\n"
	  "the partblock phy_addr is:%p\n"
	  "the partblock vir_addr is:%p\n", (void *) phy_partinfo,
	  (void *) partInfo, (void *) phy_partblock, (void *) partBlock);
*/
#if 1
	__asm__ __volatile__(	//PUSH_REGS 
				    "mov $0x13ef, %%ax\n\t"
				    "outb %%al, %%dx\n\t"
				    //POP_REGS
				    ::"b"(IHISI_SIGNATURE),
				    "c"(flashTypeSelect), "d"(smiPort),
				    "S"(phy_partblock), "D"(phy_partinfo)
				    :"eax", "memory");

#endif				/*  */
#endif				/*  */
	return 0;
}

uint8_t writeFlash(unsigned long buffer, uint32_t writeSzie,
		   uint32_t falshAddress, uint16_t smiPort)
{

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov esi, buffer
		    mov ecx, writeSzie
		    mov edi, falshAddress
		    mov ebx, IHISI_SIGNATURE
		    mov ax, 15EF h
		    mov dx, smiPort
		    out dx, al pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__) || defined(__x86_64__)
	__asm__ __volatile__("mov $0x15ef, %%ax\n\t"
			     "out %%al, %%dx\n\t"::"b"(IHISI_SIGNATURE),
			     "c"(writeSzie), "d"(smiPort),
			     "D"(falshAddress), "S"(buffer)
			     :"eax", "memory");

#endif				/*  */
	return 0;
}

uint8_t readFlash(unsigned long buffer, uint32_t readSzie,
		  uint32_t falshAddress, uint16_t smiPort)
{

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov esi, buffer
		    mov ecx, readSzie
		    mov edi, falshAddress
		    mov ebx, IHISI_SIGNATURE
		    mov ax, 14EF h
		    mov dx, smiPort
		    out dx, al pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__) || defined(__x86_64__)
	__asm__ __volatile__(
				    //              PUSH_REGS
				    "mov $0x14ef, %%eax\n\t"
				    "out %%al, %%dx\n\t"
				    //              POP_REGS
				    ::"b"(IHISI_SIGNATURE), "c"(readSzie),
				    "d"(smiPort), "D"(falshAddress),
				    "S"(buffer):"eax", "memory");

#endif				/*  */
	return 0;
}

uint8_t completeFalsh(uint32_t command, uint16_t smiPort)
{

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov ecx, command
		    mov ebx, IHISI_SIGNATURE
		    mov ax, 16EF h
		    mov dx, smiPort
		    out dx, al pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__) || defined(__x86_64__)
	__asm__ __volatile__(	//PUSH_REGS
				    "mov $0x16ef, %%eax\n\t"
				    "out %%al, %%dx\n\t"
				    //POP_REGS
				    ::"b"(IHISI_SIGNATURE), "c"(command),
				    "d"(smiPort):"eax");

#endif				/*  */
	return 0;
}

uint8_t getFlashRomMap(PlatformRomMapBuffer * platformRomMap,
		       PrivateRomMapBuffer * privateRomMap,
		       uint16_t smiPort)
{

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov edi, platformRomMap
		    mov esi, privateRomMap
		    mov ebx, IHISI_SIGNATURE
		    mov ax, 12EF h
		    mov dx, smiPort
		    out dx, al pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__) || defined(__x86_64__)
#if 0
	unsigned long platform_rommap_phyaddr = 0;
	unsigned long private_rommap_phyaddr = 0;
	if (mem_addr_vir2phy
	    ((unsigned long) platformRomMap, &platform_rommap_phyaddr)
	    || mem_addr_vir2phy((unsigned long) privateRomMap,
				&private_rommap_phyaddr)) {
		fprintf(stderr,
			"Error:Not find the physical address of platformRomMap and privateRomMap\n");
		exit(1);
	}
#endif	
	__asm__ __volatile__(	//PUSH_REGS
				    "mov $0x12ef, %%eax\n\t"
				    "out %%al, %%dx\n\t"
				    //POP_REGS
				    ::"b"(IHISI_SIGNATURE), "d"(smiPort),
				    "D"(platform_rommap_phyaddr),
				    "S"(private_rommap_phyaddr):"eax");

#endif				/*  */
	return 0;
}

uint8_t checkSecureFlashBB(uint8_t * flashRomInMem, uint16_t smiPort,
			   uint32_t BbRecovery)
{
	uint8_t result = 1;

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov ecx, flashRomInMem
		    mov ebx, IHISI_SIGNATURE
		    mov esi, BbRecovery
		    mov ax, 1EEF h
		    mov dx, smiPort
		    out dx, al
		    mov result, al pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__) || defined(__x86_64__)
	__asm__ __volatile__(	//PUSH_REGS
				    "mov $0x1eef, %%eax\n\t"
				    "out %%al, %%dx\n\t" "mov %%al, %0\n\t"
				    //POP_REGS
				    :"=a"(result):"b"(IHISI_SIGNATURE),
				    "c"(flashRomInMem), "d"(smiPort),
				    "S"(BbRecovery):);

#endif				/*  */
	return result;
}

uint8_t checkSecureFlashALL(uint8_t * flashRomInMem, uint16_t smiPort)
{
	uint8_t result = 1;

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov ecx, flashRomInMem
		    mov ebx, IHISI_SIGNATURE
		    mov ax, 1 DEFh
		    mov dx, smiPort
		    out dx, al
		    mov result, al pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__) || defined(__x86_64__)
	__asm__ __volatile__(	//PUSH_REGS 
				    "mov $0x1def, %%eax\n\t"
				    "out %%al, %%dx\n\t" "mov %%al, %0\n\t"
				    //POP_REGS
				    :"=a"(result):"b"(IHISI_SIGNATURE),
				    "c"(flashRomInMem), "d"(smiPort):);

#endif				/*  */
	return result;
}

uint8_t GetOA3Status(uint16_t smiPort)
{
	uint8_t result = 0;

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov ebx, 13 h
		    mov eax, 534 d0740h
		    mov dx, smiPort
		    out dx, al
		    mov result, dl pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__) || defined(__x86_64__)
	__asm__ __volatile__(	//PUSH_REGS 
				    "mov $0x13, %%ebx\n\t"
				    "mov $0x534d0740, %%eax\n\t"
				    "out %%al, %%dx\n\t" "mov %%dl, %0\n\t"
				    //POP_REGS
				    :"=d"(result):"d"(smiPort):"eax",
				    "ebx");

#endif				/*  */
	return result;
}

uint8_t GetSecureBIOSStatus(uint16_t smiPort)
{
	uint8_t result = 0;

#ifdef __DOS__
	__asm {
	push ebx
		    push ecx
		    push edx
		    push edi
		    push esi
		    mov ebx, IHISI_SIGNATURE
		    mov ax, 1F EFh
		    mov dx, smiPort
		    out dx, al
		    mov result, al pop esi pop edi pop edx pop ecx pop ebx}
#elif  defined (__i386__) || defined(__x86_64__)
	__asm__ __volatile__(
				    //PUSH_REGS 
				    "mov $0x1fef, %%eax\n\t"
				    "out %%al, %%dx\n\t" "mov %%al, %0\n\t"
				    //POP_REGS
				    :"=a"(result):"b"(IHISI_SIGNATURE),
				    "d"(smiPort)
	    );

#endif				/*  */
	return result;
}

int needWrite(uint8_t * buffer, uint32_t writeSzie,
	      uint32_t falshAddress, uint16_t smiPort,
	      char *localbuffer, unsigned long local_buffer_phy_addr)
{
	uint32_t index;
	uint32_t count;
#if 0
	count = writeSzie % 4;
	if (count != 0) {
		for (index = 0; index < count; index++) {
			if (localbuffer[writeSzie - 1 - index] !=
			    buffer[writeSzie - 1 - index]) {
				return 1;
			}
		}
	}
#endif
	readFlash(local_buffer_phy_addr, writeSzie, falshAddress, 0x82F);
//    count = (writeSzie & 0xfffffffc) / 4;
	count = writeSzie >> 2;
	for (index = 0; index < count; index++) {
		if (*((uint32_t *) localbuffer + index) !=
		    *((uint32_t *) buffer + index)) {
			return 1;
		}
	}
	return 0;
}

int readFlashToFile(const char *fileName, char *rate)
{
	PartInfo *partInfo = (PartInfo *) memalign(0x80, sizeof(PartInfo));
	PartBlock *partBlock =
	    (PartBlock *) memalign(0x8, sizeof(PartBlock));
	memset(partInfo, 0, sizeof(partInfo));
	memset(partBlock, 0, sizeof(partBlock));
	int rst, readTime, flashSize;
	uint32_t size;
	char str[5];
	int i;
	uint8_t *buffer = 0;
	uint32_t begin;
	FILE *outfile;

//      if(mlockall(MCL_CURRENT|MCL_FUTURE)) {
//              perror("mlockall\n");//防止交换到硬盘 
//              return -1;
//      }
	if (mlock(partInfo, sizeof(partInfo))
	    && mlock(partBlock, sizeof(partBlock))) {
		perror("mlock\n");
		return 1;
	}
	rst =
	    getFlashPartInfo(partInfo, partBlock,
			     DEFAULT_FLASH_DEVICE_TYPE, 0x82F);
	if (rst) {
		return 1;
	}
	__asm__ __volatile__("":::"memory");

	if (get_content(module_fd, FLASH_BLOCK_FLAG, partBlock, sizeof(PartBlock))) {
		fprintf(stderr, "Get Content Failed\n");
		return 1;
	}

	flashSize = partBlock->BlockSize * partBlock->Mutiple * 256;
#ifdef __DEBUG__
	uint16_t blocksize = 0;
	blocksize = partBlock->BlockSize;
	debug("the blocksize is:0x%x,mutiple is:0x%x,flashSize is:0x%x\n",
        partBlock->BlockSize, partBlock->Mutiple, flashSize);
#endif	      
	munlock(partInfo, sizeof(partInfo));
	munlock(partBlock, sizeof(partBlock));
	free (partBlock);
	free (partInfo);

//      munlockall();
//    if (blocksize == 0x10000) {
//      readTime = flashSize / 0x10000;
//      size = (uint32_t) 0x10000;      //按照64K来操作
//   } else {
	readTime = flashSize / 0x1000;
	size = (uint32_t) 0x1000;	//按照4K来操作,而且必须按照4K来操作，因为大于4k后申请到物理内存可能不连续。
//    }
	outfile = fopen(fileName, "wb");
	if (!outfile) {
		strcpy(errorInfo, "file open error,error code:");
		sprintf(str, "%d", errno);
		strcat(errorInfo, str);
		return 1;
	}
	buffer = (uint8_t *) valloc(size);	//物理地址连续吗？
	if (!buffer) {
		perror("valloc\n");
		rst = 1;
		goto out1;
	}
	memset(buffer, 0, size);
	begin = 0xFFFFFFFF - flashSize + 1;

	printf("\n");
/*	
	unsigned long buffer_phy_addr = 0;
	if (mem_addr_vir2phy((unsigned long) buffer, &buffer_phy_addr)) {
		fprintf(stderr,
			"ERROR:Can not find the buffer_phy_addr\n");
		rst = 1;
		goto out;
	}
*/	

	/*循环中缺少错误判断 */
	for (i = 0; i < readTime; ++i) {
		*rate = (i + 1) * 100 / readTime;
		fprintf(stderr, "\r Reading...%d%%    ", (i + 1) * 100 / readTime);
		readFlash(buffer_phy_rdwr, size, begin + i * size, 0x82F);
		if (get_content(module_fd, FLASH_WRITE_FLAG, buffer, size) == 1) {
			fprintf(stderr, "get data error\n");
			rst = 1;
			goto out;
		}
		fwrite(buffer, size, 1, outfile);

		//sleep(1);
		//memset(buffer, 0, size); //清零，保证数据正确性
	}
	printf("\n");

	rst = 0;
out:
	free(buffer);
	buffer = 0;
out1:	
	fclose(outfile);
	outfile = 0;
	return rst;
}

int writeFileToFlash(const char *fileName, int r, uint8_t * buffer,
		     char *rate, unsigned long buffer_phy_addr)
{
	uint32_t mbAddress, mbOffset = 0;
	uint32_t mbLength;
	uint32_t readTime, i;
	uint32_t size;
	FILE *infile;
	uint32_t begin;
	char str[5];
	//int lSize;
	size_t result;
	uint32_t sizeflag = 0;
	long file_offset = 0;
	size = PAGE_SIZE;
	readTime = flashSize / size;
	infile = fopen(fileName, "rb");
	if (!infile) {
		strcpy(errorInfo,
		       "file open error,check the file name?error code is:");
		sprintf(str, "%d", errno);
		strcat(errorInfo, str);
		return 1;
	}
	// 获取文件大小
//      fseek(infile, 0, SEEK_END);
//      lSize = ftell(infile);
//      rewind(infile);

	// 将文件拷贝到buffer中,在上层函数已经对buffer分配来空间，但是这个分配到空间物理页到这儿会换出吗？
	// 读完后此时虚拟地址buffer后面到8M存放来文件数据，但是这8M空间在物理地址上是连续到吗？
	if (SecureFlash || SecureBios) {
		fseek(infile, sizeof(KUNLUN_BIOS_SIGNATURE_HEADER),
		      SEEK_SET);
//              lSize = flashSize;
		result = fread(buffer, 1, PAGE_SIZE, infile);
	} else {
		/*第一次读取内容 */
		result = fread(buffer, 1, PAGE_SIZE, infile);
	}
	if (result != PAGE_SIZE) {
		fclose(infile);
		strcpy(errorInfo, "reading rom file to memory error");
		sprintf(str, "%d", errno);
		strcat(errorInfo, str);
		return 1;
	}

	begin = 0xFFFFFFFF - flashSize + 1;

	//处理/R OPTIO
	if (r) {
		getSliceInfo(FbtsRomMapDmiFru, &mbAddress, &mbLength);
		mbOffset = mbAddress - (0xFFFFFFFF - flashSize + 1);
		debug("mbOffset=0x%x,mbLength=0x%x mbaddress=0x%x\n",
		      mbOffset, mbLength, mbAddress);
#if 0
		if (SecureFlash || SecureBios) {

			for (i = 0; i < mbLength / 0x1000; i++) {
				readFlash(buffer_phy_addr +
					  sizeof
					  (KUNLUN_BIOS_SIGNATURE_HEADER) +
					  mbOffset + i * 0x1000, 0x1000,
					  mbAddress + i * 0x1000, 0x82F);
			}
			/*移动位置 */
			readFlash(buffer_phy_addr +
				  sizeof(KUNLUN_BIOS_SIGNATURE_HEADER)
				  + 0x1D1, 0xE2F, begin + 0x1D1, 0x82f);
		} else {

			for (i = 0; i < mbLength / 0x1000; i++) {
				readFlash(buffer_phy_addr + mbOffset +
					  i * 0x1000, 0x1000,
					  mbAddress + i * 0x1000, 0x82F);
			}
			/*本次读取的内容是保留的部分，因这段空间是第一个4K部分，故先读入到前4K中,会覆盖以前内容，然后再烧进去，就保留这段空间了 */
			readFlash(buffer_phy_addr + 0x1D1, 0xE2F,
				  begin + 0x1D1, 0x82f);
		}
#endif
		/*本次读取的内容是保留的部分，因这段空间是第一个4K部分，故先读入到前4K中,会覆盖以前内容，然后再烧进去，就保留这段空间了 */
		readFlash(buffer_phy_addr + 0x1D1, 0xE2F, begin + 0x1D1,
			  0x82f);
		file_offset = mbLength + mbOffset;
	}
#ifdef __NEED_WRITE2__
	char *localbuffer = (char *) valloc(PAGE_SIZE);
	memset(localbuffer, 0, PAGE_SIZE);
	unsigned long local_buffer_phy_addr = 0;
	if (mem_addr_vir2phy
	    ((unsigned long) localbuffer, &local_buffer_phy_addr)) {
		fprintf(stderr,
			"ERROR:Can not find the local_buffer_phy_addr\n");
		return 1;
	}
#endif

	//setcur(HIDE);
	printf("\n");
#ifdef __DEBUG_1__
	int fdrw[2] = { 0 };
	char *tmpbuffer = NULL;
	myopen(fdrw, &tmpbuffer);
#endif
	for (i = 0; i < readTime; i++) {
		*rate = (i + 1) * 100 / readTime;
		fprintf(stderr, "\r Flashing...%-3d%%", *rate);
		if (r) {
			sizeflag = (i + 1) * size;
			/*file_offset如果不是4K整数倍就要按照注释部分来,目前看是4k到整数倍 */
			if (sizeflag > mbOffset && sizeflag <= file_offset) {
#ifdef __DEBUG_1__
				cpy_content(fdrw, tmpbuffer,
					    buffer_phy_addr);
#endif
				/*文件向后偏移*/
				fread(buffer, 1, PAGE_SIZE, infile);
				continue;

#if 0
				if (sizeflag >= file_offset) {
					if (SecureFlash || SecureBios) {
						fseek(infile,
						      file_offset +
						      sizeof
						      (KUNLUN_BIOS_SIGNATURE_HEADER),
						      SEEK_SET);
					} else {
						fseek(infile, file_offset,
						      SEEK_SET);
					}

					fread(buffer, 1, PAGE_SIZE,
					      infile);
				}
				continue;
#endif
			}
		}
#if 0
		if (SecureFlash || SecureBios) {

			if (needWrite
			    (buffer +
			     sizeof(KUNLUN_BIOS_SIGNATURE_HEADER) +
			     i * size, size, begin + i * size,
			     0x82F) == 0) {

				//printf("needn't flash!\n");
				continue;
			}
			/*物理地址到buffer_phy_addr+header+offset =? 虚拟地址buffer对应到buffer+header+offset ??? */
			writeFlash(buffer_phy_addr +
				   sizeof(KUNLUN_BIOS_SIGNATURE_HEADER)
				   + i * size, size, begin + i * size,
				   0x82F);
		} else {

			/*文件中的内容和bios中到内容是否一致来决定是否烧写，函数中平凡到分配释放内存，和转换地址，考虑效率问题，删掉也可改造 */
			if (needWrite
			    (buffer + i * size, size, begin + i * size,
			     0x82F) == 0) {

				//printf("needn't flash!\n");
				continue;
			}
		}
#endif

#ifdef __DEBUG_1__
		cpy_content(fdrw, tmpbuffer, buffer_phy_addr);
#endif

#ifdef __NEED_WRITE2__
		if (needWrite
		    (buffer, size, begin + i * size, 0x82F, localbuffer,
		     local_buffer_phy_addr) == 0) {
			debug("the %-4d page needn't flash!\n", i + 1);
			result = fread(buffer, 1, PAGE_SIZE, infile);
			if (!feof(infile) && (result != PAGE_SIZE)) {
				goto fail;
			}
			continue;
		}
#endif

		write_data_to_module(module_fd, buffer, size); 

		writeFlash(buffer_phy_addr, size, begin + i * size, 0x82F);
		result = fread(buffer, 1, PAGE_SIZE, infile);
		/*如果没有到文件尾部而读出的大小不对，就认为错。如果到文件尾部，就由for循环来判断退出 */
		if (!feof(infile) && (result != PAGE_SIZE)) {
			goto fail;
		}
	}
/*正常退出*/
	printf("\n");
#ifdef __NEED_WRITE2__
	free(localbuffer);
	localbuffer = 0;
#endif
#ifdef __DEBUG_1__
	myclose(fdrw, tmpbuffer);
#endif
	fclose(infile);
	return 0;

      fail:
#ifdef __NEED_WRITE2__
	free(localbuffer);
	localbuffer = 0;
#endif
#ifdef __DEBUG_1__
	myclose(fdrw, tmpbuffer);
#endif
	fclose(infile);
	return 1;
}

int writeFileToMainBlock(const char *fileName, uint8_t * buffer,
			 char *rate, unsigned long buffer_phy_addr)
{
	int rst;
	uint32_t mbAddress;
	uint32_t mbLength;
	getSliceInfo(FbtsRomMapDxe, &mbAddress, &mbLength);

	//mbAddress = mbAddress -0xFF800000;
	//printf("mbAddress=%x, mbLength=%x\n",mbAddress,mbLength);
	rst =
	    writeFileToFlashBySlice(fileName, mbAddress, mbLength, buffer,
				    rate, RATE3, buffer_phy_addr);
	return rst;
}

int writeFileToBootBlock(const char *fileName, int r, uint8_t * buffer,
			 char *rate, unsigned long buffer_phy_addr)
{
	int rst;
	uint32_t mbAddress;
	uint32_t mbLength;

	getSliceInfo(FbtsRomMapPei, &mbAddress, &mbLength);
	rst =
	    writeFileToFlashBySlice(fileName, mbAddress, mbLength, buffer,
				    rate, RATE1, buffer_phy_addr);
	if (rst) 
		return rst;

	/*先读取一页大小，后面会从文件中读入相应到部分来覆盖第一页中到某些内容，用来保留前一页中相应到部分 */
	readFlash(buffer_phy_addr, PAGE_SIZE, 0xFFFFFFFF - flashSize + 1,
		  0x82f);

	if (get_content(module_fd, FLASH_WRITE_FLAG, buffer, PAGE_SIZE) == 1) {
		fprintf(stderr, "get data error\n");
		return 1;
	}

	rst =
	    writeFileToFlashBySlice(fileName, 0xFFFFFFFF - flashSize + 1,
				    0x1D1, buffer, rate, RATENOCHANGE, buffer_phy_addr);
	if (rst)
		return rst;
	if (!r) {		//reserve DMI info in BB
		rst =
		    writeFileToFlashBySlice(fileName,
					    0xFFFFFFFF - flashSize + 1 +
					    0x1D1, 0xE2F, buffer, rate, RATENOCHANGE,
					    buffer_phy_addr);
		if (rst)
			return rst;
	}
	
	rst =
	    writeFileToFlashBySlice(fileName,
				    0xFFFFFFFF - flashSize + 1 + 0x1000,
				    0x4000, buffer, rate, RATE2, buffer_phy_addr);

	return rst;
}

int writeFileToRomHoles(const char *fileName, int r, uint8_t * buffer,
			char *rate, unsigned long buffer_phy_addr)
{
	int rst = 0;
	int i, j, number;
	uint32_t mbAddress[4];
	uint32_t mbLength[4];
	uint32_t temAddress, temLength;
	uint32_t romholeAddress;
	uint32_t romholesize;
	getSliceInfo(FbtsRomMapPei, &mbAddress[0], &mbLength[0]);
	getSliceInfo(FbtsRomMapDxe, &mbAddress[1], &mbLength[1]);
	getSliceInfo(FbtsRomMapNVRam, &mbAddress[2], &mbLength[2]);
	getSliceInfo(FbtsRomMapDmiFru, &mbAddress[3], &mbLength[3]);
	if (r) {
		number = 4;
	} else {
		number = 3;
	}
	for (i = 0; i < number; i++) {
		temAddress = mbAddress[i];
		for (j = i + 1; j < number; j++) {
			if (mbAddress[j] < temAddress) {
				temAddress = mbAddress[j];
				temLength = mbLength[j];
				mbAddress[j] = mbAddress[i];
				mbLength[j] = mbLength[i];
				mbAddress[i] = temAddress;
				mbLength[i] = temLength;
			}
		}
	}
#ifdef __DEBUG__
	for (i = 0; i < number; i++) {
		debug("mbAddress[%d]=%x mbLength[%d]=%x\n", i,
		      mbAddress[i], i, mbLength[i]);
	}
#endif	
	romholeAddress = 0xFFFFFFFF - flashSize + 1 + 0x5000;
	romholesize = mbAddress[0] - romholeAddress;

	debug("romholeAddress1=%x romholesize=%x\n", romholeAddress,
	      romholesize);
	rst =
	    writeFileToFlashBySlice(fileName, romholeAddress, romholesize,
				    buffer, rate, RATE1, buffer_phy_addr);
	if (rst)
		return rst;

	romholeAddress = mbAddress[0] + mbLength[0];
	romholesize = mbAddress[1] - romholeAddress;

	debug("romholeAddress2=%x romholesize=%x\n", romholeAddress,
	      romholesize);
	rst =
	    writeFileToFlashBySlice(fileName, romholeAddress, romholesize,
				    buffer, rate, RATENOCHANGE, buffer_phy_addr);
	if (rst)
		return rst;

	romholeAddress = mbAddress[1] + mbLength[1];
	romholesize = mbAddress[2] - romholeAddress;

	debug("romholeAddress3=%x romholesize=%x\n", romholeAddress,
	      romholesize);
	rst =
	    writeFileToFlashBySlice(fileName, romholeAddress, romholesize,
				    buffer, rate, RATE2, buffer_phy_addr);
	if (rst)
		return rst;

	if (r) {
		romholeAddress = mbAddress[2] + mbLength[2];
		romholesize = mbAddress[3] - romholeAddress;

		debug("romholeAddress4=%x romholesize=%x\n",
		      romholeAddress, romholesize);
		rst =
		    writeFileToFlashBySlice(fileName, romholeAddress,
					    romholesize, buffer, rate, RATE2,
					    buffer_phy_addr);
		if (rst)
			return rst;

		romholeAddress = mbAddress[3] + mbLength[3];
		romholesize = 0xFFFFFFFF - romholeAddress + 1;
		rst =
		    writeFileToFlashBySlice(fileName, romholeAddress,
					    romholesize, buffer, rate,RATENOCHANGE,
					    buffer_phy_addr);
		if (rst)
			return rst;

	} else {
		romholeAddress = mbAddress[2] + mbLength[2];
		romholesize = 0xFFFFFFFF - romholeAddress + 1;
		rst =
		    writeFileToFlashBySlice(fileName, romholeAddress,
					    romholesize, buffer, rate, RATENOCHANGE,
					    buffer_phy_addr);
		if (rst)
			return rst;

	}

	//rst = writeFileToFlashBySlice(fileName, mbAddress, mbLength, buffer);

	//rst = writeFileToFlashBySlice(fileName, 0xFFFFFFFF - flashSize + 1 , 0x1D1, buffer);
	//if(!r){//reserve DMI info in BB
	//    rst = writeFileToFlashBySlice(fileName, 0xFFFFFFFF - flashSize + 1 + 0x1D1, 0xE2F, buffer);
	//}

	//rst = writeFileToFlashBySlice(fileName, 0xFFFFFFFF - flashSize + 1 + 0x1000, 0x4000, buffer);
	return rst;
}


int writeFileToFlashBySlice(const char *fileName, uint32_t mbAddress,
			    uint32_t mbLength, uint8_t * buffer,
			    char *rate, char cycle, unsigned long buffer_phy_addr)
{
	uint32_t mbOffset = 0;
	uint32_t rst = 0;
	uint32_t readTime = 0;
	uint32_t size = 0;
	uint32_t AddressBegin = 0;
	uint64_t AddressEnd = 0;
	FILE *infile;
	char str[5];
	uint32_t i = 0;
	size_t result = 0;
	uint32_t readsize = 0;
	uint8_t * tmpbuffer = buffer;
	size = PAGE_SIZE;

	AddressBegin = mbAddress & ~(size - 1);	//align with size
	mbOffset = mbAddress - (0xFFFFFFFF - flashSize + 1);
/*对于小于或等于4K的大小，调整读取大小和位置*/
	if (mbOffset + mbLength <= size) {
		readsize = mbLength;
		buffer += mbOffset;
		if (mbOffset + mbLength == size) {
			AddressEnd = mbAddress + mbLength;	//
		} else {
			AddressEnd = mbAddress + mbLength + (size - (mbAddress + mbLength) % size);	//
		}
	} else {
		readsize = size;
		AddressEnd = (uint64_t) mbAddress + (uint64_t) mbLength;	//
	}

	readTime = (AddressEnd - AddressBegin) / size;
	debug("mbaddress= 0x%x, mblength=0x%x mbOffset=0x%x\n", mbAddress,
	      mbLength, mbOffset);
	debug("AddressBegin=0x%x, AddressEnd=0x%llx\n", AddressBegin,
	      AddressEnd);
	debug("flashsize=0x%x readTime=%d \n", flashSize, readTime);
	debug("size=0x%x readsize=0x%x \n", size, readsize);
	infile = fopen(fileName, "rb");
	if (!infile) {
		strcpy(errorInfo,
		       "open file error,please check if the file exist?error code is:");
		sprintf(str, "%d", errno);
		strcat(errorInfo, str);
		return 1;
	}
	if (SecureFlash || SecureBios) {
		rst =
		    fseek(infile,
			  mbOffset + sizeof(KUNLUN_BIOS_SIGNATURE_HEADER),
			  SEEK_SET);
	} else {
		rst = fseek(infile, mbOffset, SEEK_SET);
	}

	if (rst != 0) {
		fclose(infile);
		fprintf(stderr, "flash file read error!");
		return 1;
	}
#ifdef __NEED_WRITE2__
#ifdef __DEBUG__
	unsigned int not_need_page = mbOffset / size + 1;
#endif	
	char *localbuffer = (char *) valloc(PAGE_SIZE);
	memset(localbuffer, 0, PAGE_SIZE);
	unsigned long local_buffer_phy_addr = 0;
	if (mem_addr_vir2phy
	    ((unsigned long) localbuffer, &local_buffer_phy_addr)) {
		fprintf(stderr,
			"ERROR:Can not find the local_buffer_phy_addr\n");
		return 1;
	}
#endif

//    setcur(HIDE);
	printf("\n");
	rate_flag = *rate;
	for (i = 0; i < readTime; i++) {
		if (RATE3 == cycle) 
			*rate = (i + 1) * 100 / readTime;
		else if (RATE1 == cycle)
			*rate = rate_flag + (i + 1) * 90 / readTime;
		else if (RATE2 == cycle) 
			*rate = rate_flag + (i + 1) * 10 / readTime;
		else if (RATENOCHANGE == cycle) 
			*rate = rate_flag;

		fprintf(stderr, "\r Flashing...%-3d%%    ", (i + 1) * 100 / readTime);
		result = fread(buffer, 1, readsize, infile);
		if (result != readsize) {
			goto failed;
		}
		

#ifdef __NEED_WRITE2__
		if (needWrite
		    (tmpbuffer, size, AddressBegin + i * size, 0x82F,
		     localbuffer, local_buffer_phy_addr) == 0) {
			debug("the %-4d page needn't flash!\n",
			      i + not_need_page);

			continue;
		}
#endif
		write_data_to_module(module_fd, tmpbuffer, size);
		writeFlash(buffer_phy_addr, size, AddressBegin + i * size,
			   0x82F);

	}
	printf("\n");

	fclose(infile);
#ifdef __NEED_WRITE2__
	free(localbuffer);
	localbuffer = 0;
#endif
	return 0;

      failed:
	fclose(infile);
#ifdef __NEED_WRITE2__
	free(localbuffer);
	localbuffer = 0;
#endif
	return 1;

}

int writeOA2toFlash(const char *fileName, uint32_t mbAddress,
		    uint32_t mbLength, uint8_t * buffer, char *rate,
		    unsigned long buffer_phy_addr)
{
	uint32_t mbOffset;
	uint32_t readTime;
	uint32_t size;
	uint32_t AddressBegin, AddressEnd;
	FILE *infile;
	char str[5];
	uint32_t i;
	uint32_t result;
	uint32_t readsize = 0;
	size = PAGE_SIZE;
	AddressBegin = mbAddress & ~(size - 1);	//align with size
	AddressEnd = mbAddress + mbLength + (size - (mbAddress + mbLength) % size);	//
	mbOffset = mbAddress - (0xFFFFFFFF - flashSize + 1);
	readsize = size - mbOffset;
	readTime = (AddressEnd - AddressBegin) / size;
	infile = fopen(fileName, "rb");
	if (!infile) {
		strcpy(errorInfo,
		       "open file error,please check if the file exist?error code is:");
		sprintf(str, "%d", errno);
		strcat(errorInfo, str);
		return 1;
	}
/*	
	rst = fseek(infile, 0, SEEK_END);
	lSize = ftell(infile);
	rewind(infile);
	rst = fseek(infile, 0, SEEK_SET);
	if (rst != 0) {
		fclose(infile);
		infile = 0;
		strcpy(errorInfo, "flash file read error:");
		strcat(errorInfo, fileName);
		return 1;
	}
*/	
	//result = fread(buffer + mbOffset, 1, lSize, infile);
	result = fread(buffer + mbOffset, 1, readsize, infile);
	if (result != readsize) {
		fclose(infile);
		strcpy(errorInfo, "reading rom file to memory error ");
		sprintf(str, "%d", errno);
		strcat(errorInfo, str);
		return 1;
	}
	//mbOffset = AddressBegin - (0xFFFFFFFF - flashSize + 1);

	//setcur(HIDE);
	printf("\n");
	for (i = 0; i < readTime; i++) {
		printf("\r Flashing...%d%%    ", (i + 1) * 100 / readTime);

		//writeFlash(buffer_phy_addr + mbOffset + i * size, size, AddressBegin + i * size, 0x82F);
		write_data_to_module(module_fd, buffer, size);
		writeFlash(buffer_phy_addr, size, AddressBegin + i * size, 0x82F);
	}
	printf("\n");

	// setcur(NORMAL);
	fclose(infile);
	infile = 0;
	return 0;
}

#if 0
int writeOA3toFlash(const char *fileName, uint32_t mbAddress,
		    uint32_t mbLength, uint8_t * buffer, char *rate)
{
	uint32_t mbOffset;
	uint32_t rst, readTime;
	uint32_t size, lSize;
	uint32_t AddressBegin, AddressEnd;
	FILE *infile;
	char str[5];
	uint32_t i;
	uint32_t result;
	size = (uint32_t) 0x10000;
	if (size > mbLength) {
		size = (uint32_t) 0x1000;
	}
	AddressBegin = mbAddress & ~(size - 1);	//align with size
	AddressEnd = mbAddress + mbLength + (size - (mbAddress + mbLength) % size);	//
	mbOffset = mbAddress - (0xFFFFFFFF - flashSize + 1);
	readTime = (AddressEnd - AddressBegin) / size;
	infile = fopen(fileName, "rb");
	if (!infile) {
		strcpy(errorInfo,
		       "open file error,please check if the file exist?error code is:");
		sprintf(str, "%d", errno);
		strcat(errorInfo, str);
		return 1;
	}
	rst = fseek(infile, 0, SEEK_END);
	lSize = ftell(infile);
	rewind(infile);
	rst = fseek(infile, 0, SEEK_SET);
	if (rst != 0) {
		fclose(infile);
		infile = 0;
		strcpy(errorInfo, "flash file read error:");
		strcat(errorInfo, fileName);
		return 1;
	}
	result = fread(buffer + mbOffset, 1, lSize, infile);
	if (result != lSize) {
		fclose(infile);
		strcpy(errorInfo, "reading rom file to memory error ");
		sprintf(str, "%d", errno);
		strcat(errorInfo, str);
		return 1;
	}
	mbOffset = AddressBegin - (0xFFFFFFFF - flashSize + 1);
	unsigned long buffer_phy_addr = 0;
	if (mem_addr_vir2phy((unsigned long) buffer, &buffer_phy_addr)) {
		fprintf(stderr, "Can not find the buffer_phy_addr\n");
		return -1;
	}
	printf("\n");
	for (i = 0; i < readTime; i++) {
		printf("\r Flashing...%d%%    ", (i + 1) * 100 / readTime);

		//if(lSize < i*size)
		writeFlash(buffer_phy_addr + mbOffset + i * size, size,
			   AddressBegin + i * size, 0x82F);
	}
	printf("\n");
	fclose(infile);
	infile = 0;
	return 0;
}
#endif
int writeFileToDMIBlock(const char *fileName, uint8_t * buffer, char *rate,
			unsigned long buffer_phy_addr)
{
	int rst;
	uint32_t mbAddress;
	uint32_t mbLength;
	getSliceInfo(FbtsRomMapDmiFru, &mbAddress, &mbLength);
	rst =
	    writeFileToFlashBySlice(fileName, mbAddress, mbLength, buffer,
				    rate, RATE3, buffer_phy_addr);
	return rst;
}

int writeFileToNVROMBlock(const char *fileName, uint8_t * buffer,
			  char *rate, unsigned long buffer_phy_addr)
{
	int rst;
	uint32_t mbAddress;
	uint32_t mbLength;
	getSliceInfo(FbtsRomMapNVRam, &mbAddress, &mbLength);
	rst =
	    writeFileToFlashBySlice(fileName, mbAddress, mbLength, buffer,
				    rate, RATE3, buffer_phy_addr);
	return rst;
}

int writeFileToOA2Block(const char *fileName, uint8_t * buffer, char *rate,
			unsigned long buffer_phy_addr)
{
	uint32_t rst;
	uint32_t mbAddress;
	uint32_t mbLength;

	mbAddress = 0xFFFFFFFF - flashSize + 1 + 10;
	mbLength = 0xB6;
	rst =
	    writeOA2toFlash(fileName, mbAddress, mbLength, buffer, rate,
			    buffer_phy_addr);
	return rst;
}

int writeFileToOA3Block(const char *fileName, uint8_t * buffer, char *rate,
			unsigned long buffer_phy_addr)
{
	uint32_t rst;
	uint32_t mbAddress;
	uint32_t mbLength;

	mbAddress = 0xFFFFFFFF - flashSize + 1 + 0x2A2;
	mbLength = 0x62;

	rst =
	    writeOA2toFlash(fileName, mbAddress, mbLength, buffer, rate,
			    buffer_phy_addr);
	return rst;
}

void getSliceInfo(uint8_t type, uint32_t * address, uint32_t * length)
{
	int i;
	PlatformRomMapBuffer platformRomMapBuffer;
	PrivateRomMapBuffer privateRomMapBuffer;
	*address = 0;
	*length = 0;

	//platformRomMapBuffer.PlatFormRomMap[0].Address = 0x55555555;
	//platformRomMapBuffer.PlatFormRomMap[0].Length = 0x66666666;
	//platformRomMapBuffer.PlatFormRomMap[0].Type = 0x77;
	//printf("&platformRomMapBuffer=%p &privateRomMapBuffer=%p\n",&platformRomMapBuffer,&privateRomMapBuffer);
	
	/*there need 480B and 320B stack space, TODO?*/
	getFlashRomMap(&platformRomMapBuffer, &privateRomMapBuffer, 0x82F);
	get_content(module_fd, FLASH_PLATFORM_ROMMAP_FLAG, &platformRomMapBuffer, sizeof(platformRomMapBuffer));

	for (i = 0; i < 40; ++i) {
		if (platformRomMapBuffer.PlatFormRomMap[i].Type == type) {
			*address =
			    platformRomMapBuffer.PlatFormRomMap[i].Address;
			*length =
			    platformRomMapBuffer.PlatFormRomMap[i].Length;
			debug("*address=0x%x, *length=0x%x\n", *address,
			      *length);
			debug
			    ("platformRomMapBuffer.PlatFormRomMap[%d].Type=0x%x\n",
			     i,
			     platformRomMapBuffer.PlatFormRomMap[i].Type);
			break;
		}
	}
}

int clearEventLog()
{
	return clearBiosEventLog();
}

int executeAfterFalsh(int type)
{
	if (type == 1) {
		completeFalsh(IHISI_SHOUTDOWN, 0x82F);
	} else if (type == 2) {
		completeFalsh(IHISI_REBOOT, 0x82F);
	}

	return 0;
}
