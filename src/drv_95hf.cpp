/**
  ******************************************************************************
  * @file    drv_95HF.c 
  * @author  MMY Application Team
  * @version $Revision: 1348 $
  * @date    $Date: 2015-11-05 11:36:33 +0100 (Thu, 05 Nov 2015) $
  * @brief   This file provides set of driver functions to manage communication 
  *          between MCU and xx95HF chip (CR95HF) 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
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
#include "SPI.h"
#include "stdbool.h"
#include "drv_95HF.h"
#include "lib_pcd.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */ 


/** @addtogroup XX95HF
  * @{
  */

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void drv95HF_InitializeSPI(void);
static void drv95HF_SendSPIResetByte(void);
static int8_t drv95HF_SPIPollingCommand(void);
/* Global variables ---------------------------------------------------------*/
/** @defgroup XX95HF_Global variables
  * @{
  */
/** 
* @brief  buffer to exchange data with the RF tranceiver.
*/
uint8_t	u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

/** 
 *  @brief This uTimeOut variable is used as a timeout duting the communication with the RF tranceiver 
 */

extern volatile bool	        uDataReady; 
bool EnableTimeOut              = true;

/* ConfigStructure */ 										 
drv95HF_ConfigStruct	        drv95HFConfig;
/**
  * @}
  */ 

/* Private functions ---------------------------------------------------------*/
/** @defgroup XX95HF_Private_Functions
  * @{
  */

/**
 *	@brief  this functions initializes the SPI in order to communicate with the 95HF device
 *  @param  None
 *  @retval void 
 */
static void drv95HF_InitializeSPI(void)
{
  RFTRANS_SPI_Init();
}

/**
 *	@brief  This function sends a reset command over SPI bus
 *  @param  None
 *  @retval None
 */
static void drv95HF_SendSPIResetByte(void)
{
  /* Send reset control byte */
  SPI_SendReceiveByte(RFTRANS_95HF_COMMAND_RESET);
}

/**
 *	@brief  This function polls 95HF chip until a response is ready or
 *				  the counter of the timeout overflows
 *  @param  None
 *  @retval PCD_POLLING_TIMEOUT : The time out was reached 
 *  @retval PCD_SUCCESS_CODE : A response is available
 */
static int8_t drv95HF_SPIPollingCommand( void )
{
  uint8_t Polling_Status = 0;
  int timestamp_start = -1;
  int timestamp_final = -1;
  bool uTimeOut = false;
  
  if(EnableTimeOut)
  {
	timestamp_start = millis();
  }

  DEV_SPI.beginTransaction(SPISettings(1500000, MSBFIRST, SPI_MODE3));

  do
  {
    /* Low level on NSS  */
    RFTRANS_95HF_NSS_LOW();
    delay(2);
      
    /*  poll the 95HF transceiver until he's ready ! */
    Polling_Status  = SPI_SendReceiveByte(RFTRANS_95HF_COMMAND_POLLING);
    Polling_Status &= RFTRANS_95HF_FLAG_DATA_READY_MASK;
	if(EnableTimeOut)
    {
      /* 3sec for LLCP can be improved to adjust it dynamically */
      timestamp_final = millis();
      if((timestamp_final - timestamp_start) > 3000)
	  {
        uTimeOut = true;
      }
    } 
  }
  while( Polling_Status	!= RFTRANS_95HF_FLAG_DATA_READY && uTimeOut != true );
    
  /* High level on NSS  */
  RFTRANS_95HF_NSS_HIGH();

  DEV_SPI.endTransaction();
  
  if ( uTimeOut == true )
  {
    return RFTRANS_95HF_POLLING_TIMEOUT;
  }
  
  return RFTRANS_95HF_SUCCESS_CODE;	
}


/**
 * @}
 */

/** @defgroup drv_95HF_Public_Functions
 * 	@{
 */


/**
* @brief  	Initilize the 95HF device config structure
* @param  	None
* @retval 	None
*/
void drv95HF_InitConfigStructure (void)
{
  drv95HFConfig.uInterface 		= RFTRANS_95HF_INTERFACE_SPI;
  drv95HFConfig.uState 			= RFTRANS_95HF_STATE_POWERUP;
  drv95HFConfig.uCurrentProtocol 	= RFTRANS_95HF_PROTOCOL_UNKNOWN;
  drv95HFConfig.uMode 			= RFTRANS_95HF_MODE_UNKNOWN;
}

/**
 *	@brief  Send a reset sequence over SPI bus (Reset command ,wait ,negative pulse on IRQin).
 *  @param  None
 *  @retval None
 */
void drv95HF_ResetSPI(void)
{	
  /* Deselect Rftransceiver over SPI */
  RFTRANS_95HF_NSS_HIGH();
  delay(1);
  
  DEV_SPI.beginTransaction(SPISettings(1500000, MSBFIRST, SPI_MODE3));
  /* Select 95HF device over SPI */
  RFTRANS_95HF_NSS_LOW();
  /* Send reset control byte	*/
  drv95HF_SendSPIResetByte();
  /* Deselect 95HF device over SPI */
  RFTRANS_95HF_NSS_HIGH();
  
  DEV_SPI.endTransaction();
  
  delay(3);
  /* send a pulse on IRQ_in*/
  drv95HF_SendIRQINPulse();
  delay(10);  /* mandatory before issuing a new command */
  
  drv95HFConfig.uState = RFTRANS_95HF_STATE_READY;
}

/**
 *	@brief  returns the value of interface pin. 
 *				  Low level means UART bus was choose.
 *				  High level means SPI bus was choose.
 *  @param  None
 *  @retval Bit_RESET : 0
 *  @retval Bit_SET 	: 1
 */
//int8_t drv95HF_GetInterfacePinState(void)
//{
//  return RFTRANS_95HF_GET_INTERFACE();
//}

/**  
 *  @brief  This function returns the IRQout state
 *  @param  None
 *  @retval Pin set 	: 1
 *  @retval Pin reset	: 0
 */
//int8_t drv95HF_GetIRQOutState(void)
//{
//  if (HAL_GPIO_ReadPin(IRQOUT_RFTRANS_95HF_PORT, IRQOUT_RFTRANS_95HF_PIN) != GPIO_PIN_RESET)
//  {
//    return 0x01;
//  }
//  else 
//  {
//    return 0x00;
//  }
//}

/**
 *	@brief  This function initialize MCU serial interface peripheral (SPI or UART)
 *  @param  None
 *  @retval None
 */
void drv95HF_InitilizeSerialInterface(void)
{
  /* -- Get interface pin state to select UART or SPI mode -- */
  //if (drv95HF_GetInterfacePinState() != RFTRANS_95HF_INTERFACE_UART)
  //{
    /* -- Set communication type -- */
    drv95HFConfig.uInterface = RFTRANS_95HF_INTERFACE_SPI;
    /* -- Initialize SPI Interface -- */ 
    drv95HF_InitializeSPI( );
  //}
}


/**
 *	@brief  This function returns the Interface selected(UART or SPI)
 *  @param  None
 *  @retval RFTRANS_INTERFACE_UART : the UART interface is selected
 *  @retval RFTRANS_INTERFACE_SPI : the SPI interface is selected
 */
uint8_t drv95HF_GetSerialInterface(void)
{
  return drv95HFConfig.uInterface;
}

/**
 *	@brief  This function sends a command over SPI bus
 *  @param  *pData : pointer on data to send to the xx95HF
 *  @retval None
 */
void drv95HF_SendSPICommand(const uint8_t *pData)
{
  uint8_t DummyBuffer[MAX_BUFFER_SIZE];
  
  DEV_SPI.beginTransaction(SPISettings(1500000, MSBFIRST, SPI_MODE3));
  
  /* Select xx95HF over SPI */
  RFTRANS_95HF_NSS_LOW();
  
  /* Send a sending request to xx95HF  */
  SPI_SendReceiveByte(RFTRANS_95HF_COMMAND_SEND);
  
  if(*pData == ECHO)
  {
    /* Send a sending request to xx95HF */ 
    SPI_SendReceiveByte(ECHO);
  }
  else
  {
    /* Transmit the buffer over SPI */
    SPI_SendReceiveBuffer(pData, pData[RFTRANS_95HF_LENGTH_OFFSET]+RFTRANS_95HF_DATA_OFFSET, DummyBuffer);
  }
  
  /* Deselect xx95HF over SPI  */
  RFTRANS_95HF_NSS_HIGH();
  
  DEV_SPI.endTransaction();
}



/**
 *	@brief  This fucntion recovers a response from 95HF device
 *  @param  *pData : pointer on data received from 95HF device
 *  @retval None
 */
void drv95HF_ReceiveSPIResponse(uint8_t *pData)
{
  uint8_t DummyBuffer[MAX_BUFFER_SIZE];

  DEV_SPI.beginTransaction(SPISettings(1500000, MSBFIRST, SPI_MODE3));
  
  /* Select 95HF transceiver over SPI */
  RFTRANS_95HF_NSS_LOW();
  
  /* Request a response from 95HF transceiver */
  SPI_SendReceiveByte(RFTRANS_95HF_COMMAND_RECEIVE);
  
  /* Recover the "Command" byte */
  pData[RFTRANS_95HF_COMMAND_OFFSET] = SPI_SendReceiveByte(DUMMY_BYTE);
  
  if(pData[RFTRANS_95HF_COMMAND_OFFSET] == ECHO)
  {
    pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
    /* In case we were in listen mode error code cancelled by user (0x85 0x00) must be retrieved */
    pData[RFTRANS_95HF_LENGTH_OFFSET+1] = SPI_SendReceiveByte(DUMMY_BYTE);
    pData[RFTRANS_95HF_LENGTH_OFFSET+2] = SPI_SendReceiveByte(DUMMY_BYTE);
  }
  else if(pData[RFTRANS_95HF_COMMAND_OFFSET] == 0xFF)
  {
    pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
    pData[RFTRANS_95HF_LENGTH_OFFSET+1] = SPI_SendReceiveByte(DUMMY_BYTE);
    pData[RFTRANS_95HF_LENGTH_OFFSET+2] = SPI_SendReceiveByte(DUMMY_BYTE);
  }
  else
  {
    /* Recover the "Length" byte */
    pData[RFTRANS_95HF_LENGTH_OFFSET]  = SPI_SendReceiveByte(DUMMY_BYTE);
    /* Checks the data length */
    if(pData[RFTRANS_95HF_LENGTH_OFFSET] != 0x00)
      SPI_SendReceiveBuffer(DummyBuffer, pData[RFTRANS_95HF_LENGTH_OFFSET], &pData[RFTRANS_95HF_DATA_OFFSET]);
  }
  
  /* Deselect xx95HF over SPI */
  RFTRANS_95HF_NSS_HIGH();
  
  DEV_SPI.endTransaction();
}

/**
 *	@brief  This function send a command to 95HF device over SPI or UART bus and receive its response
 *  @param  *pCommand  : pointer on the buffer to send to the 95HF device ( Command | Length | Data)
 *  @param  *pResponse : pointer on the 95HF device response ( Command | Length | Data)
 *  @retval RFTRANS_95HF_SUCCESS_CODE : the function is succesful
 */
int8_t drv95HF_SendReceive(const uint8_t *pCommand, uint8_t *pResponse)
{
  
  if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
  {
    /* First step  - Sending command 	*/
    drv95HF_SendSPICommand(pCommand);
    /* Second step - Polling	*/
    if (drv95HF_SPIPollingCommand( ) != RFTRANS_95HF_SUCCESS_CODE)
    {	
      *pResponse = RFTRANS_95HF_ERRORCODE_TIMEOUT;
      return RFTRANS_95HF_POLLING_TIMEOUT;	
    }
    /* Third step  - Receiving bytes */
    drv95HF_ReceiveSPIResponse(pResponse);
  }

  return RFTRANS_95HF_SUCCESS_CODE; 
}

/**
 *	@brief  This function send a command to 95HF device over SPI or UART bus
 *  @param  *pCommand  : pointer on the buffer to send to the 95HF ( Command | Length | Data)
 *  @retval None
 */
void drv95HF_SendCmd(const uint8_t *pCommand)
{
  if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
    /* First step  - Sending command 	*/
    drv95HF_SendSPICommand(pCommand);
  
//  else if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
    /* First step  - Sending command	*/
//    drv95HF_SendUARTCommand(pCommand);
}

/**
 *	@brief  This function is a specific command. It's made polling and reading sequence. 
 *  @param  *pResponse : pointer on the 95HF device response ( Command | Length | Data)
 *  @retval RFTRANS_95HF_SUCCESS_CODE : the function is succesful
 *  @retval RFTRANS_95HF_POLLING_RFTRANS_95HF : the polling sequence returns an error
 */
int8_t  drv95HF_PoolingReading (uint8_t *pResponse)
{
  *pResponse =RFTRANS_95HF_ERRORCODE_DEFAULT;
  *(pResponse+1) = 0x00;
  
  if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
  {
    /* First step - Polling	*/
    if (drv95HF_SPIPollingCommand( ) != RFTRANS_95HF_SUCCESS_CODE)
    {	*pResponse = RFTRANS_95HF_ERRORCODE_TIMEOUT;
    return RFTRANS_95HF_ERRORCODE_TIMEOUT;	
    }
    /* Second step  - Receiving bytes */
    drv95HF_ReceiveSPIResponse(pResponse);
  }

  return RFTRANS_95HF_SUCCESS_CODE; 
}

/**
 *	@brief  Send a negative pulse on IRQin pin
 *  @param  None
 *  @retval None
 */
void drv95HF_SendIRQINPulse(void)
{
  if (drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
  {
    /* Send a pulse on IRQ_IN */
    RFTRANS_95HF_IRQIN_HIGH() ;
    delay(1);
    RFTRANS_95HF_IRQIN_LOW() ;
    delay(1);
    RFTRANS_95HF_IRQIN_HIGH() ;
  }
  
  /* Need to wait 10ms after the pulse before to send the first command */
  delay(10); 
}

/**
 *	@brief  this functions put the ST95HF in sleep/hibernate mode
 *  @param  WU_source : Source selected to wake up the device (WU_TIMEOUT,WU_TAG,WU_FIELD,WU_IRQ,WU_SPI)
 *  @param  mode : Can be IDLE_SLEEP_MODE or IDLE_HIBERNATE_MODE
 *  @retval None 
 */
void drv95HF_Idle(const uint8_t WU_source, const uint8_t mode)
{
  uint8_t pCommand[] = {RFTRANS_95HF_COMMAND_IDLE, IDLE_CMD_LENTH, 0, 0, 0, 0, 0 ,0x18 ,0x00 ,0x00 ,0x60 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00};
  
  if (mode == IDLE_SLEEP_MODE) /* SLEEP */
  {
    /* Select the wake up source*/
    pCommand[2] = WU_source;
    /* Select SLEEP mode */
    if (WU_source == WU_FIELD)
    {
      pCommand[3] = GETMSB(SLEEP_FIELD_ENTER_CTRL);
      pCommand[4] = GETLSB(SLEEP_FIELD_ENTER_CTRL);
    }
    else
    {
      pCommand[3] = GETMSB(SLEEP_ENTER_CTRL);
      pCommand[4] = GETLSB(SLEEP_ENTER_CTRL);
    }
    pCommand[5] = GETMSB(SLEEP_WU_CTRL);
    pCommand[6] = GETLSB(SLEEP_WU_CTRL);
  }
  else /* HIBERNATE */
  {
    /* Select the wake up source, only IRQ is available for HIBERNATE mode*/
    pCommand[2] = WU_IRQ;
    /* Select HIBERNATE mode */
    pCommand[3] = GETMSB(HIBERNATE_ENTER_CTRL);
    pCommand[4] = GETLSB(HIBERNATE_ENTER_CTRL);
    pCommand[5] = GETMSB(HIBERNATE_WU_CTRL);
    pCommand[6] = GETLSB(HIBERNATE_WU_CTRL);
    pCommand[10] = 0x00;
  }
  
  /* Send the command */
  if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
    /* First step  - Sending command 	*/
    drv95HF_SendSPICommand(pCommand);
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
 
/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
