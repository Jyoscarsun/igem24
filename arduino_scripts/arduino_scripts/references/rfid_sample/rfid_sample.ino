// Implement try finally scheme for error handling!!!

#include <SPI.h>
#include <MFRC522.h>

const int SS_PIN = 10;
const int RST_PIN = 5;

const byte empty[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const byte filled[16] = {"bibir iseng usil"};

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void finally(){
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

const int block = 2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;  //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
  }
}

// void loop() {
//   auto blockcontent = filled;

//   // put your main code here, to run repeatedly:
//   if (!mfrc522.PICC_IsNewCardPresent()){
//     return;
//   }

//   if (!mfrc522.PICC_ReadCardSerial()){
//     return;
//   }

//   Serial.println("Bereit");

//   byte readbackblock[18];
//   writeBlock(block, blockcontent);
//   readBlock(block, readbackblock);
  
//   Serial.print("read block: ");
//   for (int j=0; j<16; j++){
//     Serial.write(readbackblock[j]);
//   } Serial.println();

//   mfrc522.PICC_HaltA();
// }

void loop(){
  byte read_back_block[18];
  while(end_to_end_read(2, read_back_block)){

    for (int j=0; j<16; j++){
      Serial.write(read_back_block[j]);
    } Serial.println();

    delay(5000);

  }
}

bool end_to_end_read(int block_number, byte array_address[]){
  if (!mfrc522.PICC_IsNewCardPresent()){
    return false;
  }

  if (!mfrc522.PICC_ReadCardSerial()){
    return false;
  }

  int largestModulo4Number = block_number/4*4;
  int trailerBlock = largestModulo4Number+3;
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("PCD_Authenticate() failed (read): ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  byte buffersize = 18;
  status = mfrc522.MIFARE_Read(block_number, array_address, &buffersize);
  if (status != MFRC522::STATUS_OK){
    Serial.print("Read failed");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return true;
}

int writeBlock(int blockNumber, byte arrayAddress[]){
  int largestModulo4Number = blockNumber/4*4;
  int trailerBlock = largestModulo4Number+3;
  if (blockNumber > 2 && (blockNumber+1)%4 == 0){
    Serial.print(blockNumber);
    Serial.println(" is a trailer block:");
    return 2;
  }

  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
         Serial.print("PCD_Authenticate() failed: ");
         Serial.println(mfrc522.GetStatusCodeName(status));
         return 3;
  }

  status = mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);
  if (status != MFRC522::STATUS_OK){
    Serial.print("Write failed");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 4;
  }

  Serial.println("Write successful");
}

int readBlock(int blockNumber, byte arrayAddress[]){
  int largestModulo4Number = blockNumber/4*4;
  int trailerBlock = largestModulo4Number+3;
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
         Serial.print("PCD_Authenticate() failed (read): ");
         Serial.println(mfrc522.GetStatusCodeName(status));
         return 3;//return "3" as error message
  }

  byte buffersize = 18;
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  if (status != MFRC522::STATUS_OK){
    Serial.print("Read failed");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 3;
  }

  Serial.println("Read successful");
  mfrc522.PICC_HaltA();
}