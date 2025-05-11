#include <MFRC522.h>

/*
RFID module wiring schema:
Uno      MFRC522
9     -> RST
10    -> SDA
11    -> MOSI
12    -> MISO
13    -> SCK
GND   -> GND
3.3V  -> VCC/Vin
*/

MFRC522 mfrc522(10, 9);
MFRC522::MIFARE_Key key;
byte writeblock[16] = {};

const byte blockcontent[16] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const int writeblock = 2;
const int trailerblock = 3;

void setup() {
  // put your setup code here, to run once:

  for (byte i=0; i<6; i++){
    key.keyByte[i] = 0xFF;
  }

  while (!mfrc522.PICC_IsNewCardPresent()){}
  while (!mfrc522.PICC_ReadCardSerial()){}

  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  status = mfrc522.MIFARE_Write(2, blockcontent, 16);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void loop() {
  // put your main code here, to run repeatedly:

}
