/**
  ******************************************************************************
  * @file    lib_iso7816pcd.h
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
  * @brief   Manage the iso 7816 commands
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty  
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------------------------*/
#ifndef __LIB_ISO7816_H
#define __LIB_ISO7816_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes -------------------------------------------------------------------------------------*/
#include "lib_iso14443Apcd.h"
#include "lib_pcd.h"

/*  status and error code ---------------------------------------------------------------------- */
#define ISO7816_SUCCESSCODE											RESULTOK
#define ISO7816_ERRORCODE_DEFAULT								0x71
#define ISO7816_ERRORCODE_RESPONSE							0x72
#define ISO7816_ERRORCODE_SENDERRORCODE					0x73

/*  Iblock ------------------------------------------------------------------------------------- */
#define ISO7816_IBLOCK02												0x02
#define ISO7816_IBLOCK03												0x03

#define ISO7816_SELECT_FILE     								0xA4
#define ISO7816_UPDATE_BINARY   								0xD6
#define ISO7816_READ_BINARY     								0xB0

#define ISO7816_CLASS_0X00											0x00
#define ISO7816_CLASS_STM												0xA2

// offset of the different field of the APDU
#define ISO7816_ADPUOFFSET_BLOCK		0x00
#define ISO7816_ADPUOFFSET_CLASS		ISO7816_ADPUOFFSET_BLOCK 		+ 1
#define ISO7816_ADPUOFFSET_INS			ISO7816_ADPUOFFSET_BLOCK 		+ 2
#define ISO7816_ADPUOFFSET_P1				ISO7816_ADPUOFFSET_BLOCK		+ 3
#define ISO7816_ADPUOFFSET_P2				ISO7816_ADPUOFFSET_BLOCK 		+ 4
#define ISO7816_ADPUOFFSET_LC				ISO7816_ADPUOFFSET_BLOCK 		+	5
#define ISO7816_ADPUOFFSET_DATA			ISO7816_ADPUOFFSET_BLOCK 		+ 6

/* ADPU-Header command structure ---------------------------------------------*/
typedef struct
{
  uint8_t CLA;  /* Command class */
  uint8_t INS;  /* Operation code */
  uint8_t P1;   /* Selection Mode */
  uint8_t P2;   /* Selection Option */
} Header;

/* ADPU-Body command structure -----------------------------------------------*/
typedef struct 
{
  uint8_t LC;         						  /* Data field length */	
  uint8_t pData[256];  							/* Command parameters */ // pointer on the transceiver buffer = *(ReaderRecBuf[CR95HF_DATA_OFFSET + ISO7816_ADPUOFFSET_DATA])
  uint8_t LE;          						 /* Expected length of data to be returned */
} Body;

/* ADPU Command structure ----------------------------------------------------*/
typedef struct
{
  Header Header;
  Body Body;
} APDU_Commands;

/* SC response structure -----------------------------------------------------*/
typedef struct
{
  uint8_t SW1;         						 /* Command Processing status */
  uint8_t SW2;          						/* Command Processing qualification */
} APDU_Responce;


int8_t 	ISO7816_SelectFile(const uint8_t P1byte , const uint8_t P2byte , const uint8_t LCbyte , uint8_t *PData);
int8_t 	ISO7816_ReadBinary(const uint8_t P1byte , const uint8_t P2byte , const uint8_t LEbyte , uint8_t *pDataRead);
int8_t 	ISO7816_UpdateBinary(const uint8_t P1byte , const uint8_t P2byte , const uint8_t LCbyte , uint8_t *pData);

#ifdef __cplusplus
}
#endif

#endif /* __SMARTCARD_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
