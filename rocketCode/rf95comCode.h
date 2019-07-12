/**
Header file for communication protocals for the RF95
module for rocket telemertry system
**/

/* messageType used to identify a message*/
enum messageType { STANDBY, STATE, DATA, COMMAND, TEST, ERROR,ACKNOWLEDGE };
enum commandType {SENDDATA, RESETSYSTEM};
/* Creates a array of uint8_t that combines the destination
   and senders address with the data. it also appends a CRC value
   to confirm message integraty. 
   message form
   		selfId
		receiverId
		MessageType
		Data
		CRC
 */
void sendData(RH_RF95 &rf95, uint8_t selfId, uint8_t reciverId, uint8_t *data, uint8_t bufLen, 
              messageType type){
  uint8_t package[bufLen + 5];
  byte crc = 0;
  package[0] = reciverId;
  crc ^= selfId;
  package[1] = selfId;
  crc ^= reciverId;
  package[2] = (uint8_t)type;
  crc ^= package[2];
  package[3] = bufLen;
  crc ^= package[3];
  for(int i = 0; i < bufLen;i++){
    package[4+i] = data[i];
    crc ^= data[i];
  }
  package[4+bufLen] = crc;

  rf95.send(package,5+bufLen);
  //  Serial.print("CRC: 0x"); // used for debugging
  // Serial.println(crc,HEX);  // used for debugging
}

void sendAcknoledge(RH_RF95 &rf95, uint8_t selfId, uint8_t receiverId ){
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

void sendError(RH_RF95 &rf95, uint8_t selfId, uint8_t receiverId){
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
