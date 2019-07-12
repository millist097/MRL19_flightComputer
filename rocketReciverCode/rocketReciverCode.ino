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
#include "rf95comCode.h"

/* Define onboard LED */
#define LED 13

/* Setup and constants for RF95 Transmitter */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
uint8_t myAddress = 0x00;
uint8_t reciverAddress = 0xCF;
#define RF95_FREQ 915.0
RH_RF95  rf95(RFM95_CS, RFM95_INT);

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

uint8_t dataSetRecived[50];

launchStatus r_status = PREBURN;
messageType currentMessage = STANDBY;


int count = 0;


void setup() {
  pinMode(LED,OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST,HIGH);
  digitalWrite(LED,HIGH);
  if(!rfm95_Setup() ){
    while(1){
      digitalWrite(LED,HIGH);
      delay(150);
      digitalWrite(LED,LOW);
      delay(1000);
    }
  }
  Serial.begin(115200);
  while(!Serial){
    delay(10);
  }
  delay(100);
  Serial.println("System Ready");
}

void loop() {

  if (rf95.available()){
      Serial.println("Message Recived");
      // Should be a message for us now
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      
      if (rf95.recv(buf, &len)){
        digitalWrite(LED, HIGH);
        if( buf[0] == selfId){ // if for me
          //check CRC
          byte crc = 0;
          for( int i = 0; i<len-1; i++){
            crc ^= buf[i];
          }
          if( buf[len-1] == crc){
            sendAcknoledge(&rf95,myAddress, reciverAddress);
            if(buf[2]==(unit8_t)DATA){
              for( int i = 0; i < buf[3]; i++){
                dataSetRecived[i] = buf[4+i]
              }
              set_flag(FLAG_DATA_RECIVED);
            }
            Serial.println("Aknowledgement Sent");
          }else{
            sendError(rf95, myAddress, reciverAddress);
            Serial.println("Error Sent");
          }
          digitalWrite(LED, LOW);
        }
      }else{
        Serial.println("No message.");
      }  
  }// Message Recived --end protocal

  if(flag_is_set(FLAG_DATA_RECIVED){
    
  }
}
