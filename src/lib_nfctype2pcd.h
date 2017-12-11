/**
  ******************************************************************************
  * @file    lib_nfctype2pcd.h 
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
  * @brief   Generates the NFC type2 commands
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LIB_NFCTYPE2PCD_H
#define _LIB_NFCTYPE2PCD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lib_pcd.h"

/* Error codes */
#define PCDNFCT2_OK 											PCDNFC_OK
#define PCDNFCT2_ERROR 										PCDNFC_ERROR
#define PCDNFCT2_ERROR_MEMORY_TAG					PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT2_ERROR_MEMORY_INTERNAL		PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT2_ERROR_LOCKED 						PCDNFC_ERROR_LOCKED
#define PCDNFCT2_ERROR_NOT_FORMATED				PCDNFC_ERROR_NOT_FORMATED

/* Command List */
#define PCDNFCT2_READ								0x30
#define PCDNFCT2_WRITE							0xA2
#define PCDNFCT2_SECTOR_SEL					0xC2

/* Size */
#define PCDNFCT2_READ_SIZE					16
#define PCDNFCT2_READ_SIZE_BUFFER		18
#ifndef PCDNFCT2_SECTOR_SIZE
// if not defined at application level, let's use 1024 for wider compatibility
// it can be reduced to lower values, e.g. 256, but with restrictions :
//       issue on second sector read
#define PCDNFCT2_SECTOR_SIZE		1024
#endif

/* Mask */
#define PCDNFCT2_READ_MSK						0xF0
#define PCDNFCT2_WRITE_MSK					0x0F

/* TLV */
#define PCDNFCT2_TLV_EMPTY 					0x00
#define PCDNFCT2_TLV_LOCK 					0x01
#define PCDNFCT2_TLV_MEM	 					0x02
#define PCDNFCT2_TLV_NDEF	 					0x03

#define PCDNFCT2_NDEF_MNB						0xE1

/* Functions */
uint8_t PCDNFCT2_ReadNDEF( void );
uint8_t PCDNFCT2_WriteNDEF( void );

#ifdef __cplusplus
}
#endif

#endif /* _LIB_NFCTYPE2PCD_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
