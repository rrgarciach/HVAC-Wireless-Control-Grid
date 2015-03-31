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
bool hvacPower = true;

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
  generatePage();
  react();
  sendPageSerial();
}

// Function to update state's variables:
void generatePage() {
  // PIR functionality:
  // check if movement:
  booPIR = digitalRead(pinPIR);
  // if movement, store time stamp of event:
  if (booPIR) timestampLastEvent = millis();
  // get time lapse from the last event:
  timeElapsed = millis() - timestampLastEvent;
  // if time lapse is higher than the delay threshold, switch variable:
  booQuietZone = (timeElapsed > timeDelayThreshold) ? false : true;
  
  // Temperature functionality:
  // Read temperature:
  tempCurrent = ( (5.0 * analogRead(pinLM35)*100.0)/1024.0 );
}

// Function to react/trigger physical controls:
void react() {
  // turn off HVAC if quiet zone
  if (true == booQuietZone) {
    // send IR signal to HVAC device:
    irSender.sendCommand(1);
    // update HVAC state's variable:
    hvacPower = true;
  }
}

// Function to print serialized page:
void sendPageSerial(SoftwareSerial stream) {
  // send temperature:
  stream.print(F("temp:"));
  stream.print(tempCurrent);
  // send quiet zone:
  stream.print(F("quiet:"));
  stream.print(booQuietZone);
  // send HVAC power state:
  stream.print(F("power:"));
  stream.print(hvacPower);
  // terminate serial line:
  stream.println("");
}
