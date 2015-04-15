/*
Note:
Not all pins on the Mega and Mega 2560 support change interrupts, 
so only the following can be used for RX: 
10, 11, 12, 13, 14, 15, 50**, 51**, 52**, 53**, A8 (62), A9 (63), A10 (64), 
A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).
**These pins are been used by Ethernet Shield.
*/
#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#include <HvacScout.h>
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress ip(192, 168, 1, 177);
IPAddress ip(172, 16, 19, 100);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// Definition of digital pins:
uint8_t pin222A = 2;
//#define pinKEYScout_01 3
#if defined(__AVR_ATmega328P__) // this is Uno
#define pinBtRxMobile 6
#define pinBtTxMobile 5
//#define pinBtRxScout_01 8
//#define pinBtTxScout_01 7
#elif defined(__AVR_ATmega2560__) // this is mega
#define pinSSD 4
#define pinBtRxMobile 10
#define pinBtTxMobile 22
HvacScout* scouts [10];
//#define pinBtRxScout_01 11
//#define pinBtTxScout_01 23
//#define pinBtRxScout_02 12
//#define pinBtTxScout_02 24
//#define pinBtRxScout_03 13
//#define pinBtTxScout_03 25
//#define pinBtRxScout_04 62
//#define pinBtTxScout_04 26
//#define pinBtRxScout_05 63
//#define pinBtTxScout_05 27
//#define pinBtRxScout_06 64
//#define pinBtTxScout_06 28
//#define pinBtRxScout_07 65
//#define pinBtTxScout_07 29
//#define pinBtRxScout_08 66
//#define pinBtTxScout_08 30
//#define pinBtRxScout_09 67
//#define pinBtTxScout_09 31
//#define pinBtRxScout_10 68
//#define pinBtTxScout_10 32
#else // everything else
#error "unknown MCU"
#endif

// slave Bluetooth socket to communicate with Mobile App:
// For HC-05 Bluetooth module's TX goes to arduino's TX (in this case, pin 3).
SoftwareSerial mobile(pinBtRxMobile,pinBtTxMobile);
// master Bluetooth socket to communicate with Scout Devices:
//SoftwareSerial scout_01(pinBtRxScout_01,pinBtTxScout_01);

// variable to store received characters from Bluetooth devices:
char srl;

void setup() {
  // The next two lines are to avoid the board from hanging after some requests:
  pinMode(pinSSD, OUTPUT);
  digitalWrite(pinSSD, HIGH);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  mobile.begin(9600);
//  scout_01.begin(38400);
//  pinMode(pin222AScout_01, OUTPUT);
//  pinMode(pinKEYScout_01, OUTPUT);
    // start Bluetooth's SPP protocol:
  startSPP();
//  Serial.println(F("Starting Serial"));
//  scout_01.end();
//  delay(500);
//  scout_01.begin(9600);
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

bool setScout(String name, uint8_t pinRx, uint8_t pinTx, uint8_t pinKey) {
    for (int i = 0; i < sizeof(scouts); i++) {
        if (scouts[i] == NULL) {
            scouts[i] = new HvacScout(name, pinRx, pinTx, pinKey, pin222A);
            scouts[i]->startSPP();
            scouts[i]->start();
            return true;
        }
    }
    return false;
}

int getScout(String name) {
    for (int i = 0; i < sizeof(scouts); i++) {
        if (scouts[i]->getName() == name) {
            return i;
        }
    }
    return 404;
}

void startSPP() {
//    Serial.println(F("Starting SPP"));
//    digitalWrite(pin222AScout_01, LOW);
//    digitalWrite(pinKEYScout_01, HIGH);
//    digitalWrite(pin222AScout_01, HIGH);
//    delay(500);
//    scout_01.println("AT");
//    delay(500);
//    scout_01.println("AT+INIT");
//    delay(1000);
//    digitalWrite(pin222AScout_01, LOW);
//    delay(500);
//    digitalWrite(pinKEYScout_01, LOW);
//    digitalWrite(pin222AScout_01, HIGH);
}

void checkForEthernet() {
//  Serial.print(F("checkForEthernet"));
  EthernetClient client = server.available();
  String message;
  if (client) {
    Serial.println(F("new client"));
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        message += c;
//        Serial.print(message);
        // GET verb received:
//        Serial.println(F("evaluating verb:"));
        if (message == "GET") {
          readGET(client);
        } else if (message == "POST") {
          readPOST(client);
        }
        if (!client.available()) break;
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
//          client.println(F("HTTP/1.1 200 OK"));
//          client.println(F("Content-Type: application/json"));
//          client.println(F("Connection: close"));  // the connection will be closed after completion of the response
//          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
//          client.println();

          // printing JSON response:
//          client.print(F("["));
//          printJSON_scout01(client);
//          client.print(F(","));
//          printJSON_scout02(client);
//          client.print(F("]"));

          break;
        }
        if (c == '\n') {
          // you're starting a new line
//          currentLineIsBlank = true;
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
    Serial.println(F("\nclient disconnected"));
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
      if ( message == "data:" ) {
        Serial.println();
        Serial.println(F("reading data:"));
        message = "";
      }
      if ( message == "temp:" ) {
        readTemp(scout_01);
        message = "";
      }
      if ( message == "delay_time:" ) {
        readDelayTime(scout_01);
        message = "";
      }
      if ( message == "quiet:" ) {
        readQuiet(scout_01);
        message = "";
      }
      if ( message == "power:" ) {
        readPower(scout_01);
        message = "";
      }
    }
  }
}

// Read commands from Mobile:
void checkForMobile() {
//  Serial.println(F("checkForMobile"));
  if ( mobile.available() ) {
      String message;
    while ( mobile.available() ) {
        srl = mobile.read();
        message += srl;
        Serial.print(srl);
//      delay(50);
        // on command setScout:
        if (message == "setScout:") {
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            String name = message.toInt();
            if (name == "") {
                Serial.println(F("Error on name value."));
                mobile.println(F("Error\(1\)"));
            }
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            uint8_t rx = message.toInt();
            if (rx == 0) {
                Serial.println(F("Error on rx value."));
                mobile.println(F("Error\(1\)"));
            }
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            uint8_t tx = message.toInt();
            if (tx == 0) {
                Serial.println(F("Error on tx value."));
                mobile.println(F("Error\(1\)"));
            }
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            uint8_t key = message.toInt();
            if (key == 0) {
                Serial.println(F("Error on key value."));
                mobile.println(F("Error\(1\)"));
            }
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            uint8_t vcc = message.toInt();
            if (vcc == 0) {
                Serial.println(F("Error on vcc value."));
                mobile.println(F("Error\(1\)"));
            }
            if ( setScout(name,rx,tx,key,vcc) ) {
                Serial.println(F("Scout created."));
                mobile.println(F("OK"));
            } else {
                Serial.println(F("Error while trying to create scout."));
                mobile.println(F("Error\(0\)"));
            }
        }
    }
  }
}

void readTemp(SoftwareSerial &serial) {
  Serial.println(F("readTemp..."));
//  while ( !scout_01.available() ) {}
  String strValue; //string to store entire command line
  if ( serial.available() ) {
    while ( serial.available() ) {
      srl = serial.read();
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

void readDelayTime(SoftwareSerial &serial) {
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

void readQuiet(SoftwareSerial &serial) {
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

void readPower(SoftwareSerial &serial) {
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

void readGET(EthernetClient &client) {
  Serial.print(F("\nreading GET verb"));
  if ( client.available() ) {
    String message; // String to process:
    while ( client.available() ) {
      srl = client.read();
      delay(50);
      Serial.print(srl);
      if (srl == '/') {
        message = "/";
        while ( client.available() ) {
          srl = client.read();
          delay(50);
          Serial.print(srl);
          message += srl;
          // read GET variables
          if ( message == "/01" ) {
            Serial.println(F("reading scout01"));
            printHeaders(client);
            printJSON_scout01(client);
            return;
          } else if ( message == "/02" ) {
            Serial.println(F("reading scout02"));
            printHeaders(client);
            printJSON_scout02(client);
            return;
          } else if ( message == "/03" ) {
    //          printJSON_scout03(client);
          } else if ( message == "/04" ) {
    //          printJSON_scout04(client);
          } else if ( message == "/05" ) {
          } else if ( message == "/06" ) {
          } else if ( message == "/07" ) {
          } else if ( message == "/08" ) {
          } else if ( message == "/09" ) {
          } else if ( message == "/10" ) {
          } else if ( message == "/11" ) {
          } else if ( message == "/12" ) {
          } else if ( message == "/ " ) {
            // printing JSON response:
            Serial.println(F("reading all scouts"));
            printHeaders(client);
            client.print(F("["));
            printJSON_scout01(client);
            client.print(F(","));
            printJSON_scout02(client);
            client.print(F("]"));
            return;
          } else if ( message.length() > 3) {
            Serial.println(F("No more recognized parameters."));
            return;
          }
        } // end while
      }
        
    }
  }
}

void readPOST(EthernetClient &client) {
  Serial.print(F("\nreading POST verb"));
  if ( client.available() ) {
    String message; // String to process:
    bool currentLineIsBlank = true;
    while ( client.available() ) {
      srl = client.read();
//      delay(50);
      Serial.print(srl);
      message += srl;
      if (srl == '\n') {
        currentLineIsBlank = true; // you're starting a new line
      }
      else if (srl != '\r') {
        currentLineIsBlank = false;// you've gotten a character on the current line
      }
      if (srl == '\r' && currentLineIsBlank) {
        message = "";
        Serial.print(F("POST detected!"));
        while ( client.available() ) {
          srl = client.read();
          if (srl == '&' || srl == '\n' || srl == '\r') {
            message = "";
            Serial.print(srl);
            continue;
          }
//          delay(50);
            Serial.print(srl);
            message += srl;
          // read GET variables
          if ( message == "power=on" ) {
            Serial.println(F("Setting ON power state..."));
            turnOnPowerScoutsFromPOST(client);
            message = "";
          } else if ( message == "power=off" ) {
            Serial.println(F("Setting OFF power state..."));
            turnOffPowerScoutsFromPOST(client);
            message = "";
          } else if ( message == "temp=" ) {
            Serial.println(F("Setting temperature..."));
            setDelayTimeFromPOST(client);
            message = "";
          } else if ( message == "delay_time=" ) {
            Serial.println(F("Setting delay time..."));
            setDelayTimeFromPOST(client);
            message = "";
          } else if ( message.length() > 15) {
            Serial.println(F("No more recognized parameters."));
            return;
          }
        } // end while
      }
        
    }
  }
}

//void readGETIndex(SoftwareSerial &serial) {
//  Serial.println(F("readTemp..."));
////  while ( !scout_01.available() ) {}
//  String strValue; //string to store entire command line
//  if ( serial.available() ) {
//    while ( serial.available() ) {
//      srl = serial.read();
//      if (srl == ';') break;
//      strValue += srl; //iterates char into string
//      delay(50);
//    }
//  }
//  scout_01_temp = strValue.toInt();
//  Serial.print(F("scout_01_temp: "));
//  Serial.println(scout_01_temp);
//  delay(50);
//}

void printHeaders(EthernetClient &client) {
  // send a standard HTTP response header
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));  // the connection will be closed after completion of the response
  //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
}


void printJSON_scout01(EthernetClient &client) {
  // print scout01:
  client.print(F("{"));
  // print name:
  client.print(F("\"name\":"));
  client.print(F("\""));
  client.print(scout_01_name);
  client.print(F("\","));
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
  client.print(F("}"));
}

void printJSON_scout02(EthernetClient &client) {
  // print scout02:
  client.print(F("{"));
  // print name:
  client.print(F("\"name\":"));
  client.print(F("\""));
  client.print(scout_02_name);
  client.print(F("\","));
  // print temperature:
  client.print(F("\"scout_02_temp\":"));
  client.print(scout_02_temp);
  client.print(F(","));
  // print quiet zone:
  client.print(F("\"scout_02_quiet\":"));
  client.print(scout_02_quiet);
  client.print(F(","));
  // print power status:
  client.print(F("\"scout_02_power\":"));
  client.print(scout_02_power);
  client.print(F(","));
  // print delay time:
  client.print(F("\"scout_02_delay_time\":"));
  client.print(scout_02_delay_time);
  client.print(F("}"));
}

void turnOnPowerScoutsFromPOST(EthernetClient &client) {
  if (!scout_01_power) {
    scout_01_power = true;
    scout_01.println("turn_on;");
  }
    client.print("power state:");
    client.println(scout_01_power);
}
void turnOffPowerScoutsFromPOST(EthernetClient &client) {
  if (scout_01_power) {
    scout_01_power = false;
    scout_01.println("turn_off;");
  }
    client.print("power state:");
    client.println(scout_01_power);
}

void setDelayTimeFromPOST(EthernetClient &client) {
  String strValue;
  while ( client.available() ) {
    srl = client.read();
    if (srl == ';') break;
    strValue += srl;
  }
  scout_01_delay_time = strValue.toInt();
  client.print(F("delay_time:"));
  client.println(scout_01_delay_time);
  scout_02_delay_time = strValue.toInt();
  scout_03_delay_time = strValue.toInt();
  
  scout_01.print(F("delay_time:"));
  scout_01.println(scout_01_delay_time);
}

void setPropertyFromPOST(EthernetClient &HttpClient, SoftwareSerial &scout, String strProperty, String strValue, char charTerminator) {
    // read stream:
    while ( HttpClient.available() ) {
        srl = HttpClient.read();
        // if terminator char found, stop reading stream:
        if (srl == charTerminator) break;
        // this line doesn't run if break was applied:
        strValue += srl;
    }
    // send to selected Scout property:
    scout.print(strProperty);
    scout.print(F(":"));
    scout.println(strValue);
    if (readResponseFromScout(scout,"OK",'\n') == 100) {
        // update local variable state
        scout_01_delay_time = strValue.toInt();
        HttpClient.print(strProperty);
        HttpClient.print(F(":"));
        HttpClient.println(strValue);
        scout_02_delay_time = strValue.toInt();
        scout_03_delay_time = strValue.toInt();
    } else {
    }
    
}

int readResponseFromScout(SoftwareSerial &scout, String strExpectedResponse, char charTerminator) {
    String strValue;
    // read stream:
    while ( scout.available() ) {
        srl = scout.read();
        // if terminator char found, stop reading stream:
        if (srl == charTerminator) break;
        // this line doesn't run if break was applied:
        strValue += srl;
    }
    // if  expected result, return success
    if (strValue == strExpectedResponse) {
        // 100: success:
        return 100;
    } else {
        // 0: unidentified error:
        return 0;
    }
}
