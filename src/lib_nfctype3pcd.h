/**
  ******************************************************************************
  * @file    lib_nfctype3pcd.h 
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
  * @brief   Generates the NFC type3 commands
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
#ifndef _LIB_NFCTYPE3PCD_H
#define _LIB_NFCTYPE3PCD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lib_iso18092pcd.h"

#define PCDNFCT3_OK 										PCDNFC_OK
#define PCDNFCT3_ERROR 									PCDNFC_ERROR
#define PCDNFCT3_ERROR_MEMORY_TAG 			PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT3_ERROR_MEMORY_INTERNAL 	PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT3_ERROR_LOCKED 					PCDNFC_ERROR_LOCKED

/* Command List */
#define PCDNFCT3_CHECK							0x06
#define PCDNFCT3_UPDATE							0x08

/* Service code */
#define PCDNFCT3_CODE_READ_MSB			0x00
#define PCDNFCT3_CODE_READ_LSB			0x0B
#define PCDNFCT3_CODE_WRITE_MSB			0x00
#define PCDNFCT3_CODE_WRITE_LSB			0x09
#define PCDNFCT3_FIRST_BLOC_MSB			0x80
#define PCDNFCT3_FIRST_BLOC_LSB			0x00

/* Flag */
#define PCDNFCT3_WRITE_ON						0x0F
#define PCDNFCT3_WRITE_OFF					0x00

/* Size */
#define PCDNFCT3_ATTR_SIZE					16

/* Functions */
uint8_t PCDNFCT3_ReadNDEF( void );
uint8_t PCDNFCT3_WriteNDEF( void );

#ifdef __cplusplus
}
#endif

#endif /* _LIB_NFCTYPE3PCD_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
