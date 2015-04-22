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
int pinsRxTx[10][2] = {{10,22}};
#elif defined(__AVR_ATmega2560__) // this is mega
#define pinSSD 4
#define pinBtRxMobile 10
#define pinBtTxMobile 22
HvacScout* scouts[10] = {};
int scoutArraySize = 10;
int pinsRxTx[10][2] = {
                        {11,23},
                        {12,24},
                        {13,25},
                        {62,26},
                        {63,27},
                        {64,28},
                        {65,29},
                        {66,30},
                        {67,31},
                        {68,32},
                    };
#else // everything else
#error "unknown MCU"
#endif

// slave Bluetooth socket to communicate with Mobile App:
// For HC-05 Bluetooth module's TX goes to arduino's TX (in this case, pin 3).
SoftwareSerial mobile(pinBtRxMobile, pinBtTxMobile);
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
	setHvacScout("ScoutPrueba00",0,3);
    // start Bluetooth's SPP protocol:
    startSPP();
  
    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip);
    server.begin();
    Serial.print(F("server is at "));
    Serial.println(Ethernet.localIP());
}

void loop() {
    // read each HVAC Scout data:
    checkForScouts();
    // check if is there any Mobile command:
    checkForMobile();
    // listen for incoming clients:
    checkForEthernet();
}

bool setHvacScout(String name, uint8_t slot, uint8_t pinKey) {
    for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) {
            scouts[i] = new HvacScout(name, pinsRxTx[slot][0], pinsRxTx[slot][1], pinKey, pin222A);
            scouts[i]->startSPP();
            scouts[i]->start();
            return true;
        }
    }
    Serial.println(F("ERROR: No space available for a new Scout."));
    return false;
}

int getScout(String name) {
    for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) continue;
        if (scouts[i]->getName() == name) {
            return i;
        }
    }
    return 404;
}

//	String getName();
//	uint8_t getTemperature();
//	bool getPower();
//	bool getQuiet();
//	uint16_t getDelayTime();
String scoutToJson(HvacScout* scout, int index) {
	String jsonObj;
	jsonObj += "{";
	jsonObj += "\"id\":";
	jsonObj += index;
	jsonObj += ",";
	jsonObj += "\"name\":";
	jsonObj += scout->getName();
	jsonObj += ",";
	jsonObj += "\"temperature\":";
	jsonObj += scout->getTemperature();
	jsonObj += ",";
	jsonObj += "\"power\":";
	jsonObj += scout->getPower();
	jsonObj += ",";
	jsonObj += "\"quiet\":";
	jsonObj += scout->getQuiet();
	jsonObj += ",";
	jsonObj += "\"delayTime\":";
	jsonObj += scout->getDelayTime();
	jsonObj += "}";
	return jsonObj;
}

String scoutsToJson(HvacScout* scouts[]) {
	String jsonArr;
	jsonArr += "[";
	for (int i = 0; i < scoutArraySize; i++) {
		if (scouts[i] == NULL) {
			jsonArr += scoutToJson(scouts[i],i);
			jsonArr += ",";
		}
	}
	jsonArr += "]";
	return jsonArr;
}

void startSPP() {
	Serial.print(F("size of array: "));
	Serial.println(scoutArraySize);
    for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) {
			Serial.print(i);
			Serial.println(F(" is NULL"));
			continue;
		}
		Serial.print(i);
		Serial.println(F(" is not NULL"));
		Serial.println(scouts[i]->getName());
        scouts[i]->startSPP();
        scouts[i]->start();
    }
}

// @TODO
void getTime() {
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
//    Serial.println(F("ACTION: checkForScouts"));
    String message; // String to process:
    // Read each HVAC Scout to capture their states:
    for (int i = 0; i < scoutArraySize; i++) {
        // If slot is NULL, step over:
        if (scouts[i] == NULL) continue;
        if ( scouts[i]->serial->available() ) {
            while ( scouts[i]->serial->available() ) {
              srl = scouts[i]->serial->read();
              Serial.print(srl);
              delay(50);
              message += srl;
              if ( message == "data:" ) {
                Serial.println();
                Serial.print(F("ACTION: reading data from scout "));
                Serial.println( scouts[i]->getName() );
                message = "";
              }
              if ( message == "temp:" ) {
                readTempFromHvacScout(scouts[i]);
                message = "";
              }
              if ( message == "delay_time:" ) {
                readDelayTimeFromHvacScout(scouts[i]);
                message = "";
              }
              if ( message == "quiet:" ) {
                readQuietFromHvacScout(scouts[i]);
                message = "";
              }
              if ( message == "power:" ) {
                readPowerFromHvacScout(scouts[i]);
                message = "";
              }
            }
        }
    }
}

// Read commands from Mobile:
void checkForMobile() {
//  Serial.println(F("ACTION: checkForMobile"));
  if ( mobile.available() ) {
      String message;
    while ( mobile.available() ) {
        srl = mobile.read();
        message += srl;
        Serial.print(srl);
//      delay(50);
        // on command setHvacScout:
        if (message == "setHvacScout:") {
            // Read stream for name:
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            String name = message;
            if (name == "") {
                Serial.println(F("ERROR: wrong name value."));
                mobile.println(F("Error\(1\)"));
            }
            // Read stream for slot:
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            uint8_t slot = message.toInt();
            if (slot < 0 || slot > 9) {
                Serial.println(F("ERROR: wrong slot value."));
                mobile.println(F("Error\(1\)"));
            }
            // Read stream for key:
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            uint8_t key = message.toInt();
            if (key == 0) {
                Serial.println(F("ERROR: wrong key value."));
                mobile.println(F("Error\(1\)"));
            }
            // Read stream for VCC:
            while ( mobile.available() ) {
                srl = mobile.read();
                message += srl;
                Serial.print(srl);
                if (srl == ';') break;
            }
            uint8_t vcc = message.toInt();
            if (vcc == 0) {
                Serial.println(F("ERROR: wrong vcc value."));
                mobile.println(F("Error\(1\)"));
            }
            // Create HVAC Scout:
			bool result = setHvacScout(name,slot,key);
            if (result == true) {
                Serial.println(F("SUCCESS: Scout created."));
                mobile.println(F("OK"));
            } else {
                Serial.println(F("ERROR: unable to create scout."));
                mobile.println(F("Error\(0\)"));
            }
        } else if (message == "getHvacScouts:") {
			Serial.println(F("Sending JSON of Scouts:"));
			mobile.println(scoutsToJson(scouts));
		}
    }
  }
}

void readTempFromHvacScout(HvacScout* scout) {
	Serial.println();
  Serial.println(F("ACTION: readTempFromHvacScout"));
  String strValue; //string to store entire command line
  if ( scout->serial->available() ) {
    while ( scout->serial->available() ) {
      srl = scout->serial->read();
      if (srl == ';') break;
      strValue += srl; //iterates char into string
      delay(50);
    }
  }
    uint8_t temperature = strValue.toInt();
    if (temperature > 0) {
        scout->setTemperature( temperature );
        Serial.print( scout->getName() );
        Serial.print(F(".temperature: "));
        Serial.println( scout->getTemperature() );
        delay(50);
        mobile.println(F("OK"));
    } else {
        Serial.print(F("Error reading temperature from scout: "));
        Serial.println( scout->getName() );
        mobile.println(F("Error\(0\)"));
    }
}

void readDelayTimeFromHvacScout(HvacScout* scout) {
  Serial.println();
  Serial.println(F("ACTION: readDelayTimeFromHvacScout"));
  String strValue; //string to store entire command line
  if ( scout->serial->available() ) {
    while ( scout->serial->available() ) {
      srl = scout->serial->read();
      if (srl == ';') break;
      strValue += srl; //iterates char into string
      delay(50);
    }
  }
    uint16_t delayTime = strValue.toInt();
//    delayTime *= 1000;
    if (delayTime > 0) {
        scout->setDelayTime( delayTime );
        Serial.print( scout->getName() );
        Serial.print(F(".delayTime: "));
        Serial.println( scout->getDelayTime() );
        delay(50);
        mobile.println(F("OK"));
    } else {
        Serial.print(F("Error reading delay time from scout: "));
        Serial.println( scout->getName() );
        mobile.println(F("Error\(0\)"));
    }
}

void readQuietFromHvacScout(HvacScout* scout) {
	Serial.println();
  Serial.println(F("ACTION: readQuietFromHvacScout"));
  String strValue; //string to store entire command line
  if ( scout->serial->available() ) {
    while ( scout->serial->available() ) {
      srl = scout->serial->read();
      if (srl == ';') break;
      strValue += srl; //iterates char into string
      delay(50);
    }
  }
    bool quiet = strValue;
    scout->setQuiet( quiet );
    Serial.print( scout->getName() );
    Serial.print(F(".quiet: "));
    Serial.println( scout->getQuiet() );
    delay(50);
    mobile.println(F("OK"));
}

void readPowerFromHvacScout(HvacScout* scout) {
	Serial.println();
    Serial.println(F("ACTION: readPowerFromHvacScout"));
    String strValue; //string to store entire command line
    if ( scout->serial->available() ) {
        while ( scout->serial->available() ) {
            srl = scout->serial->read();
            if (srl == ';') break;
            strValue += srl; //iterates char into string
            delay(50);
        }
    }
    bool power = strValue;
    scout->setPower( power );
    Serial.print( scout->getName() );
    Serial.print(F(".power: "));
    Serial.println( scout->getPower() );
    delay(50);
    mobile.println(F("OK"));
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
                printHeaders(client);
                printScoutJSON(client, scouts[0]);
                return;
          } else if ( message == "/02" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[1]);
                return;
          } else if ( message == "/03" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[2]);
                return;
          } else if ( message == "/04" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[3]);
                return;
          } else if ( message == "/05" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[4]);
                return;
          } else if ( message == "/06" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[5]);
                return;
          } else if ( message == "/07" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[6]);
                return;
          } else if ( message == "/08" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[7]);
                return;
          } else if ( message == "/09" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[8]);
                return;
          } else if ( message == "/10" ) {
                printHeaders(client);
                printScoutJSON(client, scouts[9]);
                return;
          } else if ( message == "/ " ) {
            // printing JSON response:
            Serial.println(F("reading all scouts"));
            printHeaders(client);
            client.print(F("["));
            printScoutsJSON(client);
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
            setScoutsDelayTimeFromPOST(client);
            message = "";
          } else if ( message == "delay_time=" ) {
            Serial.println(F("Setting delay time..."));
            setScoutsDelayTimeFromPOST(client);
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

void printHeaders(EthernetClient &client) {
  // send a standard HTTP response header
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));  // the connection will be closed after completion of the response
  // client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
}


void printScoutsJSON(EthernetClient &client) {
    for (int i = 0; i < scoutArraySize; i++) {
        // start scout JSON object:
        printScoutJSON(client, scouts[i]);
        if ( i < (scoutArraySize-1) ) client.print(F(","));
    }
}

void printScoutJSON(EthernetClient &client, HvacScout* scout) {
    Serial.print(F("ACTION: reading "));
    Serial.println( scouts[0]->getName() );
    if (scout != NULL) {
        // start scout JSON object:
        client.print(F("{"));
        // print name:
        client.print(F("\"name\":"));
        client.print(F("\""));
        client.print( scout->getName() );
        client.print(F("\","));
        // print temperature:
        client.print(F("\"temperature\":"));
        client.print( scout->getTemperature() );
        client.print(F(","));
        // print quiet zone:
        client.print(F("\"quiet\":"));
        client.print( scout->getQuiet() );
        client.print(F(","));
        // print power status:
        client.print(F("\"power\":"));
        client.print( scout->getPower() );
        client.print(F(","));
        // print delay time:
        client.print(F("\"delayTime\":"));
        client.print( scout->getDelayTime() );
        client.print(F("}"));
        // in case that the Scout doesn't exists:
    } else {
        client.print(F("{}"));
    }
}

// Turn ON ALL scouts after receiving command from HTTP client:
void turnOnPowerScoutsFromPOST(EthernetClient &client) {
    for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) continue;
        if (scouts[i]->getPower() == false) {
            scouts[i]->setPower(true);
            // send success response to HTTP client:
            client.print("power state:");
            client.println(scouts[i]->getPower());
        }
    }
}
// Turn ON scout after receiving command from HTTP client:
void turnOnPowerScoutFromPOST(EthernetClient &client, HvacScout* scout) {
    if (scout == NULL) return;
    if (scout->getPower() == false) {
        scout->setPower(true);
    }
    // send success response to HTTP client:
    client.print("power state:");
    client.println(scout->getPower());
}

// Turn OFF ALL scout after receiving command from HTTP client:
void turnOffPowerScoutsFromPOST(EthernetClient &client) {
    for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) continue;
        if (scouts[i]->getPower() == true) {
            scouts[i]->setPower(false);
        }
        client.print("power state:");
        client.println(scouts[i]->getPower());
    }
}
// Turn OFF scout after receiving command from HTTP client:
void turnOffPowerScoutsFromPOST(EthernetClient &client, HvacScout* scout) {
    if (scout == NULL) return;
    if (scout->getPower() == true) {
        scout->setPower(false);
    }
    client.print("power state:");
    client.println(scout->getPower());
}

// Read value from POST and change Delay Time of ALL scouts after receiving command from HTTP client:
void setScoutsDelayTimeFromPOST(EthernetClient &client) {
    String strValue;
    while ( client.available() ) {
        srl = client.read();
        if (srl == ';') break;
        strValue += srl;
    }
    
    uint16_t delayTime = strValue.toInt();
    if (delayTime > 0) {
        for (int i = 0; i < scoutArraySize; i++) {
            if (scouts[i] == NULL) continue;
            scouts[i]->setDelayTime( delayTime );
        }
        printScoutsJSON(client);
    } else {
        Serial.println(F("Error reading delay time from POST"));
        // Error for JSON response:
        client.println(F("{'error':{ 'code':0, 'message':'wrong delayTime provided' }"));
    }
}
// Read value from POST and change Delay Time of scout after receiving command from HTTP client:
void setScoutDelayTimeFromPOST(EthernetClient &client, HvacScout* scout) {
    String strValue;
    while ( client.available() ) {
        srl = client.read();
        if (srl == ';') break;
        strValue += srl;
    }
    
    uint16_t delayTime = strValue.toInt();
    if (delayTime > 0) {
        if (scout == NULL) return;
        scout->setDelayTime( delayTime );
        printScoutJSON(client, scout);
    } else {
        Serial.print(F("Error reading delay time from POST for scout "));
        // Error for JSON response:
        client.print(F("{'name':'"));
        client.print( scout->getName() );
        client.print(F("',"));
        client.print(F("'error':{ 'code':0, 'message':'wrong delayTime provided' }"));
        client.println(F("}"));
    }
}

// Read value from POST and change Delay Time after receiving command from HTTP client:
//void setScoutDelayTimeFromPOST(EthernetClient &client, HvacScout* scout) {}

//void setPropertyFromPOST(EthernetClient &HttpClient, HvacScout* scout, String strProperty, String strValue, char charTerminator) {
//    // read stream:
//    while ( HttpClient.available() ) {
//        srl = HttpClient.read();
//        // if terminator char found, stop reading stream:
//        if (srl == charTerminator) break;
//        // this line doesn't run if break was applied:
//        strValue += srl;
//    }
//    // send to selected Scout property:
//    scout.print(strProperty);
//    scout.print(F(":"));
//    scout.println(strValue);
//    if (readResponseFromScout(scout,"OK",'\n') == 100) {
//        // update local variable state
//        scout_01_delay_time = strValue.toInt();
//        HttpClient.print(strProperty);
//        HttpClient.print(F(":"));
//        HttpClient.println(strValue);
//        scout_02_delay_time = strValue.toInt();
//        scout_03_delay_time = strValue.toInt();
//    } else {
//    }
//    
//}
//
//int readResponseFromScout(SoftwareSerial &scout, String strExpectedResponse, char charTerminator) {
//    String strValue;
//    // read stream:
//    while ( scout.available() ) {
//        srl = scout.read();
//        // if terminator char found, stop reading stream:
//        if (srl == charTerminator) break;
//        // this line doesn't run if break was applied:
//        strValue += srl;
//    }
//    // if  expected result, return success
//    if (strValue == strExpectedResponse) {
//        // 100: success:
//        return 100;
//    } else {
//        // 0: unidentified error:
//        return 0;
//    }
//}
