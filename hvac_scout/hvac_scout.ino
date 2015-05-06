#include <SoftwareSerial.h>
#include <IRSignalSender.h>
#include <DHT.h>
//#include "EmonLib.h" // Include Emon Library (current sensor)

// Definition of digital pins:
// slave Bluetooth socket to communicate with Master Device:
// bluetooth module's TX goes to arduino's TX (in this case, pin 3).
// in HC-06 pin 3 goes to BT's TX 
#define pinBtRx 2
#define pinBtTx 3
#define pinPIR 5
#define pinRedLed 12
#define pinIRLed 13
#define DHTPIN 4
#define pinCurrent 1

SoftwareSerial master(pinBtRx, pinBtTx);
char srl = '0';

IRSignalSender irSender(pinIRLed);

//EnergyMonitor emon1; // Create an instance (current sensor)

uint8_t currentVal = 0;

bool booPIR = false;
bool booQuietZone = false;
bool hvacPower = false;
bool automatic = true;

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
uint8_t temperatureCurrent = 0;
uint8_t humidityCurrent = 0;

uint32_t timestampLastEvent = 0;
uint32_t timeDelayThreshold = 900000; // 900 seconds (15 minutes).
uint32_t timeElapsed;

void setup() {
  Serial.begin(115200);
  master.begin(38400);
  pinMode(pinPIR, INPUT);
  dht.begin();
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
  Serial.println(F("Update states"));
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
  temperatureCurrent = dht.readTemperature();
  humidityCurrent = dht.readHumidity();
}

// Function to react/trigger physical controls:
void react() {
  Serial.println(F("Reacting"));
  // turn off HVAC if quiet zone
  if (true == booQuietZone && true == hvacPower && true == automatic) turnOffHvac();
}

// Function to print serialized page:
void sendPageSerial() {
  Serial.println(F("Sending page serial:"));
  master.print(F("data:"));
  Serial.print(F("data:"));
  // send temperature:
  master.print(F("temp:"));
  master.print(temperatureCurrent);
  master.print(F(";"));
  Serial.print(F("temp:"));
  Serial.print(temperatureCurrent);
  Serial.print(F(";"));
  // send humidity:
  master.print(F("humi:"));
  master.print(humidityCurrent);
  master.print(F(";"));
  Serial.print(F("humi:"));
  Serial.print(humidityCurrent);
  Serial.print(F(";"));
  // send delaytime:
  master.print(F("delTime:"));
  master.print(timeDelayThreshold / 1000);
  master.print(F(";"));
  Serial.print(F("delTime:"));
  Serial.print(timeDelayThreshold / 1000);
  Serial.print(F(";"));
  // send quiet zone:
  master.print(F("quiet:"));
  master.print(booQuietZone);
  master.print(F(";"));
  Serial.print(F("quiet:"));
  Serial.print(booQuietZone);
  Serial.print(F(";"));
  // send HVAC power state:
  master.print(F("power:"));
  master.print(hvacPower);
  master.print(F(";"));
  Serial.print(F("power:"));
  Serial.print(hvacPower);
  Serial.print(F(";"));
  // send automatic state:
  master.print(F("auto:"));
  master.print(automatic);
  master.print(F(";"));
  Serial.print(F("auto:"));
  Serial.print(automatic);
  Serial.print(F(";"));
  // terminate serial line:
  master.println();
  Serial.println();
}

void receiveCommands() {
  if ( master.available() ) {
    Serial.println(F("Reading from master"));
    String command; //string to store entire command line
    while ( master.available() ) {
      srl = master.read();
		Serial.print(srl);
//      delay(50);
      command += srl; //iterates char into string
      //this compares catched string vs. expected command string
      if (command == F("getScout;")) {
        sendPageSerial();
		  
      } else if (command == F("getType;")) {
		Serial.print(F("hvac:HVAC Scout;"));
		  
      } else if (command == F("turnOn;")) {
        turnOnHvac();
		  
      } else if (command == F("turnOff;")) {
        turnOffHvac();
		  
      } else if (command == F("setDelTime:")) {
        changeDelayTime();
		  
      } else if (command == F("autoOn;")) {
        automatic = true;
		master.println(F("OK;"));
		  
      } else if (command == F("autoOff;")) {
        automatic = false;
		master.println(F("OK;"));
		  
      }
      
    }
  }
}

void turnOnHvac() {
master.println(F("turnOnHvac;"));
  if (true == hvacPower) {
    master.println(F("already_on;"));
    Serial.println(F("already_on;"));
  } else {
    master.println(F("turning_on;"));
    Serial.println(F("turning_on;"));
    // send IR signal to HVAC device:
    irSender.sendCommand(0);
    // update HVAC state's variable:
    hvacPower = true;
	  master.println(F("OK;"));
  }
}

void turnOffHvac() {
    master.println(F("turnOffHvac;"));
//  if (false == hvacPower) {
//    master.println(F("already_off;"));
//  } else {
    master.println(F("turning_off;"));
    Serial.println(F("turning_off;"));
    // send IR signal to HVAC device:
    irSender.sendCommand(1);
    // update HVAC state's variable:
    hvacPower = false;
	master.println(F("OK;"));
//  }
}

void changeDelayTime() {
	Serial.print(F("setDelTime:"));
    uint16_t intValue = readArgumentFromSoftwareSerial(master,';').toInt();
    if (intValue > 0) {
		Serial.println(intValue);
        timeDelayThreshold = intValue;
        timeDelayThreshold *= 1000;
        master.println(F("OK;"));
//        delay(50);
    } else {
        master.println(F("ERROR:1;"));
//        delay(50);
    }
}

String readArgumentFromSoftwareSerial(SoftwareSerial &serial, char terminator)
{
	String argument;
	char c;
	while ( serial.available() ) {
		c = serial.read();
		Serial.print(c);
//		delay(50);
		if (c == terminator) break;
		argument += c;
	}
	return argument;
}
