/**
  ******************************************************************************
  * @file    lib_NDEF_AAR.c
  * @author  MMY Application Team
  * @version $Revision: 1330 $
  * @date    $Date: 2015-11-05 10:39:11 +0100 (Thu, 05 Nov 2015) $
  * @brief   This file help to add AAR to NDEF file.
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

//#ifdef __cplusplus
// extern "C" {
//#endif

/* Includes ------------------------------------------------------------------*/
#include "lib_NDEF_AAR.h"

/** @addtogroup NFC_libraries
 *  @{
 *  @brief  <b>This is the library used to manage the content of the TAG (data)
 *          But also the specific feature of the tag, for instance
 *          password, gpo... </b>
 */


/** @addtogroup libNFC_FORUM
  * @{
  * @brief  This part of the library manage data which follow NFC forum organisation.
  */

/**
 * @brief  This buffer contains the data send/received by TAG
 */
extern uint8_t NDEF_Buffer [];

/** @defgroup libAAR_Private_Functions
  * @{
  */

/**
  * @}
  */

/** @defgroup libAAR_Public_Functions
  * @{
  * @brief  This file is used to manage AAR (stored or loaded in tag)
  */ 


/**
  * @brief  This function add AAR (Android Application Record) in the tag.
  * @param  pAARStruct : pointer on structure that contain AAR information.
  * @retval NDEF_OK : AAR added.
  * @retval NDEF_ERROR : not able to store NDEF in tag.
  * @retval NDEF_ERROR_MEMORY_INTERNAL : Cannot write to tag.
  * @retval NDEF_ERROR_NOT_FORMATED : CCFile data not supported or not present.
  * @retval NDEF_ERROR_MEMORY_TAG : Size not compatible with memory.
  * @retval NDEF_ERROR_LOCKED : Tag locked, cannot be write.
  */
uint16_t NDEF_AddAAR( sAARInfo *pAARStruct )
{
  uint16_t status = NDEF_ERROR;
  uint16_t DataSize = 0;
  uint16_t Offset = 0;
  uint32_t AAROffset = 0;
  uint16_t NDEF_Size = 0;
  uint8_t RecordFlag = 0;
  uint8_t AARRecordFlag = 0;

/* AAR: External Type Record Header */
/************************************/
/*  7 |  6 |  5 |  4 |  3 | 2  1  0 */
/*----------------------------------*/
/* MB   ME   CF   SR   IL    TNF    */  /* <---- IL=0, CF=0 and SR=1 TNF=4 NFC Forum external type*/
/*----------------------------------*/
/*          TYPE LENGTH             */
/*----------------------------------*/
/*        PAYLOAD LENGTH 3          */  /* <---- Not Used  */
/*----------------------------------*/
/*        PAYLOAD LENGTH 2          */  /* <---- Not Used  */
/*----------------------------------*/
/*        PAYLOAD LENGTH 1          */  /* <---- Not Used  */
/*----------------------------------*/
/*        PAYLOAD LENGTH 0          */
/*----------------------------------*/
/*           ID LENGTH              */  /* <---- Not Used  */
/*----------------------------------*/
/*             TYPE                 */  /* android.com:pkg */
/*----------------------------------*/
/*              ID                  */  /* <---- Not Used  */
/************************************/

  /* Do we have to add AAR to an existing NDEF message */
  /* retrieve current NDEF size and current record flag*/
  status = ReadData( 0, 3, NDEF_Buffer );
  
  if( status == NDEF_OK )
  {
    NDEF_Size = (uint16_t)(NDEF_Buffer[0] << 8);
    NDEF_Size = NDEF_Size | (uint16_t)(NDEF_Buffer[1]);
    RecordFlag = NDEF_Buffer[2];
  }

  if( NDEF_Size != 0 )
  {
    AAROffset = NDEF_Size + 2;
    RecordFlag &= 0xBF; /* remove ME flag on NDEF */
    AARRecordFlag = 0x54; /* don't put MB flag */
  }
  else
  {
    AAROffset = 2;
    AARRecordFlag = 0xD4; /* put MB and ME flag */
  }

  /* fill AAR record header */
  Offset = 0;
  NDEF_Buffer[Offset++] = AARRecordFlag;   /* Record Flag */
  NDEF_Buffer[Offset++] = AAR_TYPE_STRING_LENGTH;
  NDEF_Buffer[Offset++] = 0x00; /* Will be filled at the end when payload size is known */

  memcpy( &NDEF_Buffer[Offset], AAR_TYPE_STRING, AAR_TYPE_STRING_LENGTH );

  /* fill AAR payload */
  memcpy( &NDEF_Buffer[Offset + AAR_TYPE_STRING_LENGTH], pAARStruct->PakageName, strlen(pAARStruct->PakageName) );

  NDEF_Buffer[2] = strlen( pAARStruct->PakageName );

  DataSize = Offset + AAR_TYPE_STRING_LENGTH + strlen( pAARStruct->PakageName );

  /* Write NDEF */
  status = WriteData( AAROffset, DataSize, NDEF_Buffer );

  /* Write NDEF size to complete*/
  if( status == NDEF_OK )
  {
    DataSize = NDEF_Size + DataSize; /* Must add to the NDEF size the size of the AAR record*/
    NDEF_Buffer[0] = (DataSize & 0xFF00) >> 8;
    NDEF_Buffer[1] = DataSize & 0x00FF;
    if( NDEF_Size != 0 )
    {
      NDEF_Buffer[2] = RecordFlag;
      status = WriteData( 0x00, 3, NDEF_Buffer );
    }
    else
      status = WriteData( 0x00, 2, NDEF_Buffer );
  }

  return status;
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

//#ifdef __cplusplus
//}
//#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
