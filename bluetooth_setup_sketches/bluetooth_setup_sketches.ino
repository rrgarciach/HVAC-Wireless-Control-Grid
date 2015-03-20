/*
The bluetooth's TX goes to arduino's TX (in this case, pin 5).
*/
#include <SoftwareSerial.h>

int rx = 6;
int tx = 5;

char srl = '0';

// Here's a bluetooth device connect to pins 6 and 5
SoftwareSerial bt(rx,tx);

void setup()
{
//  pinMode(7, OUTPUT);
//  digitalWrite(7, HIGH);
  Serial.begin(9600);
  //Serial.println("System Up: BLUETOOTH CONFIG SPECS:");
  bt.begin(38400);
  delay(10);

  // DISPLAY CONFIGS:
//  bt.println("AT"); //start AT commands
//  delay(10);
//  bt.println("AT+NAME?"); //get device name
//  delay(10);
  // bt.println("AT+ADDR?"); //get device address (for connection)
  // bt.println("AT+VERSION?"); //get device version
  // bt.println("AT+LINK?"); //connect to device (address)

  // SET GENERIC CONFIGS:
//  bt.println("AT+ROLE=0"); //set device as slave
//  delay(10);
//  bt.println("AT+NAME=HVAC_Master"); //set device name
//  delay(10);

  // SET MASTER CONFIGS:
  // bt.println("AT+ROLE=1"); //set device as master
  // bt.println("AT+NAME=HVAC_Master"); //set device name
  // bt.println("AT+LINK=12,12,272056"); //connect to device (address)
  // bt.println("AT+LINK=0014, 01, 091548"); // binding to an HC-06
  // bt.println("AT+PSWD=1234"); //types slave password
}

void loop()
{
  if ( bt.available() ) {
    while ( bt.available() ) {
      srl = bt.read();
      Serial.print(srl);
      delay(50);
    }
  }

  if ( Serial.available() ) {
    while ( Serial.available() ) {
      srl = Serial.read();
      bt.print(srl);
      delay(50);
      Serial.print(srl);
    }
    Serial.println("");
    bt.println("");
  }

}
