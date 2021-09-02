//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: SOPTab.c
//创建人  	: Handry
//创建日期	:
//描述	    : SOP充电表格文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SOPTab.h"
#include "BatteryInfo.h"

//=============================================================================================
//定义全局变量
//=============================================================================================

/*************************************12AH国产电芯凯德*************************************/
#ifdef BMS_BAT_12000MAH_CH
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表 
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {850,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //15-40℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,70},	//70%*C
    {1000,50},	//50%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //40-45℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {850,60},	//60%*C
    {1000,20},	//20%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //45-50℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {800,33},	//33%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,	  NULL},
    {50,(t_SOP_TAB *)cSOPSOCTbl_1},
    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
    {400,(t_SOP_TAB *)cSOPSOCTbl_3},
    {450,(t_SOP_TAB *)cSOPSOCTbl_4},
    {500,(t_SOP_TAB *)cSOPSOCTbl_5},
    {0x7fff,NULL},
};

/*************************************12AH国产电芯创明*************************************/
#elif defined(BMS_BAT_12000MAH_CM)
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {850,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //15-40℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,70},	//70%*C
    {1000,50},	//50%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //40-45℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,60},	//60%*C
    {1000,20},	//20%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //45-50℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,33},	//33%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,NULL},
    {50,(t_SOP_TAB *)cSOPSOCTbl_1},
    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
    {400,(t_SOP_TAB *)cSOPSOCTbl_3},
    {450,(t_SOP_TAB *)cSOPSOCTbl_4},
    {500,(t_SOP_TAB *)cSOPSOCTbl_5},
    {0x7fff,NULL},
};

/*************************************15AH国产电芯凯德*************************************/
#elif defined(BMS_BAT_15000MAH_CH)
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {850,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //15-40℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,70},	//70%*C
    {1000,50},	//50%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //40-45℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {850,60},	//60%*C
    {1000,20},	//20%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //45-50℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,33},	//33%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,NULL},
    {50,(t_SOP_TAB *)cSOPSOCTbl_1},
    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
    {400,(t_SOP_TAB *)cSOPSOCTbl_3},
    {450,(t_SOP_TAB *)cSOPSOCTbl_4},
    {500,(t_SOP_TAB *)cSOPSOCTbl_5},
    {0x7fff,NULL},
};

/*************************************15AH国产电芯创明*************************************/
#elif defined(BMS_BAT_15000MAH_CM)
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {850,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //15-40℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,70},	//70%*C
    {1000,50},	//50%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //40-45℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {850,60},	//60%*C
    {1000,20},	//20%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //45-50℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,33},	//33%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,0},
    {50,(t_SOP_TAB *)cSOPSOCTbl_1},
    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
    {400,(t_SOP_TAB *)cSOPSOCTbl_3},
    {450,(t_SOP_TAB *)cSOPSOCTbl_4},
    {500,(t_SOP_TAB *)cSOPSOCTbl_5},
    {0x7fff,0},
};

/*************************************12.5AH国产电芯EVE*************************************/
#elif defined(BMS_BAT_12500MAH_EVE)
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {850,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //15-40℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,70},	//70%*C
    {1000,33},	//33%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //40-45℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {850,50},	//50%*C
    {1000,33},	//33%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //45-50℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,33},	//33%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,NULL},
    {50,(t_SOP_TAB *)cSOPSOCTbl_1},
    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
    {400,(t_SOP_TAB *)cSOPSOCTbl_3},
    {450,(t_SOP_TAB *)cSOPSOCTbl_4},
    {500,(t_SOP_TAB *)cSOPSOCTbl_5},
    {0x7fff,NULL},
};

/*************************************15AH国产电芯EVE*************************************/
#elif defined(BMS_BAT_15000MAH_EVE)
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {850,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //15-40℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,70},	//70%*C
    {1000,33},	//33%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //40-45℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {850,50},	//50%*C
    {1000,33},	//33%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //45-50℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,33},	//33%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,NULL},
    {50,(t_SOP_TAB *)cSOPSOCTbl_1},
    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
    {400,(t_SOP_TAB *)cSOPSOCTbl_3},
    {450,(t_SOP_TAB *)cSOPSOCTbl_4},
    {500,(t_SOP_TAB *)cSOPSOCTbl_5},
    {0x7fff,NULL},
};

/*************************************13AH国产电芯BAK*************************************/
#elif defined(BMS_BAT_13000MAH_BAK)
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {850,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-10℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,25},	//25%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //10-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,35},	//35%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //15-40℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {850,70},	//70%*C
    {1000,50},	//50%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //40-45℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,60},	//60%*C
    {1000,20},	//20%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_6[] = //45-50℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,33},	//33%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,NULL},
    {50,(t_SOP_TAB *)cSOPSOCTbl_1},
    {100,(t_SOP_TAB *)cSOPSOCTbl_2},
    {150,(t_SOP_TAB *)cSOPSOCTbl_3},
    {400,(t_SOP_TAB *)cSOPSOCTbl_4},
    {450,(t_SOP_TAB *)cSOPSOCTbl_5},
    {500,(t_SOP_TAB *)cSOPSOCTbl_6},
    {0x7fff,NULL},
};
	
/*************************************12.5AH三星电芯*************************************/
#elif defined(BMS_BAT_12500MAH_SAM)
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {800,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-10℃下的SOC降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {850,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //10-15℃下的SOC降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {850,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //15-40℃下的SOC降功率表
{
    {-1, 0},	//0%*C
    {850,50},	//50%*C
    {1000,33},	//33%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //40-45℃下的SOC降功率表
{
    {-1, 0},	//0%*C
    {850,40},	//40%*C
    {1000,20},	//20%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_6[] = //45-50℃下的SOC降功率表
{
    {-1, 0},	//0%*C
    {800,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC降功率表
{
    {0,NULL},
    {50,(t_SOP_TAB *)cSOPSOCTbl_1},
    {100,(t_SOP_TAB *)cSOPSOCTbl_2},
    {150,(t_SOP_TAB *)cSOPSOCTbl_3},
    {400,(t_SOP_TAB *)cSOPSOCTbl_4},
    {450,(t_SOP_TAB *)cSOPSOCTbl_5},
    {500,(t_SOP_TAB *)cSOPSOCTbl_6},
    {0x7fff,NULL},
};

/*************************************25AH国产电芯 鹏辉 LFP*************************************/
/*************************************暂用11AH国产电芯 AK LFP*************************************/
//#elif defined(LFP_HL_25000MAH_16S)
/*************************************20AH国产电芯 TB LFP*************************************/
#elif defined(LFP_TB_20000MAH_20S)
const t_SOP_TAB cSOPVoltTbl_1[] =	//0-5℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3500,10},	//10%*C
    {0x7fff,0},	//0%*C   
};

const t_SOP_TAB cSOPVoltTbl_2[] =	//5-15℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3500,30},	//30%*C
    {3550,20},	//20%*C
    {3650,5},	//5%*C
    {0x7fff,0},	//0%*C      
};

const t_SOP_TAB cSOPVoltTbl_3[] =	//15-25℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3500,50},	//50%*C
    {3550,30},	//30%*C
    {3650,20},	//20%*C    
    {0x7fff,0},	//0%*C     
};

const t_SOP_TAB cSOPVoltTbl_4[] =	//25-40℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3455,70},	//70%*C
    {3500,50},	//50%*C    
    {3550,20},	//20%*C       
    {3650,5},	//5%*C
    {0x7fff,0},	//0%*C   
};

const t_SOP_TAB cSOPVoltTbl_5[] =	//40-45℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3500,30},	//30%*C
    {3550,20},	//20%*C    
    {3650,5},	//5%*C    
    {0x7fff,0},	//0%*C   
};

const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {50,(t_SOP_TAB *)cSOPVoltTbl_1},
    {150,(t_SOP_TAB *)cSOPVoltTbl_2},
    {250,(t_SOP_TAB *)cSOPVoltTbl_3},    
    {400,(t_SOP_TAB *)cSOPVoltTbl_4},        
    {450,(t_SOP_TAB *)cSOPVoltTbl_5},            
    {0x7fff,NULL},
};
/*
const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,10},	//10%*C
    {1000,5},	//5%*C
    {0x7fff,0},	//0%*C
};

const t_SOP_TAB cSOPSOCTbl_2[] = //5-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {1000,50},	//30%*C
    {0x7fff,0},	//0%*C
};

const t_SOP_TAB cSOPSOCTbl_3[] = //15-35℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {700,100},	//70%*C
    {1000,50},	//100%*C    
    {0x7fff,0},	//0%*C
};

const t_SOP_TAB cSOPSOCTbl_4[] = //35-55℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {1000,50},	//50%*C
    {0x7fff,0},	//0%*C
};
*/
const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,100},	//10%*C
    {1000,100},	//5%*C
    {0x7fff,0},	//0%*C
};

const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,	  NULL},
    {550, (t_SOP_TAB *)cSOPSOCTbl_1},
//    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
//    {350,(t_SOP_TAB *)cSOPSOCTbl_3},
//    {550,(t_SOP_TAB *)cSOPSOCTbl_3},    
    {0x7fff,NULL},
};

/*************************************25AH国产电芯 鹏辉海雷 LFP*************************************/
/* 都是鹏辉的电芯，配方相同，容量不同 */
#elif defined(LFP_HL_25000MAH_16S) || defined(LFP_PH_20000MAH_20S)
const t_SOP_TAB cSOPVoltTbl_1[] =	//0-10℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3500,10},	//10%*C
    {0x7fff,0},	//0%*C   
};

const t_SOP_TAB cSOPVoltTbl_2[] =	//11-15℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3650,20},	//20%*C
    {0x7fff,0},	//0%*C      
};

const t_SOP_TAB cSOPVoltTbl_3[] =	//16-20℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3650,30},	//30%*C    
    {0x7fff,0},	//0%*C     
};

const t_SOP_TAB cSOPVoltTbl_4[] =	//21-35℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C
    {3650,50},	//50%*C
    {0x7fff,0},	//0%*C   
};

const t_SOP_TAB cSOPVoltTbl_5[] =	//36-40℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C 
    {3650,40},	//40%*C    
    {0x7fff,0},	//0%*C   
};

const t_SOP_TAB cSOPVoltTbl_6[] =	//41-45℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C 
    {3650,30},	//30%*C    
    {0x7fff,0},	//0%*C   
};

const t_SOP_TAB cSOPVoltTbl_7[] =	//46-55℃下的单体电压充电降功率表
{
    {2500,0},	//0%*C 
    {3650,20},	//20%*C    
    {0x7fff,0},	//0%*C   
};

const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {100,(t_SOP_TAB *)cSOPVoltTbl_1},
    {150,(t_SOP_TAB *)cSOPVoltTbl_2},
    {200,(t_SOP_TAB *)cSOPVoltTbl_3},
    {350,(t_SOP_TAB *)cSOPVoltTbl_4},
    {400,(t_SOP_TAB *)cSOPVoltTbl_5},
    {450,(t_SOP_TAB *)cSOPVoltTbl_6},
    {550,(t_SOP_TAB *)cSOPVoltTbl_7},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,100},	//10%*C
    {1000,100},	//5%*C
    {0x7fff,0},	//0%*C
};

const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,	  NULL},
    {550, (t_SOP_TAB *)cSOPSOCTbl_1},   
    {0x7fff,NULL},
};

/*************************************25AH国产电芯 赣锋 LFP*************************************/
#elif defined(LFP_GF_25000MAH_16S)
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表
{
    {2000,0},	//0%*C
    {3650,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3650,100},	//100%*C
    {0x7fff,0},	//0%*C
};

const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {550,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-7 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {600,10},	//10%*C
    {950,8},	//8%*C
    {1000,5},	//5%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //7-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {600,30},	//30%*C
    {700,25},	//25%*C
    {800,20},	//20%*C 
    {950,15},	//15%*C     
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //15-25℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {600,50},	//50%*C
    {700,40},	//40%*C
    {800,30},	//30%*C 
    {950,20},	//20%*C     
    {1000,15},	//15%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //25-45℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {500,100},	//100%*C
    {600,90},	//90%*C
    {700,80},	//80%*C 
    {800,70},	//70%*C     
    {950,50},	//50%*C     
    {1000,30},	//30%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //45-50℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {600,80},	//80%*C
    {700,70},	//70%*C 
    {800,50},	//50%*C     
    {950,30},	//30%*C     
    {1000,20},	//20%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_6[] = //50-55℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {600,50},	//50%*C
    {700,45},	//45%*C
    {800,30},	//30%*C
    {950,20},	//20%*C
    {1000,15},	//15%*C
    {0x7fff,0},	//0%*C
};

const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,NULL},
    {70, (t_SOP_TAB *)cSOPSOCTbl_1},
    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
    {250,(t_SOP_TAB *)cSOPSOCTbl_3},
    {450,(t_SOP_TAB *)cSOPSOCTbl_4},
    {500,(t_SOP_TAB *)cSOPSOCTbl_5},
    {550,(t_SOP_TAB *)cSOPSOCTbl_6},
    {0x7fff,NULL},
};

/*************************************32AH国产电芯 孚能32AH PB*************************************/
#elif defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
const t_SOP_TAB cSOPVoltTbl_0[] = //-15-0℃，电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {3780,7},	//7%*C    
    {3805,4},	//4%*C    
    {3860,3},	//3%*C    
    {3980,2},	//2%*C
    {4200,1},	//1%*C     
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_1[] = //0-5℃，电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {3750,23},	//23%*C    
    {3830,18},	//18%*C    
    {3900,14},	//14%*C    
    {4060,7},	//7%*C
    {4200,5},	//5%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//5-10℃，单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {3775,41},	//33%*C 
    {3805,30},	//30%*C     
    {3855,27},	//27%*C    
    {3915,23},	//23%*C
    {3990,18},	//18%*C
    {4180,9},	//9%*C
    {4200,5},	//5%*C    
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_3[] =	//10-15℃，单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {3765,57},	//57%*C 
    {3820,48},	//48%*C     
    {3870,40},	//40%*C    
    {3960,31},	//31%*C
    {4085,15},	//15%*C  
    {4180,10},	//10%*C     
    {4200,5},	//5%*C    
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_4[] =	//15-20℃，单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {3765,76},	//76%*C
    {3840,62},	//62%*C
    {3900,57},	//57%*C   
    {3990,48},	//48%*C
    {4180,31},	//31%*C    
    {4200,5},	//5%*C    
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_5[] =	//20-25℃，单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {3750,100},	//100%*C
    {3840,86},	//86%*C
    {3980,67},	//67%*C    
    {4180,38},	//38%*C 
    {4200,5},	//5%*C    
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_6[] =	//20-35℃，单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {3820,100},	//100%*C 
    {3890,90},	//90%*C 
    {4085,70},	//70%*C 
    {4180,50},	//50%*C     
    {4200,5},	//5%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_7[] =	//35-45℃，单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {3785,50},	//50%*C 
    {4060,40},	//40%*C 
    {4180,20},	//20%*C     
    {4200,5},	//5%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_8[] =	//45-50℃，单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {4120,33},	//33%*C 
    {4195,10},	//10%*C    
    {4200,5},	//5%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_9[] =	//50-55℃，单体电压充电降功率表
{
    {2000,0},	//0%*C
    {3300,5},	//5%*C
    {4195,10},	//10%*C 
    {4200,5},	//5%*C
    {0x7fff,0},	//0%*C
};

const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {-150,	  NULL},
    {0,	 (t_SOP_TAB *)cSOPVoltTbl_0},    
    {50, (t_SOP_TAB *)cSOPVoltTbl_1},
    {100,(t_SOP_TAB *)cSOPVoltTbl_2},
    {150,(t_SOP_TAB *)cSOPVoltTbl_3},
    {200,(t_SOP_TAB *)cSOPVoltTbl_4}, 
    {250,(t_SOP_TAB *)cSOPVoltTbl_5},    
    {350,(t_SOP_TAB *)cSOPVoltTbl_6},  
    {450,(t_SOP_TAB *)cSOPVoltTbl_7},
    {500,(t_SOP_TAB *)cSOPVoltTbl_8},    
    {550,(t_SOP_TAB *)cSOPVoltTbl_9},      
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
	{-1, 0},
	{1000,100},	//100%*C
	{0x7fff,0},
};
/*
const t_SOP_TAB cSOPSOCTbl_2[] = //5-10℃下的SOC充电降功率表
{
	{-1, 0},
	{190,40},	//40%*C
	{500,33},	//33%*C
	{720,25},	//25%*C
	{870,20},	//20%*C
	{1000,10},	//10%*C
	{0x7fff,0},
};
const t_SOP_TAB cSOPSOCTbl_3[] = //10-15℃下的SOC充电降功率表
{
	{-1, 0},
	{680,45},	//45%*C
	{750,40},	//40%*C
	{880,35},	//35%*C
	{1000,10},	//10%*C
	{0x7fff,0},
};
const t_SOP_TAB cSOPSOCTbl_4[] = //15-40℃下的SOC充电降功率表 
{
	{-1, 0},
	{850,70},	//70%*C
	{1000,50},	//50%*C
	{0x7fff,0},
};
const t_SOP_TAB cSOPSOCTbl_5[] = //40-45℃下的SOC充电降功率表 
{
	{-1, 0},
	{850,60},	//60%*C
	{1000,20},	//20%*C
	{0x7fff,0},
};
const t_SOP_TAB cSOPSOCTbl_6[] = //45-50℃下的SOC充电降功率表 
{
	{-1, 0},
	{800,33},	//33%*C
	{1000,10},	//10%*C
	{0x7fff,0},
};
*/
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,NULL},
    {550,(t_SOP_TAB *)cSOPSOCTbl_1},
    {0x7fff,NULL},
};

/*************************************10AH韩国电芯MF1*************************************/
#else
const t_SOP_TAB cSOPVoltTbl_1[] = //全温度范围，电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPVoltTbl_2[] =	//25-50℃下的单体电压充电降功率表
{
    {2000,0},	//0%*C
    {2800,5},	//5%*C
    {3000,50},	//50%*C
    {3500,50},	//50%*C
    {4300,100},	//100%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempVoltTblTbl[] = //温度对应的单体电压充电降功率表
{
    {0,	  NULL},
    {250,(t_SOP_TAB *)cSOPVoltTbl_1},		  //目前表1和表2一致，目的是为了以后方便拓展功能
    {500,(t_SOP_TAB *)cSOPVoltTbl_2},
    {0x7fff,NULL},
};

const t_SOP_TAB cSOPSOCTbl_1[] = //0-5 ℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {300,10},	//10%*C
    {850,20},	//20%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_2[] = //5-15℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {400,20},	//20%*C
    {900,40},	//40%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_3[] = //15-40℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {850,70},	//70%*C
    {1000,50},	//50%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_4[] = //40-45℃下的SOC充电降功率表 
{
    {-1, 0},	//0%*C
    {850,60},	//60%*C
    {1000,20},	//20%*C
    {0x7fff,0},	//0%*C
};
const t_SOP_TAB cSOPSOCTbl_5[] = //45-50℃下的SOC充电降功率表
{
    {-1, 0},	//0%*C
    {800,33},	//33%*C
    {1000,10},	//10%*C
    {0x7fff,0},	//0%*C
};
const t_Val_SOPTAB_TAB cSOPTempSOCTblTbl[] = //温度对应的SOC充电降功率表
{
    {0,NULL},
    {50, (t_SOP_TAB *)cSOPSOCTbl_1},
    {150,(t_SOP_TAB *)cSOPSOCTbl_2},
    {400,(t_SOP_TAB *)cSOPSOCTbl_3},
    {450,(t_SOP_TAB *)cSOPSOCTbl_4},
    {500,(t_SOP_TAB *)cSOPSOCTbl_5},
    {0x7fff,NULL},
};

#endif

/*****************************************end of SOPTab.c*****************************************/
