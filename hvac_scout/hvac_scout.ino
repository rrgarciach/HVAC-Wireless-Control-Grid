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
char srl = '0';

IRSignalSender irSender(pinIRLed);

bool booPIR = false;
bool booQuietZone = false;
bool hvacPower = true;

uint16_t tempCurrent = 0;

uint16_t timestampLastEvent = 0;
uint16_t timeDelayThreshold = 900000; // 900,000 seconds (15 minutes).
uint16_t timeElapsed;

void setup() {
  Serial.begin(115200);
  master.begin(9600);
  pinMode(pinPIR, INPUT);
  pinMode(pinLM35, INPUT);
  Serial.println(F("Beginning..."));
}

void loop() {
  if (millis() % 1000 == 0) receiveCommands(master);
  if (millis() % 1000 == 0) updateStates();
  if (millis() % 1000 == 0) react();
  if (millis() % 10000 == 0) sendPageSerial(master);
  
}

// Function to update state's variables:
void updateStates() {
  Serial.println(F("update states"));
  // PIR functionality:
  // check if movement:
  booPIR = digitalRead(pinPIR);
  // if movement, store time stamp of event:
  if (booPIR) timestampLastEvent = millis();
  // get time lapse from the last event:
  timeElapsed = millis() - timestampLastEvent;
  // if time lapse is higher than the delay threshold, switch variable:
  booQuietZone = (timeElapsed > timeDelayThreshold) ? true : false;
  
  // Temperature functionality:
  // Read temperature:
  tempCurrent = ( (5.0 * analogRead(pinLM35)*100.0)/1024.0 );
}

// Function to react/trigger physical controls:
void react() {
  Serial.println(F("reacting"));
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
  Serial.println(F("send page serial"));
  // send temperature:
  stream.print(F("temp:"));
  stream.print(tempCurrent);
  stream.print(F(";"));
  // send quiet zone:
  stream.print(F("quiet:"));
  stream.print(booQuietZone);
  stream.print(F(";"));
  // send HVAC power state:
  stream.print(F("power:"));
  stream.print(hvacPower);
  stream.print(F(";"));
  // terminate serial line:
  stream.println("");
}

void receiveCommands(SoftwareSerial stream) {
  if ( master.available() ) {
    Serial.println(F("READING"));
    String command; //string to store entire command line
    while ( master.available() ) {
      srl = master.read();
      delay(50);
      command += srl; //iterates char into string
    }
    if (command == "turnon") { //this compares catched string vs. expected command string
      if (true == hvacPower) {
        master.println(F("already_on"));
      } else {
        master.println(F("turning_on"));
        irSender.sendCommand(0);
      }
    } else if (command == "turnoff") { //this compares catched string vs. expected command string
      if (false == hvacPower) {
        master.println(F("already_off"));
      } else {
        master.println(F("turning_off"));
        irSender.sendCommand(1);
      }
    }
  }
  
}
