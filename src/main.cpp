#include <Arduino.h>
#include <PubSubClient.h>
#include <ETH.h>
#include <MicroOscUdp.h>
#include <WiFiUdp.h>

WiFiClient ethclient;
WiFiUDP udp;

unsigned int receivePort = 8888;
IPAddress broadcastIp(255, 255, 255, 255);
IPAddress sendIp(192, 168, 178, 213);
unsigned int sendPort = 7777;


// The number 1024 between the < > below  is the maximum number of bytes reserved for incomming messages.
// Outgoing messages are written directly to the output and do not need more reserved bytes.
MicroOscUdp<1024> oscUdp(&udp, sendIp, sendPort);




static bool eth_connected = false;




void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_ETH_START:
    Serial.println("ETH Started");
    // set eth hostname here
    ETH.setHostname("esp32-ethernet");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    Serial.println("ETH Connected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex())
    {
      Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.println("Mbps");
    eth_connected = true;
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH Disconnected");
    eth_connected = false;
    break;
  case ARDUINO_EVENT_ETH_STOP:
    Serial.println("ETH Stopped");
    eth_connected = false;
    break;
  default:
    break;
  }
}


// FUNCTION THAT WILL BE CALLED WHEN AN OSC MESSAGE IS RECEIVED:
void receivedOscMessage( MicroOscMessage& message) {
  // WHEN A MESSAGE IS MATCHED IT ECHOS IT THROUGH SERIAL(ASCII) AND UDP

  if ( message.fullMatch("/test/i", "i") ) {
    int32_t firstArgument = message.nextAsInt();

    oscUdp.sendMessage( "/test/i",  "i",  firstArgument);
    Serial.print("DEBUG /test/i ");
    Serial.println(firstArgument);

  } else if ( message.fullMatch("/test/f",  "f")) {
    float firstArgument = message.nextAsFloat();
    oscUdp.sendMessage( "/test/f",  "f",  firstArgument);
    Serial.print("DEBUG /test/f ");
    Serial.println(firstArgument);

  } else if ( message.fullMatch("/test/b",  "b")) {
    const uint8_t* blob;
    uint32_t length = message.nextAsBlob(&blob);
    if ( length != 0) {
      oscUdp.sendMessage( "/test/b", "b", blob, length);
      Serial.print("DEBUG /test/b ");
      for ( int i = 0; i < length; i++ ) {
        Serial.print(blob[i]);
      }
      Serial.println();
    }

  } else if ( message.fullMatch("/test/s",  "s")) {
    const char * s = message.nextAsString();
    oscUdp.sendMessage( "/test/s",  "s",  s);
    Serial.print("DEBUG /test/s ");
    Serial.println(s);

  } else if ( message.fullMatch("/test/m", "m")) {
    const uint8_t* midi;
    message.nextAsMidi(&midi);
    if ( midi != NULL ) {
      oscUdp.sendMessage( "/test/m",  "m", midi);
      Serial.print("DEBUG /test/m ");
      for ( int i = 0; i < 4; i++ ) {
        Serial.print(midi[i]);
      }
      Serial.println();
    }

  }

}


void setup()
{
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);
  ETH.begin();

  udp.begin(receivePort);


}


void loop()
{

  oscUdp.receiveMessages( receivedOscMessage );


}