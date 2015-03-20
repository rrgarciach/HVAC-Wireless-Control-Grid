#include <SoftwareSerial.h>

// Definition of digital pins:
// slave Bluetooth socket to communicate with Master Device:
// bluetooth module's TX goes to arduino's TX (in this case, pin 3).
int pinBtTx = 3;
int pinBtRx = 2;
SoftwareSerial master(pinBtRx,pinBtTx);

int pinRelay = 4;
int pinPIR = 5;
int pinLM35 = 0;

bool booRelay = false;
bool booPIR = false;

int tempCurrent = 0;
int timeCurrent = 0;
int timeLastEvent = 0;
int timeDelayThreshold = 60000; // 60 seconds.

void setup() {
  Serial.begin(9600);
  master.begin(38400);
  pinMode(pinRelay, OUTPUT);
  pinMode(pinPIR, INPUT);
  pinMode(pinLM35, INPUT);
}

void loop() {
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
}
