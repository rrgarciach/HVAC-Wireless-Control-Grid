#include <SoftwareSerial.h>
#include <IRSignalSender.h>

// Definition of digital pins:
// slave Bluetooth socket to communicate with Master Device:
// bluetooth module's TX goes to arduino's TX (in this case, pin 3).
#define pinBtRx 2
#define pinBtTx 3
#define pinPIR 4
#define pinIRLed 13
#define pinLM35 0

SoftwareSerial master(pinBtRx, pinBtTx);
IRSignalSender irSender(pinIRLed);

bool booPIR = false;
bool booQuietZone = false;

uint16_t tempCurrent = 0;

uint16_t timeLastEvent = 0;
uint16_t timeDelayThreshold = 10000; // 60 seconds.
uint16_t timeThreshold;

void setup() {
  Serial.begin(9600);
  master.begin(38400);
  pinMode(pinPIR, INPUT);
  pinMode(pinLM35, INPUT);
  Serial.println("Beginning...");
}

void loop() {
  
  tempCurrent = ( (5.0 * analogRead(pinLM35)*100.0)/1024.0 );

  // Check if movement:
  booPIR = digitalRead(pinPIR);
  Serial.println(booPIR);
  if ( booPIR ) {
    timeLastEvent = millis();
  }
  timeThreshold = millis() - timeLastEvent;
  if (timeThreshold > timeDelayThreshold) {
    booQuietZone = true;
    Serial.println("movement");
  } else {
    booQuietZone = false;
    Serial.println("quiet");
  }
  
  // If there's no movement after 60 seconds, send data to Master Device
  if ( timeThreshold > timeDelayThreshold ) {
    master.println("no_movement");
  }

  master.println(tempCurrent);
  // TURN ON command:
//  irSender.sendCommand(1);
//  delay(3000);
}

/*
0 error
1 
*/
