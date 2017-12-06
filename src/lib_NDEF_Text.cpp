/**
  ******************************************************************************
  * @file    lib_NDEF_Text.h
  * @author  MMY Application Team
  * @version $Revision: 1330 $
  * @date    $Date: 2015-11-05 10:39:11 +0100 (Thu, 05 Nov 2015) $
  * @brief   This file help to manage Text NDEF file.
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
#include "lib_NDEF_Text.h"

/** @addtogroup NFC_libraries
  * @{
  * @brief  <b>This is the library used to manage the content of the TAG (data)
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

/**
  * @}
  */

/** @defgroup libEmail_Public_Functions
  * @{
  * @brief  This file is used to manage Email (stored or loaded in tag)
  */ 

/**
  * @brief  This function write the text in the TAG.
  * @param  text : text to write.
  * @retval NDEF_OK : NDEF file data written in the tag.
  * @retval NDEF_ERROR : not able to store NDEF in tag.
  * @retval NDEF_ERROR_MEMORY_INTERNAL : Cannot write to tag.
  * @retval NDEF_ERROR_NOT_FORMATED : CCFile data not supported or not present.
  * @retval NDEF_ERROR_MEMORY_TAG : Size not compatible with memory.
  * @retval NDEF_ERROR_LOCKED : Tag locked, cannot be write.
  */
uint16_t NDEF_WriteText( char *text )
{
  uint16_t status = NDEF_ERROR;
  uint32_t textSize, Offset = 0;

  NDEF_Buffer[0] = 0;
  NDEF_Buffer[1] = 0;
  Offset = FIRST_RECORD_OFFSET;

  /* TEXT : 1+en+message */
  textSize = 3 + strlen(text);

  /* TEXT header */
  NDEF_Buffer[Offset] = 0xD1;
  if( textSize < 256 ) NDEF_Buffer[Offset] |= 0x10;                   // Set the SR bit
  Offset++;

  NDEF_Buffer[Offset++] = TEXT_TYPE_STRING_LENGTH;
  if( textSize > 255 )
  {
    NDEF_Buffer[Offset++] = (textSize & 0xFF000000) >> 24;
    NDEF_Buffer[Offset++] = (textSize & 0x00FF0000) >> 16;
    NDEF_Buffer[Offset++] = (textSize & 0x0000FF00) >> 8;
    NDEF_Buffer[Offset++] = textSize & 0x000000FF;
  }
  else
  {
    NDEF_Buffer[Offset++] = (uint8_t)textSize;
  }
  memcpy( &NDEF_Buffer[Offset], TEXT_TYPE_STRING, TEXT_TYPE_STRING_LENGTH );
  Offset += TEXT_TYPE_STRING_LENGTH;

  /* TEXT payload */
  NDEF_Buffer[Offset++] = ISO_ENGLISH_CODE_STRING_LENGTH;
  memcpy( &NDEF_Buffer[Offset], ISO_ENGLISH_CODE_STRING, ISO_ENGLISH_CODE_STRING_LENGTH );
  Offset += ISO_ENGLISH_CODE_STRING_LENGTH;

  memcpy( &NDEF_Buffer[Offset], text, strlen(text) );
  Offset += strlen(text);

  Offset -= 2; /* Must not count the 2 byte that represent the NDEF size */
  NDEF_Buffer[0] = (Offset & 0xFF00) >> 8;
  NDEF_Buffer[1] = Offset & 0x00FF;

  status = WriteData( 0x00, Offset + 2, NDEF_Buffer );

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
