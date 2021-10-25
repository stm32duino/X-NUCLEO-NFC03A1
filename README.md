# X-NUCLEO-NFC03A1

The code has been designed for ST X-NUCLEO-NFC03A1 expansion board to show how to detect, read and write NFC tags.
The X-NUCLEO-NFC03A1 is an NFC card reader evaluation board based on ST25R95 to enable expansion of the STM32 Nucleo boards.
The ST25R95 manages frame coding and decoding in Reader mode for standard applications, such as NFC, proximity and vicinity HF RFID standards.
The ST25R95 supports ISO/IEC 14443 Type A and B, ISO/IEC 15693 (single Subcarrier only) and ISO/IEC 18092 communication protocols. 
It also supports the detection, reading and writing of NFC Forum Type 1, 2, 3, 4 and 5 tags.

## Examples

There is one example with the X-NUCLEO-NFC03A1 library: 
* X_NUCLEO_NFC03A1_HelloWorld: This application is to show how to detect, read and write NFC Forum Type 1, 2, 3, 4 and 5 tags.
  Open a serial terminal with a baudrate of 115200, 8 data bits, No parity bit, 1 stop bit and follow the instructions shown by the menu.
  Pushing the User Button, you can switch to several options like writing a text record, writing a URI record and an Android Application record, 
  formatting an ST tag or reading the content of a tag.  

# Dependencies

The X-NUCLEO-NFC03A1 library requires the following STM32duino libraries:

* STM32duino NFC-RFAL: https://github.com/stm32duino/NFC-RFAL
* STM32duino ST25R95: https://github.com/stm32duino/ST25R95

## Documentation

You can find the source files at  
https://github.com/stm32duino/X-NUCLEO-NFC03A1

The ST25R95 datasheet is available at  
https://www.st.com/en/nfc/st25r95.html

