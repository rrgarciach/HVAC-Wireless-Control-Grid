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
#if defined(__AVR_ATmega2560__) // this is mega
#define pinSSD 4
#define pinBtRxMobile 10
#define pinBtTxMobile 22
HvacScout* scouts[10] = {};
int scoutArraySize = 10;
String scoutGroups[5] = {"Group 1","Group 2","Group 3","Group 4","Group 5"};
int scoutGroupsArraySize = 5;
int pinsRxTx[10][3] = {
                        {11,23,34},
                        {12,24,35},
                        {13,25,36},
                        {62,26,37},
                        {63,27,38},
                        {64,28,39},
                        {65,29,40},
                        {66,30,41},
                        {67,31,42},
                        {68,32,43},
                    };
#else // everything else
#error "unknown MCU or not enough memory"
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
    Serial2.begin(9600);
    Serial.begin(9600);
	setNewHvacScout("ScoutPrueba00",0);
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
    if (millis() % 1000 == 0) checkForScouts();
    // check if is there any Mobile command:
    if (millis() % 1000 == 0) checkForMobile();
    // listen for incoming clients:
//    checkForEthernet();
}

bool setNewHvacScout(String name, uint8_t slot) {
//    for (int i = 0; i < scoutArraySize; i++) {
//        if (scouts[i] == NULL) {
//            scouts[i] = new HvacScout(name, pinsRxTx[slot][0], pinsRxTx[slot][1], pinKey, pin222A);
//            scouts[i]->startSPP();
//			scouts[i]->start();
//            return true;
//        }
//    }
	scouts[slot] = new HvacScout(name, pinsRxTx[slot][0], pinsRxTx[slot][1], pinsRxTx[slot][2], pin222A);
	scouts[slot]->startSPP();
//	scouts[slot]->start();
//    Serial.println(F("ERROR: No space available for a new Scout."));
//    return false;
}

void setHvacGroup(uint8_t scoutId, int8_t groupId) {
	scouts[scoutId]->setGroupId(groupId);
}
void setGroupName(int8_t groupId, String name) {
	scoutGroups[groupId] = name;
}
String getGroupName(int8_t groupId) {
	return scoutGroups[groupId];
}
String groupToJson(int index) {
	String jsonObj;
	jsonObj += F("{");
	jsonObj += F("\"id\":");
	jsonObj += index;
	jsonObj += F(",");
	jsonObj += F("\"name\":");
	jsonObj += scoutGroups[index];
	jsonObj += F("}");
	return jsonObj;
}
String scoutGroupsToJson() {
	String jsonArr;
	jsonArr += F("[");
	for (int i = 0; i < scoutGroupsArraySize; i++) {
		jsonArr += groupToJson(i);
		jsonArr += F(",");
	}
	jsonArr += F("]");
	return jsonArr;
}
void triggerScoutGroup(int8_t groupId, String action) {
	for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) continue;
        if (scouts[i]->getGroupId() == groupId) {
			if (action == F("turnOn")) scouts[i]->setPower(true);
			else if (action == F("turnOff")) scouts[i]->setPower(false);
			else if (action == F("autoOn")) scouts[i]->setAutomatic(true);
			else if (action == F("autoOff")) scouts[i]->setAutomatic(false);
        }
    }
}
void setValueForScoutGroupFromHardwareSerial(int8_t groupId, String action, HardwareSerial &serial) {
	for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) continue;
        if (scouts[i]->getGroupId() == groupId) {
			if (action == F("setDelayTime")) {
				uint8_t value = readArgumentFromHardwareSerial(serial,';').toInt();
				scouts[i]->setDelayTime(value);
			} else if (action == F("setMaxTemperature")) {
				uint8_t value = readArgumentFromHardwareSerial(serial,';').toInt();
				scouts[i]->setMaxTemperature(value);
			}
        }
    }
}

//int getScoutFromId(String name) {
//    for (int i = 0; i < scoutArraySize; i++) {
//        if (scouts[i] == NULL) continue;
//        if (scouts[i]->getName() == name) {
//            return i;
//        }
//    }
//    return 404;
//}

String scoutToJson(HvacScout* scout, int index) {
	String jsonObj;
	jsonObj += F("{");
	jsonObj += F("\"id\":");
	jsonObj += index;
	jsonObj += F(",");
	jsonObj += F("\"name\":");
	jsonObj += scout->getName();
	jsonObj += F(",");
	jsonObj += F("\"temperature\":");
	jsonObj += scout->getTemperature();
	jsonObj += F(",");
	jsonObj += F("\"power\":");
	jsonObj += scout->getPower();
	jsonObj += F(",");
	jsonObj += F("\"quiet\":");
	jsonObj += scout->getQuiet();
	jsonObj += F(",");
	jsonObj += F("\"delayTime\":");
	jsonObj += scout->getDelayTime();
	jsonObj += F("}");
	return jsonObj;
}
String scoutsToJson() {
	String jsonArr;
	jsonArr += F("[");
	for (int i = 0; i < scoutArraySize; i++) {
		if (scouts[i] != NULL) {
			jsonArr += scoutToJson(scouts[i],i);
			jsonArr += F(",");
		}
	}
	jsonArr += F("]");
	return jsonArr;
}
void triggerScout(int8_t scoutId, String action) {
	if (action == F("turnOn")) scouts[scoutId]->setPower(true);
	else if (action == F("turnOff")) scouts[scoutId]->setPower(false);
	else if (action == F("autoOn")) scouts[scoutId]->setAutomatic(true);
	else if (action == F("autoOff")) scouts[scoutId]->setAutomatic(false);
}
void setValueForScoutFromHardwareSerial(int8_t scoutId, String action, HardwareSerial &serial) {
	if (action == F("setDelayTime")) {
		uint8_t value = readArgumentFromHardwareSerial(serial,';').toInt();
		scouts[scoutId]->setDelayTime(value);
	} else if (action == F("setMaxTemperature")) {
		uint8_t value = readArgumentFromHardwareSerial(serial,';').toInt();
		scouts[scoutId]->setMaxTemperature(value);
	}
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
// @TODO
void setTime(uint8_t hours, uint8_t minutes) {
}

// Read commands from Mobile:
void checkForMobile() {
//  Serial.println(F("ACTION: checkForMobile"));
  if ( Serial2.available() ) {
      String message;
    while ( Serial2.available() ) {
        // on command setNewHvacScout:
		message = readArgumentFromHardwareSerial(Serial2, ';');
		if ( message == F("getHvacScouts") ) {
			Serial.println(F("Sending JSON of Scouts:"));
			Serial2.println(scoutsToJson());
        } else if ( message == F("setNewHvacScout") ) {
            // Read stream for name:
            String name = readArgumentFromHardwareSerial(Serial2,';');
            if (name == "") {
                Serial.println(F("ERROR: wrong name value."));
                Serial2.println(F("Error\(1\)"));
            }
            // Read stream for slot:
            uint8_t slot = readArgumentFromHardwareSerial(Serial2,';').toInt();
            if (slot < 0 || slot > 9) {
                Serial.println(F("ERROR: wrong slot value."));
                Serial2.println(F("Error\(1\)"));
            }
            // Read stream for key:
            uint8_t key = readArgumentFromHardwareSerial(Serial2,';').toInt();
            if (key == 0) {
                Serial.println(F("ERROR: wrong key value."));
                Serial2.println(F("Error\(1\)"));
            }
            // Read stream for VCC:
            uint8_t vcc = readArgumentFromHardwareSerial(Serial2,';').toInt();
            if (vcc == 0) {
                Serial.println(F("ERROR: wrong vcc value."));
                Serial2.println(F("Error\(1\)"));
            }
            // Create HVAC Scout:
			bool result = setNewHvacScout(name,slot);
            if (result == true) {
                Serial.println(F("SUCCESS: Scout created."));
                Serial2.println(F("OK"));
            } else {
                Serial.println(F("ERROR: unable to create scout."));
                Serial2.println(F("Error\(0\)"));
            }
		} else if ( message == F("setHvacGroup") ) {
			Serial.println(F("Setting HVAC group:"));
			// Read groupId:
			uint8_t scoutId = readArgumentFromHardwareSerial(Serial2,',').toInt();
            int8_t groupId = readArgumentFromHardwareSerial(Serial2,';').toInt();
			if (scoutId > 9 || scoutId < 0) {
                Serial.println(F("ERROR: wrong scoutId value."));
                Serial2.println(F("Error\(1\)"));
            } else {
				if (groupId > 4 || groupId < -1) {
					Serial.println(F("ERROR: wrong groupId value."));
					Serial2.println(F("Error\(1\)"));
				} else {
					setHvacGroup(scoutId,groupId);
					Serial.println(F("SUCCESS: Scout Group set."));
					Serial2.println(F("OK"));
				}
			}
		} else if ( message == F("triggerScout") ) {
			Serial.print(F("Triggering Scout \""));
			int8_t scoutId = readArgumentFromHardwareSerial(Serial2,',').toInt();
			Serial.print( scouts[scoutId]->getName() );
			Serial.println(F("\" :"));
			String action = readArgumentFromHardwareSerial(Serial2,';');
			
			triggerScout(scoutId,action);
			
		} else if ( message == F("setValueForScoutFromHardwareSerial") ) {
			Serial.println(F("Setting Value for Scout Group \""));
			int8_t scoutId = readArgumentFromHardwareSerial(Serial2,',').toInt();
			Serial.print( scouts[scoutId]->getName() );
			Serial.println(F("\" :"));
			String action = readArgumentFromHardwareSerial(Serial2,';');
			
			setValueForScoutFromHardwareSerial(scoutId,action,Serial2);
			
		} else if ( message == F("triggerScoutGroup") ) {
			Serial.print(F("Triggering Scout Group \""));
			int8_t groupId = readArgumentFromHardwareSerial(Serial2,',').toInt();
			Serial.print(scoutGroups[groupId]);
			Serial.println(F("\" :"));
			String action = readArgumentFromHardwareSerial(Serial2,';');
			
			triggerScoutGroup(groupId,action);
			
		} else if ( message == F("setValueForScoutGroupFromHardwareSerial") ) {
			Serial.println(F("Setting Value for Scout Group \""));
			int8_t groupId = readArgumentFromHardwareSerial(Serial2,',').toInt();
			Serial.print(scoutGroups[groupId]);
			Serial.println(F("\" :"));
			String action = readArgumentFromHardwareSerial(Serial2,';');
			
			setValueForScoutGroupFromHardwareSerial(groupId,action,Serial2);
			
		}
    }
  }
}

void checkForScouts() {
//    Serial.println(F("ACTION: checkForScouts"));
    String message; // String to process:
    // Read each HVAC Scout to capture their states:
    for (int i = 0 ; i < scoutArraySize ; i++) {
        // If slot is NULL, step over:
        if (scouts[i] == NULL) continue;
		scouts[i]->start();
		delay(50);
		scouts[i]->serial->println(F("getScouts;"));
		delay(1500);
		while ( scouts[i]->serial->available() ) {
			message = readArgumentFromSoftwareSerial(scouts[i]->serial,':');
			if ( message == F("data") ) {
				Serial.println();
				Serial.print(F("ACTION: reading data from scout "));
				Serial.println( scouts[i]->getName() );
				while ( scouts[i]->serial->available() ) {
					message = readArgumentFromSoftwareSerial(scouts[i]->serial,':');
					if ( message == F("temp") ) {
						readTempFromHvacScout(scouts[i]);
					} else if ( message == F("delay_time") ) {
						readDelayTimeFromHvacScout(scouts[i]);
					} else if ( message == F("quiet") ) {
						readQuietFromHvacScout(scouts[i]);
					} else if ( message == F("power") ) {
						readPowerFromHvacScout(scouts[i]);
					}
				}
			} 
			scouts[i]->end();
		}
    }
}

// Register here the automatic events/actions that you want to be triggered:
void react() {
	for (int i = 0 ; i < scoutArraySize ; i++) {
        // If slot is NULL, step over:
        if (scouts[i] == NULL) continue;
		
		// React if it's too hot:
		if (true == scouts[i]->getAutomatic()) {
			if (scouts[i]->getTemperature() > scouts[i]->getMaxTemperature()  
				&& scouts[i]->getPower() == false) {
				scouts[i]->setPower(true);
			} else if (scouts[i]->getTemperature() < (scouts[i]->getTemperature() - 3) && scouts[i]->getPower() == true) {
				// But if it got enough cold, turn off please:
				scouts[i]->setPower(false);
			}
		}
	}
}

void readTempFromHvacScout(HvacScout* scout) {
	Serial.println(F("\nACTION: readTempFromHvacScout"));
	uint8_t temperature = readArgumentFromSoftwareSerial(scout->serial,';').toInt();
    if (temperature > 0) {
        scout->setTemperature( temperature );
        Serial.println();
        Serial.print( scout->getName() );
        Serial.print(F(".temperature: "));
        Serial.println( scout->getTemperature() );
        delay(50);
//        Serial2.println(F("OK"));
    } else {
        Serial.print(F("Error reading temperature from scout= "));
        Serial.println( scout->getName() );
        Serial2.println(F("Error\(2\)"));
    }
}

void readDelayTimeFromHvacScout(HvacScout* scout) {
  Serial.println(F("\nACTION: readDelayTimeFromHvacScout"));
    uint16_t delayTime = readArgumentFromSoftwareSerial(scout->serial,';').toInt();
    if (delayTime > 0) {
        scout->setDelayTime( delayTime );
		Serial.println();
        Serial.print( scout->getName() );
        Serial.print(F(".delayTime: "));
        Serial.println( scout->getDelayTime() );
        delay(50);
//        Serial2.println(F("OK"));
    } else {
        Serial.print(F("Error reading delay time from scout= "));
        Serial.println( scout->getName() );
        Serial2.println(F("Error\(2\)"));
    }
}

void readQuietFromHvacScout(HvacScout* scout) {
	Serial.println(F("\nACTION: readQuietFromHvacScout"));
	bool quiet = readArgumentFromSoftwareSerial(scout->serial,';').toInt();
	scout->setQuiet( quiet );
	Serial.println();
	Serial.print( scout->getName() );
	Serial.print(F(".quiet= "));
	Serial.println( scout->getQuiet() );
	delay(50);
	//    Serial2.println(F("OK"));
}

void readPowerFromHvacScout(HvacScout* scout) {
    Serial.println(F("\nACTION: readPowerFromHvacScout"));
    bool power = readArgumentFromSoftwareSerial(scout->serial,';').toInt();
    scout->setPower( power );
	Serial.println();
    Serial.print( scout->getName() );
    Serial.print(F(".power: "));
    Serial.println( scout->getPower() );
    delay(50);
//    Serial2.println(F("OK"));
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
        if (message == F("GET")) {
          readGET(client);
        } else if (message == F("POST")) {
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
          if ( message == F("/01") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[0]);
                return;
          } else if ( message == F("/02") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[1]);
                return;
          } else if ( message == F("/03") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[2]);
                return;
          } else if ( message == F("/04") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[3]);
                return;
          } else if ( message == F("/05") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[4]);
                return;
          } else if ( message == F("/06") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[5]);
                return;
          } else if ( message == F("/07") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[6]);
                return;
          } else if ( message == F("/08") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[7]);
                return;
          } else if ( message == F("/09") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[8]);
                return;
          } else if ( message == F("/10") ) {
                printHeaders(client);
                printScoutJSON(client, scouts[9]);
                return;
          } else if ( message == F("/ ") ) {
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
          if ( message == F("power=on") ) {
            Serial.println(F("Setting ON power state..."));
            turnOnPowerScoutsFromPOST(client);
            message = "";
          } else if ( message == F("power=off") ) {
            Serial.println(F("Setting OFF power state..."));
            turnOffPowerScoutsFromPOST(client);
            message = "";
          } else if ( message == F("temp=") ) {
            Serial.println(F("Setting temperature..."));
            setScoutsDelayTimeFromPOST(client);
            message = "";
          } else if ( message == F("delay_time=") ) {
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
            client.print(F("power state:"));
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
    client.print(F("power state:"));
    client.println(scout->getPower());
}

// Turn OFF ALL scout after receiving command from HTTP client:
void turnOffPowerScoutsFromPOST(EthernetClient &client) {
    for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) continue;
        if (scouts[i]->getPower() == true) {
            scouts[i]->setPower(false);
        }
        client.print(F("power state:"));
        client.println(scouts[i]->getPower());
    }
}
// Turn OFF scout after receiving command from HTTP client:
void turnOffPowerScoutsFromPOST(EthernetClient &client, HvacScout* scout) {
    if (scout == NULL) return;
    if (scout->getPower() == true) {
        scout->setPower(false);
    }
    client.print(F("power state:"));
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

bool readCommandFromHardwareSerial(HardwareSerial &serial, String command, char terminator)
{
	if (command == readArgumentFromHardwareSerial(serial,terminator)) {
		return true;
	}
	return false;
}
String readArgumentFromHardwareSerial(HardwareSerial &serial, char terminator)
{
	String argument;
	char c;
	while ( serial.available() ) {
		c = serial.read();
		Serial.print(c);
		delay(50);
		if (c == terminator) break;
		argument += c;
	}
	return argument;
}
bool readCommandFromSoftwareSerial(SoftwareSerial* serial, String command, char terminator)
{
	if (command == readArgumentFromSoftwareSerial(serial,terminator)) {
		return true;
	}
	return false;
}
String readArgumentFromSoftwareSerial(SoftwareSerial* serial, char terminator)
{
	String argument;
	char c;
	while ( serial->available() ) {
		c = serial->read();
		Serial.print(c);
		delay(50);
		if (c == terminator) break;
		argument += c;
	}
	return argument;
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

/*
* ERROR CODE LIST:
* 0: 
* 1: Wrong value provided.
* 2: Wrong value received from response.
*/
