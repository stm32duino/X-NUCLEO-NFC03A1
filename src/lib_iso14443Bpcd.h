/**
  ******************************************************************************
  * @file    lib_iso14443Bpcd.h
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
  * @brief   Manage the iso14443B communication
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
#ifndef __LIB_ISO14443B_H
#define __LIB_ISO14443B_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lib_pcd.h"
 
/*  status and error code ---------------------------------------------------------------------- */
#define ISO14443B_SUCCESSCODE											RESULTOK
#define ISO14443B_ERRORCODE_DEFAULT								0x71
#define ISO14443B_ERRORCODE_CRC										0x72


/* REQB/WUPB command
 * ---------------------------------------------------
 * 1st byte	 | 2nd byte  | 3rd byte | 4th, 5th bytes
 * ---------------------------------------------------
 *    APf    |    AFI    |   PARAM  |      CRC
 * ---------------------------------------------------
 */
#define ISO14443B_ANTICOLLISION_PREFIX_BYTE    	0x05 
#define ISO14443B_REQB											    0x05 

#define ISO14443B_AFI_ALL_FAMILIES             0x00 /*Application Family Identifier(AFI)*/

/* REQB/WUPB PARAM (1 byte)
 * -------------------------------------------------------------------------------------
 * RFU(3 bits) | Extended ATQB supported(1 bit) | REQB/WUPB(1 bit) | SLOT MARKER(3 bits)
 * -------------------------------------------------------------------------------------
 */
#define ISO14443B_EXTENDED_ATQB_SUPPORTED      1<<4
#define ISO14443B_EXTENDED_ATQB_NOT_SUPPORTED  0

#define ISO14443B_WUPB_ATTEMPT                 1<<3
#define ISO14443B_REQB_ATTEMPT                 0          

#define ISO14443B_SLOT_MARKER_1								 0
#define ISO14443B_SLOT_MARKER_2								 0x01
#define ISO14443B_SLOT_MARKER_4								 0x02
#define ISO14443B_SLOT_MARKER_8								 3<<1
#define ISO14443B_SLOT_MARKER_16							 0x04

/* ATTRIB command
 * ---------------------------------------------------------------------------------------------------------------
 * 1st byte	| 2th to 5th bytes  | 6th byte | 7th byte | 8th byte | 9th byte |	10th ... 'n'th bytes  | (2 bytes)
 * ---------------------------------------------------------------------------------------------------------------
 *   '1D'   | Identifier(PUPI)  | PARAM 1  | PARAM 2  | PARAM 3  | PARAM 4  | Higher Layer INF(opt) |	   CRC
 * ---------------------------------------------------------------------------------------------------------------
 */
#define ATTRIB_FIRST_BYTE            0x1D

/* ATTRIB PARAM 1 (1 byte)
 * ---------------------------------------------------------------------------------
 * Minimum TR0(2 bits) | Minimum TR1(2 bits) | EOF(1 bit) | SOF(1 bit) | RFU(2 bits)
 * ---------------------------------------------------------------------------------
 */
#define TR0_64_FS                    0
#define TR0_32_FS										 1<<6
#define TR0_16_FS										 1<<7

#define TR1_64_FS										 0
#define TR1_32_FS										 1<<4
#define TR1_16_FS										 1<<5

#define SOF_REQUIRED								 0
#define SOF_NOT_REQUIRED						 1<<2

#define EOF_REQUIRED								 0
#define EOF_NOT_REQUIRED						 1<<3

/* ATTRIB PARAM 2 (1 byte)
 * -------------------------------------------------------------------------------
 * PICC TO PCD SPEED(2 bits) | PCD TO PICC SPEED(2 bits) | MAX_FRAME_SIZE(2 bits)
 * -------------------------------------------------------------------------------
 */
#define MAX_FRAME_SIZE_16_BYTES      0
#define MAX_FRAME_SIZE_24_BYTES			 1
#define MAX_FRAME_SIZE_32_BYTES			 2
#define MAX_FRAME_SIZE_40_BYTES			 3
#define MAX_FRAME_SIZE_48_BYTES			 4
#define MAX_FRAME_SIZE_64_BYTES			 5
#define MAX_FRAME_SIZE_96_BYTES			 6
#define MAX_FRAME_SIZE_128_BYTES		 7
#define MAX_FRAME_SIZE_256_BYTES		 8

#define PCD_TO_PICC_106K             0
#define PCD_TO_PICC_212K						 1<<4
#define PCD_TO_PICC_424K						 1<<5
#define PCD_TO_PICC_848K						 3<<4

#define PICC_TO_PCD_106K						 0
#define PICC_TO_PCD_212K						 1<<6
#define PICC_TO_PCD_424K						 1<<7
#define PICC_TO_PCD_848K						 3<<6

/* ATTRIB PARAM 3 (1 byte)
 * -------------------------------------------------------
 * RFU(5 bits) | TR2 CODING(2 bits) | PROTOCOL TYPE(1 bit)
 * -------------------------------------------------------
 * The least significant byte is used for confirmation of 
 * the protocol type recovered in the ATQB
 */
#define TR2_32_FS                      0
#define TR2_128_FS									   1<<1
#define TR2_256_FS									   1<<2
#define TR2_512_FS									   3<<1

#define PICC_COMPLIANT_ISO14443_4		   1
#define PICC_NOT_COMPLIANT_ISO14443_4  0

/* ATTRIB PARAM 4 (1 byte)
 * --------------------------
 * RFU(4 bits) | CID(4 bits) 
 * --------------------------
 */
#define CID_0                          0
#define CID_1													 1
#define CID_2													 2
#define CID_3													 3
#define CID_4													 4
#define CID_5													 5
#define CID_6													 6
#define CID_7													 7
#define CID_8													 8
#define CID_9													 9
#define CID_10												 10
#define CID_11												 11
#define CID_12												 12
#define CID_13												 13
#define CID_14												 14

/* Sizes of ISO14443B variables */
#define ISO14443B_ATQB_SIZE	           					 13
#define ISO14443B_MAX_PUPI_SIZE			             0x04
#define ISO14443B_MAX_APPLI_SIZE			           0x04
#define ISO14443B_MAX_PROTOCOL_SIZE			         0x04
#define ISO14443B_MAX_LOG_MSG                    60


typedef struct{
	uint8_t ATQB[ISO14443B_ATQB_SIZE];
	uint8_t PUPI[ISO14443B_MAX_PUPI_SIZE],
					ApplicationField[ISO14443B_MAX_APPLI_SIZE],
					ProtocolInfo[ISO14443B_MAX_PROTOCOL_SIZE];
	bool 		IsDetected;
	char		LogMsg[ISO14443B_MAX_LOG_MSG];
}ISO14443B_CARD;


int8_t ISO14443B_Init								( void );
int8_t ISO14443B_ReqB								( uint8_t *pDataRead );
int8_t ISO14443B_AttriB							( uint8_t *pDataRead );
int8_t ISO14443B_IsPresent					( void );
int8_t ISO14443B_IsCardIntheField		( void );
int8_t ISO14443B_Anticollision			( void );

#ifdef __cplusplus
}
#endif

#endif /* __ISO14443B_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
