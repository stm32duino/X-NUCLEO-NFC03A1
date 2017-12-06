/**
  ******************************************************************************
  * @file    lib_iso7816pcd.c
  * @author  MMY Application Team
  * @version $Revision: 1334 $
  * @date    $Date: 2015-11-05 10:53:37 +0100 (Thu, 05 Nov 2015) $
  * @brief   Manage the iso 7816 commands
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
#include "lib_iso7816pcd.h"

extern TagType_t nfc_tagtype;

uint8_t bufferSend[MAX_BUFFER_SIZE+3],
				bufferReceive[MAX_BUFFER_SIZE+3];

static APDU_Commands APDUcommand ; 
static APDU_Responce APDUresponse;
static uint8_t BlockNumber = 0x02;

static int8_t ISO7816_SendReceiveAPDU ( uint8_t *pDataReceived );

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


 /** @addtogroup ISO7816_pcd
 * 	@{
 *	@brief  This part of the library is used to follow ISO7816.
 */


/** @addtogroup lib_iso7816pcd_Private_Functions
 *  @{
 */



/**
 * @brief  this function sends the APDU command to the RF transceiver and returns its response
 * @return ISO7816_SUCCESSCODE : the function is succesful
 * @return ISO7816_ERRORCODE_RESPONSE : the function is not  succesful. The tag returns an error code
 * @return ISO7816_ERRORCODE_DEFAULT : the function is not  succesful. 
 */
static int8_t ISO7816_SendReceiveAPDU ( uint8_t *pDataReceived )
{
	uint8_t 	NthByte=0;
	int8_t 		status;
	uint8_t 	bufferFWI[3];
	
	bufferSend[NthByte++] = BlockNumber; 
	BlockNumber ^= 0x01;

	// add the class byte
	bufferSend[NthByte++] = APDUcommand.Header.CLA; 
	// add the command code
	bufferSend[NthByte++] = APDUcommand.Header.INS; 
	// add the P1 and P2 fields
	bufferSend[NthByte++] = APDUcommand.Header.P1;
	bufferSend[NthByte++] = APDUcommand.Header.P2;
	// add the LC field
	if (APDUcommand.Body.LC)
		bufferSend[NthByte++] = APDUcommand.Body.LC;
	// add the datafield field
	memcpy(&(bufferSend[NthByte]),APDUcommand.Body.pData,APDUcommand.Body.LC);
	NthByte += APDUcommand.Body.LC;

	// add the LE field
	if (APDUcommand.Body.LE || (APDUcommand.Header.P1 == 0x04 && APDUcommand.Header.P2 == 0x00))
		bufferSend[NthByte++] = APDUcommand.Body.LE;
	
	// control byte append CRC + 8 bits
	if( nfc_tagtype == TT4A )
		bufferSend[NthByte++] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	 

	// send the command to the RF transceiver
	errchk(PCD_SendRecv(NthByte,bufferSend,pDataReceived))
	
	/* Test if a time extension is required */
	if (pDataReceived[2] == 0xF2)
	{				
		/* Modify temporarly FDT */
		ISO14443A_ConfigFDT(pDataReceived[3]);
		
		/* Send the same buffer in order to accept the time extension */
		memcpy(bufferFWI,&pDataReceived[2],2);
		
		if( nfc_tagtype == TT4A )
		{	
			bufferFWI[2] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;
			PCD_SendRecv(3,bufferFWI, pDataReceived);
		}
		else
			PCD_SendRecv(2,bufferFWI, pDataReceived);
		
		/* Setback FDT default value */
			ISO14443A_ConfigFDT(1);
	}
	
	if( nfc_tagtype == TT4A )
	{
		APDUresponse.SW1 = pDataReceived[pDataReceived[PCD_LENGTH_OFFSET]-5];
		APDUresponse.SW2 = pDataReceived[pDataReceived[PCD_LENGTH_OFFSET]-4];
	}
	else
	{
		APDUresponse.SW1 = pDataReceived[pDataReceived[PCD_LENGTH_OFFSET]-3];
		APDUresponse.SW2 = pDataReceived[pDataReceived[PCD_LENGTH_OFFSET]-2];
	}
		
	if (APDUresponse.SW1 == 0x90 && APDUresponse.SW2 == 0x00)
		return ISO7816_SUCCESSCODE;	
	else
		return ISO7816_ERRORCODE_RESPONSE;	
	
Error :	
		return ISO7816_ERRORCODE_DEFAULT;
}

/**
  * @}
  */

/** @addtogroup lib_iso7816pcd_Public_Functions
 *  @{
 */

/**
 * @brief  this function sends the Select command
 * @param  P1byte	: P1 value
 * @param  P2byte	: P2 value
 * @param  LCbyte	: LC value
 * @param  PData	: pointer of the File ID
 * @return ISO7816_SUCCESSCODE : the function is successful and the tag returns a success code
 * @return ISO7816_ERRORCODE_DEFAULT : the function is not successful 
 */
int8_t ISO7816_SelectFile(const uint8_t P1byte , const uint8_t P2byte , const uint8_t LCbyte ,uint8_t *pDataSel)
{
		// add the class byte
		APDUcommand.Header.CLA = ISO7816_CLASS_0X00; 
		// add the command code
		APDUcommand.Header.INS = ISO7816_SELECT_FILE; 
		// add the P1 and P2 fields
		APDUcommand.Header.P1 = P1byte;
		APDUcommand.Header.P2 = P2byte;
		// add the LC field
		APDUcommand.Body.LC = LCbyte;
		//add the FileId field
		memcpy(APDUcommand.Body.pData, pDataSel, APDUcommand.Body.LC);
		// the LE field is empty
		APDUcommand.Body.LE = 0x00;
	
		// Special case for the selectApplication
		// The block number HAS TO be 0x02
		if (APDUcommand.Header.P1 == 0x04 && APDUcommand.Header.P2 == 0x00)
			BlockNumber = 0x02;
	
		return ISO7816_SendReceiveAPDU ( bufferReceive );
}


/**
 * @brief  this function sends a ReadBinary command
 * @param  P1byte	: P1 value
 * @param  P2byte	: P2 value
 * @param  LEbyte	: LE value
 * @param  *pDataRead	: Pointer to the data read from the tag
 * @return ISO7816_SUCCESSCODE : the function is successful and the tag returns a success code
 * @return ISO7816_ERRORCODE_DEFAULT : the function is not successful 
 */
int8_t ISO7816_ReadBinary(const uint8_t P1byte , const uint8_t P2byte , const uint8_t LEbyte , uint8_t *pDataRead)
{
	// add the class byte
	APDUcommand.Header.CLA = ISO7816_CLASS_0X00; 
	// add the command code
	APDUcommand.Header.INS = ISO7816_READ_BINARY; 
	// add the P1 and P2 fields
	APDUcommand.Header.P1 = P1byte;
	APDUcommand.Header.P2 = P2byte;
	// the LC field is empty
	APDUcommand.Body.LC = 0x00;
		
	APDUcommand.Body.LE = LEbyte;

	return ISO7816_SendReceiveAPDU ( pDataRead );

}

/**
 * @brief  this function sends the UpdateBinary command.
 * @param  P1byte	: P1 value
 * @param  P2byte	: P2 value
 * @param  LCbyte	: LC value
 * @param  *pData	: Pointer to the data to write
 * @return ISO7816_SUCCESSCODE : the function is successful and the tag returns a success code
 * @return ISO7816_ERRORCODE_DEFAULT : the function is not successful 
 */
int8_t ISO7816_UpdateBinary(const uint8_t P1byte , const uint8_t P2byte , const uint8_t LCbyte , uint8_t *pData)
{
  	
	// add the class byte
	APDUcommand.Header.CLA = ISO7816_CLASS_0X00; 
	// add the command code
	APDUcommand.Header.INS = ISO7816_UPDATE_BINARY; 
	// add the P1 and P2 fields
	APDUcommand.Header.P1 = P1byte;
	APDUcommand.Header.P2 = P2byte;
	// add the datafield field
	APDUcommand.Body.LC = LCbyte;
	// add data to write
	memcpy(APDUcommand.Body.pData,pData,APDUcommand.Body.LC);
	// the LE field is empty
	APDUcommand.Body.LE = 0x00;

	return ISO7816_SendReceiveAPDU ( bufferReceive );

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
