/**
  ******************************************************************************
  * @file    drv_spi.h
  * @author  MMY Application Team
  * @version $Revision: 1508 $
  * @date    $Date: 2016-01-08 09:50:04 +0100 (Fri, 08 Jan 2016) $
  * @brief   This file provides a set of firmware functions to manages SPI communications
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

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __DRV_SPI_H
#define __DRV_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ----------------------------------------------------------------- */
#include <stdint.h>

/** @addtogroup BSP
  * @{
  */

/** @defgroup X_NUCLEO_NFC03A1_Spi
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define DEV_SPI SPI
/* set state on SPI_NSS pin */
#define RFTRANS_95HF_NSS_LOW() 			digitalWrite(10, 0)
#define RFTRANS_95HF_NSS_HIGH()  		digitalWrite(10, 1)
/* set state on IRQ_In pin */
#define RFTRANS_95HF_IRQIN_LOW() 		digitalWrite(8, 0)	
#define RFTRANS_95HF_IRQIN_HIGH()		digitalWrite(8, 1)
/* Exported functions ------------------------------------------------------- */
/** @defgroup X_NUCLEO_NFC03A1_Spi_Exported_Functions
  * @{
  */
void RFTRANS_SPI_Init(void);
void SPI_SendReceiveBuffer(const uint8_t *pCommand, uint8_t length, uint8_t *pResponse);
uint8_t SPI_SendReceiveByte(uint8_t data); 

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

#endif /* __DRV_SPI_H */

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
