/**
 ******************************************************************************
 * @file    X_NUCLEO_NFC03A1_HelloWorld.ino
 * @author  AST
 * @version V1.0.0
 * @date    2 September 2021
 * @brief   Arduino test application for the STMicrolectronics X-NUCLEO-NFC03A1
 *          NFC reader/writer expansion board.
 *          This application makes use of C++ classes obtained from the C
 *          components' drivers.
 ******************************************************************************
 * \attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 STMicroelectronics</center></h2>
 *
 * Licensed under ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        www.st.com/mix_myliberty
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

/*! \file
 *
 *  \author SRA
 *
 *  \brief Demo application
 *
 *  This demo shows how to poll for several types of NFC cards/devices and how
 *  to exchange data with these devices, using the RFAL library.
 *
 *  This demo does not fully implement the activities according to the standards,
 *  it performs the required to communicate with a card/device and retrieve
 *  its UID. Also blocking methods are used for data exchange which may lead to
 *  long periods of blocking CPU/MCU.
 *  For standard compliant example please refer to the Examples provided
 *  with the RFAL library.
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "nfc_utils.h"
#include "rfal_nfc.h"
#include "rfal_rfst25r95.h"
#include "ndef_class.h"
#include "ndef_t5t.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

#define SPI_MOSI D11
#define SPI_MISO D12
#define SPI_SCK D13
#define CS_PIN D10
#define LED_A_PIN D7
#define LED_B_PIN D6
#define LED_F_PIN D5
#define LED_V_PIN D4
#define IRQ_IN_PIN D8
#define IRQ_OUT_PIN D2
#define INTERFACE_PIN D9

/* Definition of possible states the demo state machine could have */
#define DEMO_ST_NOTINIT               0  /*!< Demo State:  Not initialized */
#define DEMO_ST_START_DISCOVERY       1  /*!< Demo State:  Start Discovery */
#define DEMO_ST_DISCOVERY             2  /*!< Demo State:  Discovery       */

#define NDEF_DEMO_READ              0U   /*!< NDEF menu read               */
#define NDEF_DEMO_WRITE_MSG1        1U   /*!< NDEF menu write 1 record     */
#define NDEF_DEMO_WRITE_MSG2        2U   /*!< NDEF menu write 2 records    */
#define NDEF_DEMO_FORMAT_TAG        3U   /*!< NDEF menu format tag         */

#if NDEF_FEATURE_FULL_API
#define NDEF_DEMO_MAX_FEATURES      4U   /*!< Number of menu items         */
#else
#define NDEF_DEMO_MAX_FEATURES      1U   /*!< Number of menu items         */
#endif /* NDEF_FEATURE_FULL_API */

#define NDEF_WRITE_FORMAT_TIMEOUT   10000U /*!< When write or format mode is selected, demo returns back to read mode after a timeout */
#define NDEF_LED_BLINK_DURATION       250U /*!< Led blink duration         */

#define DEMO_RAW_MESSAGE_BUF_LEN      8192 /*!< Raw message buffer len     */

#define DEMO_ST_MANUFACTURER_ID      0x02U /*!< ST Manufacturer ID         */

/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

/* P2P communication data */
static uint8_t NFCID3[] = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
static uint8_t GB[] = {0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03};

/* P2P communication data */
static uint8_t ndefLLCPSYMM[] = {0x00, 0x00};
static uint8_t ndefInit[] = {0x05, 0x20, 0x06, 0x0F, 0x75, 0x72, 0x6E, 0x3A, 0x6E, 0x66, 0x63, 0x3A, 0x73, 0x6E, 0x3A, 0x73, 0x6E, 0x65, 0x70, 0x02, 0x02, 0x07, 0x80, 0x05, 0x01, 0x02};
static const uint8_t ndefSnepPrefix[] = { 0x13, 0x20, 0x00, 0x10, 0x02, 0x00, 0x00, 0x00 };
static const uint8_t URL[] = "st.com";
static ndefConstBuffer bufURL = { URL, sizeof(URL) - 1 };
static uint8_t ndefUriBuffer[255];

static uint8_t *ndefStates[] = {
  (uint8_t *)"INVALID",
  (uint8_t *)"INITIALIZED",
  (uint8_t *)"READ/WRITE",
  (uint8_t *)"READ-ONLY"
};

static const uint8_t *ndefDemoFeatureDescription[NDEF_DEMO_MAX_FEATURES] = {
  (uint8_t *)"1. Tap a tag to read its content",
#if NDEF_FEATURE_FULL_API
  (uint8_t *)"2. Present a tag to write a Text record",
  (uint8_t *)"3. Present a tag to write a URI record and an Android Application record",
  (uint8_t *)"4. Present an ST tag to format",
#endif /*NDEF_FEATURE_FULL_API */
};

#if NDEF_FEATURE_FULL_API
static uint8_t ndefURI[]          = "st.com";
static uint8_t ndefTEXT[]         = "Welcome to ST NDEF demo";
static uint8_t ndefTextLangCode[] = "en";

static uint8_t ndefAndroidPackName[] = "com.st.st25nfc";
#endif /*NDEF_FEATURE_FULL_API */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */
static rfalNfcDiscoverParam discParam;
static uint8_t              state = DEMO_ST_NOTINIT;

static uint8_t              ndefDemoFeature     = NDEF_DEMO_READ;
static uint8_t              ndefDemoPrevFeature = 0xFF;
static bool                 verbose             = false;

static uint8_t              rawMessageBuf[DEMO_RAW_MESSAGE_BUF_LEN];

static uint32_t             timer;
static uint32_t             timerLed;
static bool                 ledOn;




#define MAX_HEX_STR         4
#define MAX_HEX_STR_LENGTH  128
char hexStr[MAX_HEX_STR][MAX_HEX_STR_LENGTH];
uint8_t hexStrIdx = 0;

int PushButtonState = 0;

/* SPI, Component and NFC */
SPIClass dev_spi(SPI_MOSI, SPI_MISO, SPI_SCK);
RfalRfST25R95Class rfst25r95(&dev_spi, CS_PIN, IRQ_IN_PIN, IRQ_OUT_PIN, INTERFACE_PIN);
RfalNfcClass rfal_nfc(&rfst25r95);
NdefClass ndef(&rfal_nfc);

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

static void demoNdef(rfalNfcDevice *nfcDevice);
static void ndefCCDump(void);
#if NDEF_FEATURE_T5T
static void ndefDumpSysInfo(void);
#endif /* RFAL_FEATURE_T5T */

#if NDEF_FEATURE_FULL_API
static bool ndefIsSTTag(void);
static void LedNotificationWriteDone(void);
#endif /*NDEF_FEATURE_FULL_API */

static void demoP2P(void);
ReturnCode  demoTransceiveBlocking(uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxBuf, uint16_t **rcvLen, uint32_t fwt);

static void ledsOn(void);
static void ledsOff(void);

static ReturnCode ndefRecordDump(const ndefRecord *record, bool verbose);
static ReturnCode ndefMessageDump(const ndefMessage *message, bool verbose);
static ReturnCode ndefEmptyTypeDump(const ndefType *empty);
static ReturnCode ndefRtdDeviceInfoDump(const ndefType *devInfo);
static ReturnCode ndefRtdTextDump(const ndefType *text);
static ReturnCode ndefRtdUriDump(const ndefType *uri);
static ReturnCode ndefRtdAarDump(const ndefType *ext);
static ReturnCode ndefMediaVCardDump(const ndefType *vCard);
static ReturnCode ndefMediaWifiDump(const ndefType *wifi);
static ReturnCode ndefRecordDumpType(const ndefRecord *record);
static ReturnCode ndefBufferDump(const char *string, const ndefConstBuffer *bufPayload, bool verbose);
static ReturnCode ndefBufferPrint(const char *prefix, const ndefConstBuffer *bufPayload, const char *suffix);
static ReturnCode ndefBuffer8Print(const char *prefix, const ndefConstBuffer8 *bufPayload, const char *suffix);

/*! Table to associate enums to pointer to function */
typedef struct {
  ndefTypeId typeId;                        /*!< NDEF type Id             */
  ReturnCode(*dump)(const ndefType *type);  /*!< Pointer to dump function */
} ndefTypeDumpTable;

#if NDEF_TYPE_VCARD_SUPPORT
/*! Type to associate a property and string description */
typedef struct {
  ndefConstBuffer bufType;
  char *string;
} ndefVCardTypeTable;
#endif

static const ndefTypeDumpTable typeDumpTable[] = {
  { NDEF_TYPE_ID_EMPTY,           ndefEmptyTypeDump       },
  { NDEF_TYPE_ID_RTD_DEVICE_INFO, ndefRtdDeviceInfoDump   },
  { NDEF_TYPE_ID_RTD_TEXT,        ndefRtdTextDump         },
  { NDEF_TYPE_ID_RTD_URI,         ndefRtdUriDump          },
  { NDEF_TYPE_ID_RTD_AAR,         ndefRtdAarDump          },
  { NDEF_TYPE_ID_MEDIA_VCARD,     ndefMediaVCardDump      },
  { NDEF_TYPE_ID_MEDIA_WIFI,      ndefMediaWifiDump       },
};

static char *hex2Str(unsigned char *data, size_t dataLen);


/*!
 *****************************************************************************
 * \brief Check user button
 *
 *  This function check whether the user button has been pressed
 *****************************************************************************
 */

static void checkUserButton(void)
{
  /* Check if USER button is pressed */
  if (digitalRead(USER_BTN) == PushButtonState) {
    ndefDemoFeature++;
    ndefDemoFeature %= NDEF_DEMO_MAX_FEATURES;

    ledsOff();
    ndefDemoPrevFeature = ndefDemoFeature;
    Serial.print((char *)ndefDemoFeatureDescription[ndefDemoFeature]);
    Serial.print("\r\n");

    /* Debounce button */
    delay(50);

    /* Wait until the button is released */
    while ((digitalRead(USER_BTN) == PushButtonState));

    /* Debouncing */
    delay(50);

    if (ndefDemoFeature != NDEF_DEMO_READ) {
      timer = rfst25r95.timerCalculateTimer(NDEF_WRITE_FORMAT_TIMEOUT);
      timerLed = rfst25r95.timerCalculateTimer(NDEF_LED_BLINK_DURATION);
    }
  }
}

/*!
 *****************************************************************************
 * \brief Show usage
 *
 *  This function displays usage information
 *****************************************************************************
 */
static void ndefShowDemoUsage()
{
#if NDEF_FEATURE_FULL_API
  uint32_t i;

  Serial.print("Use the User button to cycle among the different modes:\r\n");
  for (i = 0; i < SIZEOF_ARRAY(ndefDemoFeatureDescription); i++) {
    Serial.print((char *)ndefDemoFeatureDescription[i]);
    Serial.print("\r\n");
  }
  Serial.print("In Write or Format mode (menu 2, 3 or 4), the demo returns to Read mode (menu 1) if no tag detected after ");
  Serial.print((NDEF_WRITE_FORMAT_TIMEOUT / 1000));
  Serial.print(" seconds\r\n\n");
#endif /*NDEF_FEATURE_FULL_API */
}

void setup()
{
  ReturnCode err;

  Serial.begin(115200);
  dev_spi.begin();

  pinMode(LED_A_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  pinMode(LED_F_PIN, OUTPUT);
  pinMode(LED_V_PIN, OUTPUT);
  pinMode(USER_BTN, INPUT);

  /* Check what is the Push Button State when the button is not pressed. It can change across families */
  PushButtonState = (digitalRead(USER_BTN)) ?  0 : 1;

  Serial.println("Welcome to X-NUCLEO-NFC03A1");

  ndefShowDemoUsage();

  err = rfal_nfc.rfalNfcInitialize();
  if( err == ERR_NONE ) {
    discParam.compMode      = RFAL_COMPLIANCE_MODE_NFC;
    discParam.devLimit      = 1U;
    discParam.nfcfBR        = RFAL_BR_212;
    discParam.ap2pBR        = RFAL_BR_424;

    ST_MEMCPY(&discParam.nfcid3, NFCID3, sizeof(NFCID3));
    ST_MEMCPY(&discParam.GB, GB, sizeof(GB));
    discParam.GBLen         = sizeof(GB);

    discParam.notifyCb             = NULL;
    discParam.totalDuration        = 1000U;
    discParam.wakeupEnabled        = false;
    discParam.wakeupConfigDefault  = true;
#if RFAL_FEATURE_NFCA
    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_A;
#endif /* RFAL_FEATURE_NFCA */

#if RFAL_FEATURE_NFCB
    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_B;
#endif /* RFAL_FEATURE_NFCB */

#if RFAL_FEATURE_NFCF
    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_F;
#endif /* RFAL_FEATURE_NFCF */

#if RFAL_FEATURE_NFCV
    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_V;
#endif /* RFAL_FEATURE_NFCV */

#if RFAL_FEATURE_ST25TB
    discParam.techs2Find          |= RFAL_NFC_POLL_TECH_ST25TB;
#endif /* RFAL_FEATURE_ST25TB */
  
  discParam.isoDepFS           = RFAL_ISODEP_FSXI_128;          /* ST25R95 cannot support 256 bytes of data block */

    state = DEMO_ST_START_DISCOVERY;
  } else {
    Serial.println("Initialize ERROR: " + String(err));
  }
}

void loop()
{
  static rfalNfcDevice *nfcDevice;

#if RFAL_FEATURE_NFCA
  rfalNfcaSensRes       sensRes;
  rfalNfcaSelRes        selRes;
#endif /* RFAL_FEATURE_NFCA */

#if RFAL_FEATURE_NFCB
  rfalNfcbSensbRes      sensbRes;
  uint8_t               sensbResLen;
#endif /* RFAL_FEATURE_NFCB */

#if RFAL_FEATURE_NFCF
  uint8_t               devCnt = 0;
  rfalFeliCaPollRes     cardList[1];
  uint8_t               collisions = 0U;
  rfalNfcfSensfRes     *sensfRes;
#endif /* RFAL_FEATURE_NFCF */

#if RFAL_FEATURE_NFCV
  rfalNfcvInventoryRes  invRes;
  uint16_t              rcvdLen;
#endif /* RFAL_FEATURE_NFCV */

  rfal_nfc.rfalNfcWorker();                                    /* Run RFAL worker periodically */

  if ((ndefDemoFeature != NDEF_DEMO_READ) && (rfst25r95.timerIsExpired(timer))) {
    Serial.print("Timer expired, back to Read mode...\r\n");
    ledsOff();
    ndefDemoFeature = NDEF_DEMO_READ;
  }

  if (ndefDemoFeature != ndefDemoPrevFeature) {
    ndefDemoPrevFeature = ndefDemoFeature;
    Serial.print((char *)ndefDemoFeatureDescription[ndefDemoFeature]);
    Serial.print("\r\n");
  }

  if (ndefDemoFeature != NDEF_DEMO_READ) {
    if (rfst25r95.timerIsExpired(timerLed)) {
      timerLed = rfst25r95.timerCalculateTimer(NDEF_LED_BLINK_DURATION);
      ledOn = !ledOn;
    }
    if (ledOn) {
      ledsOn();
    } else {
      ledsOff();
    }
  }

  checkUserButton();

  switch (state) {
    /*******************************************************************************/
    case DEMO_ST_START_DISCOVERY:
      ledsOff();

      rfal_nfc.rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_IDLE);
      rfal_nfc.rfalNfcDiscover(&discParam);

      state = DEMO_ST_DISCOVERY;
      break;

    /*******************************************************************************/
    case DEMO_ST_DISCOVERY:
      if (rfalNfcIsDevActivated(rfal_nfc.rfalNfcGetState())) {
        rfal_nfc.rfalNfcGetActiveDevice(&nfcDevice);

        ledsOff();
        delay(50);
        ndefDemoPrevFeature = 0xFF; /* Force the display of the prompt */
        switch (nfcDevice->type) {
          /*******************************************************************************/
#if RFAL_FEATURE_NFCA
          case RFAL_NFC_LISTEN_TYPE_NFCA:

            digitalWrite(LED_A_PIN, HIGH);
            switch (nfcDevice->dev.nfca.type) {
              case RFAL_NFCA_T1T:
                Serial.print("ISO14443A/Topaz (NFC-A T1T) TAG found. UID: ");
                Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                Serial.print("\r\n");
                rfal_nfc.rfalNfcaPollerSleep();
                break;

              case RFAL_NFCA_T4T:
                Serial.print("NFCA Passive ISO-DEP device found. UID: ");
                Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                Serial.print("\r\n");
                demoNdef(nfcDevice);
                rfal_nfc.rfalIsoDepDeselect();
                break;

              case RFAL_NFCA_T4T_NFCDEP:
              case RFAL_NFCA_NFCDEP:
                Serial.print("NFCA Passive P2P device found. NFCID: ");
                Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                Serial.print("\r\n");
                demoP2P();
                break;

              default:
                Serial.print("ISO14443A/NFC-A card found. UID: ");
                Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                Serial.print("\r\n");
                demoNdef(nfcDevice);
                rfal_nfc.rfalNfcaPollerSleep();
                break;
            }
            /* Loop until tag is removed from the field */
            Serial.print("Operation completed\r\nTag can be removed from the field\r\n");
            rfal_nfc.rfalNfcaPollerInitialize();
            while (rfal_nfc.rfalNfcaPollerCheckPresence(RFAL_14443A_SHORTFRAME_CMD_WUPA, &sensRes) == ERR_NONE) {
              if (((nfcDevice->dev.nfca.type == RFAL_NFCA_T1T) && (!rfalNfcaIsSensResT1T(&sensRes))) ||
                  ((nfcDevice->dev.nfca.type != RFAL_NFCA_T1T) && (rfal_nfc.rfalNfcaPollerSelect(nfcDevice->dev.nfca.nfcId1, nfcDevice->dev.nfca.nfcId1Len, &selRes) != ERR_NONE))) {
                break;
              }
              rfal_nfc.rfalNfcaPollerSleep();
              delay(130);
            }
            break;
#endif /* RFAL_FEATURE_NFCA */

          /*******************************************************************************/
#if RFAL_FEATURE_NFCB
          case RFAL_NFC_LISTEN_TYPE_NFCB:

            Serial.print("ISO14443B/NFC-B card found. UID: ");
            Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
            Serial.print("\r\n");
            digitalWrite(LED_B_PIN, HIGH);

            if (rfalNfcbIsIsoDepSupported(&nfcDevice->dev.nfcb)) {
              demoNdef(nfcDevice);
              rfal_nfc.rfalIsoDepDeselect();
            } else {
              rfal_nfc.rfalNfcbPollerSleep(nfcDevice->dev.nfcb.sensbRes.nfcid0);
            }
            /* Loop until tag is removed from the field */
            Serial.print("Operation completed\r\nTag can be removed from the field\r\n");
            rfal_nfc.rfalNfcbPollerInitialize();
            while (rfal_nfc.rfalNfcbPollerCheckPresence(RFAL_NFCB_SENS_CMD_ALLB_REQ, RFAL_NFCB_SLOT_NUM_1, &sensbRes, &sensbResLen) == ERR_NONE) {
              if (ST_BYTECMP(sensbRes.nfcid0, nfcDevice->dev.nfcb.sensbRes.nfcid0, RFAL_NFCB_NFCID0_LEN) != 0) {
                break;
              }
              rfal_nfc.rfalNfcbPollerSleep(nfcDevice->dev.nfcb.sensbRes.nfcid0);
              delay(130);
            }
            break;
#endif /* RFAL_FEATURE_NFCB */

          /*******************************************************************************/
#if RFAL_FEATURE_NFCF
          case RFAL_NFC_LISTEN_TYPE_NFCF:

            if (rfalNfcfIsNfcDepSupported(&nfcDevice->dev.nfcf)) {
              Serial.print("NFCF Passive P2P device found. NFCID: ");
              Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
              Serial.print("\r\n");
              demoP2P();
            } else {
              Serial.print("Felica/NFC-F card found. UID: ");
              Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
              Serial.print("\r\n");
              demoNdef(nfcDevice);
            }

            digitalWrite(LED_F_PIN, HIGH);
            /* Loop until tag is removed from the field */
            Serial.print("Operation completed\r\nTag can be removed from the field\r\n");
            devCnt = 1;
            rfal_nfc.rfalNfcfPollerInitialize(RFAL_BR_212);
            while (rfal_nfc.rfalNfcfPollerPoll(RFAL_FELICA_1_SLOT, RFAL_NFCF_SYSTEMCODE, RFAL_FELICA_POLL_RC_NO_REQUEST, cardList, &devCnt, &collisions) == ERR_NONE) {
              /* Skip the length field byte */
              sensfRes = (rfalNfcfSensfRes *) & ((uint8_t *)cardList)[1];
              if (ST_BYTECMP(sensfRes->NFCID2, nfcDevice->dev.nfcf.sensfRes.NFCID2, RFAL_NFCF_NFCID2_LEN) != 0) {
                break;
              }
              delay(130);
            }
            break;
#endif /* RFAL_FEATURE_NFCF */

          /*******************************************************************************/
#if RFAL_FEATURE_NFCV
          case RFAL_NFC_LISTEN_TYPE_NFCV: {
              uint8_t devUID[RFAL_NFCV_UID_LEN];

              ST_MEMCPY(devUID, nfcDevice->nfcid, nfcDevice->nfcidLen);     /* Copy the UID into local var */
              REVERSE_BYTES(devUID, RFAL_NFCV_UID_LEN);                   /* Reverse the UID for display purposes */
              Serial.print("ISO15693/NFC-V card found. UID: ");
              Serial.print(hex2Str(devUID, RFAL_NFCV_UID_LEN));
              Serial.print("\r\n");

              digitalWrite(LED_V_PIN, HIGH);

              demoNdef(nfcDevice);

              /* Loop until tag is removed from the field */
              Serial.print("Operation completed\r\nTag can be removed from the field\r\n");
              rfal_nfc.rfalNfcvPollerInitialize();
              while (rfal_nfc.rfalNfcvPollerInventory(RFAL_NFCV_NUM_SLOTS_1, RFAL_NFCV_UID_LEN * 8U, nfcDevice->dev.nfcv.InvRes.UID, &invRes, &rcvdLen) == ERR_NONE) {
                delay(130);
              }
            }
            break;
#endif /* RFAL_FEATURE_NFCV */

          /*******************************************************************************/
#if RFAL_FEATURE_ST25TB
          case RFAL_NFC_LISTEN_TYPE_ST25TB:

            Serial.print("ST25TB card found. UID: ");
            Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
            Serial.print("\r\n");
            digitalWrite(LED_B_PIN, HIGH);
            break;
#endif /* RFAL_FEATURE_ST25TB */

          /*******************************************************************************/
          case RFAL_NFC_LISTEN_TYPE_AP2P:

            Serial.print("NFC Active P2P device found. NFCID3: ");
            Serial.print(hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
            Serial.print("\r\n");

            demoP2P();
            break;

          /*******************************************************************************/
          default:
            break;
        }

        rfal_nfc.rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_IDLE);
        delay(500);
        state = DEMO_ST_START_DISCOVERY;
      }
      break;

    /*******************************************************************************/
    case DEMO_ST_NOTINIT:
    default:
      break;
  }
}


/*!
 *****************************************************************************
 * \brief Demo P2P Exchange
 *
 * Sends a NDEF URI record 'http://www.ST.com' via NFC-DEP (P2P) protocol.
 *
 * This method sends a set of static predefined frames which tries to establish
 * a LLCP connection, followed by the NDEF record, and then keeps sending
 * LLCP SYMM packets to maintain the connection.
 *
 *
 *****************************************************************************
 */
void demoP2P(void)
{
  #if RFAL_FEATURE_NFC_DEP && NDEF_FEATURE_FULL_API
  uint16_t   *rxLen;
  uint8_t    *rxData;
  ReturnCode err;

  ndefBuffer  bufPayload;
  ndefMessage message;
  ndefRecord  record;
  ndefType    uri;

  Serial.print(" Initialize device .. ");
  err = demoTransceiveBlocking(ndefInit, sizeof(ndefInit), &rxData, &rxLen, RFAL_FWT_NONE);
  if (err != ERR_NONE) {
    Serial.print("failed.");
    return;
  }
  Serial.print("succeeded.\r\n");

  err  = ndefRtdUriInit(&uri, NDEF_URI_PREFIX_HTTP_WWW, &bufURL);
  err |= ndefRtdUriToRecord(&uri, &record);

  err |= ndefMessageInit(&message);
  err |= ndefMessageAppend(&message, &record);  /* To get MB and ME bits set */

  /* Build the SNEP buffer made of the prefix, the length byte and the record */
  ST_MEMCPY(ndefUriBuffer, ndefSnepPrefix, sizeof(ndefSnepPrefix));

  /* Skip 1 byte for length byte */
  bufPayload.buffer = ndefUriBuffer + sizeof(ndefSnepPrefix) + 1;
  bufPayload.length = sizeof(ndefUriBuffer) - sizeof(ndefSnepPrefix);
  err |= ndefMessageEncode(&message, &bufPayload);

  ndefUriBuffer[sizeof(ndefSnepPrefix)] = bufPayload.length;

  bufPayload.buffer = ndefUriBuffer;
  bufPayload.length = sizeof(ndefSnepPrefix) + 1 + bufPayload.length;

  if (err != ERR_NONE) {
    Serial.print("NDEF message creation failed\r\n");
    return;
  }

  ndefBufferDump("URL converted to SNEP:\r\n", (ndefConstBuffer *)&bufPayload, true);

  Serial.print(" Push NDEF Uri: www.ST.com .. ");
  err = demoTransceiveBlocking(bufPayload.buffer, bufPayload.length, &rxData, &rxLen, RFAL_FWT_NONE);
  if (err != ERR_NONE) {
    Serial.print("failed.");
    return;
  }
  Serial.print("succeeded.\r\n");


  Serial.print(" Device present, maintaining connection ");
  while (err == ERR_NONE) {
    err = demoTransceiveBlocking(ndefLLCPSYMM, sizeof(ndefLLCPSYMM), &rxData, &rxLen, RFAL_FWT_NONE);
    Serial.print(".");
    delay(50);
  }
  Serial.print("\r\n Device removed.\r\n");
#endif /* RFAL_FEATURE_NFC_DEP */
}

/*!
 *****************************************************************************
 * \brief Demo Blocking Transceive
 *
 * Helper function to send data in a blocking manner via the rfalNfc module
 *
 * \warning A protocol transceive handles long timeouts (several seconds),
 * transmission errors and retransmissions which may lead to a long period of
 * time where the MCU/CPU is blocked in this method.
 * This is a demo implementation, for a non-blocking usage example please
 * refer to the Examples available with RFAL
 *
 * \param[in]  txBuf      : data to be transmitted
 * \param[in]  txBufSize  : size of the data to be transmitted
 * \param[out] rxData     : location where the received data has been placed
 * \param[out] rcvLen     : number of data bytes received
 * \param[in]  fwt        : FWT to be used (only for RF frame interface,
 *                                          otherwise use RFAL_FWT_NONE)
 *
 *
 *  \return ERR_PARAM     : Invalid parameters
 *  \return ERR_TIMEOUT   : Timeout error
 *  \return ERR_FRAMING   : Framing error detected
 *  \return ERR_PROTO     : Protocol error detected
 *  \return ERR_NONE      : No error, activation successful
 *
 *****************************************************************************
 */
ReturnCode demoTransceiveBlocking(uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxData, uint16_t **rcvLen, uint32_t fwt)
{
  ReturnCode err;

  err = rfal_nfc.rfalNfcDataExchangeStart(txBuf, txBufSize, rxData, rcvLen, fwt);
  if (err == ERR_NONE) {
    do {
      rfal_nfc.rfalNfcWorker();
      err = rfal_nfc.rfalNfcDataExchangeGetStatus();
    } while (err == ERR_BUSY);
  }
  return err;
}

static void demoNdef(rfalNfcDevice *pNfcDevice)
{
  ReturnCode       err;
  ndefMessage      message;
  uint32_t         rawMessageLen;
  ndefInfo         info;
  ndefBuffer       bufRawMessage;
  ndefConstBuffer  bufConstRawMessage;

  #if NDEF_FEATURE_FULL_API
  ndefRecord       record1;
  ndefRecord       record2;

  ndefType         text;
  ndefType         uri;
  ndefType         aar;

  ndefConstBuffer8 bufTextLangCode;
  ndefConstBuffer bufTextLangText;
  ndefConstBuffer bufUri;
  ndefConstBuffer bufAndroidPackName;
#endif /*NDEF_FEATURE_FULL_API */

  /*
   * Perform NDEF Context Initialization
   */
  err = ndef.ndefPollerContextInitializationWrapper(pNfcDevice);
  if (err != ERR_NONE) {
    Serial.print("NDEF NOT DETECTED (ndefPollerContextInitializationWrapper returns ");
    Serial.print(err);
    Serial.print(")\r\n");
    return;
  }

#if NDEF_FEATURE_T5T
  if (verbose & (pNfcDevice->type == RFAL_NFC_LISTEN_TYPE_NFCV)) {
    ndefDumpSysInfo();
  }
#endif /* RFAL_FEATURE_T5T */

  /*
   * Perform NDEF Detect procedure
   */
  err = ndef.ndefPollerNdefDetectWrapper(&info);
  if (err != ERR_NONE) {
    Serial.print("NDEF NOT DETECTED (ndefPollerNdefDetectWrapper returns ");
    Serial.print(err);
    Serial.print(")\r\n");
    if (ndefDemoFeature != NDEF_DEMO_FORMAT_TAG) {
      return;
    }
  } else {
    Serial.print((char *)ndefStates[info.state]);
    Serial.print(" NDEF detected.\r\n");
    ndefCCDump();

    if (verbose) {
      Serial.print("NDEF Len: ");
      Serial.print(ndef.ctx.messageLen);
      Serial.print(", Offset=");
      Serial.print(ndef.ctx.messageOffset);
      Serial.print("\r\n");
    }
  }

  switch (ndefDemoFeature) {
    /*
     * Demonstrate how to read the NDEF message from the Tag
     */
    case NDEF_DEMO_READ:
      if (info.state == NDEF_STATE_INITIALIZED) {
        /* Nothing to read... */
        return;
      }
      err = ndef.ndefPollerReadRawMessageWrapper(rawMessageBuf, sizeof(rawMessageBuf), &rawMessageLen, true);
      if (err != ERR_NONE) {
        Serial.print("NDEF message cannot be read (ndefPollerReadRawMessageWrapper returns ");
        Serial.print(err);
        Serial.print(")\r\n");
        return;
      }
      if (verbose) {
        bufRawMessage.buffer = rawMessageBuf;
        bufRawMessage.length = rawMessageLen;
        ndefBufferDump(" NDEF Content", (ndefConstBuffer *)&bufRawMessage, verbose);
      }
      bufConstRawMessage.buffer = rawMessageBuf;
      bufConstRawMessage.length = rawMessageLen;
      err = ndefMessageDecode(&bufConstRawMessage, &message);
      if (err != ERR_NONE) {
        Serial.print("NDEF message cannot be decoded (ndefMessageDecode  returns ");
        Serial.print(err);
        Serial.print(")\r\n");
        return;
      }
      err = ndefMessageDump(&message, verbose);
      if (err != ERR_NONE) {
        Serial.print("NDEF message cannot be displayed (ndefMessageDump returns ");
        Serial.print(err);
        Serial.print(")\r\n");
        return;
      }
      break;

#if NDEF_FEATURE_FULL_API
    /*
     * Demonstrate how to encode a text record and write the message to the tag
     */
    case NDEF_DEMO_WRITE_MSG1:
      ndefDemoFeature = NDEF_DEMO_READ; /* returns to READ mode after write */
      err  = ndefMessageInit(&message); /* Initialize message structure */
      bufTextLangCode.buffer = ndefTextLangCode;
      bufTextLangCode.length = strlen((char *)ndefTextLangCode);

      bufTextLangText.buffer = ndefTEXT;
      bufTextLangText.length = strlen((char *)ndefTEXT);

      err |= ndefRtdTextInit(&text, TEXT_ENCODING_UTF8, &bufTextLangCode, &bufTextLangText); /* Initialize Text type structure */
      err |= ndefRtdTextToRecord(&text, &record1); /* Encode Text Record */
      err |= ndefMessageAppend(&message, &record1); /* Append Text record to message */
      if (err != ERR_NONE) {
        Serial.print("Message creation failed\r\n");
        return;
      }
      err = ndef.ndefPollerWriteMessageWrapper(&message); /* Write message */
      if (err != ERR_NONE) {
        Serial.print("Message cannot be written (ndefPollerWriteMessageWrapper return ");
        Serial.print(err);
        Serial.print(")\r\n");
        return;
      }
      Serial.print("Wrote 1 record to the Tag\r\n");
      if (verbose) {
        /* Dump raw message */
        bufRawMessage.buffer = rawMessageBuf;
        bufRawMessage.length = sizeof(rawMessageBuf);
        err = ndefMessageEncode(&message, &bufRawMessage);
        if (err == ERR_NONE) {
          ndefBufferDump("Raw message", (ndefConstBuffer *)&bufRawMessage, verbose);
        }
      }
      LedNotificationWriteDone();
      break;

    /*
     * Demonstrate how to encode a URI record and a AAR record, how to encode the message to a raw buffer and then how to write the raw buffer
     */
    case NDEF_DEMO_WRITE_MSG2:
      ndefDemoFeature = NDEF_DEMO_READ;  /* returns to READ mode after write */
      err  = ndefMessageInit(&message);  /* Initialize message structure */
      bufUri.buffer = ndefURI;
      bufUri.length = strlen((char *)ndefURI);
      err |= ndefRtdUriInit(&uri, NDEF_URI_PREFIX_HTTP_WWW, &bufUri); /* Initialize URI type structure */
      err |= ndefRtdUriToRecord(&uri, &record1); /* Encode URI Record */

      bufAndroidPackName.buffer = ndefAndroidPackName;
      bufAndroidPackName.length = sizeof(ndefAndroidPackName) - 1U;
      err |= ndefRtdAarInit(&aar, &bufAndroidPackName); /* Initialize AAR type structure */
      err |= ndefRtdAarToRecord(&aar, &record2); /* Encode AAR record */

      err |= ndefMessageAppend(&message, &record1); /* Append URI to message */
      err |= ndefMessageAppend(&message, &record2); /* Append AAR to message (record #2 is an example of preformatted record) */

      bufRawMessage.buffer = rawMessageBuf;
      bufRawMessage.length = sizeof(rawMessageBuf);
      err |= ndefMessageEncode(&message, &bufRawMessage); /* Encode the message to the raw buffer */
      if (err != ERR_NONE) {
        Serial.print("Raw message creation failed\r\n");
        return;
      }
      err = ndef.ndefPollerWriteRawMessageWrapper(bufRawMessage.buffer, bufRawMessage.length);
      if (err != ERR_NONE) {
        Serial.print("Message cannot be written (ndefPollerWriteRawMessageWrapper return ");
        Serial.print(err);
        Serial.print(")\r\n");
        return;
      }
      Serial.print("Wrote 2 records to the Tag\r\n");
      if (verbose) {
        /* Dump raw message */
        ndefBufferDump("Raw message", (ndefConstBuffer *)&bufRawMessage, verbose);
      }
      LedNotificationWriteDone();
      break;

    /*
     * Demonstrate how to format a Tag
     */
    case NDEF_DEMO_FORMAT_TAG:
      ndefDemoFeature = NDEF_DEMO_READ;
      if (!ndefIsSTTag()) {
        Serial.print("Manufacturer ID not found or not an ST tag. Format aborted \r\n");
        return;
      }
      Serial.print("Formatting Tag...\r\n");
      /* Format Tag */
      err = ndef.ndefPollerTagFormatWrapper(NULL, 0);
      if (err != ERR_NONE) {
        Serial.print("Tag cannot be formatted (ndefPollerTagFormatWrapper returns ");
        Serial.print(err);
        Serial.print(")\r\n");
        return;
      }
      Serial.print("Tag formatted\r\n");
      LedNotificationWriteDone();
      break;
#endif /*NDEF_FEATURE_FULL_API */

    default:
      ndefDemoFeature = NDEF_DEMO_READ;
      break;
  }
  return;
}

#if NDEF_FEATURE_T2T
static void ndefT2TCCDump()
{
  ndefConstBuffer bufCcBuf;

  Serial.print(" * Magic: ");
  Serial.print(ndef.ctx.cc.t2t.magicNumber, HEX);
  Serial.print("h Version: ");
  Serial.print(ndef.ctx.cc.t2t.majorVersion);
  Serial.print(".");
  Serial.print(ndef.ctx.cc.t2t.minorVersion);
  Serial.print(" Size: ");
  Serial.print(ndef.ctx.cc.t2t.size);
  Serial.print(" (");
  Serial.print((ndef.ctx.cc.t2t.size * 8U));
  Serial.print(" bytes) \r\n * readAccess: ");
  Serial.print(ndef.ctx.cc.t2t.readAccess, HEX);
  Serial.print("h writeAccess: ");
  Serial.print(ndef.ctx.cc.t2t.writeAccess, HEX);
  Serial.print("h \r\n");
  bufCcBuf.buffer = ndef.ctx.ccBuf;
  bufCcBuf.length = 4;
  ndefBufferDump(" CC Raw Data", &bufCcBuf, verbose);

}
#endif /* NDEF_FEATURE_T2T */

#if NDEF_FEATURE_T3T
static void ndefT3TAIBDump()
{
  ndefConstBuffer bufCcBuf;

  Serial.print(" * Version: ");
  Serial.print(ndef.ctx.cc.t3t.majorVersion);
  Serial.print(".");
  Serial.print(ndef.ctx.cc.t3t.minorVersion);
  Serial.print(" Size: ");
  Serial.print(ndef.ctx.cc.t3t.nMaxB);
  Serial.print(" (");
  Serial.print((ndef.ctx.cc.t3t.nMaxB * 16U));
  Serial.print(" bytes) NbR: ");
  Serial.print(ndef.ctx.cc.t3t.nbR);
  Serial.print(" NbW: ");
  Serial.print(ndef.ctx.cc.t3t.nbW);
  Serial.print("\r\n * WriteFlag: ");
  Serial.print(ndef.ctx.cc.t3t.writeFlag, HEX);
  Serial.print("h RWFlag: ");
  Serial.print(ndef.ctx.cc.t3t.rwFlag, HEX);
  Serial.print("h \r\n");
  bufCcBuf.buffer = ndef.ctx.ccBuf;
  bufCcBuf.length = 16;
  ndefBufferDump(" CC Raw Data", &bufCcBuf, verbose);
}
#endif /* NDEF_FEATURE_T3T */

#if NDEF_FEATURE_T4T
static void ndefT4TCCDump()
{
  ndefConstBuffer bufCcBuf;

  Serial.print(" * CCLEN: ");
  Serial.print(ndef.ctx.cc.t4t.ccLen);
  Serial.print(" T4T_VNo: ");
  Serial.print(ndef.ctx.cc.t4t.vNo, HEX);
  Serial.print("h MLe: ");
  Serial.print(ndef.ctx.cc.t4t.mLe);
  Serial.print(" MLc: ");
  Serial.print(ndef.ctx.cc.t4t.mLc);
  Serial.print(" FileId: ");
  Serial.print(ndef.ctx.cc.t4t.fileId[0], HEX);
  Serial.print(ndef.ctx.cc.t4t.fileId[1], HEX);
  Serial.print("h FileSize: ");
  Serial.print(ndef.ctx.cc.t4t.fileSize);
  Serial.print("\r\n * readAccess: ");
  Serial.print(ndef.ctx.cc.t4t.readAccess, HEX);
  Serial.print("h writeAccess: ");
  Serial.print(ndef.ctx.cc.t4t.writeAccess, HEX);
  Serial.print("h\r\n");
  bufCcBuf.buffer = ndef.ctx.ccBuf;
  bufCcBuf.length = ndef.ctx.cc.t4t.ccLen;
  ndefBufferDump(" CC File Raw Data", &bufCcBuf, verbose);
}
#endif /* NDEF_FEATURE_T4T */

#if NDEF_FEATURE_T5T
static void ndefT5TCCDump()
{
  ndefConstBuffer bufCcBuf;

  Serial.print(" * Block Length: ");
  Serial.print(ndef.ctx.subCtx.t5t.blockLen);
  Serial.print("\r\n");
  Serial.print(" * ");
  Serial.print(ndef.ctx.cc.t5t.ccLen);
  Serial.print(" bytes CC\r\n * Magic: ");
  Serial.print(ndef.ctx.cc.t5t.magicNumber, HEX);
  Serial.print("h Version: ");
  Serial.print(ndef.ctx.cc.t5t.majorVersion);
  Serial.print(".");
  Serial.print(ndef.ctx.cc.t5t.minorVersion);
  Serial.print(" MLEN: ");
  Serial.print(ndef.ctx.cc.t5t.memoryLen);
  Serial.print(" (");
  Serial.print((ndef.ctx.cc.t5t.memoryLen * 8U));
  Serial.print(" bytes) \r\n * readAccess: ");
  Serial.print(ndef.ctx.cc.t5t.readAccess, HEX);
  Serial.print("h writeAccess: ");
  Serial.print(ndef.ctx.cc.t5t.writeAccess, HEX);
  Serial.print("h \r\n");
  Serial.print(" * [");
  Serial.print((ndef.ctx.cc.t5t.specialFrame ? 'X' : ' '));
  Serial.print("] Special Frame\r\n");
  Serial.print(" * [");
  Serial.print((ndef.ctx.cc.t5t.multipleBlockRead ? 'X' : ' '));
  Serial.print("] Multiple block Read\r\n");
  Serial.print(" * [");
  Serial.print((ndef.ctx.cc.t5t.lockBlock ? 'X' : ' '));
  Serial.print("] Lock Block\r\n");
  bufCcBuf.buffer = ndef.ctx.ccBuf;
  bufCcBuf.length = ndef.ctx.cc.t5t.ccLen;
  ndefBufferDump(" CC Raw Data", &bufCcBuf, verbose);
}
#endif /* NDEF_FEATURE_T5T */

static void ndefCCDump()
{
  if (!verbose) {
    return;
  }
  Serial.print(((ndef.ctx.device.type ==  RFAL_NFC_LISTEN_TYPE_NFCF) ? "NDEF Attribute Information Block\r\n" : "NDEF Capability Container\r\n"));
  switch (ndef.ctx.device.type) {
    case RFAL_NFC_LISTEN_TYPE_NFCA:
      switch (ndef.ctx.device.dev.nfca.type) {
        case RFAL_NFCA_T2T:
          ndefT2TCCDump();
          break;
        case RFAL_NFCA_T4T:
          ndefT4TCCDump();
          break;
        default:
          break;
      }
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCB:
      ndefT4TCCDump();
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCF:
      ndefT3TAIBDump();
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCV:
      ndefT5TCCDump();
      break;
    default:
      break;
  }
}

#if NDEF_FEATURE_T5T
static void ndefDumpSysInfo()
{
  ndefSystemInformation *sysInfo;

  if (!verbose) {
    return;
  }

  if (!ndef.ctx.subCtx.t5t.sysInfoSupported) {
    return;
  }

  sysInfo = &ndef.ctx.subCtx.t5t.sysInfo;
  Serial.print("System Information\r\n");
  Serial.print(" * ");
  Serial.print(ndefT5TSysInfoMOIValue(sysInfo->infoFlags) + 1);
  Serial.print(" byte(s) memory addressing\r\n");
  if (ndefT5TSysInfoDFSIDPresent(sysInfo->infoFlags)) {
    Serial.print(" * DFSID=");
    Serial.print(sysInfo->DFSID, HEX);
    Serial.print("h\r\n");
  }
  if (ndefT5TSysInfoAFIPresent(sysInfo->infoFlags)) {
    Serial.print(" * AFI=");
    Serial.print(sysInfo->AFI, HEX);
    Serial.print("h\r\n");
  }
  if (ndefT5TSysInfoMemSizePresent(sysInfo->infoFlags)) {
    Serial.print(" * ");
    Serial.print(sysInfo->numberOfBlock);
    Serial.print(" blocks, ");
    Serial.print(sysInfo->blockSize);
    Serial.print(" bytes per block\r\n");
  }
  if (ndefT5TSysInfoICRefPresent(sysInfo->infoFlags)) {
    Serial.print(" * ICRef=");
    Serial.print(sysInfo->ICRef, HEX);
    Serial.print("h\r\n");
  }
  if (ndefT5TSysInfoCmdListPresent(sysInfo->infoFlags)) {
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoReadSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ReadSingleBlock                \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoWriteSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] WriteSingleBlock               \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoLockSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] LockSingleBlock                \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoReadMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ReadMultipleBlocks             \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoWriteMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] WriteMultipleBlocks            \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoSelectSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] Select                         \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoResetToReadySupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ResetToReady                   \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoGetMultipleBlockSecStatusSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] GetMultipleBlockSecStatus      \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoWriteAFISupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] WriteAFI                       \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoLockAFISupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] LockAFI                        \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoWriteDSFIDSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] WriteDSFID                     \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoLockDSFIDSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] LockDSFID                      \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoGetSystemInformationSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] GetSystemInformation           \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoCustomCmdsSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] CustomCmds                     \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoFastReadMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] FastReadMultipleBlocks         \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoExtReadSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ExtReadSingleBlock             \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoExtWriteSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ExtWriteSingleBlock            \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoExtLockSingleBlockSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ExtLockSingleBlock             \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoExtReadMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ExtReadMultipleBlocks          \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoExtWriteMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ExtWriteMultipleBlocks         \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoExtGetMultipleBlockSecStatusSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] ExtGetMultipleBlockSecStatus   \r\n");
    Serial.print(" * [");
    Serial.print(ndefT5TSysInfoFastExtendedReadMultipleBlocksSupported(sysInfo->supportedCmd) ? 'X' : ' ');
    Serial.print("] FastExtendedReadMultipleBlocks \r\n");
  }
  return;
}
#endif /* NDEF_FEATURE_T5T */

#if NDEF_FEATURE_FULL_API
static bool ndefIsSTTag()
{
  bool ret = false;

  switch (ndef.ctx.device.type) {
    case RFAL_NFC_LISTEN_TYPE_NFCA:
      if ((ndef.ctx.device.dev.nfca.nfcId1Len != 4) && (ndef.ctx.device.dev.nfca.nfcId1[0] == 0x02)) {
        ret = true;
      }
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCF:
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCB:
      break;
    case RFAL_NFC_LISTEN_TYPE_NFCV:
      if (ndef.ctx.device.dev.nfcv.InvRes.UID[6] == 0x02) {
        ret = true;
      }
      break;
    default:
      break;
  }
  return (ret);
}

static void LedNotificationWriteDone(void)
{
  uint32_t i;

  for (i = 0; i < 3; i++) {
    ledsOn();
    delay(100);

    ledsOff();
    delay(100);
  }
}
#endif /*NDEF_FEATURE_FULL_API */

static void ledsOn(void)
{
  digitalWrite(LED_A_PIN, HIGH);
  digitalWrite(LED_B_PIN, HIGH);
  digitalWrite(LED_F_PIN, HIGH);
  digitalWrite(LED_V_PIN, HIGH);
}

static void ledsOff(void)
{
  digitalWrite(LED_A_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
  digitalWrite(LED_F_PIN, LOW);
  digitalWrite(LED_V_PIN, LOW);
}

/*****************************************************************************/
static bool isPrintableASCII(const uint8_t *str, uint32_t strLen)
{
  uint32_t i;

  if ((str == NULL) || (strLen == 0)) {
    return false;
  }

  for (i = 0; i < strLen; i++) {
    if ((str[i] < 0x20U) || (str[i] > 0x7EU)) {
      return false;
    }
  }

  return true;
}


/*****************************************************************************/
ReturnCode ndefRecordDump(const ndefRecord *record, bool verbose)
{
  static uint32_t index;
  const uint8_t *ndefTNFNames[] = {
    (uint8_t *)"Empty",
    (uint8_t *)"NFC Forum well-known type [NFC RTD]",
    (uint8_t *)"Media-type as defined in RFC 2046",
    (uint8_t *)"Absolute URI as defined in RFC 3986",
    (uint8_t *)"NFC Forum external type [NFC RTD]",
    (uint8_t *)"Unknown",
    (uint8_t *)"Unchanged",
    (uint8_t *)"Reserved"
  };
  uint8_t *headerSR = (uint8_t *)"";
  ReturnCode err;

  if (record == NULL) {
    Serial.print("No record\r\n");
    return ERR_NONE;
  }

  if (ndefHeaderIsSetMB(record)) {
    index = 1U;
  } else {
    index++;
  }

  if (verbose == true) {
    headerSR = (uint8_t *)(ndefHeaderIsSetSR(record) ? " - Short Record" : " - Standard Record");
  }

  Serial.print("Record #");
  Serial.print(index);
  Serial.print((char *)headerSR);
  Serial.print("\r\n");

  /* Well-known type dump */
  err = ndefRecordDumpType(record);
  if (verbose == true) {
    /* Raw dump */
    //Serial.print(" MB:%d ME:%d CF:%d SR:%d IL:%d TNF:%d\r\n", ndefHeaderMB(record), ndefHeaderME(record), ndefHeaderCF(record), ndefHeaderSR(record), ndefHeaderIL(record), ndefHeaderTNF(record));
    Serial.print(" MB ME CF SR IL TNF\r\n");
    Serial.print("  ");
    Serial.print(ndefHeaderMB(record));
    Serial.print("  ");
    Serial.print(ndefHeaderME(record));
    Serial.print("  ");
    Serial.print(ndefHeaderCF(record));
    Serial.print("  ");
    Serial.print(ndefHeaderSR(record));
    Serial.print("  ");
    Serial.print(ndefHeaderIL(record));
    Serial.print("  ");
    Serial.print(ndefHeaderTNF(record));
    Serial.print("\r\n");
  }
  if ((err != ERR_NONE) || (verbose == true)) {
    Serial.print(" Type Name Format: ");
    Serial.print((char *)ndefTNFNames[ndefHeaderTNF(record)]);
    Serial.print("\r\n");

    uint8_t tnf;
    ndefConstBuffer8 bufRecordType;
    ndefRecordGetType(record, &tnf, &bufRecordType);
    if ((tnf == NDEF_TNF_EMPTY) && (bufRecordType.length == 0U)) {
      Serial.print(" Empty NDEF record\r\n");
    } else {
      ndefBuffer8Print(" Type: \"", &bufRecordType, "\"\r\n");
    }

    if (ndefHeaderIsSetIL(record)) {
      /* ID Length bit set */
      ndefConstBuffer8 bufRecordId;
      ndefRecordGetId(record, &bufRecordId);
      ndefBuffer8Print(" ID: \"", &bufRecordId, "\"\r\n");
    }

    ndefConstBuffer bufRecordPayload;
    ndefRecordGetPayload(record, &bufRecordPayload);
    ndefBufferDump(" Payload:", &bufRecordPayload, verbose);
    if (ndefRecordGetPayloadLength(record) != bufRecordPayload.length) {
      Serial.print(" Payload stored as a well-known type\r\n");
    }
  }

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode ndefMessageDump(const ndefMessage *message, bool verbose)
{
  ReturnCode  err;
  ndefRecord *record;

  if (message == NULL) {
    Serial.print("Empty NDEF message\r\n");
    return ERR_NONE;
  } else {
    Serial.print("Decoding NDEF message\r\n");
  }

  record = ndefMessageGetFirstRecord(message);

  while (record != NULL) {
    err = ndefRecordDump(record, verbose);
    if (err != ERR_NONE) {
      return err;
    }
    record = ndefMessageGetNextRecord(record);
  }

  return ERR_NONE;
}


#if NDEF_TYPE_EMPTY_SUPPORT
/*****************************************************************************/
ReturnCode ndefEmptyTypeDump(const ndefType *type)
{
  if ((type == NULL) || (type->id != NDEF_TYPE_ID_EMPTY)) {
    return ERR_PARAM;
  }

  Serial.print(" Empty record\r\n");

  return ERR_NONE;
}
#endif


#if NDEF_TYPE_RTD_DEVICE_INFO_SUPPORT
/*****************************************************************************/
ReturnCode ndefRtdDeviceInfoDump(const ndefType *type)
{
  ndefTypeRtdDeviceInfo devInfoData;
  ReturnCode err;
  uint32_t info;
  uint32_t i;

  static const uint8_t *ndefDeviceInfoName[] = {
    (uint8_t *)"Manufacturer",
    (uint8_t *)"Model",
    (uint8_t *)"Device",
    (uint8_t *)"UUID",
    (uint8_t *)"Firmware version",
  };

  err = ndefGetRtdDeviceInfo(type, &devInfoData);
  if (err != ERR_NONE) {
    return err;
  }

  Serial.print(" Device Information:\r\n");

  for (info = 0; info < NDEF_DEVICE_INFO_TYPE_COUNT; info++) {
    if (devInfoData.devInfo[info].buffer != NULL) {
      Serial.print(" - ");
      Serial.print((char *)ndefDeviceInfoName[devInfoData.devInfo[info].type]);
      Serial.print(": ");

      if (info != NDEF_DEVICE_INFO_UUID) {
        for (i = 0; i < devInfoData.devInfo[info].length; i++) {
          Serial.print(devInfoData.devInfo[info].buffer[i]); /* character */
        }
      } else {
        for (i = 0; i < devInfoData.devInfo[info].length; i++) {
          Serial.print(devInfoData.devInfo[info].buffer[i], HEX); /* hex number */
        }
      }
      Serial.print("\r\n");
    }
  }

  return ERR_NONE;
}
#endif


#if NDEF_TYPE_RTD_TEXT_SUPPORT
/*****************************************************************************/
ReturnCode ndefRtdTextDump(const ndefType *type)
{
  uint8_t utfEncoding;
  ndefConstBuffer8 bufLanguageCode;
  ndefConstBuffer  bufSentence;
  ReturnCode err;

  err = ndefGetRtdText(type, &utfEncoding, &bufLanguageCode, &bufSentence);
  if (err != ERR_NONE) {
    return err;
  }

  ndefBufferPrint(" Text: \"", &bufSentence, "");

  Serial.print("\" (");
  Serial.print((utfEncoding == TEXT_ENCODING_UTF8 ? "UTF8" : "UTF16"));
  Serial.print(",");

  ndefBuffer8Print(" language code \"", &bufLanguageCode, "\")\r\n");

  return ERR_NONE;
}
#endif


#if NDEF_TYPE_RTD_URI_SUPPORT
/*****************************************************************************/
ReturnCode ndefRtdUriDump(const ndefType *type)
{
  ndefConstBuffer bufProtocol;
  ndefConstBuffer bufUriString;
  ReturnCode err;

  err = ndefGetRtdUri(type, &bufProtocol, &bufUriString);
  if (err != ERR_NONE) {
    return err;
  }

  ndefBufferPrint("URI: (", &bufProtocol, ")");
  ndefBufferPrint("", &bufUriString, "\r\n");

  return ERR_NONE;
}
#endif

#if NDEF_TYPE_RTD_AAR_SUPPORT
/*****************************************************************************/
ReturnCode ndefRtdAarDump(const ndefType *type)
{
  ndefConstBuffer bufAarString;
  ReturnCode err;

  err = ndefGetRtdAar(type, &bufAarString);
  if (err != ERR_NONE) {
    return err;
  }

  ndefBufferPrint(" AAR Package: ", &bufAarString, "\r\n");

  return ERR_NONE;
}
#endif


#if NDEF_TYPE_VCARD_SUPPORT
/*****************************************************************************/
static ReturnCode ndefMediaVCardTranslate(const ndefConstBuffer *bufText, ndefConstBuffer *bufTranslation)
{
  typedef struct {
    uint8_t *vCardString;
    uint8_t *english;
  } ndefTranslate;

  const ndefTranslate translate[] = {
    { (uint8_t *)"N", (uint8_t *)"Name"           },
    { (uint8_t *)"FN", (uint8_t *)"Formatted Name" },
    { (uint8_t *)"ADR", (uint8_t *)"Address"        },
    { (uint8_t *)"TEL", (uint8_t *)"Phone"          },
    { (uint8_t *)"EMAIL", (uint8_t *)"Email"          },
    { (uint8_t *)"TITLE", (uint8_t *)"Title"          },
    { (uint8_t *)"ORG", (uint8_t *)"Org"            },
    { (uint8_t *)"URL", (uint8_t *)"URL"            },
    { (uint8_t *)"PHOTO", (uint8_t *)"Photo"          },
  };

  uint32_t i;

  if ((bufText == NULL) || (bufTranslation == NULL)) {
    return ERR_PROTO;
  }

  for (i = 0; i < SIZEOF_ARRAY(translate); i++) {
    if (ST_BYTECMP(bufText->buffer, translate[i].vCardString, strlen((char *)translate[i].vCardString)) == 0) {
      bufTranslation->buffer = translate[i].english;
      bufTranslation->length = strlen((char *)translate[i].english);

      return ERR_NONE;
    }
  }

  bufTranslation->buffer = bufText->buffer;
  bufTranslation->length = bufText->length;

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode ndefMediaVCardDump(const ndefType *type)
{
  ndefTypeVCard vCard;

  const uint8_t NDEF_VCARD_N[]     = "N";
  const uint8_t NDEF_VCARD_FN[]    = "FN";
  const uint8_t NDEF_VCARD_ADR[]   = "ADR";
  const uint8_t NDEF_VCARD_TEL[]   = "TEL";
  const uint8_t NDEF_VCARD_EMAIL[] = "EMAIL";
  const uint8_t NDEF_VCARD_TITLE[] = "TITLE";
  const uint8_t NDEF_VCARD_ORG[]   = "ORG";
  const uint8_t NDEF_VCARD_URL[]   = "URL";
  const uint8_t NDEF_VCARD_PHOTO[] = "PHOTO";

  const ndefConstBuffer bufVCard_N     = { NDEF_VCARD_N,     sizeof(NDEF_VCARD_N)     - 1U };
  const ndefConstBuffer bufVCard_FN    = { NDEF_VCARD_FN,    sizeof(NDEF_VCARD_FN)    - 1U };
  const ndefConstBuffer bufVCard_ADR   = { NDEF_VCARD_ADR,   sizeof(NDEF_VCARD_ADR)   - 1U };
  const ndefConstBuffer bufVCard_TEL   = { NDEF_VCARD_TEL,   sizeof(NDEF_VCARD_TEL)   - 1U };
  const ndefConstBuffer bufVCard_EMAIL = { NDEF_VCARD_EMAIL, sizeof(NDEF_VCARD_EMAIL) - 1U };
  const ndefConstBuffer bufVCard_TITLE = { NDEF_VCARD_TITLE, sizeof(NDEF_VCARD_TITLE) - 1U };
  const ndefConstBuffer bufVCard_ORG   = { NDEF_VCARD_ORG,   sizeof(NDEF_VCARD_ORG)   - 1U };
  const ndefConstBuffer bufVCard_URL   = { NDEF_VCARD_URL,   sizeof(NDEF_VCARD_URL)   - 1U };
  const ndefConstBuffer bufVCard_PHOTO = { NDEF_VCARD_PHOTO, sizeof(NDEF_VCARD_PHOTO) - 1U };

  const ndefVCardTypeTable vCardType[] = {
    { bufVCard_N, "Name"           },
    { bufVCard_FN, "Formatted Name" },
    { bufVCard_ADR, "Address"        },
    { bufVCard_TEL, "Phone"          },
    { bufVCard_EMAIL, "Email"          },
    { bufVCard_TITLE, "Title"          },
    { bufVCard_ORG, "Org"            },
    { bufVCard_URL, "URL"            },
    { bufVCard_PHOTO, "Photo"          },
  };

  ReturnCode err = ndefGetVCard(type, &vCard);
  if (err != ERR_NONE) {
    return err;
  }

  Serial.print(" vCard decoded: \r\n");

  for (uint32_t i = 0; i < SIZEOF_ARRAY(vCardType); i++) {
    ndefConstBuffer bufProperty;
    err = ndefVCardGetProperty(&vCard, &vCardType[i].bufType, &bufProperty);
    if (err == ERR_NONE) {
      ndefConstBuffer bufType, bufSubtype, bufValue;
      err = ndefVCardParseProperty(&bufProperty, &bufType, &bufSubtype, &bufValue);
      if (err != ERR_NONE) {
        Serial.print("ndefVCardParseProperty error ");
        Serial.print(err);
        Serial.print("\r\n");
      }

      if (bufValue.buffer != NULL) {
        /* Type */
        Serial.print(" ");
        Serial.print(vCardType[i].string);

        /* Subtype, if any */
        if (bufSubtype.buffer != NULL) {
          ndefBufferPrint(" (", &bufSubtype, ")");
        }

        /* Value */
        if (ndefBufferMatch(&bufType, &bufVCard_PHOTO) == false) {
          ndefBufferPrint(": ", &bufValue, "\r\n");
        } else {
          Serial.print("Photo: <Not displayed>\r\n");
        }
      }
    }
  }

  return ERR_NONE;
}
#endif

#if NDEF_TYPE_WIFI_SUPPORT
/*****************************************************************************/
ReturnCode ndefMediaWifiDump(const ndefType *wifi)
{
  ndefTypeWifi wifiConfig;

  if (wifi == NULL) {
    return ERR_PARAM;
  }


  if (wifi->id != NDEF_TYPE_ID_MEDIA_WIFI) {
    return ERR_PARAM;
  }

  ndefGetWifi(wifi, &wifiConfig);

  Serial.print(" Wifi config: \r\n");
  ndefBufferDump(" Network SSID:",       &wifiConfig.bufNetworkSSID, false);
  ndefBufferDump(" Network Key:",        &wifiConfig.bufNetworkKey, false);
  Serial.print(" Authentication: ");
  Serial.print(wifiConfig.authentication);
  Serial.print("\r\n");
  Serial.print(" Encryption: ");
  Serial.print(wifiConfig.encryption);
  Serial.print("\r\n");

  return ERR_NONE;
}
#endif


/*****************************************************************************/
ReturnCode ndefRecordDumpType(const ndefRecord *record)
{
  ReturnCode err;
  ndefType   type;
  uint32_t i;

  err = ndefRecordToType(record, &type);
  if (err != ERR_NONE) {
    return err;
  }

  for (i = 0; i < SIZEOF_ARRAY(typeDumpTable); i++) {
    if (type.id == typeDumpTable[i].typeId) {
      /* Call the appropriate function to the matching record type */
      if (typeDumpTable[i].dump != NULL) {
        return typeDumpTable[i].dump(&type);
      }
    }
  }

  return ERR_NOT_IMPLEMENTED;
}


/*****************************************************************************/
static ReturnCode ndefBufferDumpLine(const uint8_t *buffer, const uint32_t offset, uint32_t lineLength, uint32_t remaining)
{
  uint32_t j;

  if (buffer == NULL) {
    return ERR_PARAM;
  }

  Serial.print(" [");
  Serial.print(offset, HEX);
  Serial.print("] ");

  /* Dump hex data */
  for (j = 0; j < remaining; j++) {
    Serial.print(buffer[offset + j], HEX);
    Serial.print(" ");
  }
  /* Fill hex section if needed */
  for (j = 0; j < lineLength - remaining; j++) {
    Serial.print("   ");
  }

  /* Dump characters */
  Serial.print("|");
  for (j = 0; j < remaining; j++) {
    /* Dump only ASCII characters, otherwise replace with a '.' */
    Serial.print((isPrintableASCII(&buffer[offset + j], 1) ? (char)buffer[offset + j] : '.'));
  }
  /* Fill ASCII section if needed */
  for (j = 0; j < lineLength - remaining; j++) {
    Serial.print("  ");
  }
  Serial.print(" |\r\n");

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode ndefBufferDump(const char *string, const ndefConstBuffer *bufPayload, bool verbose)
{
  uint32_t bufferLengthMax = 32;
  const uint32_t lineLength = 8;
  uint32_t displayed;
  uint32_t remaining;
  uint32_t offset;

  if ((string == NULL) || (bufPayload == NULL)) {
    return ERR_PARAM;
  }

  displayed = bufPayload->length;
  remaining = bufPayload->length;

  Serial.print(string);
  Serial.print(" (length ");
  Serial.print(bufPayload->length);
  Serial.print(")\r\n");
  if (bufPayload->buffer == NULL) {
    Serial.print(" <No chunk payload buffer>\r\n");
    return ERR_NONE;
  }

  if (verbose == true) {
    bufferLengthMax = 256;
  }
  if (bufPayload->length > bufferLengthMax) {
    /* Truncate output */
    displayed = bufferLengthMax;
  }

  for (offset = 0; offset < displayed; offset += lineLength) {
    ndefBufferDumpLine(bufPayload->buffer, offset, lineLength, remaining > lineLength ? lineLength : remaining);
    remaining -= lineLength;
  }

  if (displayed < bufPayload->length) {
    Serial.print(" ... (truncated)\r\n");
  }

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode ndefBufferPrint(const char *prefix, const ndefConstBuffer *bufString, const char *suffix)
{
  uint32_t i;

  if ((prefix == NULL) || (bufString == NULL) || (bufString->buffer == NULL) || (suffix  == NULL)) {
    return ERR_PARAM;
  }

  Serial.print(prefix);
  for (i = 0; i < bufString->length; i++) {
    Serial.print((char)bufString->buffer[i]);
  }
  Serial.print(suffix);

  return ERR_NONE;
}


/*****************************************************************************/
ReturnCode ndefBuffer8Print(const char *prefix, const ndefConstBuffer8 *bufString, const char *suffix)
{
  ndefConstBuffer buf;

  if (bufString == NULL) {
    return ERR_PARAM;
  }

  buf.buffer = bufString->buffer;
  buf.length = bufString->length;

  return ndefBufferPrint(prefix, &buf, suffix);
}

char *hex2Str(unsigned char *data, size_t dataLen)
{
  unsigned char *pin = data;
  const char *hex = "0123456789ABCDEF";
  char *pout = hexStr[hexStrIdx];
  uint8_t i = 0;
  uint8_t idx = hexStrIdx;
  size_t len;

  if (dataLen == 0) {
    pout[0] = 0;
  } else {
    /* Trim data that doesn't fit in buffer */
    len = MIN(dataLen, (MAX_HEX_STR_LENGTH / 2));

    for (; i < (len - 1); ++i) {
      *pout++ = hex[(*pin >> 4) & 0xF];
      *pout++ = hex[(*pin++) & 0xF];
    }
    *pout++ = hex[(*pin >> 4) & 0xF];
    *pout++ = hex[(*pin) & 0xF];
    *pout = 0;
  }

  hexStrIdx++;
  hexStrIdx %= MAX_HEX_STR;

  return hexStr[idx];
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
