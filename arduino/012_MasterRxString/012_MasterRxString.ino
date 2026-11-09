// Wire Master Transmitter and Receiver 
//Uno, Ethernet A4 (SDA), A5 (SCL)
#include <Wire.h>

uint8_t rcv_buf[32];
int data_len = 0;

#define SLAVE_ADDR 0x68

#define GET_LEN_CMD   0X51
#define GET_DATA_CMD  0x52

void setup() {
  Serial.begin(9600);
  Wire.begin();
}


void loop() {

  Serial.println("Arduino Master");
  Serial.println("Send character \"s\" to begin");

  char in_read = '\0';
  while(in_read != 's')
  {
    while(!Serial.available());
    in_read=Serial.read();
  }
  
  Serial.println("Starting..");

  // clear old buffer
  memset(rcv_buf, 0, sizeof(rcv_buf));

  Wire.beginTransmission(SLAVE_ADDR);
  
  Wire.write(0X51); //Send this command to read the length
  Wire.endTransmission();


  Wire.requestFrom(SLAVE_ADDR,1); // Request the transmitted two bytes from the two registers

  if(Wire.available()) {  // 
    data_len = Wire.read(); // Reads the data 
  }
  Serial.print("Data Length:");
  Serial.println(String(data_len,DEC));

  Wire.beginTransmission(SLAVE_ADDR);
  
  Wire.write(GET_DATA_CMD); //Send this command to ask data
  Wire.endTransmission();

  Wire.requestFrom(SLAVE_ADDR,data_len);

  uint32_t i=0;
  for( i =0; i <= data_len ; i++)
  {
    if(Wire.available()) {  // 
      rcv_buf[i] = Wire.read(); // Reads the data 
    }
  }
  rcv_buf[i] = '\0';

  Serial.print("Data:");
  Serial.println((char*)rcv_buf);
  Serial.println("*********************END*********************");
}
