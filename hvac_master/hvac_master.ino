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
#include <ScoutGroup.h>
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
#if defined(__AVR_ATmega2560__) // this is mega
#define pinSSD 4
#define pinBtRxMobile 10
#define pinBtTxMobile 22

int scoutArraySize = 10;
HvacScout* scouts[10] = {};

int scoutGroupsArraySize = 5;
ScoutGroup* scoutGroups[5] = {new ScoutGroup("Group 1"),
							  new ScoutGroup("Group 2"),
							  new ScoutGroup("Group 3"),
							  new ScoutGroup("Group 4"),
							  new ScoutGroup("Group 5")};

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

// For HC-05 Bluetooth module's TX goes to arduino's TX (in this case, pin 3).
// slave Bluetooth socket to communicate with Mobile App:
//SoftwareSerial mobile(pinBtRxMobile, pinBtTxMobile);

void setup() {
    // The next two lines are to avoid the board from hanging after some requests:
    pinMode(pinSSD, OUTPUT);
    digitalWrite(pinSSD, HIGH);
  
    // Open serial communications and wait for port to open:
    Serial2.begin(115200);
    Serial.begin(115200);
	setNewHvacScout("ScoutPrueba00",0);
	setNewHvacScout("escautcito",1);
//	setNewHvacScout("lalalalla",2);
//	setNewHvacScout("pancrasio scout",3);
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
//	if (millis() % 500) checkForScouts();
	checkForScouts();
	delay(50);
    // check if is there any Mobile command:
//    if (millis() % 500) checkForMobile();
    checkForMobile();
	delay(50);
	// react:
	react();
	delay(50);
    // listen for incoming clients:
    checkForEthernet();
	delay(500);
}

// Read commands from Mobile:
void checkForMobile() {
//  Serial.println(F("ACTION: checkForMobile"));
  if ( Serial2.available() ) {
      String message;
    while ( Serial2.available() ) {
		message = readFromHardwareSerial(Serial2, ';');
		if ( message == F("getFullState") ) {
			Serial.println(F("Sending JSON of Full State:"));
			Serial.println( fullStateToJson() );
			Serial2.println( fullStateToJson() );
			
        } else if ( message == F("getScouts") ) {
			Serial.println(F("Sending JSON of Scouts:"));
			String scouts = scoutsToJson();
			Serial.println(scouts);
			Serial2.println(scouts);
			
        } else if ( message == F("getScoutGroups") ) {
			Serial.println(F("Sending JSON of Groups:"));
			String groups = scoutGroupsToJson();
			Serial.println(groups);
			Serial2.println(groups);
			
        // on command setNewHvacScout:
        } else if ( message == F("newScout") ) {
            // Read stream for name:
            String name = readFromHardwareSerial(Serial2,',');
            if (name == "") {
                Serial.println(F("ERROR: wrong name value."));
                Serial2.println(F("Error\(1\)"));
            }
            // Read stream for slot:
            uint8_t slot = readFromHardwareSerial(Serial2,';').toInt();
            if (slot < 0 || slot > 9) {
                Serial.println(F("ERROR: wrong slot value."));
                Serial2.println(F("Error\(1\)"));
            }
            // Create HVAC Scout:
			bool result = newScout(name,slot);
            if (result == true) {
                Serial.println(F("SUCCESS: Scout created."));
                Serial2.println(F("OK"));
            } else {
                Serial.println(F("ERROR: unable to create scout."));
                Serial2.println(F("Error\(0\)"));
            }
			
//		} else if ( message == F("setScoutName") ) {
//            // Read stream for name:
//            String name = readFromHardwareSerial(Serial2,',');
//            if (name == "") {
//                Serial.println(F("ERROR: wrong name value."));
//                Serial2.println(F("Error:1"));
//            }
//            // Read stream for slot:
//            uint8_t slot = readFromHardwareSerial(Serial2,';').toInt();
//            if (slot < 0 || slot > 9) {
//                Serial.println(F("ERROR: wrong slot value."));
//                Serial2.println(F("Error:1"));
//            }
//            // Create HVAC Scout:
//			bool result = setScoutName(name,slot);
//            if (result == true) {
//                Serial.println(F("SUCCESS: Scout's name changed."));
//                Serial2.println(F("OK"));
//            } else {
//                Serial.println(F("ERROR: unable to change scout's name."));
//                Serial2.println(F("Error:0"));
//            }
//			
		} else if ( message == F("deleteScout") ) {
            // Read stream for slot:
            uint8_t slot = readFromHardwareSerial(Serial2,';').toInt();
            if (slot < 0 || slot > 9) {
                Serial.println(F("ERROR: wrong slot value."));
                Serial2.println(F("Error:1"));
            }
            // Create HVAC Scout:
			bool result = deleteScout(slot);
            if (result == true) {
                Serial.println(F("SUCCESS: Scout removed."));
                Serial2.println(F("OK"));
            } else {
                Serial.println(F("ERROR: unable to remove scout."));
                Serial2.println(F("Error:0"));
            }
			
		} else if ( message == F("setGroupId") ) {
			Serial.println(F("Setting HVAC group:"));
			// Read groupId:
			uint8_t scoutId = readFromHardwareSerial(Serial2,',').toInt();
            int8_t groupId = readFromHardwareSerial(Serial2,';').toInt();
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
			int8_t scoutId = readFromHardwareSerial(Serial2,',').toInt();
			String action = readFromHardwareSerial(Serial2,';');
			
			Serial.print(F("Triggering Scout \""));
			Serial.print( scouts[scoutId]->getName() );
			Serial.print(F("\". Action :"));
			Serial.println(action);
			
			triggerScout(scoutId,action);
			
		} else if ( message == F("setValScout") ) {
			int8_t scoutId = readFromHardwareSerial(Serial2,',').toInt();
			String action = readFromHardwareSerial(Serial2,';');
			
			Serial.print(F("Setting Value for Scout \""));
			Serial.print( scouts[scoutId]->getName() );
			Serial.print(F("\". Action :"));
			Serial.println(action);
			
			setValueForScoutFromHardwareSerial(scoutId,action,Serial2);
			
		} else if ( message == F("triggerScoutGroup") ) {
			int8_t groupId = readFromHardwareSerial(Serial2,',').toInt();
			String action = readFromHardwareSerial(Serial2,';');
			
			Serial.print(F("Triggering Scout Group \""));
			Serial.print(scoutGroups[groupId]->getName());
			Serial.print(F("\". Action :"));
			Serial.println(action);
			
			triggerScoutGroup(groupId,action);
			
		} else if ( message == F("setValueForScoutGroupFromHardwareSerial") ) {
			int8_t groupId = readFromHardwareSerial(Serial2,',').toInt();
			String action = readFromHardwareSerial(Serial2,';');
			
			Serial.print(F("Setting Value for Scout Group \""));
			Serial.print(scoutGroups[groupId]->getName());
			Serial.print(F("\". Action :"));
			Serial.println(action);
			
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
        if (scouts[i] == NULL) {
			continue;
		} else {
			scouts[i]->start();
			delay(50);
			scouts[i]->serial->println(F("getScout;"));
			delay(1500);
			if ( scouts[i]->serial->available() ) {
				scouts[i]->resetPing();
				Serial.print(F("Reseted Ping for "));
				Serial.println( scouts[i]->getName() );
			} else {
				scouts[i]->ping();
				Serial.print(F("Ping for "));
				Serial.println( scouts[i]->getName() );
			}
			while ( scouts[i]->serial->available() ) {
				message = readFromSoftwareSerial(scouts[i]->serial,':');
				if ( message == F("data") ) {
//					Serial.println();
					Serial.print(F("ACTION: reading data from scout "));
					Serial.println( scouts[i]->getName() );
					while ( scouts[i]->serial->available() ) {
						message = readFromSoftwareSerial(scouts[i]->serial,':');
						if ( message == F("temp") ) {
							readTempFromHvacScout(scouts[i]);
						} else if ( message == F("humi") ) {
							readHumidityFromHvacScout(scouts[i]);
						} else if ( message == F("delTime") ) {
							readDelayTimeFromHvacScout(scouts[i]);
						} else if ( message == F("quiet") ) {
							readQuietFromHvacScout(scouts[i]);
						} else if ( message == F("power") ) {
							readPowerFromHvacScout(scouts[i]);
						} else if ( message == F("auto") ) {
							readPowerFromHvacScout(scouts[i]);
						}
					}
				} else {
				}
				scouts[i]->end();
			}
		}
    }
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
        Serial.println(F("evaluating verb:"));
		message = readFromEthernet(client,' ');
        if (message == F("GET")) {
          readGET(client);
        } else if (message == F("POST")) {
          readPOST(client);
        }
        if (!client.available()) break;
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
//        if (c == '\n' && currentLineIsBlank) {
//          // send a standard HTTP response header
////          client.println(F("HTTP/1.1 200 OK"));
////          client.println(F("Content-Type: application/json"));
////          client.println(F("Connection: close"));  // the connection will be closed after completion of the response
////          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
////          client.println();
//
//          // printing JSON response:
//
//          break;
//        }
//        if (c == '\n') {
//          // you're starting a new line
////          currentLineIsBlank = true;
//        }
//        else if (c != '\r') {
//          // you've gotten a character on the current line
//          currentLineIsBlank = false;
//        }
//      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println(F("\nClient disconnected"));
  }
}

// Register here the automatic events/actions that you want to be triggered:
void react() {
//	React individual Scouts:
	for (int i = 0 ; i < scoutArraySize ; i++) {
        // If slot is NULL, step over:
        if (scouts[i] == NULL) continue;
		
		// React if it's too hot:
		if (scouts[i]->getAutomatic() == true && scouts[i]->getGroupId() < 0) {
			if (scouts[i]->getTemperature() > scouts[i]->getMaxTemperature() && scouts[i]->getPower() == false) {
				scouts[i]->setPower(true);
				
			// But if it got enough cold, turn off please:
			} else if (scouts[i]->getTemperature() < (scouts[i]->getTemperature() - 3) && scouts[i]->getPower() == true) {
				scouts[i]->setPower(false);
			}
		}
	}
//	React Scout Groups:
	for (int i = 0 ; i < scoutGroupsArraySize ; i++) {
        // If slot is NULL, step over:
        if (scoutGroups[i] == NULL) continue;
		// Check if Scout is on automatic:
		if (scoutGroups[i]->getAutomatic() == true) {
		// React if it's too hot and power is off:
			if (scoutGroups[i]->getTemperature() > scoutGroups[i]->getMaxTemperature() && scoutGroups[i]->getPower() == false) {
				for (int k = 0 ; k < scoutArraySize ; k++) {
					if (scouts[k]->getGroupId() == i)
						scouts[k]->triggerPower(true);
				}
				// update Group's state to avoid repeating:
				scoutGroups[i]->setPower(true);
				
			// But if it got enough cold, turn off please:
			} else if (scoutGroups[i]->getTemperature() < (scoutGroups[i]->getTemperature() - 3) && scoutGroups[i]->getPower() == true) {
				for (int k = 0 ; k < scoutArraySize ; k++) {
					if (scouts[k]->getGroupId() == i)
						scouts[k]->triggerPower(false);
				}
				scoutGroups[i]->setPower(false);
			}
		}
	}
}

// @TODO
void getTime() {}
// @TODO
void setTime(uint8_t hours, uint8_t minutes) {}

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
// Create Scout:
bool newScout(String name, uint8_t slot) {
	scouts[slot] = new HvacScout(name, pinsRxTx[slot][0], pinsRxTx[slot][1], pinsRxTx[slot][2], pin222A);
	scouts[slot]->startSPP();
}
// Delete Scout:
bool deleteScout(uint8_t slot) {
	delete scouts[slot];
	scouts[slot] = NULL;
	return true;
}
bool isLastScout(uint8_t index) {
	for (int i = index+1; i < scoutArraySize; i++) {
		// If scout slot is not NULL, print include it in JSON:
		if (scouts[i] != NULL) return false; 
	}
	return true;
}

void triggerScout(int8_t scoutId, String action) {
	if (action == F("turnOn")) scouts[scoutId]->triggerPower(true);
	else if (action == F("turnOff")) scouts[scoutId]->triggerPower(false);
	else if (action == F("autoOn")) scouts[scoutId]->setAutomatic(true);
	else if (action == F("autoOff")) scouts[scoutId]->setAutomatic(false);
}
void setValueForScoutFromHardwareSerial(int8_t scoutId, String action, HardwareSerial &serial) {
	// Give time to read Serial buffer:
//	delay(500);
	if (action == F("setDelTime")) {
		uint32_t value = readFromHardwareSerial(serial,';').toInt();
		scouts[scoutId]->changeDelayTime(value);
		
	} else if (action == F("setGroupId")) {
		int8_t value = readFromHardwareSerial(serial,';').toInt();
		scouts[scoutId]->setGroupId(value);
		
	} else if (action == F("setMaxTemp")) {
		uint8_t value = readFromHardwareSerial(serial,';').toInt();
		scouts[scoutId]->setMaxTemperature(value);
		
	} else if (action == F("setScoutName")) {
		String name = readFromHardwareSerial(Serial2,';');
		scouts[scoutId]->setName(name);
	}
}

void readTempFromHvacScout(HvacScout* scout) {
	Serial.println(F("\nACTION: readTempFromHvacScout"));
	uint8_t temperature = readFromSoftwareSerial(scout->serial,';').toInt();
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
        Serial2.print(F("Error:2 on "));
        Serial2.print( scout->getName() );
		Serial2.print(F(" when reading temperature from scout"));
        Serial2.println(F(";"));
    }
}
void readHumidityFromHvacScout(HvacScout* scout) {
	Serial.println(F("\nACTION: readHumidityFromHvacScout"));
	uint8_t humidity = readFromSoftwareSerial(scout->serial,';').toInt();
    if (humidity >= 0) {
        scout->setHumidity( humidity );
        Serial.println();
        Serial.print( scout->getName() );
        Serial.print(F(".humidity: "));
        Serial.println( scout->getHumidity() );
        delay(50);
//        Serial2.println(F("OK"));
    } else {
        Serial.print(F("Error reading humidity from scout= "));
        Serial.println( scout->getName() );
        Serial2.print(F("Error:2 on "));
        Serial2.print( scout->getName() );
		Serial2.print(F(" when reading humidity from scout"));
        Serial2.println(F(";"));
    }
}
void readDelayTimeFromHvacScout(HvacScout* scout) {
  Serial.println(F("\nACTION: readDelayTimeFromHvacScout"));
    uint16_t delayTime = readFromSoftwareSerial(scout->serial,';').toInt();
    if (delayTime > 0) {
        scout->setDelayTime( delayTime );
		Serial.println();
        Serial.print( scout->getName() );
        Serial.print(F(".delayTime: "));
        Serial.println( scout->getDelayTime() );
        delay(50);
//        Serial2.println(F("OK"));
    } else {
        Serial.print(F("Error reading delayTime from scout= "));
        Serial.println( scout->getName() );
        Serial2.print(F("Error:2 on "));
        Serial2.print( scout->getName() );
        Serial2.print(F(" when reading delayTime from scout"));
        Serial2.println(F(";"));
    }
}
void readQuietFromHvacScout(HvacScout* scout) {
	Serial.println(F("\nACTION: readQuietFromHvacScout"));
	bool quiet = readFromSoftwareSerial(scout->serial,';').toInt();
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
    bool power = readFromSoftwareSerial(scout->serial,';').toInt();
    scout->setPower( power );
	Serial.println();
    Serial.print( scout->getName() );
    Serial.print(F(".power: "));
    Serial.println( scout->getPower() );
    delay(50);
//    Serial2.println(F("OK"));
}
void readAutoFromHvacScout(HvacScout* scout) {
    Serial.println(F("\nACTION: readAutoFromHvacScout"));
    bool automatic = readFromSoftwareSerial(scout->serial,';').toInt();
    scout->setAutomatic( automatic );
	Serial.println();
    Serial.print( scout->getName() );
    Serial.print(F(".power: "));
    Serial.println( scout->getAutomatic() );
    delay(50);
//    Serial2.println(F("OK"));
}



// set Group for Scout:
void setHvacGroup(uint8_t scoutId, int8_t groupId) {
	scouts[scoutId]->setGroupId(groupId);
}
//// set name for Scout
//void setGroupName(int8_t groupId, String name) {
//	scoutGroups[groupId]->setName(name);
//}
//// get Group's name:
//String getGroupName(int8_t groupId) {
//	return scoutGroups[groupId]->getName();
//}
void triggerScoutGroup(int8_t groupId, String action) {
	for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) continue;
        if (scouts[i]->getGroupId() == groupId) {
			if (action == F("turnOn")) scouts[i]->triggerPower(true);
			else if (action == F("turnOff")) scouts[i]->triggerPower(false);
			else if (action == F("autoOn")) scouts[i]->triggerAutomatic(true);
			else if (action == F("autoOff")) scouts[i]->triggerAutomatic(false);
        }
    }
}
void setValueForScoutGroupFromHardwareSerial(int8_t groupId, String action, HardwareSerial &serial) {
	for (int i = 0; i < scoutArraySize; i++) {
        if (scouts[i] == NULL) continue;
        if (scouts[i]->getGroupId() == groupId) {
			if (action == F("setDelTime")) {
				uint8_t value = readFromHardwareSerial(serial,';').toInt();
				scouts[i]->changeDelayTime(value);
			} else if (action == F("setGroupId")) {
				uint8_t value = readFromHardwareSerial(serial,';').toInt();
				scouts[i]->setGroupId(value);
			} else if (action == F("setMaxTemp")) {
				uint8_t value = readFromHardwareSerial(serial,';').toInt();
				scouts[i]->setMaxTemperature(value);
			} else if (action == F("setName")) {
				uint8_t value = readFromHardwareSerial(serial,';').toInt();
				scouts[i]->setMaxTemperature(value);
			}
        }
    }
}

String scoutsToJson() {
	String jsonArr;
	jsonArr += F("[");
	for (int i = 0; i < scoutArraySize; i++) {
		// If scout slot is not NULL, print include it in JSON:
		if (scouts[i] != NULL) {
			jsonArr += scouts[i]->scoutToJson(i);
			// If it's not the last one, add the comma separator
			if (isLastScout(i) == false)
				jsonArr += F(",");
		}
	}
	jsonArr += F("]");
	return jsonArr;
}
String scoutGroupsToJson() {
	String jsonArr;
	jsonArr += F("[");
	for (int i = 0; i < scoutGroupsArraySize; i++) {
		jsonArr += scoutGroups[i]->groupToJson(i);
		if ( i != scoutGroupsArraySize-1)
			jsonArr += F(",");
	}
	jsonArr += F("]");
	return jsonArr;
}
String fullStateToJson() {
	String fullState;
	fullState += "{\"scouts\":";
	fullState += scoutsToJson();
	fullState += ",\"groups\":";
	fullState += scoutGroupsToJson();
	fullState += "}";
	return fullState;
}






void readGET(EthernetClient &client) {
  Serial.println(F("\nReading GET verb"));
  if ( client.available() ) {
    String message; // String to process:
	  message = readFromEthernet(client,' ');
	  // read GET variables
	  if ( message == F("/01") ) {
			printHeaders(client);
			if (scouts[0] != NULL)
			client.println( scouts[0]->scoutToJson(0) );
			return;
	  } else if ( message == F("/02") ) {
			printHeaders(client);
			if (scouts[1] != NULL)
			client.println( scouts[1]->scoutToJson(1) );
			return;
	  } else if ( message == F("/03") ) {
			printHeaders(client);
			if (scouts[2] != NULL)
				client.println( scouts[2]->scoutToJson(2) );
			return;
  // ......................................................... //
	  } else if ( message == F("/10") ) {
			if (scouts[9] != NULL)
			printHeaders(client);
			client.println( scouts[9]->scoutToJson(9) );
			return;
	  } else if ( message == F("/") ) {
			// printing JSON response:
			Serial.println(F("Reading all scouts:"));
			printHeaders(client);
			client.print(fullStateToJson());
			return;
	  } else if ( message.length() > 3) {
			Serial.println(F("No more recognized parameters."));
			return;
	  }
  }
}

void readPOST(EthernetClient &client) {
	Serial.println(F("\nReading POST verb:"));
	if ( client.available() ) {
		String message; // String to process:
//		message = readFromEthernet(client,' ');
		bool currentLineIsBlank = true;
		while ( client.available() ) {
			char c;
			c = client.read();
//			delay(50);
			Serial.print(c);
			message += c;
			// This next conditional is to detect /r/n which means the end of all parameters:
			if (c == '\n') {
				currentLineIsBlank = true; // you're starting a new line
			} else if (c != '\r') {
				currentLineIsBlank = false; // you've gotten a character on the current line
			}
			if (c == '\r' && currentLineIsBlank) {
				// Start reding POST parameters:
				Serial.print(F("POST detected!"));
				message = "";
				// Define if action is for single Scout or for Group:
				int id;
				bool scout = false;
				bool group = false;
				while ( client.available() ) {
					c = client.read();
					if (c == '&' || c == '\n' || c == '\r') {
						message = "";
						Serial.print(c);
						continue;
					}
//					delay(50);
					Serial.print(c);
					message += c;
					// Read GET variables:
					if ( message == F("scout=") ) {
						id = readFromEthernet(client,'&').toInt();
						Serial.print("scout");
						// Trigger action for Scout or for Group:
						scout = true;
						group = false;
						message="";

					} else if ( message == F("group=") ) {
						id = readFromEthernet(client,'&').toInt();
						scout = false;
						group = true;
						message="";

					} else if ( message == F("turnOn=") ) {
						Serial.println(F("Setting ON power state..."));
						readFromEthernet(client,'&').toInt();
						// Trigger action for Scout or for Group:
						if (scout == true) {
							if (scouts[id] != NULL) scouts[id]->triggerPower(true);
						} else if (group == true) {
							if (scoutGroups[id] != NULL) scoutGroups[id]->triggerPower(true,scouts,scoutGroupsArraySize);
						}
						message="";

					} else if ( message == F("turnOff=") ) {
						Serial.println(F("Setting OFF power state..."));
						readFromEthernet(client,'&').toInt();
						// Trigger action for Scout or for Group:
						if (scout == true) {
							if (scouts[id] != NULL) scouts[id]->triggerPower(false);
						} else if (group == true) {
							if (scoutGroups[id] != NULL) scoutGroups[id]->triggerPower(false,scouts,scoutGroupsArraySize);
						}
						message="";

					} else if ( message == F("autoOn=") ) {
						Serial.println(F("Setting OFF automatic state..."));
						readFromEthernet(client,'&').toInt();
						// Trigger action for Scout or for Group:
						if (scout == true) {
							if (scouts[id] != NULL) scouts[id]->setAutomatic(true);
						} else if (group == true) {
							if (scoutGroups[id] != NULL) scoutGroups[id]->setAutomatic(true);
						}
						message="";

					} else if ( message == F("autoOff=") ) {
						Serial.println(F("Setting OFF automatic state..."));
						readFromEthernet(client,'&').toInt();
						// Trigger action for Scout or for Group:
						if (scout == true) {
							if (scouts[id] != NULL) scouts[id]->setAutomatic(false);
						} else if (group == true) {
							if (scoutGroups[id] != NULL) scoutGroups[id]->setAutomatic(false);
						}
						message="";

					} else if ( message == F("setDelTime=") ) {
						Serial.println(F("Setting delay time..."));
						uint16_t value = readFromEthernet(client,'&').toInt();
//						// Trigger action for Scout or for Group:
						if (scout == true) {
							if (scouts[id] != NULL) scouts[id]->changeDelayTime(value);
						} else if (group == true) {
							if (scoutGroups[id] != NULL) scoutGroups[id]->changeDelayTime(value,scouts,scoutGroupsArraySize);
						}
						message="";

					} else if ( message == F("setMaxTemp=") ) {
						Serial.println(F("Setting max temperature..."));
						int value = readFromEthernet(client,'&').toInt();
//						// Trigger action for Scout or for Group:
						if (scout == true) {
							if (scouts[id] != NULL) scouts[id]->changeDelayTime(value);
						} else if (group == true) {
							if (scoutGroups[id] != NULL) scoutGroups[id]->changeDelayTime(value,scouts,scoutGroupsArraySize);
						}
						message="";

					} else if ( message == F("setName=") ) {
						Serial.println(F("Setting Scout Name..."));
						String name = readFromEthernet(client,'&');
//						// Trigger action for Scout or for Group:
						if (scout == true) {
							if (scouts[id] != NULL) scouts[id]->setName(name);
						} else if (group == true) {
							if (scoutGroups[id] != NULL) scoutGroups[id]->setName(name);
						}
						message="";

					} else if ( message == F("setGroupId=") ) {
						Serial.println(F("Setting Group ID..."));
						int groupId = readFromEthernet(client,'&').toInt();
						if (groupId >= scoutGroupsArraySize) return;
						// Trigger action for Scout or for Group:
						if (scout == true) {
							if (scouts[id] != NULL) scouts[id]->setGroupId(groupId);
						} else if (group == true) {}
						message="";

					} else if ( message.length() > 15 ) {
						Serial.println(F("No more recognized parameters."));
						return;

					}
				} // end while
				printHeaders(client);
				client.print("{\"result\":\"success\"}");
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

String readFromHardwareSerial(HardwareSerial &serial, char terminator)
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
String readFromSoftwareSerial(SoftwareSerial* serial, char terminator)
{
	String argument;
	char c;
	while ( serial->available() ) {
		c = serial->read();
		Serial.print(c);
//		delay(50);
		if (c == terminator) break;
		argument += c;
	}
	return argument;
}
String readFromEthernet(EthernetClient &client, char terminator)
{
	String argument;
	char c;
	while ( client.available() ) {
		c = client.read();
		Serial.print(c);
//		delay(50);
		if (c == terminator) break;
		argument += c;
	}
	return argument;
}

/*
* ERROR CODE LIST:
* 0: Unknown error.
* 1: Wrong value provided.
* 2: Wrong value received from response.
* 3: 
* 4: 
* 5: 
*/
