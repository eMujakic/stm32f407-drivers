void setup() {
  Serial.begin(115200);
  
  Serial.println("Arduino UART Receiver");
  Serial.println("-----------------------------");
    
}

void loop() {
  //wait until something is received
  while(! Serial.available());
  //read the data
  char in_read=Serial.read();
  //print the data
  Serial.print(in_read);
}
