/**
  ******************************************************************************
  * @file    lib_nfctype3pcd.c 
  * @author  MMY Application Team
  * @version $Revision: 1334 $
  * @date    $Date: 2015-11-05 10:53:37 +0100 (Thu, 05 Nov 2015) $
  * @brief   Generates the NFC type3 commands
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
#include "lib_nfctype3pcd.h"

extern FELICA_CARD 	FELICA_Card;
//extern uint8_t TT3Tag[];
extern uint8_t *TT3AttribInfo, *TT3NDEFfile;

static uint8_t PCDNFCT3_ReadAttribInfo(uint8_t *pBufferRead);
static uint8_t PCDNFCT3_WriteAttribInfo(uint8_t *pBufferWrite);
static uint8_t PCDNFCT3_ReadMessage(const uint32_t NbByteToRead, uint8_t *pBufferRead);
static uint8_t PCDNFCT3_WriteMessage(const uint32_t NbByteToWrite, uint8_t *pBufferWrite, uint8_t maxBlocWrite);
static void PCDNFCT3_UpdateCheckSum(uint8_t *bufferAttrib);

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


/** @addtogroup NFC_type3_pcd
 * 	@{
 *	@brief  This file is used to exchange with NFC FORUM Type3 Tag.
*/


/** @addtogroup lib_nfctype3pcd_Private_Functions
 *  @{
 */


/**
 * @brief  This function generates the Check command in order to read the AttribInfo field
 * @param  pBufferRead : Pointer on the buffer which will contain the data read
 * @retval PCDNFCT3_OK : Command success
 * @retval PCDNFCT3_ERROR : Transmission error
 */
static uint8_t PCDNFCT3_ReadAttribInfo(uint8_t *pBufferRead)
{
	uint8_t buffer[] = {PCDNFCT3_CHECK,0,0,0,0,0,0,0,0,0x01,PCDNFCT3_CODE_READ_LSB,PCDNFCT3_CODE_READ_MSB,0x01,PCDNFCT3_FIRST_BLOC_MSB,PCDNFCT3_FIRST_BLOC_LSB};
	
	/* Write UID */
	memcpy(&buffer[1],FELICA_Card.UID,8);

	if (PCD_SendRecv(15,buffer, pBufferRead) == PCD_SUCCESSCODE)
		return PCDNFCT3_OK;
	else
		return PCDNFCT3_ERROR; 
}

/**
 * @brief  This function generates the Update command in order to write the AttribInfo field
 * @param  pBufferWrite : Pointer on the buffer which contains the data to write
 * @retval PCDNFCT3_OK : Command success
 * @retval PCDNFCT3_ERROR : Transmission error
 */
static uint8_t PCDNFCT3_WriteAttribInfo(uint8_t *pBufferWrite)
{
	uint8_t buffer[31] = {PCDNFCT3_UPDATE,0,0,0,0,0,0,0,0,0x01,PCDNFCT3_CODE_WRITE_LSB,PCDNFCT3_CODE_WRITE_MSB,0x01,PCDNFCT3_FIRST_BLOC_MSB,PCDNFCT3_FIRST_BLOC_LSB};
	uint8_t bufferRead[PCDNFCT3_ATTR_SIZE];

	/* Write UID */
	memcpy(&buffer[1],FELICA_Card.UID,8);
	/* Write AttribInfo */
	memcpy(&buffer[15],pBufferWrite,PCDNFCT3_ATTR_SIZE);

	if (PCD_SendRecv(31,buffer, bufferRead) == PCD_SUCCESSCODE)
	{
		if (bufferRead[11] == 0 && bufferRead[12] == 0)
			return PCDNFCT3_OK;
		else
			return PCDNFCT3_ERROR; /* Error code in reception frame */
	}
	else
		return PCDNFCT3_ERROR; /* Transmission error */
}

/**
 * @brief  This function generates check commands in order to read all the NDEF message
 * @param  NbByteToRead : Size of the NDEF message to read
 * @param  pBufferRead : Pointer on the buffer which will contain the data read
 * @retval PCDNFCT3_OK : Command success
 * @retval PCDNFCT3_ERROR : Transmission error
 */
static uint8_t PCDNFCT3_ReadMessage(const uint32_t NbByteToRead, uint8_t *pBufferRead)
{
	uint8_t nbBloc,i,j=0,maxBlocRead;
	uint8_t buffer[48] = {PCDNFCT3_CHECK,0,0,0,0,0,0,0,0,0x01,PCDNFCT3_CODE_READ_LSB,PCDNFCT3_CODE_READ_MSB};
	uint8_t bufferRead[280];

	memcpy(&buffer[1],FELICA_Card.UID,8);

	/* Check the maximum number of bloc that can be read with one command */
	maxBlocRead = TT3AttribInfo[1];

	/* Check the total number of bloc to read */
	nbBloc = (NbByteToRead>>4)+1;

	/* Send as many commands as needed */
	while (nbBloc > maxBlocRead)
	{
		nbBloc-=maxBlocRead;
		buffer[12] = maxBlocRead;
		for(i=0;i<maxBlocRead;i++) // 8001,8002,8003...
		{
			buffer[13+i*2] = PCDNFCT3_FIRST_BLOC_MSB;
			buffer[14+i*2] = PCDNFCT3_FIRST_BLOC_LSB+i+1+j*maxBlocRead;
		}
		if (PCD_SendRecv(13+maxBlocRead*2,buffer, bufferRead) != PCD_SUCCESSCODE)
			return PCDNFCT3_ERROR; 
		/* Append the result to the pBufferRead buffer */
		memcpy(&pBufferRead[j*maxBlocRead*16],&bufferRead[14],maxBlocRead*16);
		j++;
	}
	if (nbBloc != 0) /* If there are blocks remaining */
	{
		buffer[12] = nbBloc;
		for(i=0;i<nbBloc;i++) // 8001,8002,8003...
		{
			buffer[13+i*2] = PCDNFCT3_FIRST_BLOC_MSB;
			buffer[14+i*2] = PCDNFCT3_FIRST_BLOC_LSB+i+1+j*maxBlocRead;
		}
		if (PCD_SendRecv(13+nbBloc*2,buffer, bufferRead) != PCD_SUCCESSCODE)
			return PCDNFCT3_ERROR; 
		/* Append the result to the pBufferRead buffer */
		memcpy(&pBufferRead[j*maxBlocRead*16],&bufferRead[14],nbBloc*16);
	}
		
	return PCDNFCT3_OK;
}

/**
 * @brief  This function generates Update commands in order to write all the NDEF message
 * @param  NbByteToWrite : Size of the NDEF message to write
 * @param  pBufferWrite : Pointer on the buffer which contains the data to write
 * @param  maxBlocWrite : Maximum number of blocs that can be write in a single command
 * @retval PCDNFCT3_OK : Command success
 * @retval PCDNFCT3_ERROR : Transmission error
 */
static uint8_t PCDNFCT3_WriteMessage(const uint32_t NbByteToWrite, uint8_t *pBufferWrite, uint8_t maxBlocWrite)
{
	uint8_t nbBloc,i,j=0;
	uint8_t buffer[280] = {PCDNFCT3_UPDATE,0,0,0,0,0,0,0,0,0x01,PCDNFCT3_CODE_WRITE_LSB,PCDNFCT3_CODE_WRITE_MSB};
	uint8_t bufferRead[16];

	/* Write UID */
	memcpy(&buffer[1],FELICA_Card.UID,8);

	/* Check the total number of bloc to write */
	nbBloc = (NbByteToWrite>>4)+1;

	/* Send as many commands as needed */
	while (nbBloc > maxBlocWrite)
	{
		nbBloc-=maxBlocWrite;
		buffer[12] = maxBlocWrite;
		for(i=0;i<maxBlocWrite;i++) // 8001,8002,8003...
		{
			buffer[13+i*2] = PCDNFCT3_FIRST_BLOC_MSB;
			buffer[14+i*2] = PCDNFCT3_FIRST_BLOC_LSB+i+1+j*maxBlocWrite;
		}
		memcpy(&buffer[13+maxBlocWrite*2],&pBufferWrite[j*maxBlocWrite*16],maxBlocWrite*16);
		if (PCD_SendRecv(13+maxBlocWrite*2+16*maxBlocWrite,buffer, bufferRead) != PCD_SUCCESSCODE)
			return PCDNFCT3_ERROR; 
		if (bufferRead[11] != 0 || bufferRead[12] != 0)
			return PCDNFCT3_ERROR; /* Error code in reception frame */
		j++;
	}
	if (nbBloc != 0) /* If there are blocks remaining */
	{
		buffer[12] = nbBloc;
		for(i=0;i<nbBloc;i++) // 8001,8002,8003...
		{
			buffer[13+i*2] = PCDNFCT3_FIRST_BLOC_MSB;
			buffer[14+i*2] = PCDNFCT3_FIRST_BLOC_LSB+i+1+j*maxBlocWrite;
		}
		memcpy(&buffer[13+nbBloc*2],&pBufferWrite[j*maxBlocWrite*16],nbBloc*16);
		if (PCD_SendRecv(13+nbBloc*2+16*nbBloc,buffer, bufferRead) != PCD_SUCCESSCODE)
			return PCDNFCT3_ERROR; 
		if (bufferRead[11] != 0 || bufferRead[12] != 0)
			return PCDNFCT3_ERROR; /* Error code in reception frame */
	}
		
	return PCDNFCT3_OK;
}

/**
 * @brief  This function updates the checksum of the AttribInfo field
 * @param  bufferAttrib : Pointer to the memory containing the AttribInfo field
 */
static void PCDNFCT3_UpdateCheckSum(uint8_t *bufferAttrib)
{
	uint8_t i;
	uint16_t checkSum = 0;
	for(i=0;i<14;i++)
		checkSum+=bufferAttrib[i];
	bufferAttrib[14] = checkSum>>8;
	bufferAttrib[15] = checkSum&0x00FF;
}

/**
  * @}
  */

/** @addtogroup lib_nfctype3pcd_Public_Functions
 *  @{
 */

/**
 * @brief  This function reads the NDEF message from a tag type 3 and store result in the TT3Tag buffer
 * @retval PCDNFCT3_OK : Command success
 * @retval PCDNFCT3_ERROR : Transmission error
 * @retval PCDNFCT3_ERROR_LOCKED : The tag cannot be read (AttribInfo lock)
 */
uint8_t PCDNFCT3_ReadNDEF( void )
{
	uint8_t buffer[32];
	uint8_t status;
	uint32_t size;
		
	/* Read AttribInfo field */
	errchk(PCDNFCT3_ReadAttribInfo(buffer));
	memcpy(TT3AttribInfo,&buffer[14],PCDNFCT3_ATTR_SIZE);
	/* Calculate the size */
	size = TT3AttribInfo[11]<<16|TT3AttribInfo[12]<<8|TT3AttribInfo[13];
	
	// Check if there is enough memory available to read the tag
	if (size > NFCT3_MAX_TAGMEMORY)
		return PCDNFCT3_ERROR_MEMORY_INTERNAL;
	
	/* Read the NDEF message */
	errchk(PCDNFCT3_ReadMessage(size,TT3NDEFfile));
	
	return PCDNFCT3_OK;
Error:
	return PCDNFCT3_ERROR; 
}

/**
 * @brief  This function writes the NDEF message to a tag type 3 from the TT3Tag buffer
 * @retval PCDNFCT3_OK : Command success
 * @retval PCDNFCT3_ERROR : Transmission error
 * @retval PCDNFCT3_ERROR_LOCKED : The tag cannot be write (AttribInfo lock)
 * @retval PCDNFCT3_ERROR_MEMORY : Not enough memory available on the tag
 */
uint8_t PCDNFCT3_WriteNDEF( void )
{
	uint8_t buffer[32];
	uint8_t bufferAttrib[16];
	uint8_t status;
	uint32_t size, memoryAvailable;
		
	/* Read AttribInfo field */
	errchk(PCDNFCT3_ReadAttribInfo(buffer));
	memcpy(bufferAttrib,&buffer[14],PCDNFCT3_ATTR_SIZE);
	/* Check if write is available */
	if(bufferAttrib[10] != 0x01)
		return PCDNFCT3_ERROR_LOCKED; 
	/* Check if there is enough memory */
	memoryAvailable = ((bufferAttrib[3]<<8)|bufferAttrib[4]);
	size = TT3AttribInfo[11]<<16|TT3AttribInfo[12]<<8|TT3AttribInfo[13];
	if ((size>>4) > memoryAvailable)
		return PCDNFCT3_ERROR_MEMORY_TAG; 
	
	/* Writing flag set to ON */
	bufferAttrib[9] = PCDNFCT3_WRITE_ON;
	/* Update CheckSum */
	PCDNFCT3_UpdateCheckSum(bufferAttrib);
	/* Write the AttribInfo */
	errchk(PCDNFCT3_WriteAttribInfo(bufferAttrib));
	
	/* Write the message */
	errchk(PCDNFCT3_WriteMessage(size, TT3NDEFfile,bufferAttrib[2]));
	
	/* Length */
	memcpy(&bufferAttrib[11],&TT3AttribInfo[11],3);
	/* Writing Flag OFF */
	bufferAttrib[9] = PCDNFCT3_WRITE_OFF;
	/* CheckSum */
	PCDNFCT3_UpdateCheckSum(bufferAttrib);
	/* Write the AttribInfo */
	errchk(PCDNFCT3_WriteAttribInfo(bufferAttrib));
	
	return PCDNFCT3_OK;
Error:
	return PCDNFCT3_ERROR; 
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
