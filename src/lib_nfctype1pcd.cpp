/**
  ******************************************************************************
  * @file    lib_nfctype1pcd.c
  * @author  MMY Application Team
  * @version $Revision: 1334 $
  * @date    $Date: 2015-11-05 10:53:37 +0100 (Thu, 05 Nov 2015) $
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

//#ifdef __cplusplus
// extern "C" {
//#endif
 
/* Includes ------------------------------------------------------------------------------ */
#include "lib_nfctype1pcd.h"

extern uint8_t TT1Tag[];
extern uint8_t TagUID[];

static uint8_t PCDNFCT1_ReadAll(uint8_t *pBufferRead);
static uint8_t PCDNFCT1_WriteErase(uint8_t address, uint8_t byte);

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup PCD
 * 	@{
 *	@brief  This part of the library enables PCD capabilities of CR95HF & ST95HF.
 */


/** @addtogroup NFC_type1_pcd
 * 	@{
 *	@brief  This file is used to exchange with NFC FORUM Type1 Tag.
*/


/** @addtogroup lib_nfctype1pcd_Private_Functions
 *  @{
 */


/**
 * @brief  This function generates the ReadAll command
 * @param  pBufferRead : Pointer on the buffer which will contain the data read
 * @retval PCDNFCT1_OK : Command success
 * @retval PCDNFCT1_ERROR : Transmission error
 */
static uint8_t PCDNFCT1_ReadAll(uint8_t *pBufferRead)
{
	uint8_t buffer[] = {PCDNFCT1_READALL,0x00,0x00,0,0,0,0,PCDNFCT1_TOPAZ_MODE};

	memcpy(&buffer[3], &TagUID[4],4);

	if (PCD_SendRecv(8,buffer, pBufferRead) == PCD_SUCCESSCODE)
		return PCDNFCT1_OK;
	else
		return PCDNFCT1_ERROR; 
}

/**
 * @brief  This function generates the WriteErase command
 * @param  address : The address of the byte to write
 * @param  byte : The byte to write
 * @retval PCDNFCT1_OK : Command success
 * @retval PCDNFCT1_ERROR : Transmission error
 */
static uint8_t PCDNFCT1_WriteErase(uint8_t address, uint8_t byte)
{
	uint8_t buffer[] = {PCDNFCT1_WRITE_E,0,0,0,0,0,0,PCDNFCT1_TOPAZ_MODE};
	uint8_t bufferRead[16];

	/* Cannot write protected sector */
	if (address < 13 || address > 102)
		return PCDNFCT1_ERROR;

	buffer[1] = address;
	buffer[2] = byte;
	memcpy(&buffer[3], &TagUID[4],4);

	if (PCD_SendRecv(8,buffer, bufferRead) == PCD_SUCCESSCODE)
		return PCDNFCT1_OK;
	else
		return PCDNFCT1_ERROR; 
}

/**
  * @}
  */

/** @addtogroup lib_nfctype1pcd_Public_Functions
 *  @{
 */

/**
 * @brief  This function reads the NDEF message from a tag type 1 and store result in the TT1Tag buffer
 * @retval PCDNFCT1_OK : Command success
 * @retval PCDNFCT1_ERROR : Transmission error
 */
uint8_t PCDNFCT1_ReadNDEF( void )
{
	uint8_t status;
	uint8_t buffer[NFCT1_MAX_TAGMEMORY+2];
	uint16_t NDEFposition = 16, size;
	
	errchk(PCDNFCT1_ReadAll(buffer));
	
	if ((buffer[2] & PCDNFCT1_NDEF_CAPABLE_MSK) != 0x10) /* NDEF capable TT1 if H0 = 0x1X */
		return PCDNFCT1_ERROR_NOT_FORMATED;
	if (buffer[12] != PCDNFCT1_NDEF_MNB) /* CC file with NDEF info */
		return PCDNFCT1_ERROR_NOT_FORMATED;
	if (buffer[14] != 0x0E) /* Check the size (only static memory supported yet)*/
		return PCDNFCT1_ERROR_MEMORY_INTERNAL;
	
	// Searching for the first NDEF TLV
	while (buffer[NDEFposition] != PCDNFCT1_TLV_NDEF)
	{
		if (buffer[NDEFposition] == PCDNFCT1_TLV_EMPTY) // Empty TLV
			NDEFposition++;
		else if (buffer[NDEFposition] == PCDNFCT1_TLV_LOCK || buffer[NDEFposition] == PCDNFCT1_TLV_MEM)  // Lock CTRL TLV or Mem CTRL TLV
			NDEFposition+=5;
		else
			return PCDNFCT1_ERROR; // Other TLV not supported yet (proprietary)
		if (NDEFposition > NFCT1_MAX_TAGMEMORY)
			return PCDNFCT1_ERROR; // EOF and no NDEF TLV found
	}
	
	// Get the length of the message and copy it
	if (buffer[NDEFposition+1] == 0xFF) // Long message (cannot happen with static memory...)
	{
		size = buffer[NDEFposition+2]<<8|buffer[NDEFposition+3];
		memcpy(&TT1Tag[16],&buffer[NDEFposition],size+5);
	}
	else // Short message
	{
		memcpy(&TT1Tag[16],&buffer[NDEFposition],buffer[NDEFposition+1]+3);
	}
	
	return PCDNFCT1_OK;
Error:
	return PCDNFCT1_ERROR;
}

/**
 * @brief  This function writes the NDEF message to a tag type 1 from the TT1Tag buffer
 * @retval PCDNFCT1_OK : Command success
 * @retval PCDNFCT1_ERROR : Transmission error
 * @retval PCDNFCT1_ERROR_MEMORY : Not enough memory available on the tag
 */
uint8_t PCDNFCT1_WriteNDEF( void )
{
	uint8_t status;
	uint8_t buffer[NFCT1_MAX_TAGMEMORY+2];
	uint16_t NDEFposition = 16, size, i;
	
	errchk(PCDNFCT1_ReadAll(buffer));
	
	if ((buffer[2] & PCDNFCT1_NDEF_CAPABLE_MSK) != 0x10) /* NDEF capable TT1 if H0 = 0x1X */
		return PCDNFCT1_ERROR_NOT_FORMATED;
	if (buffer[12] != PCDNFCT1_NDEF_MNB) /* CC file with NDEF info */
		return PCDNFCT1_ERROR_NOT_FORMATED;
	if (buffer[14] != 0x0E) /* Check the size (only static memory supported yet)*/
		return PCDNFCT1_ERROR;
	
	// Searching for the first NDEF TLV
	while (buffer[NDEFposition] != PCDNFCT1_TLV_NDEF)
	{
		if (buffer[NDEFposition] == PCDNFCT1_TLV_EMPTY) // Empty TLV
			NDEFposition++;
		else if (buffer[NDEFposition] == PCDNFCT1_TLV_LOCK || buffer[NDEFposition] == PCDNFCT1_TLV_MEM)  // Lock CTRL TLV or Mem CTRL TLV
			NDEFposition+=5;
		else
			return PCDNFCT1_ERROR; // Other TLV not supported yet (proprietary)
		if (NDEFposition > NFCT1_MAX_TAGMEMORY)
			return PCDNFCT1_ERROR; // EOF and no NDEF TLV found
	}
	
	if (TT1Tag[15] == 0xFF)
		size = (((TT1Tag[16])<<8) | (TT1Tag[17])) + 4;
	else
		size = TT1Tag[15]+2;
	
	if (size > NFCT1_MAX_TAGMEMORY-32)
		return PCDNFCT1_ERROR_MEMORY_TAG;
	
	// Invalidate the NDEF message
	PCDNFCT1_WriteErase(8,PCDNFCT1_NDEF_DISABLE);

	NDEFposition-=3;
	for(i=15;i<15+size;i++,NDEFposition++)
	{
		errchk(PCDNFCT1_WriteErase(NDEFposition,TT1Tag[i]));
	}
	
	// Validate the NDEF message
	PCDNFCT1_WriteErase(8,PCDNFCT1_NDEF_MNB);
	
	return PCDNFCT1_OK;
Error:
	return PCDNFCT1_ERROR;
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

/**
  * @}
  */ 

//#ifdef __cplusplus
//}
//#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
