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
#include<SD.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "enums.h"
#include "RTClib.h"
#include "rf95comCode.h"

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
#define BLINKDELAY 150

/*  Time tarcking declerations */
unsigned long currentTime;
unsigned long lastTime_sensorEvent=0;
unsigned long lastTime_transmitEvent=0;
unsigned long timeDiff=0;


/* SD card constents and declerations */
#define chipSelect 10
File errorLog;
File dataLog;
char errorFile[50] = "error.log";
char dataFile[50]  = "Data.csv";

/* RTC PCF8523 constents and declerations */
RTC_PCF8523 rtc;


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
  Serial.print(event.gyro.x, 4);
  Serial.print("\tY: ");
  Serial.print(event.orientation.y, 4);
  Serial.print("\tZ: ");  
  Serial.print(event.orientation.z, 4);
  Serial.println("");
  
  delay(100);
}

char* string2char(String command){
    if (command.length() != 0) {
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}







/* --- SETUP ---- */
void setup() { 
  /* Setup for RFM95 */
  pinMode(LED,OUTPUT);
  digitalWrite(RFM95_RST,HIGH);
  digitalWrite(LED,LOW);
  delay(50);
  if(!SD.begin(chipSelect)){
    while(1){
      digitalWrite(LED,HIGH);
      delay(BLINKDELAY);
      digitalWrite(LED,LOW);
      delay(BLINKDELAY);
        digitalWrite(LED,HIGH);
        delay(3*BLINKDELAY);
        digitalWrite(LED,LOW);
      delay(1000);
   }// end fail case
  }
  
  if( !rtc.begin() ){
    while(1){
      for(int i = 0; i<2; i++){
        digitalWrite(LED,HIGH);
        delay(BLINKDELAY);
        digitalWrite(LED,LOW);
        
      }
      delay(1000);
    }
  }// end fail case

  
  DateTime now = rtc.now();

  errorLog = SD.open(errorFile,FILE_WRITE);

  
  if( !rfm95_Setup()){
    errorLog.println("ERROR: Failed to start RFM95 Module");
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

  /* Setup for BNO005 IMU Sensor */
  if(!bno.begin()){
    errorLog.println("ERROR: Failed connect to BNO055 IMU module");
    while(1){
      for(int i = 0;i<4;i++){
        digitalWrite(LED,HIGH);
        delay(BLINKDELAY);
        digitalWrite(LED,LOW);
        delay(BLINKDELAY);
      }
      delay(1000);
    }//end fail case
  }
  delay(1000);
  bno.setExtCrystalUse(true);

  
  //Serial setup for debugging
  Serial.begin(115200);

  delay(100);
  Serial.println("System Ready");
  errorLog.println("STATUS: System Reday");
  errorLog.println("STATUS: System waiting for launch Detection");
  
  errorLog.flush(); // save writen data to the card
  dataLog = SD.open(dataFile, FILE_WRITE);

}// end of setup

  sensors_event_t event[4]; 
    union Data data[4];
    Adafruit_BNO055::adafruit_vector_type_t vectors[4] ={ Adafruit_BNO055::VECTOR_ACCELEROMETER,
    Adafruit_BNO055::VECTOR_MAGNETOMETER, 
    Adafruit_BNO055::VECTOR_GYROSCOPE, 
    Adafruit_BNO055::VECTOR_EULER };










char buf[150];
int flushCount = 0;
  uint8_t dataSet[36];
 int bufCount = 0;
    int temp = 0;

    
void loop() {
  currentTime = millis();
  timeDiff = currentTime - lastTime_sensorEvent;
  
  if( timeDiff >= 100){
    for(int i = 0; i <4; i++){
      bno.getEvent(&event[i], vectors[i]);
    }
    set_flag(FLAG_DATA_AVAILABLE);
    lastTime_sensorEvent = millis();
    
  }



  

  if(flag_is_set(FLAG_DATA_AVAILABLE)){
    strcpy(buf,string2char(""));
    temp = 0;
    bufCount = 0;
    for( int i = 0; i < 4; i++)
       for( int j = 0; j <3; j++){
          data[i].fval[j] = event[i].data[j];
          strcat(buf,string2char(String(data[i].fval[j])) );
          strcat(buf,",");
          dataSet[temp] = data[i].bval[j];
          temp++;
       }
    strcat(buf,"\n");
   
    while(buf[bufCount] !=NULL){
      bufCount++;
    }
    dataLog.write(buf,bufCount);
    //dataLog.write('!');
    dataLog.write('\n');
    
    Serial.println(millis(),DEC);
    Serial.print("Data: ");
    Serial.print(buf);
    Serial.print("count: ");Serial.println(bufCount);
    dataLog.flush();
    if(currentTime -lastTime_transmitEvent >= 500){
      set_flag(FLAG_TRANSMIT_DATA);
      lastTime_transmitEvent = millis();
    }
  }

  if(flag_is_set(FLAG_TRANSMIT_DATA) ){
    
    Serial.println(millis(),DEC);
    currentMessage = DATA;
    digitalWrite(LED,HIGH);
    sendData(rf95, myAddress, reciverAddress, dataSet, 36, currentMessage);
    
    digitalWrite(LED,LOW);
    Serial.println("Data Sent");
    
//    delay(1000); // Used for debug
  }


  Serial.println("~~~~~~~~~~~~~~~~~");
  
}
