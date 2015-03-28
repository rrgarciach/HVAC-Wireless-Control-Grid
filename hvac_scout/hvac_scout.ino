#include <SoftwareSerial.h>
#include <IRSignalSender.h>

// Definition of digital pins:
// slave Bluetooth socket to communicate with Master Device:
// bluetooth module's TX goes to arduino's TX (in this case, pin 3).
const uint8_t pinBtRx = 2;
const uint8_t pinBtTx = 3;
const uint8_t pinRelay = 4;
const uint8_t pinPIR = 5;
const uint8_t pinIRLed = 13;
const uint8_t pinLM35 = 0;

SoftwareSerial master(pinBtRx,pinBtTx);
IRSignalSender irSender(pinIRLed);

bool booRelay = false;
bool booPIR = false;

uint16_t tempCurrent = 0;
uint16_t timeCurrent = 0;
uint16_t timeLastEvent = 0;
uint16_t timeDelayThreshold = 60000; // 60 seconds.
uint16_t timeThreshold;

void setup() {
//  Serial.begin(9600);
  master.begin(38400);
  pinMode(pinRelay, OUTPUT);
  pinMode(pinPIR, INPUT);
  pinMode(pinLM35, INPUT);
//  Serial.println("Beginning...");
}

void loop() {
  /*
  booPIR = digitalRead(pinPIR);
  tempCurrent = ( (5.0 * analogRead(pinLM35)*100.0)/1024.0 );
  
  // Check if movement:
  if ( booPIR ) {
    timeCurrent = millis();
    timeThreshold = timeCurrent - timeLastEvent;
  }
  // If there's no movement after 60 seconds, send data to Master Device
  if ( timeThreshold > timeDelayThreshold ) {
    master.println("no_movement");
  }
  
  master.println(tempCurrent);
  */
  irSender.sendCommand(1);
  delay(3000);
}
