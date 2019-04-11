/* Project: MRL 2019 Ground Transiver
 * Board: M0 Feather with RFM9* Transmitter
 * Wing: None
 * Sensors: None
 * 
 * Discription: This code runs and acts like a bridge
 * from the ground terminal to the rocket. Main function
 * is to recive flight data, followed by relaying commands
 * and current status.
 * 
 */

#include <SPI.h>
#include <RH_RF95.h>
#include "enums.h"

/* Define onboard LED */
#define LED 13

/* Setup and constants for RF95 Transmitter */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
uint8_t myAddress = 0x00;
uint8_t reciverAddress = 0xCF;
#define RF95_FREQ 915.0
RH_RF95 rf95(RFM95_CS, RFM95_INT);

bool rfm95_Setup(){
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(30);
  if(!rf95.init()){
    return false;
  }
  delay(15);
  if(!rf95.setFrequency(RF95_FREQ)){
    return false;
  }
  rf95.setTxPower(23,false);
  return true;
}

union Data{
  float fval[3];
  uint8_t bval[12];
};

launchStatus r_status = PREBURN;
messageType currentMessage = STANDBY;

void sendData(uint8_t selfId, uint8_t receiverId, uint8_t *data, uint8_t bufLen, 
              messageType type){
  uint8_t package[bufLen + 4];
  package[0] = selfId;
  package[1] = receiverId;
  package[2] = (uint8_t)type;
  package[3] = bufLen;
  byte crc = 0;
  for(int i = 0; i++; i < bufLen){
    package[4+i] = data[i];
    crc ^= data[i];
  }
  package[4+bufLen] = crc;

  rf95.send(package,4+bufLen);
  //rf95.send(data.bval,4);
}

void sendAcknoledge(uint8_t selfId, uint8_t receiverId ){
  uint8_t package[4];
  byte crc = 0;
  package[0] = receiverId;
  crc ^= package[0];
  package[0] = selfId;
  crc ^= package[1];
  package[2] = (uint8_t)ACKNOWLEDGE;
  crc ^= package[2];
  package[3] = crc;
  rf95.send(package,4);
}

void sendError(uint8_t selfId, uint8_t receiverId){
  uint8_t package[4];
  byte crc = 0;
  package[0] = receiverId;
  crc ^= package[0];
  package[0] = selfId;
  crc ^= package[1];
  package[2] = (uint8_t)ERROR;
  crc ^= package[2];
  package[3] = crc;
  rf95.send(package,4);
}

void setup() {
  pinMode(LED,OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST,HIGH);
  digitalWrite(LED,LOW);
  
  Serial.begin(115200);
  while(!Serial){
    delay(10);
  }
  delay(100);
  Serial.println("System Ready");
}

void loop() {
  if (rf95.available()){
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)){
      digitalWrite(LED, HIGH);
      if( buf[0] == myAddress){
        byte crc = 0;
        for( int i = 0; i<len; i++)
          crc ^= buf[i];
        if( buf[len-1] == crc){
          sendAcknoledge(myAddress, reciverAddress);
          Serial.println("Aknowledgement Sent");
        }else{
          sendError(myAddress, reciverAddress);
          Serial.println("Error Sent");
        }
      rf95.waitPacketSent();
        
      }else{
        rf95.waitPacketSent();
        Serial.println("Sent a reply");
        digitalWrite(LED, LOW);
      }
    }else{
      Serial.println("Receive failed");
    }
  }
  delay(500);
  Serial.println("Test");


}
