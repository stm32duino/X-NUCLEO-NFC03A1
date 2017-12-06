/**
  ******************************************************************************
  * @file    lib_iso18092pcd.h 
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
  * @brief   Manage the iso18092 communication
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
#ifndef __FELICA_H
#define __FELICA_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lib_pcd.h"
#include "stdbool.h"

/* 	-------------------------------------------------------------------------- */
#define ATQC_SIZE																				0x20
#define UID_SIZE_FELICA																	8

/* command code -------------------------------------------------------------- */
#define ISO18092_COMMAND_REQC														0x00
#define ISO18092_RESPONSE_REQC													0x01

/* code status	-------------------------------------------------------------------------- */
#define ISO18092_SUCCESSCODE														RESULTOK
#define ISO18092_ERRORCODE_DEFAULT											0xC1

typedef struct{
	uint8_t ATQC[ATQC_SIZE];
	uint8_t UID	[UID_SIZE_FELICA];
	bool 		IsDetected;
//	char		LogMsg[120];
}FELICA_CARD;

//extern FELICA_CARD 	FELICA_Card;

/* ---------------------------------------------------------------------------------
 * --- Local Functions  
 * --------------------------------------------------------------------------------- */
void 	 FELICA_Initialization( void );
int8_t FELICA_IsPresent		( void );
int8_t FELICA_CardTest			( void );
int8_t FELICA_Anticollision( void );

#ifdef __cplusplus
}
#endif

#endif /* __FELICA_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
