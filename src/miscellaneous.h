/**
  ******************************************************************************
  * @file    miscellaneous.h
  * @author  MMY Application Team
  * @version $Revision: 1507 $
  * @date    $Date: 2016-01-08 09:48:35 +0100 (Fri, 08 Jan 2016) $
  * @brief   miscellaneaous functions
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

#ifndef __MISCELLANEOUS_H
#define __MISCELLANEOUS_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
	 
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t  uc8;   /*!< Read Only */

	 
#define MAX(x,y) 				((x > y)? x : y)
#define MIN(x,y) 				((x < y)? x : y)  
#define ABS(x) 					((x)>0 ? (x) : -(x))  
#define CHECKVAL(val, min,max) 	((val < min || val > max) ? false : true) 
	 
#define GETMSB(val) 		((val & 0xFF00 )>>8 ) 
#define GETLSB(val) 		( val & 0x00FF ) 
 
#define RESULTOK 							0x00 
#define ERRORCODE_GENERIC 		1 

#ifndef errchk
#define errchk(fCall) if (status = (fCall), status != RESULTOK) \
	{goto Error;} else
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MISCELLANEOUS_H */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
