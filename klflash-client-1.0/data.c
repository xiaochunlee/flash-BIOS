#include "data.h"
#include "flashopt.h"
char errorInfo[100] = "";


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

unsigned long phy_partinfo = 0;
unsigned long phy_partblock = 0;
unsigned long buffer_phy_rdwr = 0; 
unsigned long platform_rommap_phyaddr = 0; 
unsigned long private_rommap_phyaddr = 0; 

int module_fd  = 0;
#define module_file "/dev/klflash"

int get_module_fd()
{

	int fd = 0;
	fd = open(module_file, O_RDWR | O_SYNC);
	if (-1 == fd) {
		perror("Open");
		return -1;
	}

	return fd;

}

int close_module_fd(fd)
{
	close(fd);
	return 0;
}

int get_content(int fd, int flags, void *buffer, unsigned long size)
{
	//	assert(buffer);

	if (FLASH_BLOCK_FLAG == flags) {

		if ((off_t) - 1 == lseek(fd, PAGE_SIZE, SEEK_SET)) {
			fprintf(stderr, "lseek Failed, Errno:%d\n", errno);
			close(fd);
			return 1;
		}

	} else if (FLASH_READ_FLAG == flags || FLASH_WRITE_FLAG == flags) {

		if ((off_t) - 1 == lseek(fd, 0, SEEK_SET)) {
			fprintf(stderr, "lseek Failed, Errno:%d\n", errno);
			close(fd);
			return 1;
		}

	} else if (FLASH_PLATFORM_ROMMAP_FLAG == flags) {

		if ((off_t) - 1 == lseek(fd, PAGE_SIZE + 8 + sizeof(PartInfo), SEEK_SET)) {
			fprintf(stderr, "lseek Failed, Errno:%d\n", errno);
			close(fd);
			return 1;
		}

	} 

	if (size != read(fd, buffer, size)) {
		fprintf(stderr, "Read %s Failed\n", module_file);
		close(fd);
		return 1;
	}

	return 0;

}

int get_phy_addr(int flags, unsigned long *phy_addr)
{
	
	int fd = 0;
	fd = open(module_file, O_RDWR | O_SYNC);
	if (-1 == fd) {
		perror("Open");
		return 1;
	}

	if (FLASH_ALL_FLAG == flags && NULL == phy_addr) {
		if ((off_t) - 1 == lseek(fd, PAGE_SIZE * 2 + 4, SEEK_SET)) {
			fprintf(stderr, "Seek Failed %s\n", module_file);
			close (fd);
			return 1;
		}

		if (sizeof(unsigned long) !=
		    read(fd, &phy_partblock, sizeof(unsigned long))) {
			fprintf(stderr, "read module failed\n");
			close (fd);
			return 1;
		}
		if (0 != phy_partblock) {
			phy_partinfo = phy_partblock + 8;	
			platform_rommap_phyaddr = phy_partinfo + sizeof(PartInfo);
			private_rommap_phyaddr = platform_rommap_phyaddr + sizeof(PlatformRomMapBuffer);
		} else {
			close (fd);
			return 1;
		}

		if (sizeof(unsigned long) !=
		    read(fd, &buffer_phy_rdwr, sizeof(unsigned long))) {
			fprintf(stderr, "read module failed\n");
			close (fd);
			return 1;
		}
		if (0 == buffer_phy_rdwr) {
			close (fd);
			return 1;
		}
			
	}else if (FLASH_BLOCK_FLAG == flags) {
		if ((off_t) - 1 == lseek(fd, PAGE_SIZE * 2 + 4, SEEK_SET)) {
			fprintf(stderr, "Seek Failed %s\n", module_file);
			close (fd);
			return 1;
		}

		if (sizeof(unsigned long) !=
		    read(fd, &phy_partblock, sizeof(unsigned long))) {
			fprintf(stderr, "read %s failed\n", module_file);
			close (fd);
			return 1;
		}

	} else if (FLASH_READ_FLAG == flags) {

	 /*TODO*/
	 } else {
	 	fprintf(stderr, "Opration Error\n");
		close (fd);
		return 1;
	 }
	
	close (fd);
	return 0;

}

int write_data_to_module(int fd, unsigned char *buffer, unsigned long size)
{
	if ((off_t) - 1 == lseek(fd, 0, SEEK_SET)) {
		fprintf(stderr, "lseek Failed, Errno:%d\n", errno);
		close(fd);
		return 1;
	}

	if (size != write(fd, buffer, size)) {
		fprintf(stderr, "Write %s Failed\n", module_file);
		close(fd);
		return 1;
	}

	return 0;

}
int send_data_to_module(unsigned long count)
{
	int ret = 0;
	char *buf = NULL;
	int i = 0;
	unsigned long writen = 0;

	int fd = open(module_file, O_RDWR | O_SYNC);
	if (-1 == fd) {
		perror("Open");
		fprintf(stderr, "Can not find device file\n");
		return 1;
	}

	buf = (char *) malloc(count);
	if (buf == NULL) {
		perror("calloc");
		close(fd);
		return 1;
	}

	memset(buf, 0, count);

	int sum = count / PAGE_SIZE;

	for (i = 0; i < sum; i++) {
		ret = write(fd, buf + writen, PAGE_SIZE);
		if (PAGE_SIZE != ret) {
			perror("Write");
			close(fd);
			return 1;
		}

		writen += ret;

//              lseek(fd, writen, SEEK_SET );   
	}
	
	free (buf);
	buf = 0;
	close(fd);
	return 0;
}

#define    page_map_file     "/proc/self/pagemap"
#define    PFN_MASK          ((((uint64_t)1)<<55)-1)
#define    PFN_PRESENT_FLAG  (((uint64_t)1)<<63)

/* phy物理地址在32位系统下内存可能超过32位，所以要用long long 型*/
int mem_addr_vir2phy(unsigned long vir, unsigned long *phy)
{
	int fd;
	int page_size = getpagesize();
	unsigned long vir_page_idx = vir / page_size;
	unsigned long pfn_item_offset = vir_page_idx * sizeof(uint64_t);
	uint64_t pfn_item;

	fd = open(page_map_file, O_RDONLY);
	if (fd < 0) {
		printf("open %s failed", page_map_file);
		close(fd);
		return 1;
	}


	if ((off_t) - 1 == lseek(fd, pfn_item_offset, SEEK_SET)) {
		printf("lseek %s failed", page_map_file);
		close(fd);
		return 1;
	}


	if (sizeof(uint64_t) != read(fd, &pfn_item, sizeof(uint64_t))) {
		printf("read %s failed", page_map_file);
		close(fd);
		return 1;
	}


	if (0 == (pfn_item & PFN_PRESENT_FLAG)) {
		fprintf(stderr, "page is not present\n");
		close(fd);
		return 1;
	}

#ifdef __DEBUG__
	uint64_t phy1 =  (pfn_item & PFN_MASK) * page_size + vir % page_size;
#endif	    
	*phy = (pfn_item & PFN_MASK) * page_size + vir % page_size;
	close(fd);
#ifdef __INDEX_DEBUG__
	if (phy1 >= 0xffffffff) {
		fprintf(stderr,
			"-----------------------------------------the phy_addr >= 4G\n");
		debug
		    ("vir_addr is:%p\t, phy_addr is:0x%lx, the actually phy_addr is:0x%llx\n",
		     (void *) vir, *phy, phy1);
		return 1;
	}
#endif
	debug("................vir_addr is:%p\t, phy_addr is:0x%lx\n",
	      (void *) vir, *phy);
	return 0;


}

#if 0
int main()
{
	int count = 999999999;
	//int  va=*(int *)(void *)vir;
	unsigned long va = (unsigned long) &count;
	unsigned long pa = 0;
	mem_addr_vir2phy(va, &pa);
	printf("the va is %p, pa is %p\n", &count, pa);
}
#endif
