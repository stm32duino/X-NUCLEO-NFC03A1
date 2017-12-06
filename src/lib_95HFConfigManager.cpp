/**
  ******************************************************************************
  * @file    lib_95HFConfigManager.c 
  * @author  MMY Application Team
  * @version $Revision: 1328 $
  * @date    $Date: 2015-11-05 10:30:08 +0100 (Thu, 05 Nov 2015) $
  * @brief   This file provides set of firmware functions to manages device modes. 
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
#include "lib_95HFConfigManager.h"

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family ( CR95HF ) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup Config_Manager
 * 	@{
 *	@brief  This part of the library manage the configuration of the chip.
 */
/**
 * @brief  This buffer contains the data send/received by xx95HF
 */
extern uint8_t	u95HFBuffer[RFTRANS_95HF_MAX_BUFFER_SIZE+3];
extern ISO14443A_CARD ISO14443A_Card;

/** @addtogroup Configuration_Manager
 * 	@{
 *	@brief  This file is used to select a configuration, PICC or PCD or Initiator/Target.
*/

/** @addtogroup lib_ConfigManager_Private_Functions
 * 	@{
 */
static void ConfigManager_Init( void);
static int8_t ConfigManager_IDN(uint8_t *pResponse);
static void ConfigManager_Start(void );
static int8_t ConfigManager_PORsequence( void );

/* Variables for the different modes */
DeviceMode_t devicemode = UNDEFINED_MODE;
TagType_t nfc_tagtype   = UNDEFINED_TAG_TYPE;

/* Variable to know IC version */
IC_VERSION IcVers       = QJE;  /* default set last IC version */

bool StopProcess        = false;
bool TargetMode         = true;
uint8_t TagUID[16];

/**
 *	@brief  This function initialize the PICC
 *  @param  None 
 *  @retval None
 */
static void ConfigManager_Init( void)
{
  /* initialize the structure of the Rf tranceiver */
  drv95HF_InitConfigStructure ();
  
  /* configure the Serial interface to communicate with the RF transceiver */
  drv95HF_InitilizeSerialInterface ( );
}

/**
 *	@brief  this function sends an IDN command to the PICC device
 *  @param  pResponse : pointer on the PICC device reply
 *  @retval PICC_SUCCESSCODE : the function is succesful 
 */
static int8_t ConfigManager_IDN(uint8_t *pResponse)
{
  uint8_t DataToSend[] = {IDN	,0x00};
  
  /* send the command to the PICC and retrieve its response */
  drv95HF_SendReceive(DataToSend, pResponse);
  
  return MANAGER_SUCCESSCODE;
}

/**
 *	@brief  This function set a variable to inform Manager that a task is on going
 *  @param  none
 *  @retval none
 */
static void ConfigManager_Start( void )
{
  StopProcess = false;
  
  devicemode = UNDEFINED_MODE;
  nfc_tagtype = UNDEFINED_TAG_TYPE;
}

/**
 *	@brief  This function sends POR sequence. It is used to initialize chip after a POR.
 *  @param  none
 *  @retval MANAGER_ERRORCODE_PORERROR : the POR sequence doesn't succeded
 *  @retval MANAGER_SUCCESSCODE : chip is ready
 */
static int8_t ConfigManager_PORsequence( void )
{
  uint16_t NthAttempt=0;
  uint8_t command[]= {ECHO};
  
  /* Power up sequence: Pulse on IRQ_IN to select UART or SPI mode */
  drv95HF_SendIRQINPulse();
  
  /* SPI Reset */
  if(drv95HF_GetSerialInterface() == RFTRANS_95HF_INTERFACE_SPI)
  {
    drv95HF_ResetSPI();		
  }
  
  do
  {
    /* send an ECHO command and checks response */
    drv95HF_SendReceive(command, u95HFBuffer);
    
    if (u95HFBuffer[0]==ECHORESPONSE)
      return MANAGER_SUCCESSCODE;	
    
    /* if the SPI interface is selected then send a reset command*/
    if(drv95HF_GetSerialInterface() == RFTRANS_95HF_INTERFACE_SPI)
    {	
      drv95HF_ResetSPI();				
    }	
    /* if the UART interface is selected then send 255 ECHO commands*/
    else if(drv95HF_GetSerialInterface() == RFTRANS_95HF_INTERFACE_UART)
    {
      do {
        /* send an ECHO command and checks response */
        drv95HF_SendReceive(command, u95HFBuffer);
        if (u95HFBuffer[0] == ECHORESPONSE)
          return MANAGER_SUCCESSCODE;	
      }while(NthAttempt++ < RFTRANS_95HF_MAX_ECHO);
    }
  } while (u95HFBuffer[0]!=ECHORESPONSE && NthAttempt++ <5);
  
  return MANAGER_ERRORCODE_PORERROR;
}


/**
  * @}
  */ 

 /** @addtogroup lib_ConfigManager_Public_Functions
 * 	@{
 */

/**
 *	@brief  This interface function inform Manager that current task must be stopped
 *  @param  none
 *  @retval none
 */
void ConfigManager_Stop(void )
{
  StopProcess = true;
}

/**
 * @brief  This function initialize the NFC chip 
 * @brief  Physical communication with chip enabled, RF communication not enabled
 * @param  None
 * @retval None
 */
void ConfigManager_HWInit (void)
{
  
  /* Initialize HW according to protocol to use */
  ConfigManager_Init();
  
  /* initilialize the RF transceiver */
  if (ConfigManager_PORsequence( ) != MANAGER_SUCCESSCODE)
  {
    /* nothing to do, this is a trap for debug purpose you can use it to detect HW issue */
    /* or GPIO config issue */
  }
  
  /* Retrieve the IC version of the chip */
  ConfigManager_IDN(u95HFBuffer);
  
  IcVers = (IC_VERSION) (u95HFBuffer[ROM_CODE_REVISION_OFFSET]);
  
}

/**  
* @brief  	this function searches if a NFC or RFID tag is in the RF field. 
* @brief  	The method used is this described by the NFC specification
* @param  	tagsToFind : Flags to select the different kinds of tag to track, same as return value
* @retval 	TRACK_NOTHING : No tag in the RF field
* @retval 	TRACK_NFCTYPE1 : A NFC type1 tag is present in the RF field
* @retval 	TRACK_NFCTYPE2 : A NFC type2 tag is present in the RF field
* @retval 	TRACK_NFCTYPE3 : A NFC type3 tag is present in the RF field
* @retval 	TRACK_NFCTYPE4A : A NFC type4A tag is present in the RF field
* @retval 	TRACK_NFCTYPE4B : A NFC type4B tag is present in the RF field
* @retval 	TRACK_NFCTYPE5 : A ISO/IEC 15693 type A tag is present in the RF field
*/
uint8_t ConfigManager_TagHunting ( uint8_t tagsToFind )
{
  /* Start the config manager*/
  ConfigManager_Start();
  
  /******* NFC type 1 ********/
  if (tagsToFind&TRACK_NFCTYPE1)
  {
    PCD_FieldOff();
    delay(5);
    ISO14443A_Init( );
    if(ISO14443A_IsPresent() == RESULTOK)
    {		
      if(TOPAZ_ID(TagUID) == RESULTOK)
        return TRACK_NFCTYPE1;	
    }
  }
  
  /******* NFC type 2 and 4A ********/
  if ((tagsToFind&TRACK_NFCTYPE2) || (tagsToFind&TRACK_NFCTYPE4A))
  {
    PCD_FieldOff();
    delay(5);
    ISO14443A_Init( );
    if(ISO14443A_IsPresent() == RESULTOK)
    {			
      if(ISO14443A_Anticollision() == RESULTOK)
      {	
        if (((ISO14443A_Card.SAK&0x60) == 0x00) && (tagsToFind&TRACK_NFCTYPE2)) /* TT2 */
          return TRACK_NFCTYPE2;
        else if (((ISO14443A_Card.SAK&0x20) != 0x00) && (tagsToFind&TRACK_NFCTYPE4A))/* TT4A */
          return TRACK_NFCTYPE4A;
      }
    }
  }
  
  /******* NFC type 3 ********/
  if (tagsToFind&TRACK_NFCTYPE3)
  {
    PCD_FieldOff();
    delay(5);
    FELICA_Initialization();
    if(FELICA_IsPresent() == RESULTOK )
      return TRACK_NFCTYPE3;
  }
  
  /******* NFC type 4B ********/
  if (tagsToFind&TRACK_NFCTYPE4B)
  {
    PCD_FieldOff();
    delay(5);
    if(ISO14443B_IsPresent() == RESULTOK )
    {
      if(ISO14443B_Anticollision() == RESULTOK)
      {
        return TRACK_NFCTYPE4B;
      }
    }
  }
  
  /******* ISO15693 ********/
  if (tagsToFind&TRACK_NFCTYPE5)
  {
    PCD_FieldOff();
    delay(5);
    if(ISO15693_GetUID (TagUID) == RESULTOK)	
      return TRACK_NFCTYPE5;
  }
  
  
  /* Turn off the field if no tag has been detected*/
  PCD_FieldOff();
  
  /* No tag found */
  return TRACK_NOTHING;
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
