/**
  ******************************************************************************
  * @file    lib_iso14443Apcd.c
  * @author  MMY Application Team
  * @version $Revision: 1334 $
  * @date    $Date: 2015-11-05 10:53:37 +0100 (Thu, 05 Nov 2015) $
  * @brief   Manage the iso14443A communication
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
#include "lib_iso14443Apcd.h"

extern PCD_PROTOCOL TechnoSelected;
extern IC_VERSION IcVers;

/* --------------------------------------------------
 * code templates for ISO14443A protocol
 * command =  Command code | Length | data(Le)
 * -------------------------------------------------- */
#define PCD_TYPEA_TIMERW                0x5A

/******************  PCD  ******************/
/* ISO14443A */
#define PCD_TYPEA_ARConfigA	        0x01
#define PCD_TYPEA_ARConfigB	        0xDF

static const uint8_t TOPAZ[] = {SEND_RECEIVE, 0x08, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8};

static u8 MultiID[200] = {0};
static u8 MultiIDPart2[64] = {0};
static u8 RemainingID = 0;


uint16_t FSC = 32;
uint16_t FWI = 4;  /* Default value */


ISO14443A_CARD 	ISO14443A_Card;
extern uint8_t	u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

uint8_t u95HFBufferAntiCol [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

/* Variables for the different modes */
extern DeviceMode_t devicemode;
extern TagType_t nfc_tagtype;

static void ISO14443A_InitStructure( void );
static uint16_t FSCIToFSC(uint8_t FSCI);
static int8_t ISO14443A_REQA( uint8_t *pDataRead );
static int8_t ISO14443A_HLTA( uint8_t *pDataRead );
static int8_t ISO14443A_RATS( uint8_t *pDataRead );
#if 0
static int8_t ISO14443A_PPS( uint8_t *pDataRead );
#endif
static int8_t ISO14443A_AC( uint8_t *pDataRead, u8 CascadeLevel );
static int8_t ISO14443A_ACLevel1 ( uint8_t *pDataRead );
static int8_t ISO14443A_ACLevel2 ( uint8_t *pDataRead );
static int8_t ISO14443A_ACLevel3 ( uint8_t *pDataRead );
static void ISO14443A_CompleteStructure ( uint8_t *pATQA );
static int8_t ISO14443A_MultiAnticollision( void );


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


 /** @addtogroup ISO14443A_pcd
 * 	@{
 *	@brief  This part of the library is used to follow ISO14443A.
 */


/** @addtogroup lib_iso14443Apcd_Private_Functions
 *  @{
 */


/**
 * @brief  Reset the ISO14443A data structure
 * @param  void
 * @return void
 */
static void ISO14443A_InitStructure( void )
{
	/* Initializes the data structure used to store results */
	memset(ISO14443A_Card.ATQA, 0x00, ISO14443A_ATQA_SIZE);
	memset(ISO14443A_Card.UID , 0x00, ISO14443A_MAX_UID_SIZE);
	ISO14443A_Card.CascadeLevel 	= 0;
	ISO14443A_Card.UIDsize 			= 0;		
	ISO14443A_Card.ATSSupported 	= false;
	ISO14443A_Card.IsDetected   	= false;

}

/**
 * @brief  this functions convert FSCI to FSC
 * @param  FSCI	: FSCI value
 * @return FSC
 */
static uint16_t FSCIToFSC(uint8_t FSCI)
{
	if (FSCI == 0) return 16;
	else if (FSCI == 1) return 24;
	else if (FSCI == 2) return 32;
	else if (FSCI == 3) return 40;
	else if (FSCI == 4) return 48;
	else if (FSCI == 5) return 64;
	else if (FSCI == 6) return 96;
	else if (FSCI == 7) return 128;
	else return 256;
}


/**
 * @brief  this functions sends the REQA command to the PCD device
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_REQA( uint8_t *pDataRead )
{
	const uint8_t ReqA[] = { 0x26, 0x07};
	int8_t	status;

	/* sends the command to the PCD device*/
	errchk(PCD_SendRecv(0x02,ReqA,pDataRead));

	/* retrieves the ATQA response */
	memcpy(ISO14443A_Card.ATQA, &pDataRead[PCD_DATA_OFFSET], ISO14443A_ATQA_SIZE);
	/* completes the strucure according to the ATQA response */
	ISO14443A_CompleteStructure ( ISO14443A_Card.ATQA );
		
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}


/**
 * @brief  This functions manage the anticollision
 * @param  *pDataRead	: Pointer to the PCD response
 * @param  CascadeLevel	: information on the current cascade level
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_AC( uint8_t *pDataRead, u8 CascadeLevel )
{
	u8 AnticolParameter [7] = {0x00, ISO14443A_NVM_20, 0x08,0x00,0x00,0x00,0x00};
	u8 NbResponseByte = 0;
	u8 NbResponseByteOrigin = 0;
	u8 Collision = 0;
	u8 ByteCollisionIndex = 0;
	u8 BitCollisionIndex = 0;

	u8 RemainingBit = 0;
	u8 NewByteCollisionIndex = 0;
	u8 NewBitCollisionIndex = 0;
	u8 UID[4] = {0x00,0x00,0x00,0x00};
	int8_t status;

	/* prepare command regarding cascade level on-going */
	AnticolParameter[0] = CascadeLevel;

	/* sends the command to the PCD device*/
	errchk(PCD_SendRecv(0x03,AnticolParameter,pDataRead));
	
	NbResponseByte = pDataRead[1]; 
	NbResponseByteOrigin = NbResponseByte;
	Collision = (pDataRead[NbResponseByte-1] & 0x80);

	ByteCollisionIndex = pDataRead[NbResponseByte];
	BitCollisionIndex = pDataRead[NbResponseByte+1];
	
	/* case that should not happend, as occurs because we have miss another collision */
	if( BitCollisionIndex == 8)
	{
		return ISO14443A_ERRORCODE_DEFAULT;
	}
			
	/* check for collision (Tag of different UID length at the same time not managed so far) */
	while( Collision == 0x80)
	{		
		/* clear collision detection */
		Collision = 0x00;
		
		/* send the command to the PCD device*/
		AnticolParameter[1] = ISO14443A_NVM_20 + ((ByteCollisionIndex) <<4) + (BitCollisionIndex+1);
		if( ByteCollisionIndex == 0)
		{	
			AnticolParameter[2] = pDataRead[2] & ((u8)(~(0xFF<<(BitCollisionIndex+1)))); /* ISO said it's better to put collision bit to value 1 */
			AnticolParameter[3] = (BitCollisionIndex+1) | 0x40; /* add split frame bit */
			UID [0] = AnticolParameter[2];
		}
		else if( ByteCollisionIndex == 1)
		{
			AnticolParameter[2] = pDataRead[2];
			AnticolParameter[3] = pDataRead[3] & ((u8)(~(0xFF<<(BitCollisionIndex+1)))); /* ISO said it's better to put collision bit to value 1 */			
			AnticolParameter[4] = (BitCollisionIndex+1) | 0x40; /* add split frame bit */
			UID [0] = AnticolParameter[2];
			UID [1] = AnticolParameter[3];
		}
		else if( ByteCollisionIndex == 2)
		{
			AnticolParameter[2] = pDataRead[2];
			AnticolParameter[3] = pDataRead[3];
			AnticolParameter[4] = pDataRead[4] & ((u8)(~(0xFF<<(BitCollisionIndex+1)))); /* ISO said it's better to put collision bit to value 1 */			
			AnticolParameter[5] = (BitCollisionIndex+1) | 0x40; /* add split frame bit */;
			UID [0] = AnticolParameter[2];
			UID [1] = AnticolParameter[3];
			UID [2] = AnticolParameter[4];
		}
		else if( ByteCollisionIndex == 3)
		{
			AnticolParameter[2] = pDataRead[2];
			AnticolParameter[3] = pDataRead[3];
			AnticolParameter[4] = pDataRead[4];
			AnticolParameter[5] = pDataRead[5] & ((u8)(~(0xFF<<(BitCollisionIndex+1)))); /* ISO said it's better to put collision bit to value 1 */			
			AnticolParameter[6] = (BitCollisionIndex+1) | 0x40; /* add split frame bit */;
			UID [0] = AnticolParameter[2];
			UID [1] = AnticolParameter[3];
			UID [2] = AnticolParameter[4];
			UID [3] = AnticolParameter[5];
		}
		else
			return ISO14443A_ERRORCODE_DEFAULT;
				
		/* send part of the UID */
		PCD_SendRecv((0x03+ByteCollisionIndex+1),AnticolParameter,pDataRead);
		
		if(pDataRead[0] != 0x80)
				return ISO14443A_ERRORCODE_DEFAULT;
		
		/* check if there is another collision to take into account*/
		NbResponseByte = pDataRead[1]; 
		Collision = (pDataRead[NbResponseByte-1]) & 0x80;
		
		if ( Collision == 0x80)
		{
			NewByteCollisionIndex = pDataRead[NbResponseByte];
			NewBitCollisionIndex = pDataRead[NbResponseByte+1];
		}
					
		/* we can check that non-alignement is the one expected */
		RemainingBit = 8 - (0x0F & (pDataRead[2+(NbResponseByte-2)-1]));
		if( RemainingBit == BitCollisionIndex+1)
		{		
			/* recreate the good UID */
			if( ByteCollisionIndex == 0)
			{
				UID [0] = ((~(0xFF << (BitCollisionIndex+1))) & AnticolParameter[2]) | pDataRead[2] ;
				UID [1] = pDataRead[3];
				UID [2] = pDataRead[4];
				UID [3] = pDataRead[5];
			}
			else if( ByteCollisionIndex == 1)
			{
				UID [1] = ((~(0xFF << (BitCollisionIndex+1))) & AnticolParameter[3]) | pDataRead[2] ;
				UID [2] = pDataRead[3];
				UID [3] = pDataRead[4];
			}
			else if( ByteCollisionIndex == 2)
			{
				UID [2] = ((~(0xFF << (BitCollisionIndex+1))) & AnticolParameter[4]) | pDataRead[2] ;
				UID [3] = pDataRead[3];
			}
			else if( ByteCollisionIndex == 3)
			{
				UID [3] = ((~(0xFF << (BitCollisionIndex+1))) & AnticolParameter[5]) | pDataRead[2] ;
			}
			else
				return ISO14443A_ERRORCODE_DEFAULT;
		}				
		else
			return ISO14443A_ERRORCODE_DEFAULT;		
		
			/* prepare the buffer expected by the caller */
			pDataRead[0] = 0x80;
			pDataRead[1] = NbResponseByteOrigin;
			pDataRead[2] = UID [0];
			pDataRead[3] = UID [1];
			pDataRead[4] = UID [2];
			pDataRead[5] = UID [3];	
			pDataRead[6] = UID[0]^UID[1]^UID[2]^UID[3];	
		
		/* if collision was detected restart anticol */
		if ( Collision == 0x80)
		{
			if( ByteCollisionIndex != NewByteCollisionIndex )
			{
				ByteCollisionIndex += NewByteCollisionIndex;
				BitCollisionIndex = NewBitCollisionIndex;	
			}
			else
			{
				ByteCollisionIndex += NewByteCollisionIndex;
				BitCollisionIndex += (NewBitCollisionIndex+1);				
			}
		}
	}
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
	
}

/**
 * @brief  this function carries out the first level of the anticollision
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_ACLevel1( uint8_t *pDataRead )
{
	uint8_t *pDataToSend = &(u95HFBuffer[PCD_DATA_OFFSET]),
					Length = 0,
					BccByte ;
	int8_t status;

	/* Perform anti-collision */
	errchk(ISO14443A_AC(pDataRead, SEL_CASCADE_LVL_1));
			
	/* Saves the UID in the structure */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_SINGLE_SIZE)
	{
		memcpy( ISO14443A_Card.UID, &pDataRead[PCD_DATA_OFFSET], ISO14443A_UID_SINGLE_SIZE );
	}
	else
	{	
		memcpy(ISO14443A_Card.UID, &pDataRead[PCD_DATA_OFFSET+1], ISO14443A_UID_PART);
	}
	
	/* copies the BCC byte of the card response*/
	BccByte = pDataRead[PCD_DATA_OFFSET + ISO14443A_UID_SINGLE_SIZE ];
	
	/* Preparing the buffer who contains the SELECT command */
	pDataToSend[Length ++]	= SEL_CASCADE_LVL_1;
	pDataToSend[Length ++] = ISO14443A_NVM_70;
	
	/* Inserts the previous reply in the next command */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_SINGLE_SIZE)
	{
		memcpy(&pDataToSend[Length], ISO14443A_Card.UID, ISO14443A_UID_SINGLE_SIZE );
		Length += ISO14443A_UID_SINGLE_SIZE ;
	}
	else
	{
		pDataToSend[Length ++] = 0x88;
		memcpy(&pDataToSend[Length], ISO14443A_Card.UID, ISO14443A_UID_PART );
		Length += ISO14443A_UID_PART ;
	}	
	pDataToSend[Length ++] = BccByte;
	/* Add the control byte : Append the CRC + 8 bits in first byte (standard frame)*/
	pDataToSend[Length ++] = PCD_ISO14443A_APPENDCRC | PCD_ISO14443A_A8BITSINFIRSTBYTE;
	
	errchk(PCD_SendRecv(Length,pDataToSend,pDataRead));

	/* Recovering SAK byte */
	ISO14443A_Card.SAK = pDataRead[PCD_DATA_OFFSET];

	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}

/**
 * @brief  this function carries out the second level of the anticollision
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_ACLevel2( uint8_t *pDataRead )
{
	uint8_t *pDataToSend = &(u95HFBuffer[PCD_DATA_OFFSET]),
					Length = 0,
					BccByte ;
	int8_t status;

	/* Perform anti-collision */
	errchk(ISO14443A_AC(pDataRead, SEL_CASCADE_LVL_2));
	//errchk(PCD_SendRecv(0x03,AnnticolParameter,pDataRead));
	
	/* Copies the UID into the data structure */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_DOUBLE_SIZE)
	{
		memcpy(&ISO14443A_Card.UID[ISO14443A_UID_PART], &pDataRead[PCD_DATA_OFFSET], ISO14443A_UID_SINGLE_SIZE);
	}
	else
	{
		memcpy(&ISO14443A_Card.UID[ISO14443A_UID_PART], &pDataRead[PCD_DATA_OFFSET+1], ISO14443A_UID_PART);
	}
	
	/* copies the BCC byte of the card response*/
	BccByte = pDataRead[PCD_DATA_OFFSET + ISO14443A_UID_SINGLE_SIZE ];
	
	/* Preparing the buffer who contains the SELECT command */
	pDataToSend[Length ++]	= SEL_CASCADE_LVL_2;
	pDataToSend[Length ++] = ISO14443A_NVM_70;
	
	/* Copies the UID into the data structure */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_DOUBLE_SIZE)
	{
		memcpy(&(pDataToSend[Length]),&(ISO14443A_Card.UID[ISO14443A_UID_PART]) , ISO14443A_UID_SINGLE_SIZE);
		Length += ISO14443A_UID_SINGLE_SIZE ;
	}
	else
	{
		pDataToSend[Length ++] = 0x88;
		memcpy(&(pDataToSend[Length]),&(ISO14443A_Card.UID[ISO14443A_UID_PART]) , ISO14443A_UID_PART);
		Length += ISO14443A_UID_PART ;
	}
	pDataToSend[Length ++] = BccByte;
	/* Add the control byte : Append the CRC + 8 bits in first byte (standard frame)*/
	pDataToSend[Length ++] = PCD_ISO14443A_APPENDCRC | PCD_ISO14443A_A8BITSINFIRSTBYTE;
	
	/* emit the select command */
	errchk(PCD_SendRecv(Length,pDataToSend,pDataRead));
	
	/* Recovering SAK byte */
	ISO14443A_Card.SAK = pDataRead[PCD_DATA_OFFSET];

	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}


/**
 * @brief  this function carries out the second level of the anticollision
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_ACLevel3 ( uint8_t *pDataRead )
{
	uint8_t *pDataToSend = &(u95HFBuffer[PCD_DATA_OFFSET]),
					Length = 0,
					BccByte ;
	int8_t status;

	/* Perform anti-collision */
	errchk(ISO14443A_AC(pDataRead, SEL_CASCADE_LVL_3));

	/* Copies the UID into the data structure */
	memcpy(&ISO14443A_Card.UID[ISO14443A_UID_PART], &pDataRead[PCD_DATA_OFFSET], ISO14443A_UID_SINGLE_SIZE);
	/* copies the BCC byte of the card response*/
	BccByte = pDataRead[PCD_DATA_OFFSET + ISO14443A_UID_SINGLE_SIZE ];
	
	/* Preparing the buffer who contains the SELECT command */
	pDataToSend[Length ++]	= SEL_CASCADE_LVL_3;
	pDataToSend[Length ++] = ISO14443A_NVM_70;
	
	/* Copies the UID into the data structure */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_TRIPLE_SIZE)
	{
		memcpy(&(pDataToSend[Length]),&(ISO14443A_Card.UID[ISO14443A_UID_PART]) , ISO14443A_UID_SINGLE_SIZE);
		Length += ISO14443A_UID_SINGLE_SIZE ;
	}

	pDataToSend[Length ++] = BccByte;
	/* Add the control byte : Append the CRC + 8 bits in first byte (standard frame)*/
	pDataToSend[Length ++] = PCD_ISO14443A_APPENDCRC | PCD_ISO14443A_A8BITSINFIRSTBYTE;
	
	/* emitthe select command */
	errchk(PCD_SendRecv(Length,pDataToSend,pDataRead));
	
	/* Recovering SAK byte */
	ISO14443A_Card.SAK = pDataRead[PCD_DATA_OFFSET];

	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}

/**
 * @brief  this functions sends the HLTA command to the PCD device
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_HLTA( uint8_t *pDataRead )
{
	const uint8_t pdata[]= { 0x50, 0x00, 0x28};

	/* send the command to the PCD device*/
	PCD_SendRecv(0x03,pdata,pDataRead);
	
	return ISO14443A_SUCCESSCODE;

}


/**
 * @brief  this functions emits the RATS command to PICC device
 * @param  *pDataRead	: Pointer to the response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_RATS( uint8_t *pDataRead )
{
	int8_t status;	
	uint8_t FSCI;
	const uint8_t pdata[]= { 0xE0, 0x80, 0x28};

	/* send the command to the PCD device*/
	errchk(PCD_SendRecv(0x03,pdata,pDataRead));
	/* check the status byte of the PCD device */
	errchk(PCD_IsReaderResultCodeOk (SEND_RECEIVE,pDataRead) );
	/* check the CRC */
	errchk(PCD_IsCRCOk (PCD_PROTOCOL_ISO14443A,pDataRead) );

	FSCI = pDataRead[3]&0x0F;
	FSC = FSCIToFSC(FSCI);
	
	/* Check if FWI is present */
	if( (pDataRead[3] & 0x20) == 0x20)
	{
		if( (pDataRead[3] & 0x10) == 0x10)
			FWI = (pDataRead[5]&0xF0)>>4;
		else
			FWI = (pDataRead[4]&0xF0)>>4;	
	}
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}

#if 0
/**
 * @brief Handles the WAKE_UP command
 * @param *pDataRead	: Pointer to the response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_PPS( uint8_t *pDataRead )
{
	const uint8_t pdata[]= { 0xD0, 0x11, 0x00, 0x28};
	int8_t status;

	/* sends the command to the PCD device*/
	errchk(PCD_SendRecv(0x04,pdata,pDataRead));

	/* checks the CRC */
	errchk(PCD_IsCRCOk (PCD_PROTOCOL_ISO14443A,pDataRead) );
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}
#endif

/**
 * @brief  this function completes the ISO144443 type A structure according to the ATQA response
 */
static void ISO14443A_CompleteStructure ( uint8_t *pATQA )
{

	/* according to FSP ISO 11443-3 the b7&b8 bits of ATQA tag answer is UID size bit frame */
	/* Recovering the UID size */
	switch ((ISO14443A_Card.ATQA[0] & ISO14443A_UID_MASK)>>6)
	{
			case ATQ_FLAG_UID_SINGLE_SIZE:
				ISO14443A_Card.UIDsize 			= ISO14443A_UID_SINGLE_SIZE;
				ISO14443A_Card.CascadeLevel 	= CASCADE_LVL_1;
			break;
			case ATQ_FLAG_UID_DOUBLE_SIZE:
				ISO14443A_Card.UIDsize 			= ISO14443A_UID_DOUBLE_SIZE;
				ISO14443A_Card.CascadeLevel 	= CASCADE_LVL_2;
			break;
			case ATQ_FLAG_UID_TRIPLE_SIZE:
				ISO14443A_Card.UIDsize 			= ISO14443A_UID_TRIPLE_SIZE;
				ISO14443A_Card.CascadeLevel 	= CASCADE_LVL_3;
			break;
	}
	
}


/**
 * @brief  Checks if a card is in the field
 * @param  None
 * @return ISO14443A_SUCCESSCODE (Anticollision done) / ISO14443A_ERRORCODE_DEFAULT (Communication issue)
 */
static int8_t ISO14443A_MultiAnticollision( void )
{
	uint8_t *pDataRead = u95HFBufferAntiCol;
	int8_t 	status;

	
	/* Checks if an error occured and execute the Anti-collision level 1*/
	errchk(ISO14443A_ACLevel1(pDataRead) );

	/* UID Complete ? */
	if(ISO14443A_Card.SAK & SAK_FLAG_UID_NOT_COMPLETE)
	{
		/* Checks if an error occured and execute the Anti-collision level 2*/
		errchk(ISO14443A_ACLevel2(pDataRead) );
	}
	/* UID Complete ? */
	if(ISO14443A_Card.SAK & SAK_FLAG_UID_NOT_COMPLETE)
	{
		/* Checks if an error occured and execute the Anti-collision level 3*/
		errchk(ISO14443A_ACLevel3(pDataRead) );
	}

	/* Quiet the founded tag */
	/* Send a HALT command */
	errchk(ISO14443A_HLTA(pDataRead) );
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 

}

/**
  * @}
  */ 


/** @addtogroup lib_iso14443Apcd_Public_Functions
 *  @{
 */


/**
 * @brief  this command initializes the PCD device for the IS014443A protocol
 * @param  none
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
int8_t ISO14443A_Init(void)
{
	int8_t 	status;
	
	ISO14443A_InitStructure( );

	/* sends a protocol Select command to the pcd to configure it */
	errchk(ISO14443A_ConfigFDTforAnticollision());
	
	TechnoSelected = PCDPROTOCOL_14443A;
	
	/* GT min time to respect before sending REQ_A */
	delay(5);
 
 return ISO14443A_SUCCESSCODE;
Error:
 return ISO14443A_ERRORCODE_DEFAULT;
	
}


/**
 * @brief  Initializes the PCD device for the IS014443A protocol
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
int8_t TOPAZ_ID( uint8_t *pDataRead)
{
	uint8_t Tag_error_check;

	if(PCD_CheckSendReceive(TOPAZ, pDataRead) != ISO14443A_SUCCESSCODE)
		return ISO14443A_ERRORCODE_DEFAULT;

	Tag_error_check = pDataRead[RFTRANS_95HF_LENGTH_OFFSET]-1;
	if((pDataRead[Tag_error_check] & ISO14443A_CRCMASK)	 == ISO14443A_CRC_ERRORCODE_TYPEA)
		return ISO14443A_ERRORCODE_CRC;

	if(pDataRead[0] == SENDRECV_RESULTSCODE_OK && pDataRead[1] == 0x04)
		return ISO14443A_ERRORCODE_DEFAULT;
		
	devicemode = PCD;
	if (pDataRead[0] == 0x80)
		nfc_tagtype = TT1;
	else
		return ISO14443A_ERRORCODE_DEFAULT;
	   	
	return ISO14443A_SUCCESSCODE;
}


/**
 * @brief  Checks if a ISO14443A card is in the field
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
int8_t ISO14443A_IsPresent( void )
{
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;
	
	errchk(ISO14443A_REQA(pDataRead));
	
	/* checks the status byte of the PCD device */
	errchk(PCD_IsReaderResultCodeOk (SEND_RECEIVE,pDataRead) );

	ISO14443A_Card.IsDetected = true;

	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}



/**
 * @brief  Checks if a card is in the field
 * @param  None
 * @return ISO14443A_SUCCESSCODE (Anticollision done) / ISO14443A_ERRORCODE_DEFAULT (Communication issue)
 */
int8_t ISO14443A_Anticollision( void )
{
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;	
	
	/* Checks if an error occured and execute the Anti-collision level 1*/
	errchk(ISO14443A_ACLevel1(pDataRead) );

	/* UID Complete ? */
	if(ISO14443A_Card.SAK & SAK_FLAG_UID_NOT_COMPLETE)
	{
		/* Checks if an error occured and execute the Anti-collision level 2*/
		errchk(ISO14443A_ACLevel2(pDataRead) );
	}
	/* UID Complete ? */
	if(ISO14443A_Card.SAK & SAK_FLAG_UID_NOT_COMPLETE)
	{
		/* Checks if an error occured and execute the Anti-collision level 2*/
		errchk(ISO14443A_ACLevel3(pDataRead) );
	}

	/* Checks if the RATS command is supported by the card */
	if(ISO14443A_Card.SAK & SAK_FLAG_ATS_SUPPORTED)
	{
		ISO14443A_ConfigFDTforRATS();
		
		ISO14443A_Card.ATSSupported = true;
		errchk(ISO14443A_RATS(pDataRead) );
	}
	
	/* Change the FDT to accept APDU */	
	ISO14443A_ConfigFDT(1);
	
	devicemode = PCD;
	/* Check the Tag type found */
	if ((ISO14443A_Card.SAK&0x60) == 0x00) /* TT2 */
		nfc_tagtype = TT2;
	else if( (ISO14443A_Card.SAK & 0x20) == 0x20) 
		nfc_tagtype = TT4A;	
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 

}

int8_t ISO14443A_ConfigFDTforAnticollision( void)
{
	u8 ProtocolSelectParameters [6]  = {0x00, 0x00, 0x00, 0x00, 0x02, 0x02}; /* last 2 bytes since QJE version */
	u8 WriteRegisterParameters [2]  = {PCD_TYPEA_TIMERW, TIMER_WINDOW_UPDATE_CONFIRM_CMD};
  u8 DemoGainParameters [2]  = {PCD_TYPEA_ARConfigA, PCD_TYPEA_ARConfigB};
	u8 NbParam = 0;
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;	
	
	if( IcVers >= QJE)
		NbParam = 6;
	else
		NbParam = 4;
	
	errchk(PCD_ProtocolSelect((NbParam+1),PCD_PROTOCOL_ISO14443A,ProtocolSelectParameters,pDataRead));
	errchk(PCD_WriteRegister    ( 0x04,TIMER_WINDOW_REG_ADD,0x00,WriteRegisterParameters,pDataRead));
	/* in order to adjust the demoduation gain of the PCD which is reseted at each protocol select */
	errchk(PCD_WriteRegister (0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,DemoGainParameters,u95HFBuffer));
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}

int8_t ISO14443A_ConfigFDTforRATS( void)
{
	u8 ProtocolSelectParameters [6]  = {0x00, 0x00, 0x00, 0x00, 0x03, 0x03}; /* last 2 bytes since QJE version */
	u8 WriteRegisterParameters [2]  = {PCD_TYPEA_TIMERW, TIMER_WINDOW_UPDATE_CONFIRM_CMD};
  u8 DemoGainParameters [2]  = {PCD_TYPEA_ARConfigA, PCD_TYPEA_ARConfigB};
	u8 NbParam = 0;
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;	
	
	if( IcVers >= QJE)
		NbParam = 6;
	else
		NbParam = 4;
	
	/* Change the PP:MM parameter to respect RATS timing TS-DP-1.1 13.8.1.1*/
	/* min to respect */
	/* FDT PCD = FWTt4at,activation = 71680 (1/fc) */
	/* (2^PP)*(MM+1)*(DD+128)*32 = 71680 ==> PP = 4 MM=0 DD=12*/
	/* max to respect not mandatory and as Tag has a FWT activation of 5.2us  */
	/* adding 16.4ms does not make sense ... */
	/* FDT PCD = FWTt4at,activation + dela(t4at,poll) = 5286us + 16.4ms ~= 21.7ms */
	/* (2^PP)*(MM+1)*(DD+128)*32 = 21,7 ==> PP = 4 MM=0 DD=12*/
	ProtocolSelectParameters[1] = 4; //  PP
	ProtocolSelectParameters[2] = 0; //  MM
	ProtocolSelectParameters[3] = 12; // DD
	errchk(PCD_ProtocolSelect((NbParam+1),PCD_PROTOCOL_ISO14443A,ProtocolSelectParameters,pDataRead));
	errchk(PCD_WriteRegister    ( 0x04,TIMER_WINDOW_REG_ADD,0x00,WriteRegisterParameters,pDataRead));
	/* in order to adjust the demoduation gain of the PCD which is reseted at each protocol select */
	errchk(PCD_WriteRegister (0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,DemoGainParameters,u95HFBuffer));
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}

int8_t ISO14443A_ConfigFDT( uint8_t WTXM)
{
	u8 ProtocolSelectParameters [6]  = {0x00, 0x00, 0x00, 0x00, 0x03, 0x03}; /* last 2 bytes since QJE version */
	u8 WriteRegisterParameters [2]  = {PCD_TYPEA_TIMERW, TIMER_WINDOW_UPDATE_CONFIRM_CMD};
  u8 DemoGainParameters [2]  = {PCD_TYPEA_ARConfigA, PCD_TYPEA_ARConfigB};
	u8 NbParam = 0;
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;	
	
	if( IcVers >= QJE)
		NbParam = 6;
	else
		NbParam = 4;
	
	/* FWI was updated thanks to ATS, if not the case use default value FWI = 4 TS-DP-1.1 13.6.2.11 */
	/* FDT PCD = FWT PICC + deltaFWT(t4at) + "deltaT(t4at,poll)" TS-DP-1.1 13.8*/
	/* If we perform some identification:
			FDT = (2^PP)*(MM+1)*(DD+128)*32/13.56 
			FDT = (2^(PP))*(1)*(2*DD+256)*16/13.56 + (2^(PP))*(MM)*(2*DD+256)*16/13.56
			FDT = (256*16/fc)*2^FWI + ((2^FWI) *256*16*1/fc)*MM with PP=FWI and DD=0
			FDT = (2^FWI)*4096*1/fc + FWT*MM (EQUATION 1)

			I_ With the choice to NOT add deltaT(t4at,poll) = 16,4ms
			1)	In the standard case (No extension time cmd received) we want
				FDT = FWT + delta FWT(T4AT) 
				FDT = FWT + 49152 (1/fc)

				If we take the rules that we will never set FWI to a value less than 4.
				(EQUATION 1 comes)
				FDT = FWT*MM + 65536*1/fc => delta FWT(T4AT) is respected

				As a conclusion with
				PP=FWI (with FWI>=4)
				MM=1
				DD=0
			we are following the specification. 
			
			2) In the case of extension time request, M will take the WTXM value.			*/
				
	if(FWI < 4 )
		FWI = 4; 
			
	ProtocolSelectParameters[1] = FWI; // PP
	ProtocolSelectParameters[2] = WTXM; //  MM
	ProtocolSelectParameters[3] = 0; // DD
		
	errchk(PCD_ProtocolSelect((NbParam+1),PCD_PROTOCOL_ISO14443A,ProtocolSelectParameters,pDataRead));
	errchk(PCD_WriteRegister    ( 0x04,TIMER_WINDOW_REG_ADD,0x00,WriteRegisterParameters,pDataRead));
  /* in order to adjust the demoduation gain of the PCD which is reseted at each protocol select */
	errchk(PCD_WriteRegister (0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,DemoGainParameters,u95HFBuffer));
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT;
}

/**
 * @brief  Checks if cards are in the field
 * @param  *pNbTag: Number of tag detected
 * @param  *pUIDout: UIDs of detected tags (11 bytes by tag, first byte indicate UID size) 
 * @return ISO14443A_SUCCESSCODE (Anticollision done) / ISO14443A_ERRORCODE_DEFAULT (Communication issue)
 */
void ISO14443A_MultiTagHunting ( uint8_t *pNbTag, uint8_t *pUIDout )
{

	bool exit = false;
	u8 loop = 0;
	u8 		i = 0;

	RemainingID = 0;
	*pNbTag = 0;
	
	delay(5);
		
	ISO14443A_InitStructure(); /* only initialize structure */

	while( exit == false && loop<= ISO14443A_NB_TAG_MAX )
	{
		ISO14443A_InitStructure();
		if(ISO14443A_IsPresent() == RESULTOK)
		{
			delayMicroseconds (50);
			if(ISO14443A_MultiAnticollision() == RESULTOK)
			{	
				MultiID[0+((*pNbTag)*11)] = ISO14443A_Card.UIDsize;
				for(i=0; i<10; i++)			
				{
					MultiID[(1+i)+((*pNbTag)*11)] = ISO14443A_Card.UID[i];
				}
				*pNbTag +=1;
			}
		}
		else
		{
			/* no more tag */
			exit = true;
		}
		loop++;
	}
	
	if( *pNbTag <= 5)
	{
		memcpy(pUIDout, &MultiID[0], (*pNbTag)*11 );
	}
	else
	{
		memcpy(pUIDout, &MultiID[0], 5*11 );
		RemainingID = (*pNbTag-5);
		memcpy(&MultiIDPart2[0], &MultiID[5*11] , RemainingID*11 );	
	}
		
}


void ISO14443A_MultiTagPart2 ( uint8_t *pNbTag, uint8_t *pUIDout )
{
	memcpy(pUIDout, &MultiIDPart2[0], RemainingID*11);
	*pNbTag = RemainingID;
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
