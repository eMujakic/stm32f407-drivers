#include <SPI.h>

#define MAX_LEN 500

bool msgComplete = false;     // whether the string is complete
uint32_t cnt = 0;             // index variable
uint8_t userBuffer[MAX_LEN];

//Initialize SPI slave.
void SPI_SlaveInit(void) 
{ 
  // Initialize SPI pins.
  pinMode(SCK, INPUT);    // Pin 13
  pinMode(MOSI, INPUT);   // Pin 11
  pinMode(MISO, OUTPUT);  // Pin 12
  pinMode(SS, INPUT);     // Pin 10
  
  // Enable SPI as slave.
  SPCR = (1 << SPE);
}

//This function returns SPDR Contents 
uint8_t SPI_SlaveReceive(void)
{
  /* Wait for reception complete */
  while(!(SPSR & (1<<SPIF)));

  /* Return Data Register */
  return SPDR;
}


//sends one byte of data 
void SPI_SlaveTransmit(uint8_t data)
{
  /* Start transmission */
  SPDR = data;
  
  /* Wait for transmission complete */
  while(!(SPSR & (1<<SPIF)));

  // discards received byte
  (void)SPDR;
}
 
void setup() 
{
  // Initialize serial for troubleshooting.
  Serial.begin(9600);
  
  // Initialize SPI Slave.
  SPI_SlaveInit();

  // set interrupt line to high (assert low)
  pinMode(8, OUTPUT);
  digitalWrite(8,HIGH);

  Serial.println("Slave Initialized");
}

void loop() {
  
  Serial.println("Type anything and send...");

  // read user data from serial terminal
  while(!msgComplete){
    if (Serial.available()) {
      //Read a byte of incoming serial data.
      char readByte = (char)Serial.read();
      //Accumalate in to the buffer
      userBuffer[cnt++] = readByte;
      if((readByte == '\r') || (readByte == '\n') || ( cnt == MAX_LEN)){
        msgComplete = true;
        userBuffer[cnt -1 ] = '\0'; //replace '\r' by '\0'
      }
    }
  }
  
  Serial.println("Your message...");
  Serial.println((char*)userBuffer);

  // void both DR and SR
  (void)SPSR;
  (void)SPDR;

  // write first byte to DR before sending interrupt
  SPDR = userBuffer[0];

  // send interrupt to controller
  digitalWrite(8,LOW);

  // wait until transfer complete & discard received byte
  while(!(SPSR & (1<<SPIF)));
  (void)SPDR;

  /*Transmit the user buffer over SPI */
  for(uint32_t i = 1; i < cnt ; i++)
  {
    SPI_SlaveTransmit(userBuffer[i]);
  }

  // pull interrupt line back high
  digitalWrite(8,HIGH);
  
  cnt = 0;
  msgComplete = false;
  Serial.println("Message sent...");

  while(!digitalRead(SS));
  Serial.println("Master ends communication");
 
}
