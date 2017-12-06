/**
  ******************************************************************************
  * @file    lib_nfctype5pcd.c
  * @author  MMY Application Team
  * @version $Revision: 1334 $
  * @date    $Date: 2015-11-05 10:53:37 +0100 (Thu, 05 Nov 2015) $
  * @brief   Generates the NFC type2 commands
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
#include "lib_nfctype5pcd.h"

extern uint8_t TT5Tag[];

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


/** @addtogroup NFC_type5_pcd
 * 	@{
 *	@brief  This file is used to exchange with NFC FORUM Type5 Tag.
*/


/** @addtogroup lib_nfctype5pcd_Private_Functions
 *  @{
 */

/**
  * @}
  */

/** @addtogroup lib_nfctype5pcd_Public_Functions
 *  @{
 */

/**
 * @brief  This function reads the NDEF message from a tag type V and store result in the TT5Tag buffer
 * @retval PCDNFCT5_OK : Command success
 * @retval PCDNFCT5_ERROR : Transmission error
 * @retval PCDNFCT5_ERROR_LOCKED : The tag cannot be read (CC lock)
 */
uint8_t PCDNFCT5_ReadNDEF( void )
{
	uint16_t size;
	uint8_t tagDensity = ISO15693_HIGH_DENSITY;
	// Try to determine the density by reading the first sector (128 bytes)
	if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRiS64K, TT5Tag, 127, 0) != ISO15693_SUCCESSCODE)
	{
		if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRiS64K, TT5Tag, 127, 0) != ISO15693_SUCCESSCODE)
			return PCDNFCT5_ERROR;
		tagDensity = ISO15693_LOW_DENSITY;
	}
	
	// NDEF capable ?
	if (TT5Tag[0] != 0xE1)
		return PCDNFCT5_ERROR_NOT_FORMATED;
	
	// Check read access
	if ((TT5Tag[1]&0x0C) != 0)
		return PCDNFCT5_ERROR_LOCKED;
	
	// Get the size of the message
	if (TT5Tag[5] == 0xFF)
		size = (TT5Tag[6]<<8)|TT5Tag[7];
	else
		size = 0x00FF&TT5Tag[5];
	
	// Check if there is enough memory to read the tag
	// If CC3 bit3 = 1 the size is higher than 2KB but we don't know the size...
	if (size+7 > NFCT5_MAX_TAGMEMORY)
		return PCDNFCT5_ERROR_MEMORY_INTERNAL;
	
	// Read the rest of the tag if needed
	if (size > 124)
	{
		if (ISO15693_ReadBytesTagData(tagDensity, ISO15693_LRiS64K, &TT5Tag[128], size-128, 128) != ISO15693_SUCCESSCODE)
		{
			return PCDNFCT5_ERROR;
		}
	}
	
	return PCDNFCT5_OK;	
}

/**
 * @brief  This function writes the NDEF message to a tag type V from the TT5Tag buffer
 * @retval PCDNFCT5_OK : Command success
 * @retval PCDNFCT5_ERROR : Transmission error
 * @retval PCDNFCT5_ERROR_LOCKED : The tag cannot be write or read (CC lock)
 * @retval PCDNFCT5_ERROR_MEMORY : Not enough memory available on the tag
 */
uint8_t PCDNFCT5_WriteNDEF( void )
{
	uint8_t RepBuffer[30];
	uint8_t firstSector[140], status;
	uint16_t size, tagSize;
	uint8_t tagDensity = ISO15693_HIGH_DENSITY;
	// Try to determine the density by ready the first sector (128 bytes)
	if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE)
	{
		if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE)
			return PCDNFCT5_ERROR;
		tagDensity = ISO15693_LOW_DENSITY;
	}
	// NDEF capable ?
	if (firstSector[0] != 0xE1)
	{
		/* Create the CC file */
		// We need the size
		if (tagDensity == ISO15693_HIGH_DENSITY)
			ISO15693_GetSystemInfo (0x0A , 0x00, RepBuffer);
		else
			ISO15693_GetSystemInfo (0x02 , 0x00, RepBuffer);
		if (RepBuffer[14] == 0xFF)
			tagSize = (((RepBuffer[15]<<8)|RepBuffer[14])+1)*(RepBuffer[16]+1);
		else
			tagSize = (RepBuffer[14]+1)*(RepBuffer[15]+1);
		// NDEF capable
		TT5Tag[0] = 0xE1;
		// Version + Read/Write allowed
		TT5Tag[1] = 0x40;
		// Size
		if (tagSize > 2040)
		{
			// If the size is above 2040 (0xFF) then we have to set the bit3 of CC4
			TT5Tag[2] = 0xFF;
			TT5Tag[3] = 0x05;
		}
		else
		{
			TT5Tag[2] = tagSize/8;
			TT5Tag[3] = 0x01;
		}
	}
	else
	{
		// Copy the CC
		memcpy(TT5Tag,firstSector,4);
		
		// Check read and write access
		if ((TT5Tag[1]&0x0F) != 0)
			return PCDNFCT5_ERROR_LOCKED;
	}
		
	// Get the size of the message to write
	if (TT5Tag[5] == 0xFF)
		size = (TT5Tag[6]<<8)|TT5Tag[7];
	else
		size = 0x00FF&TT5Tag[5];
	
	// Check if the memory available on the tag is enough
	// If CC3 bit3 = 1 the size is higher than 2KB but we don't know the size...
	if (TT5Tag[2]*8 < size+7 && (TT5Tag[3]&0x04) == 0)
		return PCDNFCT5_ERROR_MEMORY_TAG;
	if ((TT5Tag[3]&0x04) != 0) // So we use get system info command
	{
		if (tagDensity == ISO15693_HIGH_DENSITY)
			ISO15693_GetSystemInfo (0x0A , 0x00, RepBuffer);
		else
			ISO15693_GetSystemInfo (0x02 , 0x00, RepBuffer);
		
		if (RepBuffer[14] == 0xFF)
			tagSize = (((RepBuffer[15]<<8)|RepBuffer[14])+1)*(RepBuffer[16]+1);
		else
			tagSize = (RepBuffer[14]+1)*(RepBuffer[15]+1);
		
		if (tagSize < size+7)
			return PCDNFCT5_ERROR_MEMORY_TAG;
	}
	
	// Write the tag
	errchk(ISO15693_WriteBytes_TagData(tagDensity, TT5Tag, size+7, 0));
	
	return PCDNFCT5_OK;	
Error:
	return PCDNFCT5_ERROR;
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
