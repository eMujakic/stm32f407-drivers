#include <string.h>

#define REQ_LEN_CMD  0x51
#define SEND_MSG_CMD 0x52

char msg[] = "Hello from Arduino!\n";

void setup() {
  Serial.begin(115200);
}

void loop() {
  while(! Serial.available()); //wait until something is received
  
  //read the data
  char in_read=Serial.read();

  if (in_read == SEND_MSG_CMD)
    Serial.print(msg);
  else if (in_read == REQ_LEN_CMD)
    Serial.write((uint8_t)strlen(msg));
}
