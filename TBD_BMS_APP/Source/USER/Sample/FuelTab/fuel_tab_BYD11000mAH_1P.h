
#ifndef FUEL_DFFS_TAB_BAK2600mAH_5P_H
#define FUEL_DFFS_TAB_BAK2600mAH_5P_H


#ifdef __cplusplus
extern "C" {
#endif

#include "FuelDffsTab.h"

#define DFFS_CFG_VER        0x0102       //固件版本号

#define DFFS_NUM            92
#define DFFS_VER_TAB_INDEX  12           //存放固件版本号的表索引
#define DFFS_CYCLE_INDEX    36           //存放循环次数的表索引
#define DFFS_FU_INDEX       83           //存放第一次使用标志的表索引

extern unsigned short const *const  c_DFFS_pCMD[DFFS_NUM];



#ifdef __cplusplus
}
#endif
#endif 

