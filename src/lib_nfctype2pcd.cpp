/**
  ******************************************************************************
  * @file    lib_nfctype2pcd.c
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
#include "Arduino.h"
#include "lib_nfctype2pcd.h"

extern uint8_t TT2Tag[];

static uint8_t PCDNFCT2_Read(uint8_t blocNbr, uint8_t *pBufferRead);
static uint8_t PCDNFCT2_Write(uint8_t blocNbr, uint8_t *pBufferWrite);
static uint8_t PCDNFCT2_SectorSelect(uint8_t sector);

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


/** @addtogroup NFC_type2_pcd
 * 	@{
 *	@brief  This file is used to exchange with NFC FORUM Type2 Tag.
*/


/** @addtogroup lib_nfctype2pcd_Private_Functions
 *  @{
 */


/**
 * @brief  This function generates the Read command
 * @param  blocNbr : First bloc number
 * @param  pBufferRead : Pointer on the buffer which will contain the data read
 * @retval PCDNFCT2_OK : Command success
 * @retval PCDNFCT2_ERROR : Transmission error
 */
static uint8_t PCDNFCT2_Read(uint8_t blocNbr, uint8_t *pBufferRead)
{
	uint8_t buffer[] = {PCDNFCT2_READ,0,SEND_MASK_APPENDCRC|SEND_MASK_8BITSINFIRSTBYTE};
	
	buffer[1] = blocNbr;

	if (PCD_SendRecv(3,buffer, pBufferRead) == PCD_SUCCESSCODE)
		return PCDNFCT2_OK;
	else
		return PCDNFCT2_ERROR; 
}

/**
 * @brief  This function generates the Write command
 * @param  blocNbr : First bloc number
 * @param  pBufferWrite : Pointer on the buffer which contains the data to write
 * @retval PCDNFCT2_OK : Command success
 */
static uint8_t PCDNFCT2_Write(uint8_t blocNbr, uint8_t *pBufferWrite)
{
	uint8_t pBufferRead[32];
	uint8_t buffer[] = {PCDNFCT2_WRITE,0,0,0,0,0,SEND_MASK_APPENDCRC|SEND_MASK_8BITSINFIRSTBYTE};
	
	buffer[1] = blocNbr;
	memcpy(&buffer[2], pBufferWrite,4);

	PCD_SendRecv(7,buffer, pBufferRead);
	return PCDNFCT2_OK;
}

/**
 * @brief  This function generates the SectorSelect command
 * @param  sector : Sector number (0/1)
 * @retval PCDNFCT2_OK : Command success
 * @retval PCDNFCT2_ERROR : Transmission error
 */
static uint8_t PCDNFCT2_SectorSelect(uint8_t sector)
{
	uint8_t pBufferRead[32];
	uint8_t buffer[] = {PCDNFCT2_SECTOR_SEL,0xFF,SEND_MASK_APPENDCRC|SEND_MASK_8BITSINFIRSTBYTE};  // First part
	uint8_t buffer2[] = {0,0x00,0x00,0x00,SEND_MASK_APPENDCRC|SEND_MASK_8BITSINFIRSTBYTE}; // Second part
	
	buffer2[0] = sector;

	PCD_SendRecv(3,buffer, pBufferRead);
	if (pBufferRead[0] == 0x90 && pBufferRead[1] == 0x04 && pBufferRead[2] == 0x0A) // ACK
	{
		PCD_SendRecv(5,buffer2, pBufferRead);
		if (pBufferRead[0] == 0x87 && pBufferRead[1] == 0x00) // Passive ACK = no answer so ST95HF send a timeout
			return PCDNFCT2_OK;
		else
			return PCDNFCT2_ERROR; 
	}
	else
		return PCDNFCT2_ERROR; 
}

/**
  * @}
  */

/** @addtogroup lib_nfctype2pcd_Public_Functions
 *  @{
 */

/**
 * @brief  This function reads the NDEF message from a tag type 2 and store result in the TT2Tag buffer
 * @retval PCDNFCT2_OK : Command success
 * @retval PCDNFCT2_ERROR : Transmission error
 * @retval PCDNFCT2_ERROR_LOCKED : The tag cannot be read (CC lock)
 */
uint8_t PCDNFCT2_ReadNDEF( void )
{
	uint8_t status;
	uint8_t bufferRead[PCDNFCT2_READ_SIZE_BUFFER];
	uint8_t buffer[NFCT2_MAX_TAGMEMORY];
	uint16_t size, totalSize, i;
	uint16_t NDEFposition = 16;
	
	// Check if CC is present with NDEF capability
	errchk(PCDNFCT2_Read(0,buffer));
	if (buffer[14] != PCDNFCT2_NDEF_MNB)
		return PCDNFCT2_ERROR_NOT_FORMATED;
	// Check if the tag is not protected
	if ((buffer[17]&PCDNFCT2_READ_MSK) != 0x00)
		return PCDNFCT2_ERROR_LOCKED;
	// Read the size from CC
	totalSize = buffer[16]*8;
	
	// Check if there is enough memory available to read the tag
	if (totalSize > NFCT2_MAX_TAGMEMORY)
		return PCDNFCT2_ERROR_MEMORY_INTERNAL;
	
	if (totalSize > PCDNFCT2_SECTOR_SIZE)
		size = PCDNFCT2_SECTOR_SIZE;
	else
		size = totalSize;
	// Read the whole memory
	for (i=0;i<size/4;i+=4)
	{
		errchk(PCDNFCT2_Read(i,bufferRead));
		memcpy(&buffer[i*4],&bufferRead[2],PCDNFCT2_READ_SIZE);
	}
	// Second sector (if needed)
	if (totalSize > PCDNFCT2_SECTOR_SIZE)
	{
		errchk(PCDNFCT2_SectorSelect(1));
		size = totalSize-PCDNFCT2_SECTOR_SIZE;
		for (i=4;i<size/4;i+=4)
		{
			errchk(PCDNFCT2_Read(i,bufferRead));
			memcpy(&buffer[PCDNFCT2_SECTOR_SIZE+i*4],&bufferRead[2],PCDNFCT2_READ_SIZE);
		}
	}
	
	// Searching for the first NDEF TLV
	while (buffer[NDEFposition] != PCDNFCT2_TLV_NDEF)
	{
		if (buffer[NDEFposition] == PCDNFCT2_TLV_EMPTY) // Empty TLV
			NDEFposition++;
		else if (buffer[NDEFposition] == PCDNFCT2_TLV_LOCK || buffer[NDEFposition] == PCDNFCT2_TLV_MEM)  // Lock CTRL TLV or Mem CTRL TLV
			NDEFposition+=5;
		else
			return PCDNFCT2_ERROR; // Other TLV not supported yet (proprietary)
		if (NDEFposition > totalSize)
			return PCDNFCT2_ERROR; // EOF and no NDEF TLV found
	}
	
	// Get the length of the message and copy it
	if (buffer[NDEFposition+1] == 0xFF) // Long message
	{
		size = buffer[NDEFposition+2]<<8|buffer[NDEFposition+3];
		memcpy(&TT2Tag[16],&buffer[NDEFposition],size+5);
	}
	else // Short message
	{
		memcpy(&TT2Tag[16],&buffer[NDEFposition],buffer[NDEFposition+1]+3);
	}
	
	return PCDNFCT2_OK;
Error:
	return PCDNFCT2_ERROR;
}

/**
 * @brief  This function writes the NDEF message to a tag type 2 from the TT2Tag buffer
 * @retval PCDNFCT2_OK : Command success
 * @retval PCDNFCT2_ERROR : Transmission error
 * @retval PCDNFCT2_ERROR_LOCKED : The tag cannot be write or read (CC lock)
 * @retval PCDNFCT2_ERROR_MEMORY : Not enough memory available on the tag
 */
uint8_t PCDNFCT2_WriteNDEF( void )
{
	uint8_t status;
	uint8_t bufferRead[PCDNFCT2_READ_SIZE_BUFFER];
	uint8_t buffer[NFCT2_MAX_TAGMEMORY];
	uint8_t byteSize;
	uint16_t size, totalSize, i, firstBloc;
	uint16_t NDEFposition = 16;
	
	// Check if CC is present with NDEF capability
	errchk(PCDNFCT2_Read(0,buffer));
	if (buffer[14] != PCDNFCT2_NDEF_MNB)
		return PCDNFCT2_ERROR_NOT_FORMATED;
	// Check if the tag is not protected for read and write
	if ((buffer[17]&(PCDNFCT2_READ_MSK|PCDNFCT2_WRITE_MSK)) != 0x00)
		return PCDNFCT2_ERROR_LOCKED; 
	// Read the size from CC
	totalSize = buffer[16]*8;
	// Read the size of message
	if (TT2Tag[17] == 0xFF) size = TT2Tag[18]<<8|TT2Tag[19];
	else 										size = TT2Tag[17];
	
	// Check if there is enough memory available
	if (size > totalSize)
		return PCDNFCT2_ERROR_MEMORY_TAG;
	
	if (totalSize > PCDNFCT2_SECTOR_SIZE)
		size = PCDNFCT2_SECTOR_SIZE;
	else
		size = totalSize;
	// Read the whole memory
	for (i=0;i<size/4;i+=4)
	{
		errchk(PCDNFCT2_Read(i,bufferRead));
		memcpy(&buffer[i*4],&bufferRead[2],PCDNFCT2_READ_SIZE);
	}
	// Second sector (if needed)
	if (totalSize > PCDNFCT2_SECTOR_SIZE)
	{
		PCDNFCT2_SectorSelect(1);
		size = totalSize-PCDNFCT2_SECTOR_SIZE;
		for (i=4;i<size/4;i+=4)
		{
			errchk(PCDNFCT2_Read(i,bufferRead));
			memcpy(&buffer[PCDNFCT2_SECTOR_SIZE+i*4],&bufferRead[2],PCDNFCT2_READ_SIZE);
		}
	}
	
	// Searching for the first NDEF TLV
	while (buffer[NDEFposition] != PCDNFCT2_TLV_NDEF)
	{
		if (buffer[NDEFposition] == PCDNFCT2_TLV_EMPTY) // Empty TLV
			NDEFposition++;
		else if (buffer[NDEFposition] == PCDNFCT2_TLV_LOCK || buffer[NDEFposition] == PCDNFCT2_TLV_MEM)  // Lock CTRL TLV or Mem CTRL TLV
			NDEFposition+=5;
		else
			return PCDNFCT2_ERROR; // Other TLV not supported yet (proprietary)
		if (NDEFposition > totalSize)
			return PCDNFCT2_ERROR; // EOF and no NDEF TLV found
	}
	
	// Write the message
	if (TT2Tag[17] == 0xFF)
		size = ((TT2Tag[18]<<8) | (TT2Tag[19])) + 5;
	else
		size = TT2Tag[17] +3;

	memcpy(&buffer[NDEFposition],&TT2Tag[16],size);
	
	// The size has to be 0 for the first write
	byteSize = TT2Tag[17];
	buffer[NDEFposition+1] = 0;
	
	firstBloc = NDEFposition>>2;
	
	// To be sure to write the first sector
	if (totalSize > PCDNFCT2_SECTOR_SIZE)
	{
		errchk(PCDNFCT2_SectorSelect(0));
	}
	// Write all the blocks
	for (i=firstBloc;i<=(size>>2)+firstBloc;i++)
	{
		errchk(PCDNFCT2_Write(i, &buffer[i*4]));
		// Tag from NXP needs this delay...
		delay(5);
	}
	
	// Write the size (numBloc = (NDEFposition+1)>>2)
	buffer[NDEFposition+1] = byteSize;
	errchk(PCDNFCT2_Write(((NDEFposition+1)>>2), &buffer[((NDEFposition+1)>>2)*4]));
	
	return PCDNFCT2_OK;
Error:
	return PCDNFCT2_ERROR;
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
