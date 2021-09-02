/**************************************************************************//**
 * @file xmodem.c
 * @brief XMODEM protocol
 * @version x.xx
 ******************************************************************************
 * @section License
 * <b>Copyright 2015 Silicon Laboratories, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#include <stdio.h>
#include "xmodem.h"
#include "crc.h"
#include "CanComm.h"
#include "version.h"
#include "BootLoader.h"
#include "stm32g0xx_hal.h"
#include "Comm.h"
#include "BSP_IWDG.h"
#include "BSP_FLASH.h"
//#include "BSPTimer.h"
//#include "BSPFlash.h"
//#include "stm32g0xx_hal_flash.h"
//#include "stm32g0xx_hal_def.h"
//#include "BSPWatchDog.h"

#define ALIGNMENT(base,align) (((base)+((align)-1))&(~((align)-1)))

//static uint8_t XM_ACK[1] = {XMODEM_ACK};
//static uint8_t XM_NAK[1] = {XMODEM_NAK};
//static uint8_t XM_CAN[1] = {XMODEM_CAN};
//static uint8_t XM_NCG[1] = {XMODEM_NCG};

// Packet storage. Double buffered version.
//#pragma data_alignment=4
uint8_t rawPacket[2][ALIGNMENT(sizeof(XMODEM_packet),4)];
u16 flasherr = 0;    //flash写入错误

u8 IsXmodem = False;

/**************************************************************************//**
 * @brief Verifies checksum, packet numbering and
 * @param pkt The packet to verify
 * @param sequenceNumber The current sequence number.
 * @returns -1 on packet error, 0 otherwise
 *****************************************************************************/
//__ramfunc 
inline int XMODEM_verifyPacketChecksum(XMODEM_packet *pkt, int sequenceNumber)
{
  uint16_t packetCRC;
  uint16_t calculatedCRC;
    (void)sequenceNumber;
  // Check the packet number integrity.
  if (pkt->packetNumber + pkt->packetNumberC != 255)
  {
    return -1;
  }

  calculatedCRC = CRC_calc((uint8_t *) pkt->data, (uint8_t *) &(pkt->crcHigh));
  packetCRC     = pkt->crcHigh << 8 | pkt->crcLow;

  // Check the CRC value.
  if (calculatedCRC != packetCRC)
  {
    return -1;
  }
  return 0;
}

/**************************************************************************//**
 * @brief Starts a XMODEM download.
 *
 * @param baseAddress
 *   The address to start writing from
 *
 * @param endAddress
 *   The last address. This is only used for clearing the flash
 *****************************************************************************/
void XMODEM_download(uint32_t baseAddress, uint32_t endAddress)
{
    XMODEM_packet *pkt;
    uint32_t      addr;
    uint32_t      sequenceNumber = 1;
    uint32_t      dataLen = 0;
    static uint16_t cnt;
    uint8_t packetNumber;
    uint8_t XM_ACK[1],XM_NAK[1],XM_CAN[1],XM_NCG[1];
    XM_ACK[0] = XMODEM_ACK;
    XM_NAK[0] = XMODEM_NAK;
    XM_CAN[0] = XMODEM_CAN;
    XM_NCG[0] = XMODEM_NCG;
    //FM11NC08FrameTx(c_FUNCODE, XM_NCG,1,TIMEOUTMS(10));//发送'C'
    CanCommFrameTx(BOOT_FUNCODE, XM_NCG,1,TIMEOUTMS(10));//发送'C'
    IsXmodem = True;

    //擦除App
    //while (DMA->CHENS & DMA_CHENS_CH0ENS);
	//BSPFlashEraseBlock((uint32_t)&cAPPInfo);
//	BSPFLASHEraseSector(((baseAddress / FLASH_PAGE_SIZE) - 1)); //先擦除app info区
	EraseFlash( ((baseAddress - FLASH_BASE) / FLASH_PAGE_SIZE) - 1 ); //先擦除app info区		
    FeedIDog();

    //擦除应用程序标志
//    BSPEEPWriteOneWord(e_BOOT_PRO_FLAG_ADDR,0xFFFF);
	WriteFlagToFlash(e_BOOT_PRO_FLAG_ADDR,0xFFFF);
    for (addr = ((baseAddress - FLASH_BASE) / FLASH_PAGE_SIZE); addr <= 127; addr += 1)
    {
        //BSPFlashEraseBlock(addr);
//		BSPFLASHEraseSector(addr); 
		EraseFlash(addr); 
        FeedIDog();
    }

    while (1)
    {
        // Swap buffer for packet buffer.
        pkt = (XMODEM_packet *)rawPacket[sequenceNumber & 1];

        cnt = 133;
        //if(!FM11NC08FrameRx(&pkt->header,&cnt,TIMEOUTMS(30000)))    //接收xmodem帧
        if(!CanCommFrameRx(&pkt->header,&cnt,TIMEOUTMS(30000)))    //接收xmodem帧
        {
            //30s超时后返回
           goto  XMODEM_END;
        }
        IsXmodem = False;
        
        if(cnt == 1)
        {
            if(pkt->header == XMODEM_EOT)// Check for end of transfer.
            {
                // Acknowledget End of transfer.
                //FM11NC08FrameTx(c_FUNCODE, XM_ACK,1,TIMEOUTMS(10));
                CanCommFrameTx(BOOT_FUNCODE, XM_ACK,1,TIMEOUTMS(10));
                break;
            }
            else
            {
                //FM11NC08FrameTx(c_FUNCODE, XM_CAN,1,TIMEOUTMS(10));
                CanCommFrameTx(BOOT_FUNCODE, XM_CAN,1,TIMEOUTMS(10));
                goto XMODEM_END;
            }
        }
        else if(cnt!=133)
        {
            //FM11NC08FrameTx(c_FUNCODE, XM_CAN,1,TIMEOUTMS(10));
            CanCommFrameTx(BOOT_FUNCODE, XM_CAN,1,TIMEOUTMS(10));
            
            goto XMODEM_END;
        }

        //校验crc和包序号
        if (XMODEM_verifyPacketChecksum(pkt, sequenceNumber) != 0)
        {
            
            // On a malformed packet, we send a NAK, and start over.
            //FM11NC08FrameTx(c_FUNCODE, XM_NAK,1,TIMEOUTMS(10));
            CanCommFrameTx(BOOT_FUNCODE, XM_NAK,1,TIMEOUTMS(10));
            continue;
        }

        // If the header is not a start of header (SOH), then cancel the transfer.
        if (pkt->header != XMODEM_SOH)
        {
            //FM11NC08FrameTx(c_FUNCODE, XM_CAN,1,TIMEOUTMS(10));
            CanCommFrameTx(BOOT_FUNCODE, XM_CAN,1,TIMEOUTMS(10));
            goto XMODEM_END;
        }
        
        if(((sequenceNumber)*XMODEM_DATA_SIZE) > (endAddress-baseAddress))//包超过范围
        {
            //FM11NC08FrameTx(c_FUNCODE, XM_CAN,1,TIMEOUTMS(10)); 
            CanCommFrameTx(BOOT_FUNCODE, XM_CAN,1,TIMEOUTMS(10));
            goto XMODEM_END;
        }

        packetNumber = (uint8_t)(sequenceNumber&0xff);    
        if((pkt->packetNumber+1) == packetNumber)//收到的包序号比预期的少1
        {
            //FM11NC08FrameTx(c_FUNCODE, XM_ACK,1,TIMEOUTMS(10));
            CanCommFrameTx(BOOT_FUNCODE, XM_ACK,1,TIMEOUTMS(10));
            continue;
        }
        else if(pkt->packetNumber != packetNumber)
        {
            //FM11NC08FrameTx(c_FUNCODE, XM_CAN,1,TIMEOUTMS(10));  
            CanCommFrameTx(BOOT_FUNCODE, XM_CAN,1,TIMEOUTMS(10));
            goto XMODEM_END;
        }
        
        // Send ACK.
        //FM11NC08FrameTx(c_FUNCODE, XM_ACK,1,TIMEOUTMS(10));
//        CanCommFrameTx(BOOT_FUNCODE, XM_ACK,1,TIMEOUTMS(10));
        // Write data to flash.
        
//        if(BSPFlashWriteByte(baseAddress + (sequenceNumber - 1) * XMODEM_DATA_SIZE,(uint8_t *)pkt->data,XMODEM_DATA_SIZE) > 0)
		//if(WriteDataToFlash(baseAddress + (sequenceNumber - 1) * XMODEM_DATA_SIZE,(uint8_t *)pkt->data,XMODEM_DATA_SIZE) > 0)
        if(UpDataToFlash(baseAddress + (sequenceNumber - 1) * XMODEM_DATA_SIZE,(uint8_t *)pkt->data,XMODEM_DATA_SIZE) > 0)
        {
            flasherr++;//写入错误
        }
        sequenceNumber++;
        CanCommFrameTx(BOOT_FUNCODE, XM_ACK,1,TIMEOUTMS(10));
    }
    
//  // Wait for the last DMA transfer to finish.
//#if defined(_SILICON_LABS_32B_PLATFORM_2)
//  while (LDMA->CHEN & 1);
//#else
//  //while (DMA->CHENS & DMA_CHENS_CH0ENS);
//#endif
  //保存App info 到最后一个block 1k
    if(sequenceNumber < 2)
    {
        goto XMODEM_END;
    }
  
    dataLen = (sequenceNumber - 2) * 128;
    StringCopy(pkt->data, (uint8_t*)(baseAddress + dataLen),128);//拷贝info区域
  
    // Write data to flash.
    WriteDataToFlash((uint32_t)&cAPPInfo,(uint8_t *)pkt->data,XMODEM_DATA_SIZE);
//    BSPEEPWriteOneWord(e_BOOT_PRO_FLAG_ADDR,BOOT_PRO_FLAG); //写入应用程序标志
	WriteFlagToFlash(e_BOOT_PRO_FLAG_ADDR,BOOT_PRO_FLAG); //写入应用程序标志
//  BSPFlashWriteByte((void *)0x0000FC00,
//                   (uint32_t)&cAPPInfo -0xFC00,
//                   XMODEM_DATA_SIZE,
//                   (uint8_t const *)pkt->data);        
XMODEM_END:
    IsXmodem = False;
//while (DMA->CHENS & DMA_CHENS_CH0ENS);
    return;
}
