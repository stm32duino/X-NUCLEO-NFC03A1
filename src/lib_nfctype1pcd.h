/**
  ******************************************************************************
  * @file    lib_nfctype1pcd.h 
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
  * @brief   Generates the NFC type1 commands
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
#ifndef _LIB_NFCTYPE1PCD_H
#define _LIB_NFCTYPE1PCD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lib_pcd.h"

/* Error codes */
#define PCDNFCT1_OK 										PCDNFC_OK
#define PCDNFCT1_ERROR 									PCDNFC_ERROR
#define PCDNFCT1_ERROR_MEMORY_TAG 			PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT1_ERROR_MEMORY_INTERNAL 	PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT1_ERROR_NOT_FORMATED			PCDNFC_ERROR_NOT_FORMATED

/* Command List */
#define PCDNFCT1_READALL						0x00
#define PCDNFCT1_WRITE_E						0x53

/* Mask */
#define PCDNFCT1_NDEF_CAPABLE_MSK 	0xF0

/* TLV */
#define PCDNFCT1_TLV_EMPTY 					0x00
#define PCDNFCT1_TLV_LOCK 					0x01
#define PCDNFCT1_TLV_MEM	 					0x02
#define PCDNFCT1_TLV_NDEF	 					0x03

#define PCDNFCT1_NDEF_MNB						0xE1
#define PCDNFCT1_NDEF_DISABLE				0x00

#define PCDNFCT1_TOPAZ_MODE					0xA8

/* Functions */
uint8_t PCDNFCT1_ReadNDEF( void );
uint8_t PCDNFCT1_WriteNDEF( void );

#ifdef __cplusplus
}
#endif

#endif /* _LIB_NFCTYPE1PCD_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
