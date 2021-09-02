#include "FuelDffsTab.h"


//电芯配置
#ifdef LFP_HL_25000MAH_16S     //25AH国产电芯 鹏辉 LFP
#include "fuel_tab_BYD11000mAH_1P.c"

#elif defined(BMS_BAT_12500MAH_EVE)//15AH
#include "fuel_dffs_tab_EVE2500mAH_5P.c"


#endif

