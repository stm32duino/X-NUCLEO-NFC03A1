/**
  ******************************************************************************
  * @file    common.h
  * @author  MMY Application Team
  * @version $Revision: 1507 $
  * @date    $Date: 2016-01-08 09:48:35 +0100 (Fri, 08 Jan 2016) $
  * @brief   This file provides all the headers of the common functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
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

  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _COMMON_H
#define _COMMON_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#define NFC_TAG_MEMORY          512

#include "miscellaneous.h"
#include "lib_95HFConfigManager.h"
#include "lib_wrapper.h"
#include "lib_NDEF_URI.h"
#include "drv_spi.h"


#define MAX_NDEF_MEM            0x50

/* Nfc Mandatory define ------------------------------------------------------*/

#define RESULTOK		0x00 
#define ERRORCODE_GENERIC	1 
#define ACTION_COMPLETED	0x9000

/*  status and erroc code --------------------------------------------------- */
#define MAIN_SUCCESS_CODE	RESULTOK

/* Enum for ST95 state aligned with new lib Ndef*/
typedef enum {UNDEFINED_MODE=0,PICC,PCD}DeviceMode_t;
typedef enum {UNDEFINED_TAG_TYPE=0,TT1,TT2,TT3,TT4A,TT4B,TT5}TagType_t;

/* external constants --------------------------------------------------------*/
extern bool HID_TRANSACTION;

/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);

/* Exported constants --------------------------------------------------------*/
/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE         128

#define ApplicationAddress      0x8009000

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */

/* Compute the FLASH upload image size */  
#define FLASH_IMAGE_SIZE         (uint32_t) (FLASH_SIZE - (ApplicationAddress - 0x08000000))

/* TT1 */
#define NFCT1_MAX_TAGMEMORY	(120+2)

/* TT2 */
#define NFCT2_MAX_TAGMEMORY	NFC_TAG_MEMORY /*(must be a multiple of 8) */ 
#define NFCT2_MAX_CC_SIZE	4	
#define NFCT2_MAX_CONFIG	12	
#define NFCT2_MAX_NDEFFILE	(NFCT2_MAX_TAGMEMORY-NFCT2_MAX_CC_SIZE-NFCT2_MAX_CONFIG)

/* TT3 */
#define NFCT3_ATTRIB_INFO_SIZE 	16
#define NFCT3_MAX_NDEFFILE	NFC_TAG_MEMORY
#define NFCT3_MAX_TAGMEMORY 	(NFCT3_MAX_NDEFFILE+NFCT3_ATTRIB_INFO_SIZE)
#define NFCT3_NB_BLOC_MSB	((NFCT3_MAX_TAGMEMORY/16)>>8)
#define NFCT3_NB_BLOC_LSB	((NFCT3_MAX_TAGMEMORY/16)&0x00FF)

/* TT4 */
#define NFCT4_MAX_CCMEMORY	16
#define NFCT4A_MAX_NDEFMEMORY	NFC_TAG_MEMORY
#define NFCT4B_MAX_NDEFMEMORY	NFC_TAG_MEMORY
#define NFCT4_MAX_NDEFMEMORY	NFCT4A_MAX_NDEFMEMORY

/* TT5 */
#define NFCT5_MAX_TAGMEMORY	NFC_TAG_MEMORY

#define NFC_DEVICE_MAX_NDEFMEMORY	NFCT4_MAX_NDEFMEMORY
/* Exported macro ------------------------------------------------------------*/
/* Common routines */
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_H */

/*******************(C)COPYRIGHT 2016 STMicroelectronics *****END OF FILE******/
