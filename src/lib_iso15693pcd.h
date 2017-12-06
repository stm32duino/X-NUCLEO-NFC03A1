/**
  ******************************************************************************
  * @file    lib_iso15693pcd.h 
  * @author  MMY Application Team
  * @version $Revision: 1333 $
  * @date    $Date: 2015-11-05 10:49:42 +0100 (Thu, 05 Nov 2015) $
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIB_ISO15693_H
#define __LIB_ISO15693_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lib_pcd.h"

#define RFU 									0
#define ISO15693_PROTOCOL              			0x01
#define ISO15693_SELECTLENGTH          			0x02


/* data rates constants fot select commands -------------------------------------------------- */
#define ISO15693_TRANSMISSION_26	    			0
#define ISO15693_TRANSMISSION_53	    			1
#define ISO15693_TRANSMISSION_6	   	 				2
#define ISO15693_TRANSMISSION_RFU						3
// constants fot select commands ------------------------------------------------------------- */
#define ISO15693_RESPECT_312	    					0
#define ISO15693_WAIT_FOR_SOF	    					1
/* modulation constants fot select commands -------------------------------------------------- */
#define ISO15693_MODULATION_100							0 
#define ISO15693_MODULATION_10							1
/* sub carrier constants fot select commands -------------------------------------------------- */
#define ISO15693_SINGLE_SUBCARRIER   				0
#define ISO15693_DUAL_SUBCARRIER     				1
/* appendCrc constants fot select commands ---------------------------------------------------- */
#define ISO15693_APPENDCRC  								1
#define ISO15693_DONTAPPENDCRC     					0

#define ISO15693_M24LR64R										0x2C
#define ISO15693_M24LR64ER									0x5E 
#define ISO15693_M24LR16ER									0x4E
#define ISO15693_M24LR04ER									0x5A
#define ISO15693_LRiS64K										0x44
#define ISO15693_LRiS2K											0x28
#define ISO15693_LRi2K											0x20
#define ISO15693_LRi1K											0x40

#define ISO15693_LOW_DENSITY								0x00
#define ISO15693_HIGH_DENSITY								0x01

#define ISO15693_NBBYTE_BLOCKLENGTH 				4

/* mask of request flag ----------------------------------------------------------------------- */
#define ISO15693_MASK_SUBCARRIERFLAG    		0x01
#define ISO15693_MASK_DATARATEFLAG	    		0x02
#define ISO15693_MASK_INVENTORYFLAG	  			0x04
#define ISO15693_MASK_PROTEXTFLAG    				0x08
#define ISO15693_MASK_SELECTORAFIFLAG 			0x10
#define ISO15693_MASK_ADDRORNBSLOTSFLAG 		0x20
#define ISO15693_MASK_OPTIONFLAG 						0x40
#define ISO15693_MASK_RFUFLAG 							0x80
/* mask of response flag ---------------------------------------------------------------------- */
#define ISO15693_RESULTFLAG_STATUSOK   				0x00
#define ISO15693_MASK_ERRORFLAG    						0x01

/* memmory size information	------------------------------------------------------------------- */
#define ISO15693_MEMSIZEMASK_BLOCKSIZE    		0x1F00
#define ISO15693_MEMSIZEMASK_NBBLOCK    			0x00FF
#define ISO15693_MEMSIZEOFFSET_BLOCKSIZE   		0x08


/* mask for information flags ----------------------------------------------------------------- */
#define ISO15693_MASK_DSFIDFLAG    						0x01
#define ISO15693_MASK_AFIFLAG    							0x02
#define ISO15693_MASK_MEMSIZEFLAG  						0x04
#define ISO15693_MASK_ICREFFLAG  							0x08

#define	ISO15693_MASK_GETSYSINFOREPLY_MEMSIZE	0x1F

/* command code  ------------------------------------------------------------------------------ */
#define ISO15693_CMDCODE_INVENTORY	    			0x01
#define ISO15693_CMDCODE_STAYQUIET	    			0x02
#define ISO15693_CMDCODE_READSINGLEBLOCK			0x20
#define ISO15693_CMDCODE_WRITESINGLEBLOCK			0x21
#define ISO15693_CMDCODE_LOCKBLOCK						0x22
#define ISO15693_CMDCODE_READMULBLOCKS				0x23
#define ISO15693_CMDCODE_WRITEMULBLOCKS				0x24
#define ISO15693_CMDCODE_SELECT								0x25
#define ISO15693_CMDCODE_RESETTOREADY					0x26
#define ISO15693_CMDCODE_WRITEAFI							0x27
#define ISO15693_CMDCODE_LOCKAFI							0x28
#define ISO15693_CMDCODE_WRITEDSFID						0x29
#define ISO15693_CMDCODE_LOCKDSFID						0x2A
#define ISO15693_CMDCODE_GETSYSINFO						0x2B
#define ISO15693_CMDCODE_GETSECURITYINFO			0x2C





// request flags  ------------------------------------------------------------------------------ */
#define ISO15693_REQFLAG_SINGLESUBCARRIER  			0
#define ISO15693_REQFLAG_TWOSUBCARRIER     			1
#define ISO15693_REQFLAG_LOWDATARATE  					0
#define ISO15693_REQFLAG_HIGHDATARATE     			1
#define ISO15693_REQFLAG_INVENTORYFLAGNOTSET		0
#define ISO15693_REQFLAG_INVENTORYFLAGSET   		1
#define ISO15693_REQFLAG_NOPROTOCOLEXTENSION		0
#define ISO15693_REQFLAG_PROTOCOLEXTENSION  		1
	// request flag 5 to 8 definition when inventory flag is not set ----------------------------- */
#define ISO15693_REQFLAG_NOTSELECTED						0
#define ISO15693_REQFLAG_SELECTED			   				1
#define ISO15693_REQFLAG_NOTADDRESSED						0
#define ISO15693_REQFLAG_ADDRESSED			   			1
#define ISO15693_REQFLAG_OPTIONFLAGNOTSET				0
#define ISO15693_REQFLAG_OPTIONFLAGSET			   	1
#define ISO15693_REQFLAG_RFUNOTSET							0
#define ISO15693_REQFLAG_RFUSET					  		 	1
	// request flag 5 to 8 definition when inventory flag is set -------------------------------- */
#define ISO15693_REQFLAG_NOTAFI									0
#define ISO15693_REQFLAG_AFI				   					1
#define ISO15693_REQFLAG_16SLOTS								0
#define ISO15693_REQFLAG_1SLOT				   				1



// mask for parameter byte (select command) --------------------------------------------------- */
#define ISO15693_MASK_APPENDCRC 									0x01
#define ISO15693_MASK_SUBCARRIER 									0x02
#define ISO15693_MASK_MODULATION 									0x04
#define ISO15693_MASK_WAITORSOF 									0x08
#define ISO15693_MASK_DATARATE	 									0x30
#define ISO15693_MASK_RFU		 											0xC0
// bits offset for parameter byte (select command) -------------------------------------------- */
#define ISO15693_OFFSET_APPENDCRC 								0x00
#define ISO15693_OFFSET_SUBCARRIER 								0x01
#define ISO15693_OFFSET_MODULATION 								0x02
#define ISO15693_OFFSET_WAITORSOF 								0x03
#define ISO15693_OFFSET_DATARATE	 								0x04
#define ISO15693_OFFSET_RFU		 										0x06
									 	
// byte offset for tag responses -------------------------------------------------------------- */
#define ISO15693_OFFSET_FLAGS			 								0x00
#define ISO15693_OFFSET_ERRORCODE									0x01

#define ISO15693_OFFSET_GETSYSINFOREPLY_NBBLOCK		0x00
#define ISO15693_OFFSET_GETSYSINFOREPLY_MEMSIZE		0x01

#define ISO15693_OFFSET_UID												PCD_DATA_OFFSET+0x02



#define ISO15693_INVENTORYOFFSET_DSFID	 					0x01
#define ISO15693_INVENTORYOFFSET_UID	 						0x02
#define ISO15693_INVENTORYOFFSET_CRC16	 					0x05

#define ISO15693_GETSYSINFOOFFSET_INFOFLAGS				0x01
#define ISO15693_GETSYSINFOOFFSET_UID							0x02
#define ISO15693_GETSYSINFOOFFSET_DSFID						0x0A

#define ISO15693_READSINGLEOFFSET_SECURITY				0x01
#define ISO15693_GETMULSECURITIOFFSET_SECURITY		0x01


/* number of byte of parameters ------------------------------------------------------------- */
#define ISO15693_NBBYTE_UID	 										0x08
#define ISO15693_NBBYTE_CRC16	 									0x02
#define ISO15693_NBBYTE_DSFID 									0x01
#define ISO15693_NBBYTE_AFI 										0x01
#define ISO15693_NBBYTE_BLOCKSECURITY		   			0x01
#define ISO15693_NBBYTE_REPLYFLAG		   					0x01
#define ISO15693_NBBYTE_INFOFLAG		   					0x01
#define ISO15693_NBBYTE_MEMORYSIZE		   				0x02
#define ISO15693_NBBYTE_ICREF			   						0x01
#define ISO15693_NBBYTE_REQUESTFLAG							0x01

#define ISO15693_NBBIT_INVENTORYSCANCHAIN				2+8+8+64
#define ISO15693_NBBYTE_INVENTORYSCANCHAIN			11  // =(2+8+8+64) /8 +1 
#define ISO15693_NBBITS_MASKPARAMETER   				64

/* success and error code --------------------------------------------------------------------- */
#define ISO15693_SUCCESSCODE										RESULTOK
#define ISO15693_ERRORCODE_DEFAULT							0xF1
#define ISO15693_ERRORCODE_REQUESTFLAG					0xF2
#define ISO15693_ERRORCODE_PARAMETERLENGTH			0xF3
#define ISO15693_ERRORCODE_CRCRESIDUE						0xF4
#define ISO15693_ERRORCODE_NOTAGFOUND								0xF5



/* command length ----------------------------------------------------------------------------- */
#define ISO15693_MAXLENGTH_INVENTORY 						13	 	// 8 + 8 + 8 + 64 + 16 = 104bits => 13 bytes
#define ISO15693_MAXLENGTH_STAYQUIET 						12		// 8 + 8 	 + 64 + 16 = 96 => 12 bytes
#define ISO15693_MAXLENGTH_READSINGLEBLOCK 			13		// 8 + 8 + 8 + 64 + 16 = 104bits => 13 bytes
#define ISO15693_MAXLENGTH_LOCKSINGLEBLOCK 			13		// 8 + 8 + 8 + 64 + 16 = 104bits => 13 bytes
#define ISO15693_MAXLENGTH_READMULBLOCK 				14		// 8 + 8 + 8 + 64 + 8 + 16 = 112 bits => 14 bytes
#define ISO15693_MAXLENGTH_SELECT	 							12		// 8 + 8 	 + 64 + 16 = 96 bits => 12 bytes
#define ISO15693_MAXLENGTH_RESETTOREADY					12		// 8 + 8 	 + 64 + 16 = 96 bits => 12 bytes
#define ISO15693_MAXLENGTH_WRTITEAFI						13		// 8 + 8 + 8 + 64 + 16 = 104bits => 13 bytes
#define ISO15693_MAXLENGTH_WRTITEDSFID					13		// 8 + 8 + 8 + 64 + 16 = 104bits => 13 bytes
#define ISO15693_MAXLENGTH_LOCKAFI							12		// 8 + 8 	 + 64 + 16 = 96 bits => 12 bytes
#define ISO15693_MAXLENGTH_LOCKDSFID						12		// 8 + 8 	 + 64 + 16 = 96 bits => 12 bytes
#define ISO15693_MAXLENGTH_GETMULSECURITY				14		// 8 + 8 + 8 + 64 + 8 + 16 = 112 bits => 14 bytes
#define ISO15693_MAXLENGTH_GETSYSTEMINFO				12		// 8 + 8 	 + 64 + 16 = 96 => 12 bytes
// reply length
#define ISO15693_MAXLENGTH_REPLYINVENTORY				12	 	// 8 + 8 + 64 + 16  = 96 => 12 bytes
#define ISO15693_MAXLENGTH_REPLYSTAYQUIET				4		// No reply
#define ISO15693_MAXLENGTH_REPLYREADSINGLEBLOCK 	MAX_BUFFER_SIZE		// 8 + 8 + 8 + 64 + 16 = 104bits => 13 bytes
#define ISO15693_MAXLENGTH_REPLYWRITESINGLEBLOCK	4		// 8 + 8 + 16 = 32 bits => 4 bytes
#define ISO15693_MAXLENGTH_REPLYLOCKSINGLEBLOCK 	4		// 8 + 8 + 16 = 32 bits => 4 bytes
#define ISO15693_MAXLENGTH_REPLYREADMULBLOCK 		MAX_BUFFER_SIZE		
#define ISO15693_MAXLENGTH_REPLYSELECT	 				4		// 8 + 8 + 16 = 32 bits => 4 bytes
#define ISO15693_MAXLENGTH_REPLYRESETTOREADY		4		// 8 + 8 + 16 = 32 bits => 4 bytes
#define ISO15693_MAXLENGTH_REPLYWRTITEAFI				4		// 8 + 8 + 16 = 32 bits => 4 bytes
#define ISO15693_MAXLENGTH_REPLYWRTITEDSFID			4		// 8 + 8 + 16 = 32 bits => 4 bytes
#define ISO15693_MAXLENGTH_REPLYLOCKAFI					4		// 8 + 8 + 16 = 32 bits => 4 bytes
#define ISO15693_MAXLENGTH_REPLYLOCKDSFID				4		// 8 + 8 + 16 = 32 bits => 4 bytes
#define ISO15693_MAXLENGTH_REPLYGETMULSECURITY		MAX_BUFFER_SIZE
#define ISO15693_MAXLENGTH_REPLYGETSYSTEMINFO		15		// 8 + 8 + 64 + 8 + 8 + 8 + 16 = 112 bits => 15 bytes


/* CRC parameters ----------------------------------------------------------------------------- */
#define ISO15693_OFFSET_LENGTH														PCD_LENGTH_OFFSET
#define ISO15693_CRC_MASK																	0x02
#define ISO15693_CRC_ERROR_CODE														0x02
#define ISO15693_COLISION_MASK														0x01
#define ISO15693_COLISION_ERROR_CODE											0x01

#define ISO15693_CRC_MASK_TYPE_A													0x20
#define ISO15693_CRC_ERROR_CODE_TYPE_A										0x20


#define ISO15693_MAXLENGTH_BLOCKSIZE											0x32

typedef struct {
int8_t 		DataRate		,	// 0 => 26k 	1 => 52 k 	2 => 6k	3=>RFU
					WaitOrSOF		,	// 0 => respect 312 µs 	1=> wait for SOF
					ModulationDepth	,	// 0 => 100% modulation	1=> 10% modulation
					SubCarrier		,	// 0 => single	1=> Dual
					AppendCRC		;	// 1=> append	0=> don't append			
}ISO15693ConfigStruct;

// CRC 16 constants
#define ISO15693_PRELOADCRC16 						0xFFFF 
#define ISO15693_POLYCRC16 								0x8408 
#define ISO15693_MASKCRC16 								0x0001
#define ISO15693_RESIDUECRC16 						0xF0B8



/* ---------------------------------------------------------------------------------
 * --- Local Functions  
 * --------------------------------------------------------------------------------- */

/* ISO15693 functions --- */

/* configure the PCD --- */
int8_t ISO15693_Init(void);

/* ISO15693 commands --- */
int8_t ISO15693_RunInventory16slots(const uint8_t Flags , const uint8_t AFI,uint8_t *NbTag,uint8_t *pUIDout);
int8_t ISO15693_RunAntiCollision(const uint8_t Flags , const uint8_t AFI,uint8_t *NbTag,uint8_t *pUIDout);

// Get functions
int8_t ISO15693_GetUID(uint8_t *UIDout);
int8_t ISO15693_GetTagIdentification(uint16_t *Length_Memory_TAG, uint8_t *Tag_Density, uint8_t *IC_Ref_TAG);
int8_t ISO15693_GetSystemInfo(const uint8_t Flags, const uint8_t *UIDin ,uint8_t *pResponse);

// Tag functions
uint8_t ISO15693_ReadBytesTagData(uint8_t Tag_Density, uint8_t IC_Ref_Tag, uint8_t *Data_To_Read, uint16_t NbBytes_To_Read, uint16_t FirstBytes_To_Read);
uint8_t ISO15693_WriteBytes_TagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBytes_To_Write, uint16_t FirstBytes_To_Write);

#ifdef ISO15693_ALLCOMMANDS 

int8_t ISO15693_LockSingleBlock(const uint8_t Flags, const uint8_t *UIDin, const uint8_t BlockNumber,uint8_t *pResponse);

int8_t ISO15693_Select(const uint8_t Flags, const uint8_t *UIDin, const uint8_t AppendCRC,const uint8_t *CRC16,uint8_t *pResponse);
int8_t ISO15693_WriteAFI(const uint8_t Flags, const uint8_t *UIDin,const uint8_t AFIToWrite,uint8_t *pResponse);
int8_t ISO15693_LockAFI(const uint8_t Flags, const uint8_t *UIDin, uint8_t *pResponse);
int8_t ISO15693_WriteDSFID(const uint8_t Flags, const uint8_t *UIDin,const uint8_t DSFIDToWrite,uint8_t *pResponse);
int8_t ISO15693_LockDSFID(const uint8_t Flags, const uint8_t *UIDin, uint8_t *pResponse);
int8_t ISO15693_GetMultipleBlockSecutityStatus(const uint8_t Flags, const uint8_t *UIDin, const uint8_t BlockNumber, const uint8_t NbBlocks,uint8_t *pResponse);
#endif /* ISO15693_ALLCOMMANDS*/


int8_t ISO15693_SplitInventoryResponse(const uint8_t *ReaderResponse,const uint8_t Length,uint8_t *Flags , uint8_t *DSFIDextract, uint8_t *UIDoutIndex);

#ifdef __cplusplus
}
#endif

#endif /* __ISO15693_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
