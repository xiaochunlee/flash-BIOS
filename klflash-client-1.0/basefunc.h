
#ifndef _BASEFUNC_H

#define _BASEFUNC_H

#include "data.h"

typedef struct {
	char fileName[101];
	uint8_t fileNameLength;
	char options[30][12];
	uint8_t optionNumber;
	char command[12];
	uint8_t hasCommand;
	char rate;
}CommandLineOpt;

/**
 * @brief 处理命令行参数
 * @param argc
 * @param argv
 * @param CommandLineOpt *
 * @return  1：有命令行参数，0：没有命令行参数；-1：系统错误；-2：参数错误；-3：文件错误（文件名超长）；-4：缺少参数
 */
int getCommandLineOpt(int argc, char **argv, CommandLineOpt * );

/**
 * @brief 输出操作说明
 */
void printMan();

/**
 * @brief 执行实际的flash任务
 * @param opt
 * @return  0：任务成功；>0：错误号，任务失败
 */
void *executeTask(void *tmp);

#endif
