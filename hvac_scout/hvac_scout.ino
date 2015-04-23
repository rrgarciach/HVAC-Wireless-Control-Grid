#include <SoftwareSerial.h>
#include <IRSignalSender.h>
//#include "EmonLib.h" // Include Emon Library (current sensor)

// Definition of digital pins:
// slave Bluetooth socket to communicate with Master Device:
// bluetooth module's TX goes to arduino's TX (in this case, pin 3).
// in HC-06 pin 3 goes to BT's TX 
#define pinBtRx 2
#define pinBtTx 3
#define pinPIR 4
#define pinRedLed 12
#define pinIRLed 13
#define pinLM35 0
#define pinCurrent 1

SoftwareSerial master(pinBtRx, pinBtTx);
char srl = '0';

IRSignalSender irSender(pinIRLed);

//EnergyMonitor emon1; // Create an instance (current sensor)

uint8_t currentVal = 0;

bool booPIR = false;
bool booQuietZone = false;
bool hvacPower = false;

uint8_t tempCurrent = 0;

uint32_t timestampLastEvent = 0;
uint32_t timeDelayThreshold = 900000; // 900 seconds (15 minutes).
uint32_t timeElapsed;

void setup() {
  Serial.begin(9600);
  master.begin(9600);
  pinMode(pinPIR, INPUT);
  pinMode(pinLM35, INPUT);
  pinMode(pinRedLed, OUTPUT);
//  emon1.current(pinCurrent, 6.7); // Current: input pin, calibration (current sensor)
  Serial.println(F("Beginning..."));
}

void loop() {
  if (millis() % 1000 == 0) receiveCommands();
  if (millis() % 1000 == 0) updateStates();
  if (millis() % 10000 == 0) react();
//  if (millis() % 10000 == 0) sendPageSerial();
}

// Function to update state's variables:
void updateStates() {
  Serial.println(F("update states"));
  // PIR functionality:
  // check if movement:
  booPIR = digitalRead(pinPIR);
  // if movement:
  if (booPIR) {
      // store time stamp of event:
      timestampLastEvent = millis();
      Serial.println(F("movement..!"));
      // turn on led:
      digitalWrite(pinRedLed, HIGH);
  } else {
      // turn off led:
      digitalWrite(pinRedLed, LOW);
  }
  // get time lapse from the last event:
  timeElapsed = millis() - timestampLastEvent;
  // if time lapse is higher than the delay threshold, switch variable:
  booQuietZone = (timeElapsed > timeDelayThreshold) ? true : false;
  
//  currentVal = emon1.calcIrms(1480);  // Calculate Irms only (current sensor)
//  int Amps = currentVal*230.0;
//  power = (Amps > 1) ? true : false;
  
  // Temperature functionality:
  // Read temperature:
  tempCurrent = ( (5.0 * analogRead(pinLM35)*100.0)/1024.0 );
}

// Function to react/trigger physical controls:
void react() {
  Serial.println(F("reacting"));
  // turn off HVAC if quiet zone
  if (true == booQuietZone && true == hvacPower) turnOffHvac();
}

// Function to print serialized page:
void sendPageSerial() {
  Serial.println(F("sending page serial"));
  master.print(F("data:"));
  // send temperature:
  master.print(F("temp:"));
  master.print(tempCurrent);
  master.print(F(";"));
  // send delaytime:
  master.print(F("delay_time:"));
  master.print(timeDelayThreshold / 1000);
  master.print(F(";"));
  // send quiet zone:
  master.print(F("quiet:"));
  master.print(booQuietZone);
  master.print(F(";"));
  // send HVAC power state:
  master.print(F("power:"));
  master.print(hvacPower);
  master.print(F(";"));
  // terminate serial line:
  master.println("");
}

void receiveCommands() {
  if ( master.available() ) {
    Serial.println(F("reading from master"));
    String command; //string to store entire command line
    while ( master.available() ) {
      srl = master.read();
      delay(50);
      command += srl; //iterates char into string
      
      if (command == "getScouts;") { //this compares catched string vs. expected command string
        sendPageSerial();
      } else if (command == "turn_on;") { //this compares catched string vs. expected command string
        turnOnHvac();
      } else if (command == "turn_off;") { //this compares catched string vs. expected command string
        turnOffHvac();
      } else if (command == "delay_time:") { //this compares catched string vs. expected command string
        changeDelayTime();
      }
      
    }
  }
}

void turnOnHvac() {
//  if (true == hvacPower) {
//    master.println(F("already_on;"));
//  } else {
    master.println(F("turning_on;"));
    // send IR signal to HVAC device:
    irSender.sendCommand(0);
    // update HVAC state's variable:
    hvacPower = true;
//  }
}

void turnOffHvac() {
//  if (false == hvacPower) {
//    master.println(F("already_off;"));
//  } else {
    master.println(F("turning_off;"));
    // send IR signal to HVAC device:
    irSender.sendCommand(1);
    // update HVAC state's variable:
    hvacPower = false;
//  }
}

void changeDelayTime() {
  while ( !master.available() ) {}
  String strValue; //string to store entire command line
  if ( master.available() ) {
    while ( master.available() ) {
      srl = master.read();
      if (srl == ';') break;
      strValue += srl; //iterates char into string
      delay(50);
    }
  }
    uint16_t intValue = strValue.toInt();
    if (intValue > 0) {
        timeDelayThreshold = intValue;
        timeDelayThreshold *= 1000;
        master.print(F("OK"));
        delay(50);
    } else {
        master.print(F("ERROR\(1\)"));
        delay(50);
    }
}
