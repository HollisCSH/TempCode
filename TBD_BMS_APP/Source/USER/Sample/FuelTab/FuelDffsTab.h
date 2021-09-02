
#ifndef FUEL_DFFS_TAB_H
#define FUEL_DFFS_TAB_H


#ifdef __cplusplus
extern "C" {
#endif

#include "BSPTypeDef.h"


//电芯配置
#ifdef LFP_HL_25000MAH_16S     //25AH国产电芯 鹏辉 LFP
#include "fuel_tab_BYD11000mAH_1P.h"


#else

#endif



////电量计数据配置
//typedef struct
//{
//    u16 regaddr;    //电量计memory寄存器地址
//    u16 data;       //配置的数据
//}t_Max17205_Update;


#ifdef __cplusplus
}
#endif
#endif 

