/**
  ******************************************************************************
  * @file    lib_iso15693pcd.c 
  * @author  MMY Application Team
  * @version $Revision: 1334 $
  * @date    $Date: 2015-11-05 10:53:37 +0100 (Thu, 05 Nov 2015) $
  * @brief   This file provides set of function defined into ISO15693 specification
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
#include "lib_iso15693pcd.h"

/* ISO15693 */
#define PCD_TYPEV_ARConfigA	        0x01
#define PCD_TYPEV_ARConfigB	        0xD1

 /** @ brief memory allocation for CR95Hf response */
extern uint8_t 	u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3]; 	// buffer for SPI ou UART reception

/* Variables for the different modes */
extern DeviceMode_t devicemode;
extern TagType_t nfc_tagtype;

extern PCD_PROTOCOL TechnoSelected;

/* Get functions --- */
static int8_t ISO15693_GetSelectOrAFIFlag (const uint8_t FlagsByte);
static int8_t ISO15693_GetAddressOrNbSlotsFlag (const uint8_t FlagsByte);
static int8_t ISO15693_GetOptionFlag (const uint8_t FlagsByte);
static int8_t ISO15693_GetProtocolExtensionFlag (const uint8_t FlagsByte);
//static int8_t ISO15693_GetIcRef (uint8_t *IcRefOut);
/* Invotory functions --- */
static int8_t ISO15693_Inventory(const uint8_t Flags , const uint8_t AFI, const uint8_t MaskLength, const uint8_t *MaskValue, uint8_t *pResponse);
static int8_t ISO15693_InventoryOneSlot(const uint8_t Flags , const uint8_t AFI, const uint8_t MaskLength, const uint8_t *MaskValue, uint8_t *pResponse);
static int16_t ISO15693_Inventory16Slots(const uint8_t Flags , const uint8_t AFI, const uint8_t MaskLength, const uint8_t *MaskValue, uint8_t *NbTag, uint8_t *pResponse);
/* Command functions --- */
static int8_t ISO15693_CreateRequestFlag (const uint8_t SubCarrierFlag,const uint8_t DataRateFlag,const uint8_t InventoryFlag,const uint8_t ProtExtFlag,const uint8_t SelectOrAFIFlag,const uint8_t AddrOrNbSlotFlag,const uint8_t OptionFlag,const uint8_t RFUFlag);
static int8_t ISO15693_StayQuiet(const uint8_t Flags,const uint8_t *UIDin);
static int8_t ISO15693_ReadSingleBlock(const uint8_t Flags, const uint8_t *UID, const uint16_t BlockNumber,uint8_t *pResponse);
static int8_t ISO15693_WriteSingleBlock(const uint8_t Flags, const uint8_t *UIDin, const uint16_t BlockNumber,const uint8_t *DataToWrite,uint8_t *pResponse);
static int8_t ISO15693_ReadMultipleBlock(const uint8_t Flags, const uint8_t *UIDin, uint16_t BlockNumber, const uint8_t NbBlock, uint8_t *pResponse);
static int8_t ISO15693_SendEOF(uint8_t *pResponse);
/* Is functions --- */
static int8_t ISO15693_IsInventoryFlag (const uint8_t FlagsByte);
static int8_t ISO15693_IsAddressOrNbSlotsFlag(const uint8_t FlagsByte);
static int8_t ISO15693_IsATagInTheField(const uint8_t *pTagReply);
static int8_t ISO15693_IsCollisionDetected(const uint8_t *pTagReply);
/* CRC16 commands --- */
static int16_t ISO15693_CRC16(const uint8_t *DataIn,const uint8_t Length);
static int8_t ISO15693_IsCorrectCRC16Residue(const uint8_t *DataIn,const uint8_t Length);
/* Tag functions --- */
static uint8_t ISO15693_ReadMultipleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read);
static uint8_t ISO15693_ReadSingleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read);
static uint8_t ISO15693_TagSave(uint8_t Tag_Density, uint16_t NbByte_To_Write, uint16_t FirstByte_To_Write, uint8_t *Data_To_Save, uint8_t *Length_Low_Limit, uint8_t *Length_High_Limit);
static uint8_t ISO15693_WriteTagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBlock_To_Write, uint16_t FirstBlock_To_Write);


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


 /** @addtogroup ISO15693_pcd
 * 	@{
 *	@brief  This part of the library is used to follow ISO15693.
 */


/** @addtogroup lib_iso15693pcd_Private_Functions
 *  @{
 */



/**  
* @brief  	this function returns Select Or AFI flag  (depending on inventory flag)
* @param  	FlagsByte	: Request flags on one byte	
* @retval 	Select Or AFI
*/
static int8_t ISO15693_GetSelectOrAFIFlag(const uint8_t FlagsByte)
{

	if ((FlagsByte & ISO15693_MASK_SELECTORAFIFLAG) != 0x00)
		return true ;
	else
		return false ;
}

/**  
* @brief  	this function returns address Or Number of slots flag  (depending on inventory flag)
* @param  	FlagsByte	: Request flags on one byte	
* @retval 	address Or Number of slots
*/
static int8_t ISO15693_GetAddressOrNbSlotsFlag(const uint8_t FlagsByte)
{

	if ((FlagsByte & ISO15693_MASK_ADDRORNBSLOTSFLAG) != 0x00)
		return true ;
	else
		return false ;
}

/**  
* @brief  this function returns Option flag  (depending on inventory flag)
* @param  	FlagsByte	: the byts cantaining the eight flags  	
* @retval 	Option flag
*/
static int8_t ISO15693_GetOptionFlag(const uint8_t FlagsByte)
{

	if ((FlagsByte & ISO15693_MASK_OPTIONFLAG) != 0x00)
		return true ;
	else
		return false ;
}


/**  
* @brief  this function returns Option flag  (depending on inventory flag)
* @param  	FlagsByte	: the byts cantaining the eight flags  	
* @retval 	Option flag
*/
static int8_t ISO15693_GetProtocolExtensionFlag(const uint8_t FlagsByte)
{

	if ((FlagsByte & ISO15693_MASK_PROTEXTFLAG) != 0x00)
		return true ;
	else
		return false ;
}



/**  
* @brief  this function return a Byte, which is concatenation of iventory flags 
* @param  SubCarrierFlag	:  
* @param	DataRateFlag
* @param	InventoryFlag
* @param	ProtExtFlag
* @param	SelectOrAFIFlag
* @param	AddrOrNbSlotFlag
* @param	OptionFlag
* @param	RFUFlag
* @retval 	Flags byte
*/
static int8_t ISO15693_CreateRequestFlag(const uint8_t SubCarrierFlag,const uint8_t DataRateFlag,const uint8_t InventoryFlag,const uint8_t ProtExtFlag,const uint8_t SelectOrAFIFlag,const uint8_t AddrOrNbSlotFlag,const uint8_t OptionFlag,const uint8_t RFUFlag)
{
int32_t FlagsByteBuf=0;

		FlagsByteBuf = 	(SubCarrierFlag 	& 0x01)					|
										((DataRateFlag  	& 0x01)	<< 1)		|
										((InventoryFlag 	& 0x01) << 2)		|
										((ProtExtFlag		& 0x01)	<< 3)			|
										((SelectOrAFIFlag   & 0x01)	<< 4)	|
										((AddrOrNbSlotFlag  & 0x01)	<< 5)	|
										((OptionFlag  		& 0x01) << 6)		|
										((RFUFlag  			& 0x01) << 7);

	return (int8_t) FlagsByteBuf; 
}

/**  
* @brief  this function returns the tag AFI word
* @param  	AFIout		: 	tag AFI read
* @retval status function
*/
// static int8_t ISO15693_GetIcRef (uint8_t *IcRefOut) 
// { 
// int8_t 	FlagsByteData;
// uint8_t	TagReply	[ISO15693_MAXLENGTH_REPLYGETSYSTEMINFO+3];
// int8_t	status;

// 	*IcRefOut = 0x00;
// 	FlagsByteData = ISO15693_CreateRequestFlag 	(	ISO15693_REQFLAG_SINGLESUBCARRIER,
// 												ISO15693_REQFLAG_HIGHDATARATE,
// 												ISO15693_REQFLAG_INVENTORYFLAGNOTSET,
// 												ISO15693_REQFLAG_NOPROTOCOLEXTENSION,
// 												ISO15693_REQFLAG_NOTSELECTED,
// 												ISO15693_REQFLAG_NOTADDRESSED,
// 												ISO15693_REQFLAG_OPTIONFLAGNOTSET,
// 												ISO15693_REQFLAG_RFUNOTSET);
// 	// select 15693 protocol
// 	errchk(ISO15693_Init	(	));

// 	// Uid and CRC = 0x00
// 	errchk(ISO15693_GetSystemInfo (FlagsByteData,
// 							0x00,
// 							TagReply));
// 	

// 	*IcRefOut = TagReply[PCD_DATA_OFFSET + 12];


// 	return ISO15693_SUCCESSCODE;

// Error:
// 	return ISO15693_ERRORCODE_DEFAULT;

// }
 

/**  
* @brief  this function send an inventory command to a contacless tag.  
* @param  Flags		:  	Request flags
* @param	AFI			:	AFI byte (optional)
* @param	MaskLength 	: 	Number of bits of mask value
* @param	MaskValue	:  	mask value which is compare to Contacless tag UID 
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_PARAMETERLENGTH	: 	MaskLength value is erroneous
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_Inventory(const uint8_t Flags , const uint8_t AFI, const uint8_t MaskLength, const uint8_t *MaskValue, uint8_t *pResponse)
{
uint8_t 	NthByte = 0,
					InventoryBuf [ISO15693_MAXLENGTH_INVENTORY],
					NbMaskBytes = 0,
					NbSignificantBits=0;
int8_t 		FirstByteMask,
					NthMaskByte = 0,
					status;
	
	// initialize the result code to 0xFF and length to 0  in case of error 
	*pResponse = SENDRECV_ERRORCODE_SOFT;
	*(pResponse+1) = 0x00;

	if (MaskLength>ISO15693_NBBITS_MASKPARAMETER)
		return ISO15693_ERRORCODE_PARAMETERLENGTH;

	errchk(ISO15693_IsInventoryFlag (Flags));
		
	
	InventoryBuf[NthByte++] = Flags;
	InventoryBuf[NthByte++] = ISO15693_CMDCODE_INVENTORY;
	
	if (ISO15693_GetSelectOrAFIFlag (Flags) == true)
		InventoryBuf[NthByte++] = AFI;
	
	InventoryBuf[NthByte++] = MaskLength;

	if (MaskLength !=0)
	{
		// compute the number of bytes of mask value(2 border exeptions)
	   	if (MaskLength == 64)
			NbMaskBytes = 8;
		else
	   		NbMaskBytes = MaskLength / 8 + 1;
	
		NbSignificantBits = MaskLength - (NbMaskBytes-1) * 8;
		if (NbSignificantBits !=0)
	   		FirstByteMask = (0x01 <<NbSignificantBits)-1;
		else 
			FirstByteMask = 0xFF;
	
	   	// copy the mask value 
		if (NbMaskBytes >1)
		{
			for (NthMaskByte = 0; NthMaskByte < NbMaskBytes - 1; NthMaskByte ++ )
				InventoryBuf[NthByte++] = MaskValue[NthMaskByte];
		}
	
		if (NbSignificantBits !=0)
			InventoryBuf[NthByte++] = MaskValue[NthMaskByte] & FirstByteMask;
	}

 	errchk(PCD_SendRecv(NthByte,InventoryBuf,pResponse));

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != ISO15693_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
Error:
	return ISO15693_ERRORCODE_DEFAULT;
}


/**  
* @brief  	this function send an inventory command to a contactless tag. 
* @brief  	The NbSlot flag of Request flags is set (1 value). 
* @param  	Flags		:  	Request flags
* @param	AFI			:	AFI byte (optional)
* @param	MaskLength 	: 	Number of bits of mask value
* @param	MaskValue	:  	mask value which is compare to Contacless tag UID 
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_PARAMETERLENGTH	: 	MaskLength value is erroneous
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_InventoryOneSlot(const uint8_t Flags , const uint8_t AFI, const uint8_t MaskLength, const uint8_t *MaskValue, uint8_t *pResponse)
{
int8_t 	NewFlags,
		status;
	
	// force NbSlot Flag to 1;
	NewFlags = Flags | ISO15693_MASK_ADDRORNBSLOTSFLAG;
	 
	status = ISO15693_Inventory( 
									NewFlags ,
									AFI, 
									MaskLength, 
									MaskValue,  
									pResponse  );

	return status;
}

/**  
* @brief  	this function send an inventory command to a contacless tag. 
* @brief  	The NbSlot flag of Request flags is reset (0 value). 
* @param  	Flags		:  	Request flags
* @param	AFI			:	AFI byte (optional)
* @param	MaskLength 	: 	Number of bits of mask value
* @param	MaskValue	:  	mask value which is compare to Contacless tag UID 
* @param	NbTag		: 	Nbtag inventoried
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_PARAMETERLENGTH	: 	MaskLength value is erroneous
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int16_t ISO15693_Inventory16Slots(const uint8_t Flags , const uint8_t AFI, const uint8_t MaskLength, const uint8_t *MaskValue, uint8_t *NbTag, uint8_t *pResponse)
{
int8_t 		NthSlot = 0,
			status,
			NewFlags;
int16_t		ReturnValue=0;
uint8_t		pOneTagResponse [ISO15693_NBBYTE_UID+3+2];
	
	// force NbSlot Flag to 0;
	NewFlags = Flags & ~ISO15693_MASK_ADDRORNBSLOTSFLAG;

	*NbTag = 0;
	status = ISO15693_Inventory( 
									NewFlags ,
									AFI, 
									MaskLength, 
									MaskValue, 
									pOneTagResponse  );
	

	if (status == ISO15693_SUCCESSCODE && ISO15693_IsCollisionDetected (pOneTagResponse)== ISO15693_SUCCESSCODE)
			ReturnValue = 0x0001;
	else if (status == ISO15693_SUCCESSCODE)
	{
		memcpy(pResponse,pOneTagResponse,pOneTagResponse[PCD_LENGTH_OFFSET]+2);
	   	(*NbTag)++;
		
	}
	
		
	while (NthSlot++ <15 )
	{
		delay(5);

		status = ISO15693_SendEOF(pOneTagResponse  );

		if (status == ISO15693_SUCCESSCODE && ISO15693_IsCollisionDetected (pOneTagResponse)== ISO15693_SUCCESSCODE)
			ReturnValue |= (1<<NthSlot);
		else if (status == ISO15693_SUCCESSCODE)
		{	memcpy(&(pResponse[(*NbTag)*(pOneTagResponse[PCD_LENGTH_OFFSET]+2)]),pOneTagResponse,pOneTagResponse[PCD_LENGTH_OFFSET]+2);
			(*NbTag)++;
		}
		
		
	}

	return ReturnValue;

}


/**  
* @brief  	this function send an stay_quiet command to contacless tag.
* @param  	Flags		:  	Request flags
* @param		UIDin		:  	pointer on contactless tag UID
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_StayQuiet(const uint8_t Flags,const uint8_t *UIDin)
{
uint8_t DataToSend[ISO15693_MAXLENGTH_STAYQUIET],
	 	NthByte = 0,
		pResponse[ISO15693_MAXLENGTH_REPLYSTAYQUIET];
int8_t	status;

	/* the StayQuiet command shall always be executed in adrressed mode ( Select_flag is set 	*/
	/* to 0 and addreess flag is set to 1)	*/
	errchk (ISO15693_IsAddressOrNbSlotsFlag (Flags));

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_STAYQUIET;

	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
	NthByte +=ISO15693_NBBYTE_UID;	

   PCD_SendRecv(NthByte,DataToSend,pResponse);


	return ISO15693_SUCCESSCODE;  
Error:	
	*pResponse = SENDRECV_ERRORCODE_SOFT;
	*(pResponse+1) = 0x00;
	return ISO15693_ERRORCODE_DEFAULT;
	 
}	

/**  
* @brief  	this function send an ReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_ReadSingleBlock(const uint8_t Flags, const uint8_t *UIDin, const uint16_t BlockNumber,uint8_t *pResponse)
{
uint8_t DataToSend[ISO15693_MAXLENGTH_READSINGLEBLOCK],
		NthByte=0;

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_READSINGLEBLOCK;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	if (ISO15693_GetProtocolExtensionFlag (Flags) 	== false)
		DataToSend[NthByte++] = BlockNumber;
	else 
	{
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;	
	}

	PCD_SendRecv(NthByte,DataToSend,pResponse);

//	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
//		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
}


/**  
* @brief  	this function send an WriteSingleblock command and returns ISO15693_SUCCESSCODE if the command 
* @brief  	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to write
* @param	BlockLength :	Nb of byte of block length
* @param	DataToWrite :	Data to write into contacless tag memory
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_WriteSingleBlock(const uint8_t Flags, const uint8_t *UIDin, const uint16_t BlockNumber,const uint8_t *DataToWrite,uint8_t *pResponse )
{
uint8_t DataToSend[MAX_BUFFER_SIZE],
		NthByte=0,
		BlockLength = ISO15693_NBBYTE_BLOCKLENGTH;

	
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_WRITESINGLEBLOCK;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	if (ISO15693_GetProtocolExtensionFlag (Flags) 	== false)
		DataToSend[NthByte++] = BlockNumber;
	else 
	{
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
		
	}
	
	memcpy(&(DataToSend[NthByte]),DataToWrite,BlockLength);
	NthByte +=BlockLength;

	if (ISO15693_GetOptionFlag (Flags) == false)
		PCD_SendRecv(NthByte,DataToSend,pResponse);
	else 
	{	PCD_SendRecv(NthByte,DataToSend,pResponse);
	 	delay(20);
		ISO15693_SendEOF (pResponse);
	}	

//	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
//		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
	
}

/**  
* @brief  this function send an ReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_ReadMultipleBlock(const uint8_t Flags, const uint8_t *UIDin, uint16_t BlockNumber, const uint8_t NbBlock, uint8_t *pResponse)
{
uint8_t DataToSend[ISO15693_MAXLENGTH_READSINGLEBLOCK],
		NthByte=0;


	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_READMULBLOCKS;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}
	BlockNumber=BlockNumber<<5; // *32
	if (ISO15693_GetProtocolExtensionFlag (Flags) 	== false)
		DataToSend[NthByte++] = BlockNumber;
	else 
	{
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
		
	}
	
	DataToSend[NthByte++] = NbBlock;

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == PCD_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;


}

/**  
* @brief  	this function send an EOF pulse to contactless tag.
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_SendEOF(uint8_t *pResponse  )
{
	
	PCD_SendEOF(pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != ISO15693_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;
		
	return ISO15693_SUCCESSCODE;

}


/**  
* @brief  	this function returns ISO15693_SUCCESSCODE is Inventorye flag is set
* @param  	FlagsByte	: the byts cantaining the eight flags  	
* @retval 	Inventory flag
*/
static int8_t ISO15693_IsInventoryFlag(const uint8_t FlagsByte)
{
	if ((FlagsByte & ISO15693_MASK_INVENTORYFLAG) != 0x00)
		return ISO15693_SUCCESSCODE ;
	else
		return ISO15693_ERRORCODE_DEFAULT ;
}

/**  
* @brief  	this function returns ISO15693_SUCCESSCODE if address Or Number of slots flag is set (depending on inventory flag)
* @param  	FlagsByte	: Request flag  	
* @retval 	ISO15693_SUCCESSCODE : address or Number of slots flag is set
* @retval 	ISO15693_ERRORCODE_DEFAULT : address Or Number of slots flag is reset
*/
static int8_t ISO15693_IsAddressOrNbSlotsFlag(const uint8_t FlagsByte)
{

	if ((FlagsByte & ISO15693_MASK_ADDRORNBSLOTSFLAG) != 0x00)
		return ISO15693_SUCCESSCODE ;
	else
		return ISO15693_ERRORCODE_DEFAULT ;
}


 /**  
* @brief  	this function returns ISO15693_SUCCESSCODE if a tag has reply, ISO15693_ERRORCODE_DEFAULT otherwise
* @param	pResponse	: 	pointer on PCD  response	
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_IsATagInTheField(const uint8_t *pResponse)
{
	
	if ( pResponse[READERREPLY_STATUSOFFSET] == SENDRECV_RESULTSCODE_OK)
		return ISO15693_SUCCESSCODE;
	// When a collision occurs between two or more tags the PCD  response can be 88 00
	if ( pResponse[READERREPLY_STATUSOFFSET] == SENDRECV_ERRORCODE_SOF)
		return ISO15693_SUCCESSCODE;
	// When a collision occurs between two or more tags the PCD  response can be 8E 00
	if ( pResponse[READERREPLY_STATUSOFFSET] == SENDRECV_ERRORCODE_RECEPTIONLOST)
		return ISO15693_SUCCESSCODE;

	return ISO15693_ERRORCODE_DEFAULT;

}

/**  
* @brief  	this function returns ISO15693_SUCCESSCODE if a collision has been detected, ISO15693_ERRORCODE_DEFAULT otherwise
* @param	pResponse	: 	pointer on PCD  response 	
* @retval 	ISO15693_SUCCESSCODE	: 	collision detected
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 no collision detected
*/
static int8_t ISO15693_IsCollisionDetected(const uint8_t *pResponse)
{
	int8_t tmp=pResponse[PCD_LENGTH_OFFSET+pResponse[PCD_LENGTH_OFFSET]]& (CONTROL_15693_CRCMASK | CONTROL_15693_COLISIONMASK);

	// if reply status is SOF invalid, a collision might be occured
//	if (pResponse[READERREPLY_STATUSOFFSET] == SENDRECV_ERRORCODE_SOF)
//		return ISO15693_SUCCESSCODE;

 //  if (pResponse[READERREPLY_STATUSOFFSET] == SENDRECV_ERRORCODE_RECEPTIONLOST)
//		return ISO15693_SUCCESSCODE;

	// if reply status is Ok but the CRC is invalid, a collision might be occured
//	if (pResponse[READERREPLY_STATUSOFFSET] == SENDRECV_RESULTSCODE_OK &&
//		ISO15693_IsCorrectCRC16Residue (&(pResponse[PCD_DATA_OFFSET]),pResponse[PCD_LENGTH_OFFSET]-1)== ISO15693_ERRORCODE_DEFAULT)
//		return ISO15693_SUCCESSCODE;

	// if the two last bits of control byte is egual to one,  a collision occured
//	if ((pResponse[PCD_LENGTH_OFFSET+pResponse[PCD_LENGTH_OFFSET]]) & (CONTROL_15693_CRCMASK | CONTROL_15693_COLISIONMASK) == 0x03)	
 	if (tmp  == 0x03)	
		return ISO15693_SUCCESSCODE;	
	
	return ISO15693_ERRORCODE_DEFAULT;

}

/**  
* @brief  	this function computes the CRC16 as defined by CRC ISO/IEC 13239
* @param  	DataIn		:	input data 
* @param	NbByte 		: 	Number of byte of DataIn
* @retval	ResCrc		: 	CRC16 computed
*/
static int16_t ISO15693_CRC16(const uint8_t *DataIn,const uint8_t NbByte) 
{ 
int8_t 	i,
		j; 
int32_t ResCrc = ISO15693_PRELOADCRC16;
	
	for (i=0;i<NbByte;i++) 
	{ 
		ResCrc=ResCrc ^ DataIn[i];
		for (j=8;j>0;j--) 
		{
			ResCrc = (ResCrc & ISO15693_MASKCRC16) ? (ResCrc>>1) ^ ISO15693_POLYCRC16 : (ResCrc>>1); 
		}
	} 

	return ((~ResCrc) & 0xFFFF);
	
} 

/**  
* @brief  	this function computes the CRC16 residue as defined by CRC ISO/IEC 13239
* @param  	DataIn		:	input to data 
* @param	Length 		: 	Number of bits of DataIn
* @retval 	ISO15693_SUCCESSCODE  	:   CRC16 residue is correct	
* @retval 	ISO15693_ERRORCODE_DEFAULT  	:  CRC16 residue is false
*/
static int8_t ISO15693_IsCorrectCRC16Residue(const uint8_t *DataIn,const uint8_t Length)
{
int16_t ResCRC=0;

	// check the CRC16 Residue 
	if (Length !=0)
		ResCRC=ISO15693_CRC16 (DataIn, Length);
	
	if (((~ResCRC) & 0xFFFF) != ISO15693_RESIDUECRC16)
		return ISO15693_ERRORCODE_DEFAULT;
	
	return ISO15693_SUCCESSCODE;
}

/**
* @brief  Save the data from a TAG block for the uncomplete block to write
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  NbByte_To_Write : Numbre of Byte to write in the TAG
* @param  FirstByte_To_Write : First Byte to write in the TAG
* @param  *Data_To_Save : Data store before write the complete block of the TAG
* @param  *Length_Low_Limit : Number of Byte save for the first block to write (0 =< Length_Low_Limit =< 4)
* @param  *Length_High_Limit : Number of Byte save for the last block to write (0 =< Length_High_Limit =< 4)	
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
static uint8_t ISO15693_TagSave (uint8_t Tag_Density, uint16_t NbByte_To_Write, uint16_t FirstByte_To_Write, uint8_t *Data_To_Save, uint8_t *Length_Low_Limit, uint8_t *Length_High_Limit)
{				 
	const uint8_t NbBytePerBlock = 0x04; 		 
	
	uint8_t RepBuffer[32],
					ReadSingleBuffer [4]={0x02, 0x20, 0x00, 0x00};

	uint8_t Nb_Byte_To_Save_Low;
	uint8_t Nb_Byte_To_Save_High;						
	uint16_t Num_Block_Low;
	uint16_t Num_Block_High;
					
					
/**********Find Limit Block*****************/ 
			 Num_Block_Low = FirstByte_To_Write / NbBytePerBlock;
			 Num_Block_High = (FirstByte_To_Write + NbByte_To_Write) / NbBytePerBlock;
					
			 Nb_Byte_To_Save_Low	= (FirstByte_To_Write) % NbBytePerBlock;
			 Nb_Byte_To_Save_High	= 4-((FirstByte_To_Write + NbByte_To_Write/*+ Nb_Byte_To_Save_Low*/) % NbBytePerBlock); 

/**********Read Low Limit******************/					 
				if(Tag_Density == ISO15693_LOW_DENSITY)		
				{
				ReadSingleBuffer[2] = Num_Block_Low;
				PCD_SendRecv (0x03,ReadSingleBuffer,RepBuffer);
				}
				
				
				else if(Tag_Density == ISO15693_HIGH_DENSITY)
				{
				 ReadSingleBuffer[0] = 0x0A;
				 ReadSingleBuffer[2] =  Num_Block_Low & 0x00FF;
				 ReadSingleBuffer[3] = (Num_Block_Low & 0xFF00) >> 8;
				 PCD_SendRecv (0x04,ReadSingleBuffer,RepBuffer);
				}		
				
				if(RepBuffer[0] != 0x80)
					return ISO15693_ERRORCODE_DEFAULT;
				
				memcpy(&Data_To_Save[0], &RepBuffer[3], Nb_Byte_To_Save_Low);
				*Length_Low_Limit = Nb_Byte_To_Save_Low;
				
/**********Read High Limit******************/				 		
				if(Tag_Density == ISO15693_LOW_DENSITY)	
				{	
				ReadSingleBuffer[2] = Num_Block_High;
				PCD_SendRecv (0x03,ReadSingleBuffer,RepBuffer);
				}
				
				else if(Tag_Density == ISO15693_HIGH_DENSITY)
				{
				 ReadSingleBuffer[0] = 0x0A;
				 ReadSingleBuffer[2] =  Num_Block_High & 0x00FF;
				 ReadSingleBuffer[3] = (Num_Block_High & 0xFF00) >> 8;
				 PCD_SendRecv (0x04,ReadSingleBuffer,RepBuffer);	
				}
				
				if(RepBuffer[0] != 0x80)
					return ISO15693_ERRORCODE_DEFAULT;
				
				/*Data temp*/
				memcpy(&Data_To_Save[4], &RepBuffer[3+(4-Nb_Byte_To_Save_High)], (Nb_Byte_To_Save_High));
				*Length_High_Limit = Nb_Byte_To_Save_High;
			return ISO15693_SUCCESSCODE;
}	


//#endif

/**
* @brief  Read Multiple Block in the TAG (sector size : 0x20 = 32 Blocks)
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Read : return the data read in the TAG
* @param  NbBlock_To_Read : Number of block to read in the TAG
* @param  FirstBlock_To_Read : First block to read in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/
static uint8_t ISO15693_ReadMultipleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read)
{
	uint8_t /*ReadMultipleBuffer [5]={0x02, 0x23, 0x00, 0x00, 0x00},*/
				RepBuffer[140],
				Requestflags = 0x02;

	uint16_t NbSectorToRead = 0,
				SectorStart = 0,
				NthDataToRead;
	
	/*Convert the block number in sector number*/
  NbSectorToRead = NbBlock_To_Read/32+1;	
	SectorStart = FirstBlock_To_Read/32;

	// update the RequestFlags
	/*if (Tag_Density == ISO15693_LOW_DENSITY)
		NumSectorToRead = (NumSectorToRead*0x20) & 0x00FF;
	else */if (Tag_Density == ISO15693_HIGH_DENSITY)
		Requestflags = 0x0A;	
	/*else
		return ISO15693_ERRORCODE_DEFAULT;*/
	
	for ( NthDataToRead=0; NthDataToRead < NbSectorToRead; NthDataToRead++)
	{			
			//NumSectorToRead += NthDataToRead;

			if ( ISO15693_ReadMultipleBlock (Requestflags, 0x00,NthDataToRead+SectorStart,0x1F,RepBuffer ) !=ISO15693_SUCCESSCODE)
						return ISO15693_ERRORCODE_DEFAULT;	
			/*Data Temp*/
			memcpy(&Data_To_Read[NthDataToRead*128],&RepBuffer[3],128);
	}
				
	return ISO15693_SUCCESSCODE;
}



/**
* @brief  Read Single Block in the TAG
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Read : return the data read in the TAG
* @param  NbBlock_To_Read : Number of block to read in the TAG
* @param  FirstBlock_To_Read : First block to read in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
static uint8_t ISO15693_ReadSingleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read)
{
	uint8_t /*ReadBuffer [4]={0x02, 0x20, 0x00, 0x00},*/
				RepBuffer[16],
				Requestflags = 0x02;
	uint16_t NthDataToRead= 0x0000;
	uint16_t Num_DataToRead = FirstBlock_To_Read;

		// update the RequestFlags
		if (Tag_Density == ISO15693_LOW_DENSITY)
			Num_DataToRead &=  0x00FF;
		else if (Tag_Density == ISO15693_HIGH_DENSITY)
			Requestflags = 0x0A;	
		else
			return ISO15693_ERRORCODE_DEFAULT;

	for ( NthDataToRead= 0; NthDataToRead < NbBlock_To_Read; NthDataToRead++)
	{			
					Num_DataToRead += NthDataToRead;
								
// 					if (Tag_Density == ISO15693_LOW_DENSITY)
// 					{
// 						Num_DataToRead &=  0x00FF;
// 						ReadBuffer [2] = (Num_DataToRead) & 0x00FF;
//  					memset (RepBuffer, 0x00,8);
//  					PCD_SendRecv (0x03,ReadBuffer,RepBuffer);
// 					}
// 					else if (Tag_Density == ISO15693_HIGH_DENSITY)
// 					{
// 					ReadBuffer [0] = 0x0A;	
// 					ReadBuffer [2] = (Num_DataToRead) & 0x00FF;
// 					ReadBuffer [3] = ((Num_DataToRead) & 0xFF00) >> 8;
// 					memset (RepBuffer, 0x00,8);
// 					PCD_SendRecv (0x04,ReadBuffer,RepBuffer);						
// 					}
// 					else
// 						return ISO15693_ERRORCODE_DEFAULT;

				if ( ISO15693_ReadSingleBlock (Requestflags, 0x00,Num_DataToRead,RepBuffer ) !=ISO15693_SUCCESSCODE)
						return ISO15693_ERRORCODE_DEFAULT;
					
 				 /*Data Temp*/
				 memcpy(&Data_To_Read[NthDataToRead*4],&RepBuffer[3],ISO15693_NBBYTE_BLOCKLENGTH);
	}
				
	return ISO15693_SUCCESSCODE;
}

/**
* @brief  Write data by blocks in the TAG
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Write : Data to write in the TAG
* @param  NbBlock_To_Write : Number of block to write in the TAG
* @param  FirstBlock_To_Write : First block to write in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
static uint8_t ISO15693_WriteTagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBlock_To_Write, uint16_t FirstBlock_To_Write)
{
//	uint8_t WriteBuffer [8]={0x02, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		uint8_t		RepBuffer[32],
				Requestflags =0x02;
	uint16_t NthDataToWrite =0,
				 IncBlock =0;

		// update the RequestFlags
		if (Tag_Density == ISO15693_LOW_DENSITY)
			FirstBlock_To_Write &=  0x00FF;
		else if (Tag_Density == ISO15693_HIGH_DENSITY)
			Requestflags = 0x0A;	
		else
			return ISO15693_ERRORCODE_DEFAULT;

		for ( NthDataToWrite=FirstBlock_To_Write; NthDataToWrite<(FirstBlock_To_Write+NbBlock_To_Write); NthDataToWrite++)
		{
// 				/*Block To Write*/				
// 				if(Tag_Density == ISO15693_LOW_DENSITY)	
// 				
// 				WriteBuffer [2] = (NthDataToWrite) & 0x00FF;

// 					else if(Tag_Density == ISO15693_HIGH_DENSITY)
// 				 {
// 				 WriteBuffer [0] = 0x0A;	 
// 				 WriteBuffer [2] = NthDataToWrite & 0x00FF;
// 				 WriteBuffer [3] = (NthDataToWrite& 0xFF00) >> 8;
// 				 }

// 				/*Data To Write in the Block*/
// 				WriteBuffer [3+Tag_Density] = Data_To_Write[0+(IncBlock)];
// 				WriteBuffer [4+Tag_Density] = Data_To_Write[1+(IncBlock)];
// 				WriteBuffer [5+Tag_Density] = Data_To_Write[2+(IncBlock)];
// 				WriteBuffer [6+Tag_Density] = Data_To_Write[3+(IncBlock)];

// 				 memset (RepBuffer, 0x00,32);
// 				 if (Tag_Density == ISO15693_LOW_DENSITY)
// 				 PCD_SendRecv (0x07,WriteBuffer,RepBuffer);

// 				 else if (Tag_Density == ISO15693_HIGH_DENSITY)
// 				 PCD_SendRecv (0x08,WriteBuffer,RepBuffer);
				 	if ( ISO15693_WriteSingleBlock (Requestflags, 0x00,NthDataToWrite,&Data_To_Write[NthDataToWrite<<2],RepBuffer ) !=ISO15693_SUCCESSCODE)
						return ISO15693_ERRORCODE_DEFAULT;
				 
				 IncBlock += 4;
		}
					
	return ISO15693_SUCCESSCODE;				
}	

/**
  * @}
  */ 


/** @addtogroup lib_iso15693pcd_Public_Functions
 *  @{
 */ 


/**  
* @brief  this function selects 15693 protocol accoording to input parameters
* @retval ISO15693_SUCCESSCODE : the function is successful
* @retval ISO15693_ERRORCODE_DEFAULT : an error occured
*/
int8_t ISO15693_Init	( void )
{
	uint8_t 	ParametersByte=0,
					  pResponse[PROTOCOLSELECT_LENGTH];
	int8_t		status;
	u8 DemoGainParameters []  = {PCD_TYPEV_ARConfigA, PCD_TYPEV_ARConfigB}; 

	ParametersByte =  	((ISO15693_APPENDCRC << ISO15693_OFFSET_APPENDCRC ) 	&  ISO15693_MASK_APPENDCRC) |
											((ISO15693_SINGLE_SUBCARRIER << ISO15693_OFFSET_SUBCARRIER)	& ISO15693_MASK_SUBCARRIER)	|
											((ISO15693_MODULATION_100 << ISO15693_OFFSET_MODULATION) & ISO15693_MASK_MODULATION) |
											((ISO15693_WAIT_FOR_SOF <<  ISO15693_OFFSET_WAITORSOF ) & ISO15693_MASK_WAITORSOF) 	|
											((ISO15693_TRANSMISSION_26 <<   ISO15693_OFFSET_DATARATE  )	& ISO15693_MASK_DATARATE);
	
	errchk(PCD_ProtocolSelect(ISO15693_SELECTLENGTH,ISO15693_PROTOCOL,&(ParametersByte),pResponse));
	TechnoSelected = PCDPROTOCOL_15693;

	/* in order to adjust the demoduation gain of the PCD*/
  errchk(PCD_WriteRegister    ( 0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,DemoGainParameters,pResponse)); 

//	if (PCD_IsReaderResultCodeOk (PROTOCOL_SELECT,pResponse) == ISO15693_ERRORCODE_DEFAULT)
//		return ISO15693_ERRORCODE_DEFAULT;
	

	return ISO15693_SUCCESSCODE;
Error:
	return ISO15693_ERRORCODE_DEFAULT;
}


/**  
* @brief  this function return a tag UID 
* @param  	UIDout: 	UID of a tag in the field
* @retval status function
*/
int8_t ISO15693_GetUID (uint8_t *UIDout) 
{ 
int8_t 	FlagsByteData, status;
uint8_t	TagReply	[ISO15693_NBBYTE_UID+7];
uint8_t Tag_error_check;
	

	/* select 15693 protocol */
	errchk(ISO15693_Init	(	));

	FlagsByteData = ISO15693_CreateRequestFlag 	(	ISO15693_REQFLAG_SINGLESUBCARRIER,
													ISO15693_REQFLAG_HIGHDATARATE,
													ISO15693_REQFLAG_INVENTORYFLAGSET,
													ISO15693_REQFLAG_NOPROTOCOLEXTENSION,
													ISO15693_REQFLAG_NOTAFI,
													ISO15693_REQFLAG_1SLOT,
													ISO15693_REQFLAG_OPTIONFLAGNOTSET,
													ISO15693_REQFLAG_RFUNOTSET);
	
        status = ISO15693_Inventory (	FlagsByteData, 0x00, 0x00, 0x00, TagReply );
	errchk( status );

	Tag_error_check = TagReply[ISO15693_OFFSET_LENGTH]+1;
	if((TagReply[Tag_error_check] & ISO15693_CRC_MASK) != 0x00 )
		return ISO15693_ERRORCODE_DEFAULT;
	

	if (status == ISO15693_SUCCESSCODE)
		memcpy(UIDout,&(TagReply[ISO15693_OFFSET_UID]),ISO15693_NBBYTE_UID);

	devicemode = PCD;
	nfc_tagtype = TT5;
	return ISO15693_SUCCESSCODE; 
Error:
	return ISO15693_ERRORCODE_DEFAULT;	
}

/**  
* @brief  this function send an GetSystemInfo command and returns ISO15693_SUCCESSCODE if the command 
* @brief	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  	Flags		:  	inventory flags
* @param	UID			:  	Tag UID
* @retval ISO15693_SUCCESSCODE : the function is successful
* @retval ISO15693_ERRORCODE_DEFAULT : an error occured
*/
int8_t ISO15693_GetSystemInfo(const uint8_t Flags, const uint8_t *UIDin, uint8_t *pResponse)
{
uint8_t DataToSend[ISO15693_MAXLENGTH_GETSYSTEMINFO],
		NthByte=0;
int8_t	status;
			
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_GETSYSINFO;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	errchk(PCD_SendRecv(NthByte,DataToSend,pResponse));
//	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
//		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
Error:
	*pResponse = SENDRECV_ERRORCODE_SOFT;
	*(pResponse+1) = 0x00;
	return ISO15693_ERRORCODE_DEFAULT;
}

/**  
* @brief  this function splits inventory response. If the residue of tag response	is incorrect the function returns ERRORCODE_GENERIC, otherwise ISO15693_SUCCESSCODE
* @param  ReaderResponse	:  	pointer on PCD  response
* @param	Length		:	Number of byte of Reader Response
* @param  Flags		:  	Response flags
* @param	DSFIDextract: 	DSPID of tag response
* @param	UIDoutIndex		:  	index of UIDout of tag response
* @retval ISO15693_SUCCESSCODE : Contactless tag response validated			
* @retval CR95HF_ERROR_CODE : PCD  teturned an error code
* @retval ISO15693_ERRORCODE_CRCRESIDUE : CRC16 residue is erroneous
*/
int8_t ISO15693_SplitInventoryResponse(const uint8_t *ReaderResponse,const uint8_t Length,uint8_t *Flags , uint8_t *DSFIDextract, uint8_t *UIDoutIndex)
{
	int8_t status;
	
	uint8_t	ResultCode = ReaderResponse[PCD_DATA_OFFSET],
		NbTagReplyByte =ReaderResponse[PCD_LENGTH_OFFSET]-CONTROL_15693_NBBYTE,
		TagReplyIndex = PCD_DATA_OFFSET;
	
	/*  PCD returned an error code */
	if (ResultCode  != ISO15693_RESULTFLAG_STATUSOK)
		return ISO15693_ERRORCODE_DEFAULT;	
	
	errchk(ISO15693_IsCorrectCRC16Residue (&ReaderResponse[TagReplyIndex],NbTagReplyByte));
	
	*Flags = ReaderResponse[TagReplyIndex+ISO15693_OFFSET_FLAGS];
	*DSFIDextract =	ReaderResponse[TagReplyIndex+ISO15693_INVENTORYOFFSET_DSFID];

	*UIDoutIndex=TagReplyIndex+ISO15693_INVENTORYOFFSET_UID;
	
	return ISO15693_SUCCESSCODE;
Error:
	return ISO15693_ERRORCODE_CRCRESIDUE;
	
}


/**  
* @brief  	this function runs an anticollision sequence and returns the number of tag seen and their UID.
* @brief	The protocol select command has to be send first and the Flags parameterns shall be compliant
* @brief	with the parameters of the Protocol Select command.
* @param  	Flags		: 	request flags
* @param  	AFI			: 	AFI parameter (optional)
* @param  	NbTag		: 	Number of tag seen
* @param	pUIDout		: 	pointer on tag UID
* @retval 	ISO15693_SUCCESSCODE	: 	function succesful executed  
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	
*/
int8_t ISO15693_RunAntiCollision(const uint8_t Flags , const uint8_t AFI,uint8_t *NbTag,uint8_t *pUIDout)
{
	int8_t		status;
	uint8_t		MaskValue[ISO15693_NBBYTE_UID],
			MaskLength = 0,
			MaskStack [0x80*ISO15693_NBBYTE_UID];	//	can save 16 mask. the size of a mask is ISO15693_NBBYTE_UID.
												// the first byte is mask length (bit size). the orthers bytre are mask value
	uint8_t		RequestFlags = Flags,
			offset=0,
			ReplyFlag,
			NbTagInventoried,
			DSFIDout,
			NthStackValue=1,
			i=0,
			Nbloop = 0,
			StayQuietFlags = (Flags & ~ISO15693_MASK_INVENTORYFLAG) | ISO15693_MASK_ADDRORNBSLOTSFLAG,
			UIDoutOffet = PCD_DATA_OFFSET + 2 ;
	uint16_t	SlotOccupancy= 0x0000	;

	memset(MaskStack,0x00,16*ISO15693_NBBYTE_UID);
	*NbTag = 0;

	
	// checks if at least a tag is in the field 
	 ISO15693_InventoryOneSlot( 
								RequestFlags ,
								AFI,
								MaskLength,
								MaskValue,
								u95HFBuffer);
		

	// no tag in the field
	if (ISO15693_IsATagInTheField (u95HFBuffer) != ISO15693_SUCCESSCODE)
		return ISO15693_SUCCESSCODE;

	// check TagReply CRC residue and get tag UID
	status = ISO15693_SplitInventoryResponse(	u95HFBuffer,
												u95HFBuffer[PCD_LENGTH_OFFSET],
												&ReplyFlag ,
												&DSFIDout,
												&UIDoutOffet);
	
	
	
	// residue CRC ok (=> Only one tag in the field)
	if (status == ISO15693_SUCCESSCODE) 
	{
		*NbTag = 1;	
		// send a stay quiet command to the inventoried tag 
		ISO15693_StayQuiet(StayQuietFlags ,&(u95HFBuffer[UIDoutOffet]));
		// no collision => copy DSFID
		pUIDout[0] = DSFIDout ;
		// no collision => copy tag UID
		memcpy(&(pUIDout[1]),&(u95HFBuffer[UIDoutOffet]),ISO15693_NBBYTE_UID);
		return ISO15693_SUCCESSCODE;
	}

	delay(10);

	(*NbTag)=0;	

	// the folowing anticollsion sequence is preorder traversal algorithm 
	// the UIDs can be represnt as a binary tree
	do{

		memset(MaskValue,0x00,ISO15693_NBBYTE_UID);
		// unstack mask value
		NthStackValue -- ;
	   	memcpy(MaskValue,(MaskStack+NthStackValue*ISO15693_NBBYTE_UID+1),ISO15693_NBBYTE_UID);	


		// at this point : at least two tags are in the field
		// => run an inventory 16 slots

		// the u95HFBuffer is MAX_BUFFER_SIZE + 3 (=256 +3  bytes)
		// if a response is available in eack slot => 16 * 15 (max size of inventory reply + 3 control bytes)
		//						=>	16 * 15  = 240 bytes < MAX_BUFFER_SIZE + 3
		SlotOccupancy =ISO15693_Inventory16Slots( 
								RequestFlags ,
								AFI,
								MaskLength,
								MaskValue,
								&NbTagInventoried,
								u95HFBuffer);
		   	

					
		offset = 0;

		// loop on tag inventoried and copy the UID,
		for (i=0;i<NbTagInventoried;i++)
		{
			status = ISO15693_SplitInventoryResponse(&(u95HFBuffer[offset]),
										u95HFBuffer[PCD_LENGTH_OFFSET],
										&ReplyFlag ,
										&DSFIDout,
										&UIDoutOffet);
		
			if (status==ISO15693_SUCCESSCODE)
			{
				// send a stay quiet command to the inventoried tag 
				ISO15693_StayQuiet(StayQuietFlags ,&(u95HFBuffer[offset + UIDoutOffet]));
				// no collision => copy DSFID
				pUIDout[(*NbTag)*(ISO15693_NBBYTE_UID+1)] = DSFIDout ;
				// no collision => copy tag UID
				memcpy((pUIDout+(1+(*NbTag)*(ISO15693_NBBYTE_UID+1))),&(u95HFBuffer[offset + UIDoutOffet]),ISO15693_NBBYTE_UID);
				(*NbTag)++ ;
			}
			offset += u95HFBuffer[offset+PCD_LENGTH_OFFSET] + 2;
		} // for (i=0;i<NbTagInventoried;i++)
	
	   		
		// add 4 bits to the mask
		MaskLength +=4;
	 	// create and stack the masks for the next inventory commands
		for (i=0;i<16;i++)
		{
			/*
			A collision was detected in the inventory 16 slots
				=> add 4 bits to the mask
			*/
		
			if ((SlotOccupancy & (0x01 <<i)) != 0)
			{
				// create the mask which include the slot whare the collision occured
				MaskValue[(MaskLength-4)/8] |= (i << ((MaskLength-4)%8)) ;  
				// copy masklength and maskvalue into mask stack
				MaskStack[NthStackValue*ISO15693_NBBYTE_UID]= MaskLength;
				memcpy((MaskStack+(NthStackValue*ISO15693_NBBYTE_UID)+1),MaskValue,ISO15693_NBBYTE_UID);
				NthStackValue++;
			}
		}

		 			
		Nbloop ++;

	} while (NthStackValue >0 && Nbloop < 0x20);
	
	 return ISO15693_SUCCESSCODE;

}


/**  
* @brief  	this function runs an anticollision sequence and returns the number of tag seen and their UID.
* @brief	The protocol select command has to be send first and the Flags parameterns shall be compliant
* @brief	with the parameters of the Protocol Select command.
* @param  	Flags		: 	request flags
* @param  	AFI			: 	AFI parameter (optional)
* @param  	NbTag		: 	Number of tag seen
* @param	pUIDout		: 	pointer on tag UID
* @retval 	ISO15693_SUCCESSCODE	: 	function succesful executed  
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	
*/
int8_t ISO15693_RunInventory16slots(const uint8_t Flags , const uint8_t AFI,uint8_t *NbTag,uint8_t *pUIDout)
{
	int8_t		status;
	uint8_t		MaskValue[ISO15693_NBBYTE_UID],
			MaskLength = 0,
			MaskStack [0x40*ISO15693_NBBYTE_UID];	//	can save 64 masks. the size of a mask is ISO15693_NBBYTE_UID.
												// the first byte is mask length (bit size). the orthers bytre are mask value
	uint8_t		RequestFlags = Flags,
			offset=0,
			ReplyFlag,
			NbTagInventoried,
			DSFIDout,
 			i=0,
			UIDoutOffet = PCD_DATA_OFFSET + 2 ;


	memset(MaskStack,0x00,16*ISO15693_NBBYTE_UID);
	*NbTag = 0;


	ISO15693_Inventory16Slots( 
							RequestFlags ,
							AFI,
							MaskLength,
							MaskValue,
							&NbTagInventoried,
							u95HFBuffer);
	
   	
	if (NbTagInventoried == 0)
	{	*NbTag = 0;		
		return ISO15693_SUCCESSCODE;
	}

			
	offset = 0;
	// loop on tag inventoried and if there is no colission if the slot copy the UID,
	// otherwise get the slot where the  collision occured
	for (i=0;i<NbTagInventoried;i++)
	{
			
			status = ISO15693_SplitInventoryResponse(&(u95HFBuffer[offset]),
										u95HFBuffer[PCD_LENGTH_OFFSET],
										&ReplyFlag ,
										&DSFIDout,
										&UIDoutOffet);
		
			if (status==ISO15693_SUCCESSCODE)
			{
				// no collision => copy DSFID
				pUIDout[(*NbTag)*(ISO15693_NBBYTE_UID+1)] = DSFIDout ;
				// no collision => copy tag UID
				memcpy((pUIDout+(1+(*NbTag)*(ISO15693_NBBYTE_UID+1))),&(u95HFBuffer[UIDoutOffet+offset]),ISO15693_NBBYTE_UID);
				(*NbTag)++ ;
			}
//		}
		

		offset += u95HFBuffer[offset+PCD_LENGTH_OFFSET] + 2;
	} // for (i=0;i<NbTagInventoried;i++)


	 return ISO15693_SUCCESSCODE;

}

/**
* @brief  Find the TAG ISO15693 present in the field
* @param  *Length_Memory_TAG : return the memory size of the TAG
* @param  *Tag_Density : return 0 if the TAG is low density (< 16kbits), return 1 if the TAG is high density
* @param  *IC_Ref_TAG : return the IC_Ref of the TAG ( the 2 LSB bits are mask for the TAG LRiS2K, LRi2K and LRi1K)
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/
int8_t ISO15693_GetTagIdentification (uint16_t *Length_Memory_TAG, uint8_t *Tag_Density, uint8_t *IC_Ref_TAG)
{
	uint8_t RepBuffer[32],
				RequestFlags = 0x0A;

	uint8_t IC_Ref;
		
		/*Use ISO15693 Protocol*/
		ISO15693_Init();
		
		
		/*Send Get_System_Info with Protocol Extention Flag Set*/
		if ( ISO15693_GetSystemInfo ( RequestFlags, 0x00, RepBuffer) == ISO15693_SUCCESSCODE)
			IC_Ref = RepBuffer[17];
		else
		{
				RequestFlags = 0x02;
				if ( ISO15693_GetSystemInfo ( RequestFlags, 0x00, RepBuffer) == ISO15693_SUCCESSCODE)
					IC_Ref = RepBuffer[16];
				else 
					return ISO15693_ERRORCODE_NOTAGFOUND; 	
		}
		
		switch (IC_Ref)
		{
			case ISO15693_M24LR64R :
				*Length_Memory_TAG = ((((RepBuffer[15] <<8)& 0xFF00) | RepBuffer[14])+1);
			  *Tag_Density = ISO15693_HIGH_DENSITY;
				break;
				
			case ISO15693_M24LR64ER :
				*Length_Memory_TAG = ((((RepBuffer[15] <<8)& 0xFF00) | RepBuffer[14])+1);
				*Tag_Density = ISO15693_HIGH_DENSITY;
				break;
				
			case ISO15693_M24LR16ER :
				*Length_Memory_TAG = ((((RepBuffer[15] <<8)& 0xFF00) | RepBuffer[14])+1);
				*Tag_Density = ISO15693_HIGH_DENSITY;
				break;
				
			case ISO15693_M24LR04ER :
				*Length_Memory_TAG = (RepBuffer[14]+1);
				*Tag_Density = ISO15693_LOW_DENSITY;
				break;
				
			case ISO15693_LRiS64K :
				*Length_Memory_TAG = ((((RepBuffer[15] <<8)& 0xFF00) | RepBuffer[14])+1);
				*Tag_Density = ISO15693_HIGH_DENSITY;
				break;

			default :
				/*Flag IC_REF LSB For LRiXX*/
				IC_Ref &= 0xFC;
			
				switch (IC_Ref)
				{		
					case ISO15693_LRiS2K :
						*Length_Memory_TAG = (RepBuffer[14]+1);
						*Tag_Density = ISO15693_LOW_DENSITY;
					break;
				
					case ISO15693_LRi2K :
						*Length_Memory_TAG = (RepBuffer[14]+1);
						*Tag_Density = ISO15693_LOW_DENSITY;
					break;
				
					case ISO15693_LRi1K :
						*Length_Memory_TAG = (RepBuffer[14]+1);
						*Tag_Density = ISO15693_LOW_DENSITY;
					break;
			
					default :
						return ISO15693_ERRORCODE_DEFAULT;
				}
		}
		
		*IC_Ref_TAG = IC_Ref;
		return ISO15693_SUCCESSCODE;
}


/**
* @brief  Read data by Bytes in the TAG (with read multiple if it is possible)
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  IC_Ref_Tag : The IC_Ref is use to use read multiple or read single
* @param  *Data_To_Read : return the data read in the TAG
* @param  NbBytes_To_Read : Number of Bytes to read in the TAG
* @param  FirstBytes_To_Read : First Bytes to read in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
uint8_t ISO15693_ReadBytesTagData(uint8_t Tag_Density, uint8_t IC_Ref_Tag, uint8_t *Data_To_Read, uint16_t NbBytes_To_Read, uint16_t FirstBytes_To_Read)
{
	uint8_t status = ISO15693_ERRORCODE_DEFAULT;
/*0x80 => Page size read multiple 0x20 & 1 block = 4 bytes*/
	uint16_t NbBlock_To_Read = NbBytes_To_Read/4;
	uint16_t FirstBlock_To_Read = FirstBytes_To_Read/4;
	
	/*LRiS2K don't support read multiple*/
	if(IC_Ref_Tag  == ISO15693_LRiS2K)
	{	
		 NbBlock_To_Read = NbBytes_To_Read/4;		
		 status = ISO15693_ReadSingleTagData(Tag_Density, Data_To_Read, NbBlock_To_Read, FirstBlock_To_Read);
	}
	
	else
		status = ISO15693_ReadMultipleTagData(Tag_Density, Data_To_Read, NbBlock_To_Read, FirstBlock_To_Read);		
	
	//memcpy(Data_To_Read,&Data_To_Read[FirstBytes_To_Read], NbBytes_To_Read);
	
	return status;
	
}

/**
* @brief  Write data by bytes in the TAG
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Write : Data to write in the TAG
* @param  NbBytes_To_Write : Number of Bytes to write in the TAG
* @param  FirstBytes_To_Write : First Bytes to write in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	 
uint8_t ISO15693_WriteBytes_TagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBytes_To_Write, uint16_t FirstBytes_To_Write)
{
	/*1 block = 4 bytes*/
	uint16_t NbBlock_To_Write;

	/*Convert in Blocks the number of bytes to write*/
	uint16_t FirstBlock_To_Write = FirstBytes_To_Write/4;

	uint8_t Length_Low_Limit =0;
	uint8_t Length_High_Limit = 0;
	uint8_t Data_To_Save[8];
	
	NbBlock_To_Write = (NbBytes_To_Write/4)+1;
	
	/*Save the data of the uncomplete block to write*/
	ISO15693_TagSave(Tag_Density, NbBytes_To_Write, FirstBytes_To_Write, Data_To_Save, &Length_Low_Limit, &Length_High_Limit);
	
	
	memcpy(&Data_To_Write[-Length_Low_Limit],&Data_To_Save[0], Length_Low_Limit);
	memcpy(&Data_To_Write[NbBytes_To_Write],&Data_To_Save[4], Length_High_Limit);
	
	if (ISO15693_WriteTagData(Tag_Density, &Data_To_Write[-Length_Low_Limit], NbBlock_To_Write, FirstBlock_To_Write) != ISO15693_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;	
}

#ifdef ISO15693_ALLCOMMANDS 


/**  
* @brief  	this function send an LockSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	RESULTOK	: 	PCD  returns a succesful code
* @retval 	ERRORCODE_GENERIC	: 	 PCD  returns an error code
*/
int8_t ISO15693_LockSingleBlock(const uint8_t Flags, const uint8_t *UIDin, const uint8_t BlockNumber,uint8_t *pResponse)
{
	uint8_t DataToSend[ISO15693_MAXLENGTH_LOCKSINGLEBLOCK],
		NthByte=0;
	
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_LOCKBLOCK;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== TRUE)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	DataToSend[NthByte++] = BlockNumber;

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;

}

/**  
* @brief  	this function send an Lock AFI command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	pResponse	: 	pointer on PCD  response
* @retval 	RESULTOK	: 	PCD  returns a succesful code
* @retval 	ERRORCODE_GENERIC	: 	 PCD  returns an error code
*/
int8_t ISO15693_LockAFI(const uint8_t Flags, const uint8_t *UIDin,uint8_t *pResponse )
{
	uint8_t DataToSend[ISO15693_MAXLENGTH_LOCKAFI],
				NthByte=0;

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_LOCKAFI;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== TRUE)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;

}

/**  
* @brief  this function send an Lock DSFID command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	pResponse	: 	pointer on PCD  response
* @retval 	RESULTOK	: 	PCD  returns a succesful code
* @retval 	ERRORCODE_GENERIC	: 	 PCD  returns an error code
*/
int8_t ISO15693_LockDSFID(const uint8_t Flags, const uint8_t *UIDin,uint8_t *pResponse)
{
	uint8_t 	DataToSend[ISO15693_MAXLENGTH_LOCKDSFID],
					NthByte=0;

	errchk (ISO15693_IsReaderConfigMatchWithFlag (GloParameterSelected,Flags));
			
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_LOCKDSFID;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== TRUE)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}
	
	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
}



/**  
* @brief  	this function send a Get Multiple Block Secutity Status command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	NbBlocks	: 	Number of blocks
* @param	AppendCRC	:  	CRC16 management. If set PCD  appends CRC16. 
* @param	CRC16		: 	pointer on CRC16 (optional) in case of user has choosed to manage CRC16 (see ProtocolSelect command PCD  layer)
* @param	pResponse	: 	pointer on PCD  response
* @retval 	RESULTOK	: 	PCD  returns a succesful code
* @retval 	ERRORCODE_GENERIC	: 	 PCD  returns an error code
*/
int8_t ISO15693_GetMultipleBlockSecutityStatus(const uint8_t Flags, const uint8_t *UIDin, const uint8_t BlockNumber, const uint8_t NbBlocks,uint8_t *pResponse)
{
	uint8_t DataToSend[ISO15693_MAXLENGTH_GETMULSECURITY],
					NthByte=0;
	int8_t	status;

	errchk (ISO15693_IsReaderConfigMatchWithFlag (GloParameterSelected,Flags));

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_GETSECURITYINFO;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== RESULTOK)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	DataToSend[NthByte++] = BlockNumber;
	DataToSend[NthByte++] = NbBlocks;


	errchk(CR95HF_SendRecv(NthByte,DataToSend,pResponse));

	if (CR95HF_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ERRORCODE_GENERIC)
		return ERRORCODE_GENERIC;

	return RESULTOK;
Error:
	*pResponse = SENDRECV_ERRORCODE_SOFT;
	*(pResponse+1) = 0x00;
	return ERRORCODE_GENERIC;
}



#endif /*ISO15693_ALLCOMMANDS*/


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
