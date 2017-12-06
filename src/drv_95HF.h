/**
  ******************************************************************************
  * @file    drv_95HF.h 
  * @author  MMY Application Team
  * @version $Revision: 1347 $
  * @date    $Date: 2015-11-05 11:36:05 +0100 (Thu, 05 Nov 2015) $
  * @brief   This file provides set of firmware functions to manage communication
  * @brief   between MCU and 95HF device (CR95HF) 
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

/* Define to prevent recursive inclusion ------------------------------------------------ */
#ifndef __DRV_95HF_H
#define __DRV_95HF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ----------------------------------------------------------------------------- */
#include "drv_spi.h"
//#include "drv_uart.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 

/** @defgroup XX95HF
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup XX95HF_Exported_Constants
  * @{
  */
/* RFtransceiver HEADER command definition ---------------------------------------------- */
#define RFTRANS_95HF_COMMAND_SEND															0x00
#define RFTRANS_95HF_COMMAND_RESET														0x01
#define RFTRANS_95HF_COMMAND_RECEIVE													0x02
#define RFTRANS_95HF_COMMAND_POLLING													0x03
#define RFTRANS_95HF_COMMAND_IDLE															0x07

/* RFtransceiver mask and data to check the data (SPI polling)--------------------------- */
#define RFTRANS_95HF_FLAG_DATA_READY													0x08
#define RFTRANS_95HF_FLAG_DATA_READY_MASK											0x08

/* RF transceiver status	--------------------------------------------------------------- */
#define RFTRANS_95HF_SUCCESS_CODE															RESULTOK
#define RFTRANS_95HF_NOREPLY_CODE															0x01
#define	RFTRANS_95HF_ERRORCODE_DEFAULT												0xFE
#define	RFTRANS_95HF_ERRORCODE_TIMEOUT												0xFD
#define RFTRANS_95HF_ERRORCODE_POR														0x44

/* RF transceiver polling status	------------------------------------------------------- */
#define RFTRANS_95HF_POLLING_RFTRANS_95HF											0x00
#define RFTRANS_95HF_POLLING_TIMEOUT													0x01

/* RF transceiver number of byte of the buffers------------------------------------------ */
#define RFTRANS_95HF_RESPONSEBUFFER_SIZE											0xFF
#define RFTRANS_95HF_MAX_BUFFER_SIZE													0xFF
/* Max of 256 (for QJC) and 768 (for QJE. Should be 512 but bug Design) */
#define RFTRANS_95HF_MAX_ECHO																	768		

/* RF transceiver Offset of the command and the response -------------------------------- */
#define RFTRANS_95HF_COMMAND_OFFSET														0x00
#define RFTRANS_95HF_LENGTH_OFFSET														0x01
#define RFTRANS_95HF_DATA_OFFSET															0x02

/* ECHO response ------------------------------------------------------------------------ */
#define ECHORESPONSE																					0x55

/* Sleep parameters --------------------------------------------------------------------- */
#define IDLE_SLEEP_MODE																				0x00
#define IDLE_HIBERNATE_MODE																		0x01

#define IDLE_CMD_LENTH																				0x0E

#define WU_TIMEOUT																						0x01
#define WU_TAG																								0x02
#define WU_FIELD																							0x04
#define WU_IRQ																								0x08
#define WU_SPI																								0x10

#define HIBERNATE_ENTER_CTRL																	0x0400
#define SLEEP_ENTER_CTRL																			0x0100
#define SLEEP_FIELD_ENTER_CTRL																0x0142

#define HIBERNATE_WU_CTRL																			0x0400
#define SLEEP_WU_CTRL																					0x3800

#define LEAVE_CTRL																						0x1800

/* Calibration parameters---------------------------------------------------------------- */
#define WU_SOURCE_OFFSET																			0x02
#define WU_PERIOD_OFFSET																			0x09
#define DACDATAL_OFFSET																				0x0C
#define DACDATAH_OFFSET																				0x0D
#define NBTRIALS_OFFSET																				0x0F



#define MAX_BUFFER_SIZE  																			256 

 #define ECHO																					    0x55
#define LISTEN																								0x05
#define DUMMY_BYTE																						0xFF

/* set state on IRQ in pin */
//#define RFTRANS_95HF_GET_INTERFACE() 		HAL_GPIO_ReadPin(INTERFACE_GPIO_PORT,INTERFACE_PIN)

#define READERREPLY_MAX_BUFFER_SIZE			0x40	
/**
  * @}
  */
  
/* Exported types ------------------------------------------------------------*/
/** @defgroup XX95HF_Exported_Types
  * @{
  */
typedef enum {
	RFTRANS_95HF_INTERFACE_UART = 0,
	RFTRANS_95HF_INTERFACE_SPI,
	RFTRANS_95HF_INTERFACE_TWI
}RFTRANS_95HF_INTERFACE;


/**
 *	@brief  the different states of the RF transceiver
 */
typedef enum {
	RFTRANS_95HF_STATE_UNKNOWN = 0,
	RFTRANS_95HF_STATE_HIBERNATE ,
	RFTRANS_95HF_STATE_SLEEP,
	RFTRANS_95HF_STATE_POWERUP,
	RFTRANS_95HF_STATE_TAGDETECTOR,
	RFTRANS_95HF_STATE_READY,
	RFTRANS_95HF_STATE_READER,
	RFTRANS_95HF_STATE_TAGHUNTING,
}RFTRANS_95HF_STATE;

/**
 *	@brief  the RF transceiver can be configured as either a reader or a card emulator
 *	@brief  or as P2P device
 */
typedef enum {
	RFTRANS_95HF_MODE_UNKNOWN = 0,
	RFTRANS_95HF_MODE_READER ,
	RFTRANS_95HF_MODE_CARDEMULATOR ,
	RFTRANS_95HF_MODE_PASSIVEP2P ,
	RFTRANS_95HF_MODE_ACTIVEP2P ,
}RFTRANS_95HF_MODE;

/**
 *	@brief  the Rf transceiver supports the differrent protocols
 */
typedef enum {
	RFTRANS_95HF_PROTOCOL_UNKNOWN = 0,
	RFTRANS_95HF_PCD_14443A,
	RFTRANS_95HF_PCD_14443B,
	RFTRANS_95HF_PCD_15693,
	RFTRANS_95HF_PCD_18092,
	RFTRANS_95HF_PICC_14443A,
	RFTRANS_95HF_PICC_14443B,
	RFTRANS_95HF_PICC_15693,
	RFTRANS_95HF_PICC_18092,
}RFTRANS_95HF_PROTOCOL;		


/**
 *	@brief  structure to store driver information
 */
typedef struct {
	RFTRANS_95HF_INTERFACE 			uInterface;
	RFTRANS_95HF_STATE 					uState;
	RFTRANS_95HF_MODE 					uMode;
	RFTRANS_95HF_PROTOCOL				uCurrentProtocol;
}drv95HF_ConfigStruct;
/**
  * @}
  */

/* External variables --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/** @defgroup XX95HF_Exported_Functions
  * @{
  */
void 		drv95HF_ResetSPI(void);
//int8_t 	drv95HF_GetInterfacePinState(void);
uint8_t drv95HF_GetSerialInterface(void);
//int8_t 	drv95HF_GetIRQOutState(void);
void		drv95HF_InitConfigStructure(void);
void 		drv95HF_InitilizeSerialInterface(void);
void 		drv95HF_ReceiveSPIResponse(uint8_t *pData);
void 		drv95HF_SendSPICommand(const uint8_t *pData );
int8_t  drv95HF_SendReceive(const uint8_t *pCommand, uint8_t *pResponse);
void  	drv95HF_SendCmd(const uint8_t *pCommand);
int8_t  drv95HF_PoolingReading(uint8_t *pResponse);

void drv95HF_Idle(const uint8_t WU_source, const uint8_t mode);
void drv95HF_SendIRQINPulse(void);

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

#ifdef __cplusplus
}
#endif
  
#endif /* __DRV_95HF_H */

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
