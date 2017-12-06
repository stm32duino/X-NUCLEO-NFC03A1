/**
  ******************************************************************************
  * @file    lib_wrapper.h
  * @author  MMY Application Team
  * @version $Revision: 1329 $
  * @date    $Date: 2015-11-05 10:34:25 +0100 (Thu, 05 Nov 2015) $
  * @brief   This file help to have upper layer independent from HW
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
#ifndef __LIB_WRAPPER_H
#define __LIB_WRAPPER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "common.h"


uint16_t ReadData( uint16_t Offset, uint16_t DataSize, uint8_t* pData );
uint16_t WriteData( uint16_t Offset, uint32_t DataSize, uint8_t* pData );

#ifdef __cplusplus
}
#endif

#endif /* __LIB_WRAPPER_H */


/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
