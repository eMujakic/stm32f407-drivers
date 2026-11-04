// Wire Slave Transmitter and receiver
//Uno, Ethernet A4 (SDA), A5 (SCL)
#include <Wire.h>

uint8_t active_command = 0xff;
char name_msg[32] = "Hello from Arduino!";
uint16_t device_id = 0xFF45;

#define SLAVE_ADDR 0x68
#define LEN_CMD   0x51
#define DATA_CMD  0x52

uint8_t get_len_of_data(void)
{
  return (uint8_t)strlen(name_msg);
}

void setup() {
  Serial.begin(9600);
  
  // Start the I2C Bus as Slave on address 9
  Wire.begin(SLAVE_ADDR); 
  
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  Serial.println("Slave is ready");
}


//write
void receiveEvent(int bytes) {
  active_command = Wire.read();    // read one character from the I2C
  Serial.print("Received Command: ");
  Serial.println(active_command);
}

//read
void requestEvent() {

  if(active_command == LEN_CMD)
  {
    uint8_t len = get_len_of_data();
    Wire.write(&len,1);
    active_command = 0xff;
  }
  

  if(active_command == DATA_CMD)
  {
    Wire.write(name_msg,get_len_of_data());
    active_command = 0xff;
  }
}
void loop() {
  

}
