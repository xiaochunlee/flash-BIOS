#include <stdio.h>
#include <string.h>
#include  <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <sys/io.h>
#include <pthread.h>
#include <unistd.h>

#include "basefunc.h"
#include "flashopt.h"
#include "data.h"
uint8_t SecureFlash = 0;
uint8_t SecureBios = 0;
uint32_t flashSize;
int checkOpt(const char *);
int checkSecureFlashApp(FILE *);

#ifdef __DEBUG__
extern unsigned long phy_partblock;
extern unsigned long phy_partinfo;
extern unsigned long platform_rommap_phyaddr;
extern unsigned long private_rommap_phyaddr;
#endif
extern unsigned long buffer_phy_rdwr;
extern int module_fd;
/*
 *
 * 返回0： COMMAND参数；返回1：Option参数；返回2：COMMAND又是OPTION；返回-1：未知参数
*/
int checkOptType(const char *);

int mystrcmp(const char *s1, const char *s2);

int findOption(const char (*)[12], int, const char *);

int processCommand(const char *);

int processFileAndCommand(const char *, const char *, char *);

int processOptions(const char *, const char (*)[12], int, char *);


int mystrcmp(const char *s1, const char *s2)
{
	for (; *s1 == *s2 || *s1 + 32 == *s2 || *s1 - 0x20 == *s2;
	     s1++, s2++)
		if (!*s1)
			return 0;

	return *s1 - *s2;
}

int findOption(const char (*options)[12], int len, const char *opt)
{
	int i = 0;
	for (i = 0; i < len; ++i) {
		if (mystrcmp(options[i], opt) == 0)
			return 1;
	}
	return 0;
}

int checkOpt(const char *param)
{
	char c = param[0];

	/*是可选参数或命令参数 */
	if (c == '/')
		return 1;

	/*不是可选参数或命令参数 */
	return 0;
}

int checkOptType(const char *name)
{
	/*COMMAND参数 */
	if (mystrcmp(name, "/O") == 0)
		return 0;

	if (mystrcmp(name, "/U") == 0)
		return 0;

	if (mystrcmp(name, "/S") == 0)
		return 2;

	if (mystrcmp(name, "/D") == 0)
		return 0;

	if (mystrcmp(name, "/A") == 0)
		return 2;

	if (mystrcmp(name, "/OAD") == 0)
		return 2;

//      if ( mystrcmp ( name, "/CLNENVLOG" ) == 0 ) return 2;

	/*OPTION参数 */
	if (mystrcmp(name, "/ALL") == 0)
		return 1;

	if (mystrcmp(name, "/P") == 0)
		return 1;

	if (mystrcmp(name, "/B") == 0)
		return 1;

	if (mystrcmp(name, "/N") == 0)
		return 1;

	if (mystrcmp(name, "/R") == 0)
		return 1;

	if (mystrcmp(name, "/L") == 0)
		return 1;

	if (mystrcmp(name, "/OA2") == 0)
		return 1;
	if (mystrcmp(name, "/OA3") == 0)
		return 1;

	if (mystrcmp(name, "/L") == 0)
		return 1;

	//if ( mystrcmp ( name, "/SEC" ) == 0 ) return 1;

	if (mystrcmp(name, "/REBOOT") == 0)
		return 1;

	if (mystrcmp(name, "/SHUTDOWN") == 0)
		return 1;
	return -1;
}

void printMan()
{

	printf
	    ("           KLFLASH--KunLun Firmware Update Utility V1.0                    \n");
	printf
	    (" Copyright (C) 2010-2014 ZD Tech (Beijing) Ltd. All Rights Reserved.       \n");
	printf
	    ("+--------------------------------------------------------------------------+\n");
	printf
	    ("| Usage:    KLFLASH	<ROM File Name>  [Option 1]  [Option 2] ...        |\n");
	printf
	    ("|           Or                                                             |\n");
	printf
	    ("|           KLFLASH	<Input or Output File Name>  <Command>             |\n");
	printf
	    ("|           Or                                                             |\n");
	printf
	    ("|           KLFLASH	 <Command>                                         |\n");
	printf
	    ("| Commands: /O  -   Save current ROM image to file                         |\n");
	// printf ( "|           /CLNENVLOG  -   Clear Event Log                                |\n" );
	printf
	    ("| Options: /ALL  -  Program All Flash                                      |\n");
	printf
	    ("|          /R  -   Preserve ALL SMBIOS structure during programming        |\n");
	printf
	    ("|          /B  -   Program Boot Block                                      |\n");
	printf
	    ("|          /P  -   Program Main Block                                      |\n");
	printf
	    ("|          /N  -   Program NVRAM                                           |\n");
	printf
	    ("|          /OA2  -  Program OA2 marker file                                |\n");
	printf
	    ("|          /OA3  -  Program OA3 bin file                                	 |\n");
	printf
	    ("|          /L  -  Program All Rom Holes                                    |\n");
	//printf ( "|          /SEC  -  Secure Flash                                           |\n" );
	printf
	    ("|         /REBOOT   -   Reboot after programming.                          |\n");
	printf
	    ("|         /SHUTDOWN - Shutdown after programming.                          |\n");
	printf
	    ("| Note:                                                                    |\n");
	printf
	    ("|  1. The expression enclosed by <> means it is a mandatory field.         |\n");
	printf
	    ("|  2. The expression enclosed by [] means it is an optional field.         |\n");
	//printf
	//    ("|  3. A command without parameter means it is a read command.              |\n");
	//printf
	//    ("|  4. A command with necessary parameter means it is a write command.      |\n");
	printf
	    ("+--------------------------------------------------------------------------+\n");
}

int getCommandLineOpt(int argc, char **argv, CommandLineOpt * opt)
{

	int val = 0;
	int i;

	if (argc == 1)
		return 0;

	/*最多30个可选参数 */
	if (argc > 32) {
		strcpy(errorInfo, "parameter number must be under 30");
		return -2;
	}

	if (!checkOpt(argv[1]))
		/*作为文件参数看待 */
	{
		unsigned int len = strlen(argv[1]);

		/*输入文件名超长》100字符 */
		if (len > 100) {
			strcpy(errorInfo,
			       "file name is too long,must under 100 character");
			return -3;
		}

		opt->fileNameLength = len;
		strcpy(opt->fileName, argv[1]);

		/*没有命令行参数 */
		if (argc == 2) {
			strcpy(errorInfo,
			       "the command line parameters not find");
			return -4;
		}

	} else
		/*作为命令参数看待 */
	{
		/*命令行参数超长 */
		if (strlen(argv[1]) > 10) {
			strcpy(errorInfo,
			       "command line parameters is too long:");
			strcat(errorInfo, argv[1]);
			return -2;
		}

		/*命令参数只能有一个 */
		if (argc > 2) {
			strcpy(errorInfo,
			       "The command line parameters can be only one in case not specified file name");
			return -2;
		}

		if (!checkOpt(argv[1])) {
			strcpy(errorInfo,
			       "The command line parameter is not in the correct format:");
			strcat(errorInfo, argv[1]);
			return -2;
		}

		val = checkOptType(argv[1]);

		if ((val == 0) || (val == 2)) {

			opt->hasCommand = 1;
			strcpy(opt->command, argv[1]);
			return 1;

		} else {
			strcpy(errorInfo,
			       "the input is not a command parameter:");
			strcat(errorInfo, argv[1]);
			return -2;
		}
	}

	/*具有文件名的命令行参数处理 */
	/*只有COMMAND命令行参数 */
	if (argc == 3) {
		/*命令行参数超长 */
		if (strlen(argv[2]) > 10) {
			strcpy(errorInfo,
			       "The command line parameter is too long:");
			strcat(errorInfo, argv[2]);
			return -2;
		}

		val = checkOptType(argv[2]);


		if ((val == 0) || (val == 2)) {
			opt->hasCommand = 1;
			strcpy(opt->command, argv[2]);

		} else if (val == 1) {
			opt->optionNumber = 1;
			strcpy(opt->options[0], argv[2]);

		} else {
			strcpy(errorInfo,
			       "The command line parameter is not in the correct format:");
			strcat(errorInfo, argv[2]);
			return -2;
		}
	}

	/*有多个OPTION命令行参数 */
	else {
		opt->optionNumber = argc - 2;

		for (i = 2; i < argc; ++i) {
			if (!checkOpt(argv[i]))
				/*不是命令行参数 */
			{
				strcpy(errorInfo,
				       "The command line parameter is not in the correct format:");
				strcat(errorInfo, argv[i]);

			} else {
				/*命令行参数超长 */
				if (strlen(argv[i]) > 10) {
					strcpy(errorInfo,
					       "The command line parameter is too long:");
					strcat(errorInfo, argv[i]);
					return -2;
				}

				val = checkOptType(argv[i]);


				if (val == 0) {
					strcpy(errorInfo,
					       "the parameter must be [OPTION] parameter:");
					strcat(errorInfo, argv[i]);
					return -2;
				}

				if (val == -1) {
					strcpy(errorInfo,
					       "wrong parameter,unsupport parameter:");
					strcat(errorInfo, argv[i]);
					return -2;
				}

				strcpy(opt->options[i - 2], argv[i]);
			}
		}
	}

	return 1;
}

int processCommand(const char *param)
{
	//int rst = 0;
	if (mystrcmp(param, "/O") == 0) {
		strcpy(errorInfo, "File name not specified");
		return 1;
	} else if (mystrcmp ( param, "/reboot" ) == 0 ){
 		executeAfterFalsh(2);            
	} else if (mystrcmp ( param, "/shutdown" ) == 0 ){
 		executeAfterFalsh(1);            

	} else {
		strcpy(errorInfo, "the command is not implemented");
		return 1;
	}

	return 0;
}

int processFileAndCommand(const char *fileName, const char *optName,
			  char *rate)
{
	int value = 0;
	if (mystrcmp(optName, "/O") == 0) {
		value = readFlashToFile(fileName, rate);
	}

	return value;
}

int processOptions(const char *fileName, const char (*opts)[12],
		   int optslen, char *rate)
{
	/*对命令行参数进行处理 */
	int isR = 0;
	uint32_t rst;
	//uint32_t i, size, readTime;
	//PartInfo partInfo;
	//PartBlock partBlock;
	uint8_t *buffer = 0;
	uint8_t secureCheck;
	//uint32_t begin;
	char str[5];
	//uint8_t SecureBios = 0;
	KUNLUN_BIOS_SIGNATURE_HEADER header;
	//uint8_t SecureFlash = 0;

	FILE *infile = NULL;
	size_t result;
	uint32_t lSize;
/*内存对齐申请*/
	PartInfo *partInfo = (PartInfo *) memalign(0x80, sizeof(PartInfo));
	PartBlock *partBlock =
		(PartBlock *) memalign(0x8, sizeof(PartBlock));
	memset(partInfo, 0, sizeof(partInfo));
	memset(partBlock, 0, sizeof(partBlock));

	if (!findOption(opts, optslen, "/OA2")
	    && !findOption(opts, optslen, "/OA3")) {
		rst = GetSecureBIOSStatus(0x82f);
		if (rst == 1) {
			debug("------------------IS secureFlash\n");
			SecureFlash = 1;
		} else {
			SecureFlash = 0;
		}

		infile = fopen(fileName, "rb");
		if (!infile) {
			strcpy(errorInfo,
			       "open file error,please check if the file exist?error code is:");
			sprintf(str, "%d", errno);
			strcat(errorInfo, str);
			return 1;
		}

		fseek(infile, 0, SEEK_SET);
		result =
		    fread(&header, 1, sizeof(KUNLUN_BIOS_SIGNATURE_HEADER),
			  infile);
		if (result != sizeof(KUNLUN_BIOS_SIGNATURE_HEADER)) {
			fclose(infile);
			free(buffer);
			strcpy(errorInfo,
			       "reading rom file to memory error");
			sprintf(str, "%d", errno);
			strcat(errorInfo, str);
			return 1;
		}

		if (*(uint32_t *) (header.Magic) == 0x424c4b5F
		    && header.Magic[4] == 0x53) {
			SecureBios = 1;
			debug("------------------IS secureBios\n");
		} else {
			SecureBios = 0;
		}

	}
	rst =
	    getFlashPartInfo(partInfo, partBlock,
			     DEFAULT_FLASH_DEVICE_TYPE, 0x82F);
	if (rst)
		return 1;

	if (get_content(module_fd, FLASH_BLOCK_FLAG, partBlock, sizeof(PartBlock))) {
		fprintf(stderr, "get PartBlock error\n");
		return 1;
	}	

	flashSize = partBlock->BlockSize * partBlock->Mutiple * 256;

	debug("the blocksize is:0x%x,mutiple is:0x%x,flashSize is:0x%x\n",
	      partBlock->BlockSize, partBlock->Mutiple, flashSize);

	free (partBlock);
	free (partInfo);

	if (SecureFlash && SecureBios) {	//if secureflash, should read the hole file
		fseek(infile, 0, SEEK_END);
		lSize = ftell(infile);
		rewind(infile);

		buffer = (uint8_t *) valloc(PAGE_SIZE);
		//buffer = ( uint8_t* )0x60000000;
		if (buffer == 0) {
			fclose(infile);
			strcpy(errorInfo,
			       "malloc memmory for the rom file error");
			sprintf(str, "%d", errno);
			strcat(errorInfo, str);
			return 1;
		}

		result = fread(buffer, 1, PAGE_SIZE, infile);

		if (result != PAGE_SIZE) {
			fclose(infile);
			free(buffer);
			strcpy(errorInfo,
			       "reading rom file to memory error");
			sprintf(str, "%d", errno);
			strcat(errorInfo, str);
			return 1;
		}

		if ((*(uint32_t *) buffer != 0x424c4b5F)
		    || (*(buffer + 4) != 0x53)) {
			printf("signature:%x %x\n", *(uint32_t *) buffer,
			       *(buffer + 4));
			printf("The image is an invalid image!\n");

			fclose(infile);
			free(buffer);
			buffer = NULL;
			return 1;
		}

#ifdef __AUTHENTICATE__
	/*认证过程，此过程在authenticate下面*/
		rewind(infile);
		debug("Start authorize the image\n");
		secureCheck = checkSecureFlashApp(infile);	//check and return
		if (secureCheck == 1) {
			printf("The image is an unauthorized image!\n");
			fclose(infile);
			free(buffer);
			buffer = NULL;
			return 1;
		}
#endif		
#if 0
		if (findOption(opts, optslen, "/ALL")) {
			if (findOption(opts, optslen, "/R")) {
				secureCheck = checkSecureFlashBB(buffer, 0x82f, 0x11);	// not return
			} else {
				secureCheck = checkSecureFlashBB(buffer, 0x82f, 0x01);	// not return
			}
		} else if (findOption(opts, optslen, "/B")) {
			if (findOption(opts, optslen, "/R")) {
				secureCheck = checkSecureFlashBB(buffer, 0x82f, 0x12);	//not return 
			} else {
				secureCheck = checkSecureFlashBB(buffer, 0x82f, 0x02);	//not return 
			}
		} else {
			secureCheck = checkSecureFlashALL(buffer, 0x82f);	//check and return
		}

		if (secureCheck == 1) {
			printf("The image is an unauthorized image!\n");
			fclose(infile);
			free(buffer);
			buffer = NULL;
			return 1;
		}
#endif
	} else if ((SecureFlash == 1 && SecureBios == 0)) {
		printf("The image is an unauthorized image!\n");
		fclose(infile);

		return 1;
	} else {
		if (SecureBios == 1) {
//                      fseek(infile, 0, SEEK_END);
//                      lSize = ftell(infile);
//                      rewind(infile);

			buffer = (uint8_t *) valloc(PAGE_SIZE);
			memset(buffer, 0, PAGE_SIZE);

		} else {
			/*此处必须按照4k大小来分配 */
			buffer = (uint8_t *) valloc(PAGE_SIZE);
			memset(buffer, 0, PAGE_SIZE);
		}

		if (buffer == 0) {
			strcpy(errorInfo,
			       "malloc memmory for the rom file error");
			sprintf(str, "%d", errno);
			strcat(errorInfo, str);
			
			if (!findOption(opts, optslen, "/OA2")
	    			&& !findOption(opts, optslen, "/OA3")) 
					fclose(infile);

			return 1;
		}
	}
#if 0
	if (SecureFlash || SecureBios
	    || !(findOption(opts, optslen, "/ALL"))) {
		size = (uint32_t) 0x10000;
		readTime = flashSize / size;
		begin = 0xFFFFFFFF - flashSize + 1;
/*此处的转换是针对SecureFlash,SecureBios以及不是/all,在下面到writeFileToFlash中需要再次转换*/
		unsigned long buffer_phy_addr = 0;
		if (mem_addr_vir2phy
		    ((unsigned long) buffer, &buffer_phy_addr)) {
			fprintf(stderr,
				"Can not find the physical address\n");
			free(buffer);
			buffer = NULL;
			fclose(infile);
			return -1;
		}

		if (SecureFlash || SecureBios) {
			for (i = 0; i < readTime; i++) {
				readFlash(buffer_phy_addr +
					  sizeof
					  (KUNLUN_BIOS_SIGNATURE_HEADER) +
					  i * size, size, begin + i * size,
					  0x82F);
			}
		} else {
			if (!(findOption(opts, optslen, "/ALL"))) {	//if flash all, don't read the flash 
				//printf("not program all");
				for (i = 0; i < readTime; i++) {
					readFlash(buffer_phy_addr +
						  i * size, size,
						  begin + i * size, 0x82F);
				}
			}
		}
	}
#endif

/*
	unsigned long buffer_phy_addr = 0;
	if (mem_addr_vir2phy((unsigned long) buffer, &buffer_phy_addr)) {
		fprintf(stderr,
			"Error:Can not find the physical address\n");
		free(buffer);
		buffer = NULL;
		fclose(infile);
		return 1;
	}
*/
	if (findOption(opts, optslen, "/ALL")) {
		if (findOption(opts, optslen, "/R")) {
			isR = 1;
		}
		rst =
		    writeFileToFlash(fileName, isR, buffer, rate,
				     buffer_phy_rdwr);
		if (rst) {
			fprintf(stderr, "ERROR: Flash Failed\n");
			goto failed;
		}
	} else {
		if (findOption(opts, optslen, "/P")) {
			rst =
			    writeFileToMainBlock(fileName, buffer, rate,
						 buffer_phy_rdwr);
			if (rst)
				goto failed;
		}

		if (findOption(opts, optslen, "/B")) {
			if (findOption(opts, optslen, "/R")) {
				isR = 1;
			}
			/*先读取一页大小，后面会从文件中读入相应到部分来覆盖第一页中到某些内容，用来保留前一页中相应到部分 */
			//readFlash(buffer_phy_addr, 0x1000, 0xFFFFFFFF - flashSize + 1, 0x82f);
			rst =
			    writeFileToBootBlock(fileName, isR, buffer,
						 rate, buffer_phy_rdwr);
		}
		if (findOption(opts, optslen, "/N")) {
			rst =
			    writeFileToNVROMBlock(fileName, buffer, rate,
						  buffer_phy_rdwr);
			if (rst)
				goto failed;
		}

		/*if is /OA2 or /OA3 ,then is not sercueFlash and SecureBios*/
		if (findOption(opts, optslen, "/OA2")) {
			rst = GetOA3Status(0x82f);
			printf("OAx Status=%x\n", rst);
			if (rst != 0x55) {
				printf
				    ("The OAx marker key area is locked!\n");
				free(buffer);
				buffer = NULL;
				return 1;
			}

			readFlash(buffer_phy_rdwr, PAGE_SIZE,
				  0xFFFFFFFF - flashSize + 1, 0x82f);
			/*write the buffer data to module*/
			if (get_content(module_fd, FLASH_WRITE_FLAG, buffer, PAGE_SIZE) == 1) {
				fprintf(stderr, "get data error\n");
				free(buffer);
				buffer = NULL;
				return 1;
			}

			rst =
			    writeFileToOA2Block(fileName, buffer, rate,
						buffer_phy_rdwr);
			if (rst)
				goto failedOA;
		}
		if (findOption(opts, optslen, "/OA3")) {
			rst = GetOA3Status(0x82f);
			printf("OAx Status=%x\n", rst);
			if (rst != 0x55) {
				printf
				    ("The OAx marker key area is locked!\n");
				free(buffer);
				buffer = NULL;
				return 1;
			}
			readFlash(buffer_phy_rdwr, 0x1000,
				  0xFFFFFFFF - flashSize + 1, 0x82f);

			if (get_content(module_fd, FLASH_WRITE_FLAG, buffer, PAGE_SIZE) == 1) {
				fprintf(stderr, "get data error\n");
				free(buffer);
				buffer = NULL;
				return 1;
			}
			rst =
			    writeFileToOA3Block(fileName, buffer, rate,
						buffer_phy_rdwr);
			if (rst)
				goto failedOA;
		}

		if (findOption(opts, optslen, "/L")) {
			if (findOption(opts, optslen, "/R")) {
				isR = 1;
			}
			rst =
			    writeFileToRomHoles(fileName, isR, buffer,
						rate, buffer_phy_rdwr);
			if (rst)
				goto failed;
		}
	}

	// if(findOption(opts, optslen, "/CLNENVLOG")){
	// clearEventLog();
	// }

	if (findOption(opts, optslen, "/SHUTDOWN")) {
		executeAfterFalsh(1);
	}

	if (findOption(opts, optslen, "/REBOOT")) {
		executeAfterFalsh(2);
	}

      failed:
	fclose(infile);
      failedOA:
	free(buffer);
	buffer = NULL;
	return rst;
}


void *executeTask(void *tmp)
{
	
	CommandLineOpt *opt = (CommandLineOpt *) tmp;

	/*开放端口到权限 */
	if (iopl(3)) {
		fprintf(stderr, "iopl(): %s\n", strerror(errno));
		return NULL;
	}

	/*使驱动分配地址 */
	if (send_data_to_module(0x3 * PAGE_SIZE)) {
		return NULL;
	}	

	if (get_phy_addr(FLASH_ALL_FLAG, NULL)) {
		fprintf(stderr, "Get Phyaddr Failed\n");
		return NULL;
	}
	debug("phyaddr:\tpartblock:0x%lx\tpartinfo:0x%lx\trdwr:0x%lx\n",phy_partblock, phy_partinfo, buffer_phy_rdwr);
	debug("phyaddr:\tplatform_rommap:0x%lx\tprivate_rommap:0x%lx\n",platform_rommap_phyaddr, private_rommap_phyaddr);
	
	if ((module_fd = get_module_fd()) == -1) {
		fprintf(stderr, "get module fd error\n");
		return NULL;
	}

	/*只有命令行参数 */
	if (opt->fileNameLength == 0) {
		processCommand(opt->command);
		close(module_fd);				      
		return NULL;
	}
	/*有文件和COMMAND参数 */
	else if (opt->fileNameLength > 0 && opt->hasCommand > 0) {
		processFileAndCommand(opt->fileName, opt->command,
				      &opt->rate);
		close(module_fd);				      
		return NULL;
	}
	/*OPTION参数 */
	else {
		processOptions(opt->fileName, opt->options,
			       opt->optionNumber, &opt->rate);
		close(module_fd);
		return NULL;
	}
}

#if 1
int smi_interface(CommandLineOpt * opt)
{
	pthread_t pid;
	int ret;
	void *retval;
	if (pthread_create(&pid, NULL, executeTask, opt) != 0)
		return 1;
#if 0		
	ret = pthread_join(pid, &retval);
	if (ret != 0 )
		perror("pthread_join");

	debug("Join with thread:%ld; returned value was:%s\n", (unsigned long)pid, (char*)retval);
#endif	
	return 0;

}
#endif
