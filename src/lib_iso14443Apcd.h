/**
  ******************************************************************************
  * @file    lib_iso14443Apcd.h
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
  * @brief   Manage the iso14443A communication
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
  
/* Define to prevent recursive inclusion -------------------------------------------------------*/
#ifndef __ISO14443APCD_H
#define __ISO14443APCD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lib_pcd.h"
#include "lib_iso14443A.h"

/*  status and error code ---------------------------------------------------------------------- */
#define ISO14443A_SUCCESSCODE									RESULTOK
#define ISO14443A_ERRORCODE_DEFAULT						0x61
#define ISO14443A_ERRORCODE_CRC								0x62


/* Anticollison levels (commands)  ------------------------------------------------------------- */
#define SEL_CASCADE_LVL_1											0x93
#define SEL_CASCADE_LVL_2											0x95
#define SEL_CASCADE_LVL_3											0x97
#define COMMAND_RATS													0xE0
#define COMMAND_PPS														0xD0

/* Iblock  ------------------------------------------------------------------------------------- */
#define COMMAND_IBLOCK02												0x02
#define COMMAND_IBLOCK03												0x03
#define COMMAND_SBLOCK													0xC2
#define COMMAND_NACKBLOCK												0xB2
#define COMMAND_ACKBLOCK												0xA2


/* numbr of the cascade level  ----------------------------------------------------------------- */
#define CASCADE_LVL_1															1
#define CASCADE_LVL_2															2
#define CASCADE_LVL_3															3
		
#define ISO14443A_NVM_10													0x10
#define ISO14443A_NVM_20													0x20
#define ISO14443A_NVM_30													0x30
#define ISO14443A_NVM_40													0x40
#define ISO14443A_NVM_50													0x50
#define ISO14443A_NVM_60													0x60
#define ISO14443A_NVM_70													0x70

/* UID Sizes ---------------------------------------------------------------------------------- */
#define ISO14443A_UIDSIZE_UNDEFINED								-1
#define ISO14443A_UID_PART												3
#define ISO14443A_UID_SINGLE_SIZE									4
#define	ISO14443A_UID_DOUBLE_SIZE									7
#define ISO14443A_UID_TRIPLE_SIZE									10


/* Mask used for ATQA ------------------------------------------------------------------------ */
#define ISO14443A_UID_MASK												0xC0
#define ISO14443A_AC_BIT_FRAME_MASK								0x1F
#define ISO14443A_CID_MASK												0x0F
#define ISO14443A_FSDI_MASK												0xF0

/* Size for ISO14443A variables ------------------------------------------------------------- */
#define ISO14443A_MAX_NAME_SIZE										50
#define ISO14443A_MAX_UID_SIZE			 							10
#define ISO14443A_ATQA_SIZE												2

/* SAK FLAG --------------------------------------------------------------------------------- */
#define SAK_FLAG_ATS_SUPPORTED										0x20
#define SAK_FLAG_UID_NOT_COMPLETE									0x04

/* ATQ FLAG */
#define ATQ_FLAG_UID_SINGLE_SIZE		0
#define	ATQ_FLAG_UID_DOUBLE_SIZE		1
#define ATQ_FLAG_UID_TRIPLE_SIZE		2

/* MultiTag Huntig -------------------------------------------------------------------------- */
#define ISO14443A_NB_TAG_MAX				10

/* ---------------------------------------------------------------------------------
 * --- Local Functions  
 * --------------------------------------------------------------------------------- */
void ISO14443A_Reset			( void );
int8_t ISO14443A_Init 		( void );
int8_t ISO14443A_IsPresent		( void );
int8_t ISO14443A_Anticollision	( void );
int8_t TOPAZ_ID( uint8_t *pDataRead);

/* To manage dynamicaly FDT */
int8_t ISO14443A_ConfigFDTforAnticollision( void);
int8_t ISO14443A_ConfigFDTforRATS( void);
int8_t ISO14443A_ConfigFDT( uint8_t WTXM);

void ISO14443A_MultiTagHunting ( uint8_t* pNbTag, uint8_t *pUIDout );
void ISO14443A_MultiTagPart2 ( uint8_t *pNbTag, uint8_t *pUIDout );

#ifdef __cplusplus
}
#endif

#endif /* __ISO14443A_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
