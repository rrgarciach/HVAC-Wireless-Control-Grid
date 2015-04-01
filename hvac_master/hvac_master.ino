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
#define pinBtRxMobile 2
#define pinBtTxMobile 3
#define pinBtRxScout_01 5
#define pinBtTxScout_01 6

// slave Bluetooth socket to communicate with Mobile App:
// For HC-05 Bluetooth module's TX goes to arduino's TX (in this case, pin 3).
SoftwareSerial mobile(pinBtRxMobile,pinBtTxMobile);
// master Bluetooth socket to communicate with Scout Devices:
SoftwareSerial scout_01(pinBtRxScout_01,pinBtTxScout_01);

// String to process:
String message;
// variable to store received characters from Bluetooth devices:
char character;

void setup() {
  // The next two lines are to avoid the board from hanging after some requests:
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  mobile.begin(38400);
  scout_01.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
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
          client.print(F("\"tempZone01\":"));
          client.print(F(","));
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
  if ( scout_01.available() ) {
    character = scout_01.read();
    Serial.print(character);
    delay(50);
    message += character;
  }
  Serial.println(F(""));
  scout_01.println(F(""));
  if ( message == "no_movement" ) {
    Serial.println(F("More than 60 seconds without movement."));
  } else if ( message == "temperature" ) {}
}

void checkForMobile() {
  if ( mobile.available() ) {
    if ( mobile.available() ) {
    character = mobile.read();
    Serial.print(character);
    delay(50);
  }
  Serial.println("");
  mobile.println("");
  }
}
