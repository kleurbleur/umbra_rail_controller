#include <Arduino.h>
#include <PubSubClient.h>
#include <ETH.h>
#include <MicroOscUdp.h>
#include <WiFiUdp.h>
#include <AccelStepper.h>


// DEBUG SETTINGS 1 = OSC INPUT
u_int8_t DEBUG = 1;

// PIN ASSIGNMENT
#define dirPin 4                // Direction pin output for the Stepper Driver
#define stepPin 5               // Step pin output for the Stepper Driver
u_int8_t ind_sensor_a = 32;     // Input pin from sensor a - WARNING! -> only to be connected via an optocoupler!
u_int8_t ind_sensor_b = 33;     // Input pin from sensor b - WARNING! -> only to be connected via an optocoupler!


// NETWORK SETTINGS
IPAddress sendIp(192, 168, 178, 213);         // the ip address of the receiving party
unsigned int receivePort = 8888;              // the port in which the OSC messsage come in
unsigned int sendPort = 7777;                 // and the port towards the OSC messages are send
IPAddress broadcastIp(255, 255, 255, 255);    // did not test it yet, but looks like a broadcast option



// ================================================================================================
// AFTER THIS DON'T EDIT UNLESS YOU KNOW WHAT IS WHAT
// ================================================================================================



// LIB SETUP
#define motorInterfaceType 1 // Motor interface type must be set to 1 when using a driver:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
WiFiClient ethclient;
WiFiUDP udp;
MicroOscUdp<1024> oscUdp(&udp, sendIp, sendPort);


// GLOBAL VARIABLES 
bool stepper_enable = false;
bool stepper_calibrated = false;
u_int32_t relative_position = 0;


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

  if ( message.fullMatch("/calibration/i", "i") ) {       // check for the full message match "/calibration/i" so for the calibration command
    int32_t val = message.nextAsInt();                    // make val a local var with the value received from the matched OSC message
    if (DEBUG == 1)
    {
      Serial.print("DEBUG /calibration/i ");
      Serial.println(val);
    }
    if (val)                                              // if the received val is true (1)...
    {                                            
      stepperCalibration();                               // 1. start the calibration function
    }
    else if (!val)                                         // if the received val is false (0)...
    {
      Serial.printf("stepper_enable: ", stepper_enable);   // 2. inform via Serial
      oscUdp.sendMessage( "/enable/i",  "i",  val);        // 3. send a validation back via OSC
    }
  }                                                        // end check for the enable command 


  if ( message.fullMatch("/enable/i", "i") ) {            // check for the full message match "/enable/i" so for the enable command
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
  }                                                        // end check for the enable command 

  if ( message.fullMatch("/speed/i", "i") ) {              // check for the full message match "/speed/i" so for the speed command
    int32_t val = message.nextAsInt();                     // make val a local var with the value received from the matched OSC message
    if (DEBUG == 1)
    {
    Serial.print("DEBUG /speed/i ");
    Serial.println(val);
    }
    stepper.setMaxSpeed(val);                              // set the stepper speed with the received val
    oscUdp.sendMessage( "/speed/i",  "i",  val);           // send a validation back via OSC
  }  

  if ( message.fullMatch("/position/i", "i") ) {           // check for the full message match "/position/i" so for the position command
    int32_t val = message.nextAsInt();                     // make val a local var with the value received from the matched OSC message
    if (DEBUG == 1)
    {
    Serial.print("DEBUG /position/i ");
    Serial.println(val);
    }
    stepper.moveTo(val+relative_position);                 // make sure to use the calibrated distance from the sensor with incoming position 
    Serial.print("Target position: ");                     // inform via Serial
    Serial.println(stepper.distanceToGo());
    oscUdp.sendMessage( "/position/i",  "i",  val);        // send a validation back via OSC (only val, because the calibrated distance will alter per setup)
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
bool sensorA()                                             // setup a true/false function to check if the sensors have input or not
{
  if (digitalRead(ind_sensor_a))                           // if we have input from sensor a...
  {
    stepper_enable = false;                                // 0. flag the stepper to stop
    stepper.stop();                                        // 1. then stop the stepper
    return true;                                           // 2. let the system know that the stepper is there
  }
  else if (!digitalRead(ind_sensor_a))                     // if we don't have input from sensor a
  {
    return false;                                          // 1. let the system know that the stepper is NOT there
  }
}
bool sensorB()                                             // setup a true/false function to check if the sensors have input or not
{
  if (digitalRead(ind_sensor_b))                           // if we have input from sensor a...
  {
    stepper_enable = false;                                // 0. flag the stepper to stop
    stepper.stop();                                        // 1. then stop the stepper
    return true;                                           // 2. let the system know that the stepper is there
  }
  else if (!digitalRead(ind_sensor_b))                     // if we don't have input from sensor a
  {
    return false;                                          // 1. let the system know that the stepper is NOT there
  }
}

// THIS FUNCTION HAS TO BE TESTED - THE CALIBRATION FUNCTION
bool stepperCalibration(){                                 // setup a true/false function to check if the stepper is calibrated or not
  Serial.println("calibrating steppper");
  stepper.setMaxSpeed(1000);                               // set the speed quite slow to not make any accidents
  stepper.setAcceleration(1000);                           // same for acceleration
  stepper_enable = true;                                   // set the flag so the stepper.run() code is executed continously in the main loop

  if (sensorA || sensorB)                                  // if either sensor is active...
  {
    relative_position = stepper.currentPosition();         // set the relative position with the current value so we know the home position
    stepper_calibrated = true;                             // set the stepper calibrated flag so the sytem knows it's calibrated
    oscUdp.sendMessage( "/calibration/i",  "i",  stepper_calibrated);  // 2. send a validation back via OSC
    return true;                                           // return true when this function is checked 
  }
  else if (!sensorA || !sensorB)                           // if neither of the sensors are active
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

  stepper.setMaxSpeed(0);                                   // make sure that the stepper doesn't go anywhere when only enabling
  stepper.setAcceleration(0);                               // make sure that the stepper doesn't go anywhere when only enabling
  // stepper.disableOutputs();                              // does not seem to do anything

  if (!stepper_calibrated)                                  // if the stepper is NOT calibrated...  
  {
    stepperCalibration();                                   // run the calibration function
  }


}


void loop()
{

  sensorA();                                                 // continously check for sensor input
  sensorB();                                                 // continously check for sensor input


  oscUdp.receiveMessages( receivedOscMessage );              // handle the incoming OSC messages

  if (stepper_enable && stepper_calibrated)                  // if the stepper enable flag is set AND the stepper is calibrated...
  {
    stepper.run();                                           // run the stepper
  }
  else if (stepper_enable && !stepper_calibrated)            // if the stepper enable flag is set but NOT the stepper calibration
  {
    Serial.println("calibrate the rails before usage");      // inform the user that is needs to calibrate first 
    oscUdp.sendMessage("/calibration/i", "i", stepper_calibrated);  // send a message via OSC with the calibration on 0
  }


  if (DEBUG == 2)
  {
    if (stepper.isRunning())
    {
      Serial.println(stepper.distanceToGo());
    }
  }



}