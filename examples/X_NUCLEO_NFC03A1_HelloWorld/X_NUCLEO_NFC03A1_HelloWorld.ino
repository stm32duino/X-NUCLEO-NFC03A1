/**
 ******************************************************************************
 * @file    X_NUCLEO_NFC03A1_HelloWorld.ino
 * @author  AST
 * @version V1.0.0
 * @date    6 December 2017
 * @brief   Arduino test application for the STMicrolectronics X-NUCLEO-NFC03A1
 *          NFC reader/writer expansion board.
 *          This application makes use of C++ classes obtained from the C
 *          components' drivers.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "lib_NDEF_URI.h"
#include "lib_NDEF_SMS.h"
#include "lib_NDEF_Text.h"
#include "lib_NDEF_Email.h"
#include "lib_NDEF_Geo.h"
#include "lib_95HFConfigManager.h"
#include "miscellaneous.h"
#include "lib_95HFConfigManager.h"
#include "lib_wrapper.h"
#include "lib_NDEF_URI.h"
#include "drv_spi.h"

#define SerialPort Serial

/* Exported define -----------------------------------------------------------*/
#define BULK_MAX_PACKET_SIZE            0x00000040

/* Regarding board antenna (and matching) appropriate 
value may be modified to optimized RF performances */
/* Analogue configuration register
 ARConfigB	bits  7:4	MOD_INDEX	Modulation index to modulator
                      3:0	RX_AMP_GAIN	Defines receiver amplifier gain
For type A you can also adjust the Timer Window
*/

/******************  PICC  ******************/
/* ISO14443A */
#define PICC_TYPEA_ACConfigA            0x27  /* backscaterring */

/* ISO14443B */
#define PICC_TYPEB_ARConfigD            0x0E  /* card demodulation gain */
#define PICC_TYPEB_ACConfigA            0x17  /* backscaterring */

/* Felica */
#define PICC_TYPEF_ACConfigA            0x17  /* backscaterring */

/* Private variables ---------------------------------------------------------*/

/* TT1 (PCD only)*/
uint8_t TT1Tag[NFCT1_MAX_TAGMEMORY];

/* TT2 */
uint8_t TT2Tag[NFCT2_MAX_TAGMEMORY];

/* TT3 */
uint8_t TT3Tag[NFCT3_MAX_TAGMEMORY];
uint8_t *TT3AttribInfo = TT3Tag, *TT3NDEFfile = &TT3Tag[NFCT3_ATTRIB_INFO_SIZE];

/* TT4 */
uint8_t CardCCfile      [NFCT4_MAX_CCMEMORY];
uint8_t CardNDEFfileT4A [NFCT4_MAX_NDEFMEMORY];
uint8_t CardNDEFfileT4B [NFCT4_MAX_NDEFMEMORY];

/* TT5 (PCD only)*/
uint8_t TT5Tag[NFCT5_MAX_TAGMEMORY];

sURI_Info url; 
extern uint8_t NDEF_Buffer []; 
extern DeviceMode_t devicemode;

sRecordInfo_uri RecordStruct;
  
int8_t TagType = TRACK_NOTHING;
bool TagDetected = false;
bool terminal_msg_flag = false ;
uint8_t status = ERRORCODE_GENERIC;
static char dataOut[256];

#define X_NUCLEO_NFC03A1_LED1 D7
#define X_NUCLEO_NFC03A1_LED2 D6
#define X_NUCLEO_NFC03A1_LED3 D5
#define X_NUCLEO_NFC03A1_LED4 D4

void setup() {
  // 95HF HW Init
  ConfigManager_HWInit();

  // LED1
  pinMode(X_NUCLEO_NFC03A1_LED1, OUTPUT);

  // LED2
  pinMode(X_NUCLEO_NFC03A1_LED2, OUTPUT);

  // LED3
  pinMode(X_NUCLEO_NFC03A1_LED3, OUTPUT);

  // LED4
  pinMode(X_NUCLEO_NFC03A1_LED4, OUTPUT);
  
  // Configure USB serial interface
  SerialPort.begin(115200);
  
  SerialPort.print("\r\n\r\n---------------------------------------\r\n******Welcome to x-nucleo-nfc03a1 demo******\r\n----------------------------------------");
  SerialPort.print("\r\n\r\nPlease bring an NFC tag to the board vicinity and Press User Button B1 on the board to start URI Writer/Reader demo on the tag");
  
  terminal_msg_flag = true;

  digitalWrite(X_NUCLEO_NFC03A1_LED1, HIGH);
}


/* Loop ----------------------------------------------------------------------*/

void loop()
{
  devicemode = PCD;
    
  /* Scan to find if there is a tag */
  TagType = ConfigManager_TagHunting(TRACK_ALL);
    
  switch(TagType)
  {
    case TRACK_NFCTYPE1:
    {
      TagDetected = true;
        
      if(terminal_msg_flag == true )
      {
        terminal_msg_flag = false ;
        /*---HT UI msg----------*/
        SerialPort.print( "\r\n\r\nTRACK_NFCTYPE1 NFC tag detected nearby");
        digitalWrite(X_NUCLEO_NFC03A1_LED2, HIGH);
      }
        
    }
    break;
    case TRACK_NFCTYPE2:
    {	
      TagDetected = true;
        
      if(terminal_msg_flag == true )
      {
        terminal_msg_flag = false ;
          
        /*---HT UI msg----------*/
        SerialPort.print( "\r\n\r\nTRACK_NFCTYPE2 NFC tag detected nearby");
        digitalWrite(X_NUCLEO_NFC03A1_LED2, HIGH);
      }
    }
    break;
      
    case TRACK_NFCTYPE3:
    {
      TagDetected = true;
        
      if(terminal_msg_flag == true )
      {
        terminal_msg_flag = false ;
          
        /*---HT UI msg----------*/
        SerialPort.print( "\r\n\r\nTRACK_NFCTYPE3 NFC tag detected nearby");
        digitalWrite(X_NUCLEO_NFC03A1_LED2, HIGH);
      }
    }
    break;
      
    case TRACK_NFCTYPE4A:
    {
      TagDetected = true;
        
      if(terminal_msg_flag == true )
      {
        terminal_msg_flag = false ;
          
        /*---HT UI msg----------*/
        SerialPort.print( "\r\n\r\nTRACK_NFCTYPE4A NFC tag detected nearby");
        digitalWrite(X_NUCLEO_NFC03A1_LED2, HIGH);
      }
    }
    break;
      
    case TRACK_NFCTYPE4B:
    {
      TagDetected = true;
        
      if(terminal_msg_flag == true )
      {
        terminal_msg_flag = false ;
          
        /*---HT UI msg----------*/
        SerialPort.print( "\r\n\r\nTRACK_NFCTYPE4B NFC tag detected nearby");
        digitalWrite(X_NUCLEO_NFC03A1_LED2, HIGH);
      }
    }
    break;

    case TRACK_NFCTYPE5:
    {
      TagDetected = true;
        
      if(terminal_msg_flag == true )
      {
        terminal_msg_flag = false ;
          
        /*---HT UI msg----------*/
        SerialPort.print( "\r\n\r\nTRACK_NFCTYPE5 NFC tag detected nearby");
        digitalWrite(X_NUCLEO_NFC03A1_LED2, HIGH);
      }
    }
    break;
      
    default:
    {
      TagDetected = false;
      
      if(terminal_msg_flag == false)
      {
        terminal_msg_flag = true ;
        /*---HT UI msg----------*/
        SerialPort.print( "\r\n\r\nCurrently there is no NFC tag in the vicinity");
        digitalWrite(X_NUCLEO_NFC03A1_LED2, LOW);
        digitalWrite(X_NUCLEO_NFC03A1_LED3, LOW);
        digitalWrite(X_NUCLEO_NFC03A1_LED4, LOW);
      }
    }
    break;
  }
   
  delay(300);
    
  if (TagDetected == true)
  {       
    TagDetected = false;
      
    /* Fill the structure of the NDEF URI */
    strcpy(url.Information,"ST website for near field communication");
    strcpy(url.protocol,"http://");
    strcpy(url.URI_Message,"st.com/st25");

    status = NDEF_WriteURI(&url);
      
    delay(500);
      
    if(status == RESULTOK) /*---if URI write passed----------*/
    {
      status = ERRORCODE_GENERIC;
        
      snprintf( dataOut, 256, "\r\n\r\n--------------------\r\n*****URI Writer*****\r\n--------------------\r\nURI Information written successfully on the tag: \r\n     URI Information: [%s], \r\n     URI Protocol: [%s] ,  \r\n     URI Message: [%s]", (char *)url.Information, (char *)url.protocol, (char *)url.URI_Message );
      SerialPort.print( dataOut );

      digitalWrite(X_NUCLEO_NFC03A1_LED3, HIGH);
        
      memset(url.Information,'\0',400); /*Clear url buffer before reading*/
        
      if (TagType == TRACK_NFCTYPE1)
      {
        status = PCDNFCT1_ReadNDEF();
      } else if (TagType == TRACK_NFCTYPE2)
      {
        status = PCDNFCT2_ReadNDEF();
      } else if (TagType == TRACK_NFCTYPE3)
      {
        status = PCDNFCT3_ReadNDEF();
      } else if (TagType == TRACK_NFCTYPE4A || TagType == TRACK_NFCTYPE4B)
      {
        status = PCDNFCT4_ReadNDEF();
      } else if (TagType == TRACK_NFCTYPE5)
      {
        status = PCDNFCT5_ReadNDEF();
      }
        
      if ( status == RESULTOK )
      {
        status = ERRORCODE_GENERIC;
          
        memset(NDEF_Buffer,'\0',20); /* Avoid printing useless characters */
        status = NDEF_IdentifyNDEF( &RecordStruct, NDEF_Buffer);
          
        if(status == RESULTOK && RecordStruct.TypeLength != 0)
        {
          if (NDEF_ReadURI(&RecordStruct, &url)==RESULTOK) /*---if URI read passed---*/
          {
            snprintf( dataOut, 256, "\r\n\r\n--------------------\r\n*****URI Reader*****\r\n--------------------\r\nURI Information read successfully from the tag: \r\n     URI Information: [%s], \r\n     URI Protocol: [%s] ,  \r\n     URI Message: [%s]", (char *)url.Information, (char *)url.protocol, (char *)url.URI_Message );
              
            SerialPort.print( dataOut );

            digitalWrite(X_NUCLEO_NFC03A1_LED4, HIGH);
          }
        }
      }
    }
  }  
}

