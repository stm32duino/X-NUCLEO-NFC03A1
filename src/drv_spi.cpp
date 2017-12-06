/**
  ******************************************************************************
  * @file    drv_spi.c 
  * @author  MMY Application Team
  * @version $Revision: 1508 $
  * @date    $Date: 2016-01-08 09:50:04 +0100 (Fri, 08 Jan 2016) $
  * @brief   This file provides a set of firmware functions to manages SPI communications
  ******************************************************************************
  * @copyright
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
	******************************************************************************
*/ 

//#ifdef __cplusplus
// extern "C" {
//#endif

/* Includes ------------------------------------------------------------------*/
#include "SPI.h"
#include "drv_spi.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup X_NUCLEO_NFC03A1_Spi
 * @{
 */
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
 /* Public Functions ----------------------------------------------------------*/
/** @defgroup X_NUCLEO_NFC03A1_Spi_Public_Functions
 * @{
 */
/**  
 *	@brief  Initialise HAL SPI for NFC03A1
 *  @param  None
 *  @retval None
 */
void RFTRANS_SPI_Init(void) 
{
  // Configure NSS pin for CR95HF
  pinMode(10, OUTPUT);

  // Configure interface pin select for CR95HF
  pinMode(9, OUTPUT);
  
  // Configure interrupt input pin for CR95HF
  pinMode(8, OUTPUT);
  
  // Set the interface pin select high in order to configure the NFC reader to use the SPI interface
  digitalWrite(9, HIGH);
  
  /* SPI_NSS  = High Level  */
  RFTRANS_95HF_NSS_HIGH();
  
  /* Set signal to high */
  RFTRANS_95HF_IRQIN_HIGH();

  DEV_SPI.begin();
}

/**  
 *	@brief  Sends one byte over SPI and recovers a response
 *  @param  data : data to send
 *  @retval data response from SPIx 
 */
uint8_t SPI_SendReceiveByte(uint8_t data) 
{	
  return DEV_SPI.transfer(data);
}

/**
 *	@brief  reveive a byte array over SPI
 *  @param  pCommand  : pointer on the buffer to send
 *  @param  length	 	: length of the buffer to send
 *  @param  pResponse : pointer on the buffer response
 *  @retval None 
 */
void SPI_SendReceiveBuffer(const uint8_t *pCommand, uint8_t length, uint8_t *pResponse) 
{
  uint8_t i;
  
  for(i=0; i<length; i++)
    pResponse[i] = SPI_SendReceiveByte(pCommand[i]);

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

//#ifdef __cplusplus
//}
//#endif
 
/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
