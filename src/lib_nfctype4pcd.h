/**
  ******************************************************************************
  * @file    lib_nfctype4pcd.h 
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
  * @brief   Generates the NFC type4 commands
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
#ifndef _LIB_NFCTYPE4PCD_H
#define _LIB_NFCTYPE4PCD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lib_iso7816pcd.h"

#define PCDNFCT4_OK 										PCDNFC_OK
#define PCDNFCT4_ERROR 									PCDNFC_ERROR
#define PCDNFCT4_ERROR_MEMORY_TAG				PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT4_ERROR_MEMORY_INTERNAL	PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT4_ERROR_LOCKED 					PCDNFC_ERROR_LOCKED

/* Command List */
#define PCDNFCT4_SELECT_APPLI				{0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00}		
#define PCDNFCT4_CC_ID							{0xE1,0x03}

/* Size */
#define PCDNFCT4_BUFFER_READ				256

#define PCDNFCT4_ACCESS_ALLOWED			0x00

uint8_t PCDNFCT4_ReadNDEF(void);
uint8_t PCDNFCT4_WriteNDEF( void );

#ifdef __cplusplus
}
#endif

#endif /* _LIB_NFCTYPE4PCD_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
