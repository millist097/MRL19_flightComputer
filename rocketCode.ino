/* Project: MRL 2019 Flight Computer
 * Board: M0 Feather with RFM9* Transmitter
 * Wing: DataLogger wing from arduino
 * Sensor: BNO055 IMU
 * 
 * Discription: This code will record all flight data
 * and current operating status during the flight of the 
 * rocket. It will also Transmitte telemetry and status
 * to the ground station.
 * 
 * */


//Librarys to include
#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "enums.h"

/* Setup and constants for RF95 Transmitter */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
uint8_t myAddress = 0xCF;
uint8_t reciverAddress = 0x00;
#define RF95_FREQ 915.0
RH_RF95 rf95(RFM95_CS, RFM95_INT);

/* LED Pin decleration */
#define LED 13

Adafruit_BNO055 bno = Adafruit_BNO055(55);

int16_t packetnum = 0; // packet counter, we increment per xmission

union Data{
  float fval[3];
  uint8_t bval[12];
};

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


launchStatus r_status = PREBURN;
messageType currentMessage = STANDBY;


void testBNO(){
    /* Get a new sensor event */ 
  sensors_event_t event; 
  bno.getEvent(&event);
  
  /* Display the floating point data */
  Serial.print("X: ");
  Serial.print(event.orientation.x, 4);
  Serial.print("\tY: ");
  Serial.print(event.orientation.y, 4);
  Serial.print("\tZ: ");  
  Serial.print(event.orientation.z, 4);
  Serial.println("");
  
  delay(100);
}

void sendData(uint8_t selfId, uint8_t reciverId, uint8_t *data, uint8_t bufLen, 
              messageType type){
  uint8_t package[bufLen + 4];
  byte crc = 0;
  package[0] = reciverId;
  crc ^= selfId;
  package[1] = selfId;
  crc ^= reciverId;
  package[2] = (uint8_t)type;
  crc ^= package[2];
  package[3] = bufLen;
  crc ^= package[3];
  for(int i = 0; i++; i < bufLen){
    package[4+i] = data[i];
    crc ^= data[i];
  }
  package[4+bufLen] = crc;

  rf95.send(package,4+bufLen);
  //rf95.send(data.bval,4);
}

void setup() {
  /* Setup for RFM95 */
  pinMode(LED,OUTPUT);
  pinMode(LED,OUTPUT);
  digitalWrite(RFM95_RST,HIGH);
  digitalWrite(LED,LOW);
  if( !rfm95_Setup()){
    while(1){
      for(int i = 0;i<3;i++){
        digitalWrite(LED,HIGH);
        delay(100);
        digitalWrite(LED,LOW);
        delay(100);
      }
      delay(1000);
    }
  }//end rfm95_Setup()

  /* Setup for BNO005 IMU Sensor */
  if(!bno.begin()){
    while(1){
      for(int i = 0;i<4;i++){
        digitalWrite(LED,HIGH);
        delay(100);
        digitalWrite(LED,LOW);
        delay(100);
      }
      delay(1000);
    }//end fail case
  }
  delay(1000);
  bno.setExtCrystalUse(true);

  
  //Serial setup for debugging
  Serial.begin(115200);
    Serial.begin(115200);
  while(!Serial){
    delay(10);
  }
  delay(100);
  Serial.println("System Ready");
}

void loop() {

  sensors_event_t event; 
  bno.getEvent(&event);
  union Data data;
  data.fval[0] = event.orientation.x;
  data.fval[1] = event.orientation.y;
  data.fval[2] = event.orientation.z;
  currentMessage = DATA;
  digitalWrite(LED,HIGH);
  sendData(myAddress, reciverAddress, data.bval, 12, currentMessage);
  digitalWrite(LED,LOW);
  delay(75);
  Serial.println("Data Sent");
  delay(1000);
    
}
