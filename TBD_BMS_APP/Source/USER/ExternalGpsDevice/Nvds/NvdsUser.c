
#include "Common.h"
#include "NvdsUser.h"

#define NVDS_ITEM_COUNT 3

CfgInfo	g_cfgInfo;
PdoInfo	g_pdoInfo;
DbgInfo	g_degInfo;

//g_pdoInfo的镜像对象，用于和原值作比较，决定是否写入FLASH
static CfgInfo g_cfgInfo_mirror;
static PdoInfo g_pdoInfo_mirror;
//static DbgInfo g_dbgInfo_mirror;

//上面多个存储变量的公用的交换缓冲区，长度取最大值
static uint8_t		g_exchBuf[sizeof(PdoInfo)];
static SectorMgr    g_nvdsSecMgr[NVDS_ITEM_COUNT];

#if 0
void g_pdoInfo_Dump(void)
{
	#define PRINTF_NVDS(_field) Printf("\t%s=%d\n", #_field, g_pdoInfo._field);
	
	Printf("Dump g_pdoInfo:\n");
	
	PRINTF_NVDS(isFlashOk);
	PRINTF_NVDS(isGyroOk);
}
#endif

//从Flsah读取数据
Bool Nvde_Read(uint32 addr, void* buf, int len)
{
//	spi_flash_buffer_read(addr,buf,len);	//待定
	return True;
}

//写数据到Flsah
Bool Nvds_Write(uint32 addr, const void* pData, int len)
{
//	spi_flash_buffer_write(addr,(uint8_t*)pData,len);	//待定
	return True;
}

//Flsah擦除
void Nvds_Erase(uint32 addr, int len)
{
//	spi_flash_sector_erase(addr);	//待定
}

Bool Nvds_Verify(const uint8_t* pItem)	//LogItem
{
	return True;
}

static Bool CfgInfo_Event(CfgInfo* p, NvdsEventID eventId)
{
	//从Flash读取的数据错误，设定默认值
	if (eventId == BE_DATA_ERROR)
	{
		memset(p, 0, sizeof(CfgInfo));
	}

	return True;
}

static Bool PdoInfo_Event(PdoInfo* p, NvdsEventID eventId)
{
	//从Flash读取的数据错误，设定默认值
	if (eventId == BE_DATA_ERROR)
	{
		memset(p, 0, sizeof(PdoInfo));
	}
	return True;
}
#if 0
static Bool DbgInfo_Event(DbgInfo* p, NvdsEventID eventId)
{
	//从Flash读取的数据错误，设定默认值
	if (eventId == BE_DATA_ERROR)
	{
		//数据错误，设定默认值
		memset(p, 0, sizeof(DbgInfo));

		//不能再这里打印输出，如果UART没初始化，会导致程序堵塞
		g_dwDebugLevel = 0xF;
	}
	else if (eventId == BE_DATA_OK)
	{
		g_dwDebugLevel = p->debugLevel;
	}
	else if (eventId == BE_ON_WRITE_BEFORE)
	{
		p->debugLevel = g_dwDebugLevel;
	}

	return True;
}
#endif

//配置每个存储结构存放在FLASH的位置EX_FLASH_SECTOR_SIZE
const NvdsItem g_NvdsItems[NVDS_ITEM_COUNT] =
{
	//待定

	{NVDS_CFG_INFO, {e_Nvds_Cfg_Info, sizeof(CfgInfo), &g_cfgInfo , sizeof(g_cfgInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase}, 
		&g_nvdsSecMgr[0], (NvdsEventFn)CfgInfo_Event, & g_cfgInfo_mirror},
	{NVDS_PDO_INFO, {e_Nvds_Pdo_Info, sizeof(PdoInfo), &g_pdoInfo , sizeof(g_pdoInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase},
		&g_nvdsSecMgr[1], (NvdsEventFn)PdoInfo_Event,& g_pdoInfo_mirror},

#if 0		
	{NVDS_DBG_INFO, {EX_FLASH_CFG_DBG_INFO_ADDR, EX_FLASH_SECTOR_SIZE, &g_degInfo , sizeof(g_degInfo), 
		g_exchBuf, sizeof(g_exchBuf),(ItemVerifyFn)Nvds_Verify, Nvde_Read, Nvds_Write, Nvds_Erase},
		&g_nvdsSecMgr[2], (NvdsEventFn)DbgInfo_Event,& g_dbgInfo_mirror},
#endif		
};

//写入指定的NvdsID
void NvdsUser_Write(NvdsID id)
{
	Nvds_write(id);
}

//Nvds初始化函数，使用改模块的任何功能之前，必须先调用该函数
void NvdsUser_Init()
{
	uint32_t flash_id = 0;
	
	Nvds_Init(g_NvdsItems, GET_ELEMENT_COUNT(g_NvdsItems));
//	flash_id = spi_flash_read_id();				//待定
//	Printf("The Flash_ID:0x%X\n\r",flash_id);
	g_pdoInfo.isFlashOk = (flash_id == 0xC22015)? 1:0;
}
