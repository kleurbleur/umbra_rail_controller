#include <Arduino.h>
#include <PubSubClient.h>
#include <ETH.h>
#include <MicroOscUdp.h>
#include <WiFiUdp.h>
#include <AccelStepper.h>


// DEBUG SETTINGS 1 = OSC INPUT
u_int8_t DEBUG = 1;

// PIN ASSIGNMENT
#define dirPin 4
#define stepPin 5
u_int8_t ind_sensor_a = 32;
u_int8_t ind_sensor_b = 33;


// NETWORK SETTINGS
unsigned int receivePort = 8888;
unsigned int sendPort = 7777;
IPAddress broadcastIp(255, 255, 255, 255);
IPAddress sendIp(192, 168, 178, 213);


// LIB SETUP
#define motorInterfaceType 1 // Motor interface type must be set to 1 when using a driver:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
WiFiClient ethclient;
WiFiUDP udp;
MicroOscUdp<1024> oscUdp(&udp, sendIp, sendPort);


// GLOBAL VARIABLES 
bool stepper_enable = false;
u_int32_t relative_position = 10000;


// ETHERNET FUNCTION
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


// OSC CALLBACK
void receivedOscMessage( MicroOscMessage& message) {

  if ( message.fullMatch("/enable/i", "i") ) {            // check for the full message match "/enable/i"
    int32_t val = message.nextAsInt();                    // make val a local var with the value received from the matched OSC message
    if (DEBUG == 1)
    {
      Serial.print("DEBUG /enable/i ");
      Serial.println(val);
    }
    if (val)                                              // if the received val is true (1)...
    {                                            
      stepper_enable = true;                              // 1. enable the stepper in the main loop via this flag
      Serial.printf("stepper_enable: ", stepper_enable);  // 2. inform via Serial 
      oscUdp.sendMessage( "/enable/i",  "i",  val);       // 3. send a validation back via OSC
      // stepper.enableOutputs(); // this does not seem to work
    }
    else if (!val)                                         // if the received val is false (0)...
    {
      stepper_enable = false;                              // 1. enable the stepper in the main loop via this flag
      Serial.printf("stepper_enable: ", stepper_enable);   // 2. inform via Serial
      oscUdp.sendMessage( "/enable/i",  "i",  val);        // 3. send a validation back via OSC
      // stepper.disableOutputs();  // this does not seem to work
    }
  }

  if ( message.fullMatch("/speed/i", "i") ) {
    int32_t val = message.nextAsInt();

    oscUdp.sendMessage( "/speed/i",  "i",  val);
    Serial.print("DEBUG /speed/i ");
    Serial.println(val);

    stepper.setMaxSpeed(val);
  }  

  if ( message.fullMatch("/position/i", "i") ) {
    int32_t val = message.nextAsInt();

    oscUdp.sendMessage( "/position/i",  "i",  val);
    Serial.print("DEBUG /position/i ");
    Serial.println(val);

    stepper.moveTo(val);
    Serial.println("Run to position");
    Serial.println(stepper.distanceToGo());
  }
  // // example code for later use
  // if ( message.fullMatch("/test/i", "i") ) {
  //   int32_t firstArgument = message.nextAsInt();
  //   oscUdp.sendMessage( "/test/i",  "i",  firstArgument);
  //   Serial.print("DEBUG /test/i ");
  //   Serial.println(firstArgument);
  // } else if ( message.fullMatch("/test/f",  "f")) {
  //   float firstArgument = message.nextAsFloat();
  //   oscUdp.sendMessage( "/test/f",  "f",  firstArgument);
  //   Serial.print("DEBUG /test/f ");
  //   Serial.println(firstArgument);
  // } else if ( message.fullMatch("/test/b",  "b")) {
  //   const uint8_t* blob;
  //   uint32_t length = message.nextAsBlob(&blob);
  //   if ( length != 0) {
  //     oscUdp.sendMessage( "/test/b", "b", blob, length);
  //     Serial.print("DEBUG /test/b ");
  //     for ( int i = 0; i < length; i++ ) {
  //       Serial.print(blob[i]);
  //     }
  //     Serial.println();
  //   }
  // } else if ( message.fullMatch("/test/s",  "s")) {
  //   const char * s = message.nextAsString();
  //   oscUdp.sendMessage( "/test/s",  "s",  s);
  //   Serial.print("DEBUG /test/s ");
  //   Serial.println(s);
  // } else if ( message.fullMatch("/test/m", "m")) {
  //   const uint8_t* midi;
  //   message.nextAsMidi(&midi);
  //   if ( midi != NULL ) {
  //     oscUdp.sendMessage( "/test/m",  "m", midi);
  //     Serial.print("DEBUG /test/m ");
  //     for ( int i = 0; i < 4; i++ ) {
  //       Serial.print(midi[i]);
  //     }
  //     Serial.println();
  //   }
  // }

}

// SENSOR FUNCTIONS
bool sensorA()
{
  if (digitalRead(ind_sensor_a)) // if we have input from sensor a...
  {
    stepper.stop(); // 1. then stop the stepper
    return true;    // 2. let the system know that the stepper is there
  }
  else if (!digitalRead(ind_sensor_a)) // if we don't have input from sensor a
  {
    return false;   // 1. let the system know that the stepper is NOT there
  }
}
bool sensorB()
{
  if (digitalRead(ind_sensor_b)) // if we have input from sensor a...
  {
    stepper.stop(); // 1. then stop the stepper
    return true;    // 2. let the system know that the stepper is there
  }
  else if (!digitalRead(ind_sensor_b))  // if we don't have input from sensor a
  {
    return false;   // 1. let the system know that the stepper is NOT there
  }
}

bool stepper_calibrated(){
  Serial.println("calibrating steppper");
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(1000);
  stepper.run();

  if (sensorA || sensorB)
  {
    relative_position = stepper.currentPosition(); // te testen
    return true;
  }
  else if (!sensorA || !sensorB)
  {
    return false;
  }

}


void setup()
{
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);
  ETH.begin();

  udp.begin(receivePort);

  stepper.setMaxSpeed(15000);
  stepper.setAcceleration(1500);
  stepper.disableOutputs();


}


void loop()
{

  sensorA();
  sensorB();

  if (!stepper_calibrated)
  {
    stepper_calibrated();
  }

  oscUdp.receiveMessages( receivedOscMessage );

  if (stepper_enable && stepper_calibrated)
  {
    stepper.run();
  }
  else if (stepper_enable && !stepper_calibrated)
  {
    Serial.println("calibrate the rails before usage");
  }

  if (stepper.isRunning())
  {
    Serial.println(stepper.distanceToGo());
  }



}