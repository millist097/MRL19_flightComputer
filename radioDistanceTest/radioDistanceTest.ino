

//Librarys to include

#include <RH_RF95.h>
#include <Wire.h>

#include "enums.h"
#include "RTClib.h"
#include "rf95comCode.h"


/* Setup and constants for RF95 Transmitter */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

uint8_t myAddress = 0x00;
uint8_t reciverAddress = 0xCF;
//uint8_t myAddress = 0xCF;
//uint8_t reciverAddress = 0x00;
#define RF95_FREQ 915.0
RH_RF95 rf95(RFM95_CS, RFM95_INT);

/* LED Pin decleration */
#define LED 13
#define BLINKDELAY 150


union Data{
  byte buf[5];
  struct{
    unsigned long micro;
    int8_t rssi;
  };
};

union Data data;
unsigned long myCurrentTime;
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

messageType currentMessage = STANDBY;

void serverCode(){
   if (rf95.available()){
      
      // Should be a message for us now
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      
      if (rf95.recv(buf, &len)){
        Serial.println("Message Recived");
        digitalWrite(LED, HIGH);
        if( buf[0] == myAddress){ // if for me
          //check CRC
          byte crc = 0;
          for( int i = 0; i<len-1; i++)
            crc ^= buf[i];
          Serial.print("recived CRC: "); Serial.println(buf[len-1],HEX);
          Serial.print("Calculated CRC: "); Serial.println(crc,HEX);
          if( buf[len-1] == crc){
            Serial.println("CRC Match");
            if(buf[2]==(uint8_t)DATA){
              sendAcknoledge(rf95,myAddress, reciverAddress);
              Serial.print("Sending to: ");Serial.println(reciverAddress, HEX);
              Serial.println("Aknowledgement Sent!");
              for( int i = 0; i < buf[3]; i++)
                data.buf[i] = buf[4+i];
              set_flag(FLAG_DATA_RECIVED);
              
            }else if(buf[2]==(uint8_t)ACKNOWLEDGE){
              set_flag(FLAG_ACKNOWLEDGE_RECIVED);
              Serial.println("Aknowledge Recived");
              
            }else if( buf[len-1] == (uint8_t)ERROR){
              Serial.println("Incorrect message was recived by Server");
            }
          }else{ // CRC does not match
            sendError(rf95, myAddress, reciverAddress);
            Serial.println("Error Sent");
            }
          digitalWrite(LED, LOW);
        }
      }else{
        Serial.println("No message.");
      }
  }// Message Recived --end protocal

  if(flag_is_set(FLAG_DATA_RECIVED)){
    Serial.print(data.micro);
    Serial.print(",");
    Serial.print(data.rssi,DEC);
    Serial.print(",");
    Serial.println(rf95.lastRssi(),DEC);
    clear_flag(FLAG_DATA_RECIVED);
  }
}

void clientCode(){
  data.micro = myCurrentTime;
  data.rssi = rf95.lastRssi();
  currentMessage = DATA;
  sendData(rf95, myAddress, reciverAddress, data.buf, 5, currentMessage);
  rf95.waitPacketSent();

  // wait for reply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if( rf95.waitAvailableTimeout(3000)){
     if (rf95.recv(buf, &len)){
      Serial.println("Message Recived");
      digitalWrite(LED, HIGH);
      Serial.print("Reciver Address: "); Serial.println(buf[1],HEX);
      Serial.print("Recived message length: "); Serial.println( len, DEC);
      Serial.print("Recived message's RSSI: "); Serial.println(rf95.lastRssi(),DEC);
      if( buf[0] == myAddress){ // if for me
          //check CRC
          byte crc = 0;
          for( int i = 0; i<len-1; i++)
            crc ^= buf[i];
          Serial.print("recived CRC: "); Serial.println(buf[len-1],HEX);
          Serial.print("Calculated CRC: "); Serial.println(crc,HEX);
          if( buf[len-1] == crc){
            Serial.println("CRC Match");
            if(buf[2]==(uint8_t)DATA){
              sendAcknoledge(rf95,myAddress, reciverAddress);
              Serial.println("Aknowledgement Sent");
              for( int i = 0; i < buf[3]; i++)
                data.buf[i] = buf[4+i];
              set_flag(FLAG_DATA_RECIVED);
              
            }else if(buf[2]==(uint8_t)ACKNOWLEDGE){
              set_flag(FLAG_ACKNOWLEDGE_RECIVED);
              Serial.println("Aknowledge Recived");
              
            }else if( buf[len-1] == (uint8_t)ERROR){
              Serial.println("Incorrect message was recived by Server");
            }
          }else{ // CRC does not match
            sendError(rf95, myAddress, reciverAddress);
            Serial.println("Error Sent");
            }
          digitalWrite(LED, LOW);
        }
     }
  }else{
        Serial.println("No message.");
  }
}


void setup() {
    pinMode(LED,OUTPUT);
  digitalWrite(RFM95_RST,HIGH);
  digitalWrite(LED,LOW);
  delay(50);
  if( !rfm95_Setup()){
    while(1){
      for(int i = 0;i<3;i++){
        digitalWrite(LED,HIGH);
        delay(BLINKDELAY);
        digitalWrite(LED,LOW);
        delay(BLINKDELAY);
      }
      delay(1000);
    }
  }//end rfm95_Setup()
  Serial.begin(115200);
  delay(1500);
  //while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init()){
    Serial.println("init failed");  
  }
      Serial.print("Time recived client,"); 
    Serial.print(" Time recived home,");
    Serial.print(" Recived Message RSSI,");
    Serial.println(" Sent Message RSSI,");
    Serial.print("My Address: "); Serial.println(myAddress,HEX);
    
}

void loop() {
  myCurrentTime = micros();

  if( myAddress == 0x00){
    serverCode();
  }
  else{
    clientCode();
    delay(1000);
  }
}
