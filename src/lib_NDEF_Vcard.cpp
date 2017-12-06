/**
  ******************************************************************************
  * @file    lib_NDEF_Vcard.c
  * @author  MMY Application Team
  * @version $Revision: 1330 $
  * @date    $Date: 2015-11-05 10:39:11 +0100 (Thu, 05 Nov 2015) $
  * @brief   This file help to manage NDEF file that represent Vcard.
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
#include "lib_NDEF_Vcard.h"

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

/** @defgroup libVcard_Private_Functions
  * @{
  */


static void NDEF_FillVcardStruct( uint8_t* pPayload, uint32_t PayloadSize, char* pKeyWord, uint32_t SizeOfKeyWord, uint8_t* pString );
static void NDEF_ExtractVcard( sRecordInfo_vcard *pRecordStruct, sVcardInfo *pVcardStruct );

/**
  * @brief  This function fill Vcard structure with information of NDEF message.
  * @param  pPayload : pointer on the payload data of the NDEF message.
  * @param  PayloadSize : number of data in the payload.
  * @param  pKeyWord : pointer on the keyword to look for.
  * @param  SizeOfKeyWord : number of byte of the keyword we are looking for.
  * @param  pString : Pointer on the data string to fill.
  */
static void NDEF_FillVcardStruct( uint8_t* pPayload, uint32_t PayloadSize, char* pKeyWord, uint32_t SizeOfKeyWord, uint8_t* pString )
{
  uint8_t* pLastByteAdd, *pLook4Word, *pEndString;

  /* First character force to NULL in case not matching found */
  *pString = 0;
  
  /* Interresting information are stored before picture if any */
  /* Moreover picture is not used in this demonstration SW */
  pLastByteAdd = pPayload;
  while( memcmp( pLastByteAdd, JPEG, JPEG_STRING_SIZE ) && (pLastByteAdd < (pPayload + PayloadSize)) )
  {
    pLastByteAdd++;
  }

  pLook4Word = pPayload;
  while( memcmp( pLook4Word, pKeyWord, SizeOfKeyWord ) && (pLook4Word < pLastByteAdd) )
  {
    pLook4Word++;
  }

  /* Word found */
  if( pLook4Word != pLastByteAdd )
  {
    pLook4Word += SizeOfKeyWord;
    pEndString = pLook4Word;
    while( memcmp( pEndString, LIMIT, LIMIT_STRING_SIZE ) && (pEndString < pLastByteAdd) )
    {
      pEndString++;
    }
    if( pEndString != pLastByteAdd )
    {
      memcpy( pString, pLook4Word, pEndString-pLook4Word );
      /* add end of string character */
      pString += pEndString - pLook4Word;
      *pString = '\0';
    }
  }	
}

/**
  * @brief  This function read the Vcard and store data in a structure.
  * @param  pRecordStruct : Pointer on the record structure.
  * @param  pSMSStruct : pointer on the structure to fill.
  */
static void NDEF_ExtractVcard( sRecordInfo_vcard *pRecordStruct, sVcardInfo *pVcardStruct )
{
  uint32_t PayloadSize;
  uint8_t* pPayload;


  PayloadSize = ((uint32_t)(pRecordStruct->PayloadLength3) << 24) | ((uint32_t)(pRecordStruct->PayloadLength2) << 16) |
                ((uint32_t)(pRecordStruct->PayloadLength1) << 8)  | pRecordStruct->PayloadLength0;

  /* Read record header */
  pPayload = (uint8_t*)(pRecordStruct->PayloadBufferAdd);

  NDEF_FillVcardStruct( pPayload, PayloadSize, VERSION, VERSION_STRING_SIZE, (uint8_t*)(pVcardStruct->Version) );
  if( !memcmp( pVcardStruct->Version, VCARD_VERSION_2_1, VCARD_VERSION_2_1_SIZE ) )
  {
    NDEF_FillVcardStruct( pPayload, PayloadSize, FIRSTNAME, FIRSTNAME_STRING_SIZE, (uint8_t*)(pVcardStruct->FirstName) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, TITLE, TITLE_STRING_SIZE, (uint8_t*)(pVcardStruct->Title) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, ORG, ORG_STRING_SIZE, (uint8_t*)(pVcardStruct->Org) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, HOME_ADDRESS, HOME_ADDRESS_STRING_SIZE, (uint8_t*)(pVcardStruct->HomeAddress) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, WORK_ADDRESS, WORK_ADDRESS_STRING_SIZE, (uint8_t*)(pVcardStruct->WorkAddress) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, HOME_TEL, HOME_TEL_STRING_SIZE, (uint8_t*)(pVcardStruct->HomeTel) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, WORK_TEL, WORK_TEL_STRING_SIZE, (uint8_t*)(pVcardStruct->WorkTel) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, CELL_TEL, CELL_TEL_STRING_SIZE, (uint8_t*)(pVcardStruct->CellTel) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, HOME_EMAIL, HOME_EMAIL_STRING_SIZE, (uint8_t*)(pVcardStruct->HomeEmail) );
    NDEF_FillVcardStruct( pPayload, PayloadSize, WORK_EMAIL, WORK_EMAIL_STRING_SIZE, (uint8_t*)(pVcardStruct->WorkEmail) );
  }
  else if( !memcmp( pVcardStruct->Version, VCARD_VERSION_3_0, VCARD_VERSION_3_0_SIZE ) )
  {
    /* need to be implemented */
  }
  else
  {
    /* maybe new version but not supported in this sw */
  }

}

/**
  * @}
  */

/** @defgroup libVcard_Public_Functions
  * @{
  * @brief  This file is used to manage Vcard (stored or loaded in tag)
  */ 

/**
  * @brief  This function read NDEF and retrieve Vcard information if any.
  * @param  pRecordStruct : Pointer on the record structure.
  * @param  pVcardStruct : pointer on the structure to fill.
  * @retval NDEF_OK : Vcard information from NDEF have been retrieved.
  * @retval NDEF_ERROR : Not able to retrieve Vcard information.
  */
uint16_t NDEF_ReadVcard( sRecordInfo_vcard *pRecordStruct, sVcardInfo *pVcardStruct )
{
  uint16_t status = NDEF_ERROR;

  if( pRecordStruct->NDEF_Type == VCARD_TYPE )
  {
    NDEF_ExtractVcard( pRecordStruct, pVcardStruct );
    status = NDEF_OK;
  }

  return status;
}

/**
  * @brief  This function write the NDEF file with the Vcard data given in the structure.
  * @param  pVcardStruct : pointer on structure that contain the Vcard information.
  * @retval NDEF_OK : NDEF file data written in the tag.
  * @retval NDEF_ERROR : not able to store NDEF in tag.
  * @retval NDEF_ERROR_MEMORY_INTERNAL : Cannot write to tag.
  * @retval NDEF_ERROR_NOT_FORMATED : CCFile data not supported or not present.
  * @retval NDEF_ERROR_MEMORY_TAG : Size not compatible with memory.
  * @retval NDEF_ERROR_LOCKED : Tag locked, cannot be write.
  */
uint16_t NDEF_WriteVcard( sVcardInfo *pVcardStruct )
{
  uint16_t status = NDEF_ERROR, Offset = 0;

  NDEF_PrepareVcardMessage( pVcardStruct, &NDEF_Buffer[FIRST_RECORD_OFFSET], &Offset );

  /* Write NDEF */
  NDEF_Buffer[0] = (Offset & 0xFF00) >> 8;
  NDEF_Buffer[1] = Offset & 0x00FF;

  status = WriteData( 0x00, Offset + FIRST_RECORD_OFFSET, NDEF_Buffer );

  return status;
}

/**
  * @brief  This function write the NDEF file with the Vcard data given in the structure.
  * @param  pVcardStruct : pointer on structure that contain the Vcard information.
  * @param  pNDEFMessage : pointer on the NDEF message.
  * @param  size : to store the size of the NDEF message generated.
  */
void NDEF_PrepareVcardMessage( sVcardInfo *pVcardStruct, uint8_t *pNDEFMessage, uint16_t *size )
{

  uint32_t PayloadSize = 0;

  /* Vcard Record Header */
/************************************/
/*  7 |  6 |  5 |  4 |  3 | 2  1  0 */
/*----------------------------------*/
/* MB   ME   CF   SR   IL    TNF    */  /* <---- CF=0, IL=0 and SR=0 TNF=2 NFC Forum Media type*/
/*----------------------------------*/
/*          TYPE LENGTH             */
/*----------------------------------*/
/*        PAYLOAD LENGTH 3          */
/*----------------------------------*/
/*        PAYLOAD LENGTH 2          */
/*----------------------------------*/
/*        PAYLOAD LENGTH 1          */
/*----------------------------------*/
/*        PAYLOAD LENGTH 0          */
/*----------------------------------*/
/*           ID LENGTH              */  /* <---- Not Used  */
/*----------------------------------*/
/*              TYPE                */
/*----------------------------------*/
/*               ID                 */  /* <---- Not Used  */
/************************************/

  /* As we don't have embedded a jpeg encoder/decoder in this firmware */
  /* We have made the choice to manage only string content of the vCard */
  /* For demonstration purpose in order to fill the 8kB of the M24SR */
  /* We have embedded a NDEF vCard in the STM32 to be able to fill M24SR */

  /* fill record header */
  pNDEFMessage[0] = 0xC2;   /* Record Flag */
  pNDEFMessage[1] = VCARD_TYPE_STRING_LENGTH;
  pNDEFMessage[2] = 0x00; /* Will be filled at the end when payload size is known */
  pNDEFMessage[3] = 0x00;
  pNDEFMessage[4] = 0x00;
  pNDEFMessage[5] = 0x00;	
  memcpy( &pNDEFMessage[6], VCARD_TYPE_STRING, VCARD_TYPE_STRING_LENGTH );

  /* Payload is positionned in the NDEF after record header */
  PayloadSize = 6 + VCARD_TYPE_STRING_LENGTH;

  /* "BEGIN:VCARD\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], BEGIN, BEGIN_STRING_SIZE );
  PayloadSize += BEGIN_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], VCARD,VCARD_STRING_SIZE );
  PayloadSize += VCARD_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "VERSION:2.1\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], VERSION, VERSION_STRING_SIZE );
  PayloadSize += VERSION_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], VCARD_VERSION_2_1, VCARD_VERSION_2_1_SIZE );
  PayloadSize += VCARD_VERSION_2_1_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "FN:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], FIRSTNAME, FIRSTNAME_STRING_SIZE );
  PayloadSize += FIRSTNAME_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->FirstName, strlen(pVcardStruct->FirstName) );
  PayloadSize += strlen( pVcardStruct->FirstName );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "TITLE:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], TITLE, TITLE_STRING_SIZE );
  PayloadSize += TITLE_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->Title, strlen(pVcardStruct->Title) );
  PayloadSize += strlen( pVcardStruct->Title );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "ORG:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], ORG, ORG_STRING_SIZE );
  PayloadSize += ORG_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->Org, strlen(pVcardStruct->Org) );
  PayloadSize += strlen( pVcardStruct->Org );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "ADR;HOME:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], HOME_ADDRESS, HOME_ADDRESS_STRING_SIZE );
  PayloadSize += HOME_ADDRESS_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->HomeAddress, strlen(pVcardStruct->HomeAddress) );
  PayloadSize += strlen( pVcardStruct->HomeAddress );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "ADR;WORK:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], WORK_ADDRESS, WORK_ADDRESS_STRING_SIZE );
  PayloadSize += WORK_ADDRESS_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->WorkAddress, strlen(pVcardStruct->WorkAddress) );
  PayloadSize += strlen( pVcardStruct->WorkAddress );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "TEL;HOME:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], HOME_TEL, HOME_TEL_STRING_SIZE );
  PayloadSize += HOME_TEL_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->HomeTel, strlen(pVcardStruct->HomeTel) );
  PayloadSize += strlen( pVcardStruct->HomeTel );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "TEL;WORK:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], WORK_TEL, WORK_TEL_STRING_SIZE );
  PayloadSize += WORK_TEL_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->WorkTel, strlen(pVcardStruct->WorkTel) );
  PayloadSize += strlen( pVcardStruct->WorkTel );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "TEL;CELL:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], CELL_TEL, CELL_TEL_STRING_SIZE );
  PayloadSize += CELL_TEL_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->CellTel, strlen(pVcardStruct->CellTel) );
  PayloadSize += strlen( pVcardStruct->CellTel );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "EMAIL;HOME:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], HOME_EMAIL, HOME_EMAIL_STRING_SIZE );
  PayloadSize += HOME_EMAIL_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->HomeEmail, strlen(pVcardStruct->HomeEmail) );
  PayloadSize += strlen( pVcardStruct->HomeEmail );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "EMAIL;WORK:\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], WORK_EMAIL, WORK_EMAIL_STRING_SIZE );
  PayloadSize += WORK_EMAIL_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], pVcardStruct->WorkEmail, strlen(pVcardStruct->WorkEmail) );
  PayloadSize += strlen( pVcardStruct->WorkEmail );
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  /* "END:VCARD\r\n" */
  memcpy( &pNDEFMessage[PayloadSize], END, END_STRING_SIZE );
  PayloadSize += END_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], VCARD, VCARD_STRING_SIZE );
  PayloadSize += VCARD_STRING_SIZE;
  memcpy( &pNDEFMessage[PayloadSize], LIMIT, LIMIT_STRING_SIZE );
  PayloadSize += LIMIT_STRING_SIZE;

  *size = (uint16_t)(PayloadSize); /* Must not count the 2 byte that represent the NDEF size */
  PayloadSize -= 6 + VCARD_TYPE_STRING_LENGTH;

  pNDEFMessage[2] = (PayloadSize & 0xFF000000) >> 24;
  pNDEFMessage[3] = (PayloadSize & 0x00FF0000) >> 16;
  pNDEFMessage[4] = (PayloadSize & 0x0000FF00) >> 8;
  pNDEFMessage[5] = PayloadSize & 0x000000FF;
  
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
