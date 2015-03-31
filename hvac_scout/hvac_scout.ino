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

uint16_t timestampLastEvent = 0;
uint16_t timeDelayThreshold = 10000; // 60 seconds.
uint16_t timeElapsed;

void setup() {
  Serial.begin(9600);
  master.begin(38400);
  pinMode(pinPIR, INPUT);
  pinMode(pinLM35, INPUT);
  Serial.println(F("Beginning..."));
}

void loop() {
  // CREATE PAGE:
  // Read temperature:
  tempCurrent = ( (5.0 * analogRead(pinLM35)*100.0)/1024.0 );

  // PIR functionality:
  // check if movement:
  booPIR = digitalRead(pinPIR);
  // if movement, store time stamp of event:
  if (booPIR) timestampLastEvent = millis();
  // get time lapse from the last event:
  timeElapsed = millis() - timestampLastEvent;
  // if time lapse is higher than the delay threshold, switch variable:
  booQuietZone = (timeElapsed > timeDelayThreshold) ? false : true;

  // TURN ON command:
//  irSender.sendCommand(1);
//  delay(3000);
}

/*
0 error
1 
*/
