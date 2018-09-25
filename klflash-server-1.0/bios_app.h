#ifndef __BIOS_APP_H_
#define __BIOS_APP_H_

#include "basefunc.h"
#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief 执行实际的flash任务
 * @param opt
 * @return  0：任务成功；>0：错误号，任务失败
 */
int smi_interface(CommandLineOpt *opt);
#ifdef __cplusplus
    }
#endif
#endif
