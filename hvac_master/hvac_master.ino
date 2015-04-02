#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// Definition of digital pins:
#define pinBtRxMobile 6
#define pinBtTxMobile 5
#define pinBtRxScout_01 3
#define pinBtTxScout_01 2

// slave Bluetooth socket to communicate with Mobile App:
// For HC-05 Bluetooth module's TX goes to arduino's TX (in this case, pin 3).
SoftwareSerial mobile(pinBtRxMobile,pinBtTxMobile);
// master Bluetooth socket to communicate with Scout Devices:
SoftwareSerial scout_01(pinBtRxScout_01,pinBtTxScout_01);

// variable to store received characters from Bluetooth devices:
char srl;

// scout's state variables:
uint8_t scout_01_temp = 0;
uint32_t scout_01_delay_time = 0;
bool scout_01_quiet = false;
bool scout_01_power = false;

void setup() {
  // The next two lines are to avoid the board from hanging after some requests:
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  mobile.begin(9600);
  scout_01.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for Leonardo only
//  }
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print(F("server is at "));
  Serial.println(Ethernet.localIP());
}

void loop() {
  checkForScouts();
  checkForMobile();
  // listen for incoming clients
  checkForEthernet();
}

void checkForEthernet() {
//  Serial.print(F("checkForEthernet"));
  EthernetClient client = server.available();
  if (client) {
    Serial.println(F("new client"));
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: application/json"));
          client.println(F("Connection: close"));  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();

          // printing JSON response:
          client.print(F("{"));
          // print temperature:
          client.print(F("\"scout_01_temp\":"));
          client.print(scout_01_temp);
          client.print(F(","));
          // print quiet zone:
          client.print(F("\"scout_01_quiet\":"));
          client.print(scout_01_quiet);
          client.print(F(","));
          // print power status:
          client.print(F("\"scout_01_power\":"));
          client.print(scout_01_power);
          client.print(F(","));
          // print delay time:
          client.print(F("\"scout_01_delay_time\":"));
          client.print(scout_01_delay_time);
//          client.print(F(","));
          client.print(F("}"));

          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  }
}

void checkForScouts() {
//  Serial.println(F("checkForScouts"));
//  while ( !scout_01.available() ) {}
  String message; // String to process:
  if ( scout_01.available() ) {
    while ( scout_01.available() ) {
      srl = scout_01.read();
      Serial.print(srl);
      delay(50);
      message += srl;
//      if ( message == "data:" ) {
//        Serial.println(F("reading data:"));
//        while ( scout_01.available() ) {
          
//          message = "";
//          srl = scout_01.read();
//          Serial.print(srl);
//          delay(50);
//          message += srl;
          if ( message == "data:" ) {
            Serial.println();
            Serial.println(F("reading data:"));
            message = "";
          }
          if ( message == "temp:" ) {
            scout_01_ReadTemp();
            message = "";
          }
          if ( message == "delay_time:" ) {
            scout_01_ReadDelayTime();
            message = "";
          }
          if ( message == "quiet:" ) {
            scout_01_ReadQuiet();
            message = "";
          }
          if ( message == "power:" ) {
            scout_01_ReadPower();
            message = "";
          }
//        }
//      }
    }
  }
}

void checkForMobile() {
  Serial.println(F("checkForMobile"));
  if ( mobile.available() ) {
    if ( mobile.available() ) {
      srl = mobile.read();
      Serial.print(srl);
      delay(50);
    }
  }
}

void scout_01_ReadTemp() {
  Serial.println(F("readTemp..."));
//  while ( !scout_01.available() ) {}
  String strValue; //string to store entire command line
  if ( scout_01.available() ) {
    while ( scout_01.available() ) {
      srl = scout_01.read();
      if (srl == ';') break;
      strValue += srl; //iterates char into string
      delay(50);
    }
  }
  scout_01_temp = strValue.toInt();
  Serial.print(F("scout_01_temp: "));
  Serial.println(scout_01_temp);
  delay(50);
}

void scout_01_ReadDelayTime() {
  Serial.println(F("readDelayTime..."));
//  while ( !serial.available() ) {}
  String strValue; //string to store entire command line
  if ( scout_01.available() ) {
    while ( scout_01.available() ) {
      srl = scout_01.read();
      if (srl == ';') break;
      strValue += srl; //iterates char into string
      delay(50);
    }
  }
  scout_01_delay_time = strValue.toInt();
  scout_01_delay_time *= 1000;
  Serial.print(F("scout_01_delay_time: "));
  Serial.println(scout_01_delay_time);
  delay(50);
}

void scout_01_ReadQuiet() {
  Serial.println(F("readQuiet..."));
//  while ( !serial.available() ) {}
  String strValue; //string to store entire command line
  if ( scout_01.available() ) {
    while ( scout_01.available() ) {
      srl = scout_01.read();
      if (srl == ';') break;
      strValue += srl; //iterates char into string
      delay(50);
    }
  }
  scout_01_quiet = strValue.toInt();
  Serial.print(F("scout_01_quiet: "));
  Serial.println(scout_01_quiet);
  delay(50);
}

void scout_01_ReadPower() {
  Serial.println(F("readPower..."));
//  while ( !serial.available() ) {}
  String strValue; //string to store entire command line
  if ( scout_01.available() ) {
    while ( scout_01.available() ) {
      srl = scout_01.read();
      if (srl == ';') break;
      strValue += srl; //iterates char into string
      delay(50);
    }
  }
  scout_01_power = strValue.toInt();
  Serial.print(F("scout_01_power: "));
  Serial.println(scout_01_power);
  delay(50);
}
