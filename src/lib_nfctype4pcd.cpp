/**
  ******************************************************************************
  * @file    lib_nfctype4pcd.c 
  * @author  MMY Application Team
  * @version $Revision: 1334 $
  * @date    $Date: 2015-11-05 10:53:37 +0100 (Thu, 05 Nov 2015) $
  * @brief   Generates the NFC type4 commands
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
#include "lib_nfctype4pcd.h"

extern uint8_t CardNDEFfileT4A[];
extern uint8_t CardNDEFfileT4B[];

/* Variables for the different modes */
//extern DeviceMode_t devicemode;
extern TagType_t nfc_tagtype;

extern uint16_t FSC;

static int8_t PCDNFCT4_SelectApplication(void);
static int8_t PCDNFCT4_SelectCCfile(void);
static int8_t PCDNFCT4_SelectNDEFfile(const uint8_t NDEF_ID_MSB, const uint8_t NDEF_ID_LSB);
static uint8_t PCDNFCT4_ReadBinary(const uint16_t Offset ,const uint8_t NbByteToRead , uint8_t *pBufferRead);
static uint8_t PCDNFCT4_UpdateBinary(const uint16_t Offset ,const uint8_t NbByteToWrite , uint8_t *pBufferWrite);

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


/** @addtogroup NFC_type4_pcd
 * 	@{
 *	@brief  This file is used to exchange with NFC FORUM Type4 Tag.
*/


/** @addtogroup lib_nfctype4pcd_Private_Functions
 *  @{
 */


/**
  * @brief  This function sends the SelectApplication command
	* @retval PCDNFCT4_OK : Command success
	* @retval PCDNFCT4_ERROR : Transmission error
  */
static int8_t PCDNFCT4_SelectApplication(void)
{
uint8_t 	P1byte = 0x04,
					P2byte = 0x00,
					LCbyte = 0x07,
 					NDEFaplicationField[] = PCDNFCT4_SELECT_APPLI;
					
	if (ISO7816_SelectFile ( P1byte , P2byte , LCbyte ,NDEFaplicationField ) == ISO7816_SUCCESSCODE)
		return PCDNFCT4_OK;
	else
		return PCDNFCT4_ERROR;
}

/**
  * @brief  This function sends the SelectCCfile command
	* @retval PCDNFCT4_OK : Command success
	* @retval PCDNFCT4_ERROR : Transmission error
  */
static int8_t PCDNFCT4_SelectCCfile(void)
{
uint8_t 	P1byte = 0x00,
					P2byte = 0x0C,
					LCbyte = 0x02,
 					FileId[] = PCDNFCT4_CC_ID;

if (ISO7816_SelectFile ( P1byte , P2byte , LCbyte ,FileId ) == ISO7816_SUCCESSCODE)
		return PCDNFCT4_OK;
	else
		return PCDNFCT4_ERROR;

}

/**
  * @brief  This function sends the SelectNDEF command
  * @param  NDEF_ID_MSB : NDEF identifier most significant bit
  * @param  NDEF_ID_LSB : NDEF identifier least significant bit
	* @retval PCDNFCT4_OK : Command success
	* @retval PCDNFCT4_ERROR : Transmission error
  */
static int8_t PCDNFCT4_SelectNDEFfile(const uint8_t NDEF_ID_MSB, const uint8_t NDEF_ID_LSB)
{
	uint8_t P1byte = 0x00,
					P2byte = 0x0C,
					LCbyte = 0x02,
					FileId[2];
				
	FileId[0] = NDEF_ID_MSB;
	FileId[1] = NDEF_ID_LSB;			

	if (ISO7816_SelectFile ( P1byte , P2byte , LCbyte ,FileId ) == ISO7816_SUCCESSCODE)
		return PCDNFCT4_OK;
	else
		return PCDNFCT4_ERROR;
}

/**
  * @brief  This function sends a read binary command
	* @param	Offset : first byte to read
	* @param	NbByteToRead : number of byte to read
	* @param	pBufferRead : pointer of the buffer read from the tag
	* @retval PCDNFCT4_OK : Command success
	* @retval PCDNFCT4_ERROR : Transmission error
  */
static uint8_t PCDNFCT4_ReadBinary(const uint16_t Offset ,const uint8_t NbByteToRead , uint8_t *pBufferRead)
{
	uint8_t P1byte = GETMSB(Offset),
					P2byte = GETLSB(Offset),
					LEbyte = NbByteToRead;
					
	if (ISO7816_ReadBinary ( P1byte , P2byte , LEbyte ,pBufferRead ) == ISO7816_SUCCESSCODE)
		return PCDNFCT4_OK;
	else
		return PCDNFCT4_ERROR;		
	
}

/**
  * @brief  This function sends a update binary command
	* @param	Offset : first byte to write
	* @param	NbByteToWrite : number of byte to write
	* @param	pBufferWrite : pointer of the buffer which contains data to write
	* @retval PCDNFCT4_OK : Command success
	* @retval PCDNFCT4_ERROR : Transmission error
  */
static uint8_t PCDNFCT4_UpdateBinary(const uint16_t Offset ,const uint8_t NbByteToWrite , uint8_t *pBufferWrite)
{
	uint8_t P1byte = GETMSB(Offset),
					P2byte = GETLSB(Offset),
					LEbyte = NbByteToWrite;

	if (ISO7816_UpdateBinary ( P1byte , P2byte , LEbyte ,pBufferWrite ) == ISO7816_SUCCESSCODE)
		return PCDNFCT4_OK;
	else
		return PCDNFCT4_ERROR;		
}

/**
  * @}
  */

/** @addtogroup lib_nfctype4pcd_Public_Functions
 *  @{
 */

/**
 * @brief  This function reads the NDEF message from a tag type 4 and store result in the CardNDEFfile buffer
 * @retval PCDNFCT4_OK : Command success
 * @retval PCDNFCT4_ERROR : Transmission error
 * @retval PCDNFCT4_ERROR_LOCKED : The tag cannot be read (CCfile lock)
 */
uint8_t PCDNFCT4_ReadNDEF( void )
{
	uint8_t status, NDEF_ID_MSB, NDEF_ID_LSB;
	uint8_t limit = 0;
	uint16_t size, i = 0, MLe;
	uint8_t buffer[PCDNFCT4_BUFFER_READ];
	uint8_t *CardNDEFfile;
	
	// Choose the correct buffer
	if (nfc_tagtype == TT4A)
		CardNDEFfile = CardNDEFfileT4A;
	else
		CardNDEFfile = CardNDEFfileT4B;

	// SelectAppli
	errchk(PCDNFCT4_SelectApplication());

	// SelectCC
	errchk(PCDNFCT4_SelectCCfile());
	errchk(PCDNFCT4_ReadBinary(0x00, 0x0F, buffer));
	NDEF_ID_MSB = buffer[12];
	NDEF_ID_LSB = buffer[13];
	MLe = MIN((buffer[6]<<8|buffer[7]),0xF6);
	// Check if read access is allowed
	if (buffer[16] != PCDNFCT4_ACCESS_ALLOWED)
		return PCDNFCT4_ERROR_LOCKED;
	// SelectNDEF
	errchk(PCDNFCT4_SelectNDEFfile(NDEF_ID_MSB,NDEF_ID_LSB));
	// Read length
	errchk(PCDNFCT4_ReadBinary(0x00, 0x02, buffer));
	size = (((buffer[3])<<8) | (buffer[4])) + 2;
	
	// Check if there is enough memory available to read the tag
	if (size > NFCT4_MAX_NDEFMEMORY)
		return PCDNFCT4_ERROR_MEMORY_INTERNAL;
	
	/* Mle have a good chance to be bigger than FSC but just in case */
	/* FSC - PCB, SW1, SW2, CRC1, CRC2 */
	limit = MIN(MLe,(FSC-5));
	
	// Read data
	while (size>limit)
	{
		errchk(PCDNFCT4_ReadBinary(i*limit, limit, buffer));
		memcpy(&CardNDEFfile[i*limit],&buffer[PCD_DATA_OFFSET+1],limit);
		size -= limit;
		i++;
	}
	if (size > 0)
	{
		errchk(PCDNFCT4_ReadBinary(i*limit, size, buffer));
		memcpy(&CardNDEFfile[i*limit],&buffer[PCD_DATA_OFFSET+1],size);
	}
		
	return PCDNFCT4_OK;
Error:
	return PCDNFCT4_ERROR;
}

/**
 * @brief  This function writes the NDEF message to a tag type 4 from the CardNDEFfile buffer
 * @retval PCDNFCT4_OK : Command success
 * @retval PCDNFCT4_ERROR : Transmission error
 * @retval PCDNFCT4_ERROR_LOCKED : The tag cannot be write (CCfile lock)
 * @retval PCDNFCT4_ERROR_MEMORY : Not enough memory available on the tag
 */
uint8_t PCDNFCT4_WriteNDEF( void )
{
	uint8_t status, NDEF_ID_MSB, NDEF_ID_LSB;
	uint16_t size, i = 0, MLc, memoryAvailable;
	uint8_t buffer[32], bufferSize[2];
	uint8_t *CardNDEFfile;
	
	// Choose the correct buffer
	if (nfc_tagtype == TT4A)
		CardNDEFfile = CardNDEFfileT4A;
	else
		CardNDEFfile = CardNDEFfileT4B;

	// SelectAppli
	errchk(PCDNFCT4_SelectApplication());
	// SelectCC
	errchk(PCDNFCT4_SelectCCfile());
	errchk(PCDNFCT4_ReadBinary(0x00, 0x0F, buffer));
	NDEF_ID_MSB = buffer[12];
	NDEF_ID_LSB = buffer[13];
	// Check if there is enough memory available on the tag 
	memoryAvailable = buffer[14]<<8|buffer[15];
	size = (((CardNDEFfile[0])<<8) | (CardNDEFfile[1])) + 2;
	if (size > memoryAvailable)
		return PCDNFCT4_ERROR_MEMORY_TAG;
	// Check if write access is allowed
	if (buffer[17] != PCDNFCT4_ACCESS_ALLOWED)
		return PCDNFCT4_ERROR_LOCKED;
	// Chaining not supported yet... and the maximum for ST95HF is 0xF6
	MLc = MIN((buffer[8]<<8|buffer[9]),0xF6);
	// SelectNDEF
	errchk(PCDNFCT4_SelectNDEFfile(NDEF_ID_MSB,NDEF_ID_LSB));
	// Write NDEF
	// The size must be 0
	bufferSize[0] = CardNDEFfile[0]; bufferSize[1] = CardNDEFfile[1];
	CardNDEFfile[0] = CardNDEFfile[1] = 0;
	
	while (size>MLc)
	{
		status = PCDNFCT4_UpdateBinary(i*MLc, MLc, &CardNDEFfile[i*MLc]);
		// If an error occur we have to write back the size to the NDEF buffer
		if (status == PCDNFCT4_ERROR)
		{
			CardNDEFfile[0]=bufferSize[0]; 
			CardNDEFfile[1]=bufferSize[1];
			return PCDNFCT4_ERROR;
		}
		size -= MLc;
		i++;
	}
	if (size > 0)
	{
		status = PCDNFCT4_UpdateBinary(i*MLc, size, &CardNDEFfile[i*MLc]);
		// If an error occur we have to write back the size to the NDEF buffer
		if (status == PCDNFCT4_ERROR)
		{
			CardNDEFfile[0]=bufferSize[0]; 
			CardNDEFfile[1]=bufferSize[1];
			return PCDNFCT4_ERROR;
		}
	}
	
	// Write the size
	errchk(PCDNFCT4_UpdateBinary(0,2,bufferSize));
		
	return PCDNFCT4_OK;
Error:
	return PCDNFCT4_ERROR;
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
