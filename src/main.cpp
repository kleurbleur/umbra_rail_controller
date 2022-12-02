#include <Arduino.h>

u_int8_t ind_sensor_a = 32;     // Input pin from sensor a - WARNING! -> only to be connected via an optocoupler!
u_int8_t ind_sensor_b = 33;     // Input pin from sensor b - WARNING! -> only to be connected via an optocoupler!


// DEBUG SETTINGS
u_int8_t DEBUG = 1;             // 1 = OSC messages 2 = distance to go


// ================================================================================================
// AFTER THIS DON'T EDIT UNLESS YOU KNOW WHAT IS WHAT
// ================================================================================================



u_int32_t sensor_a_position = 0;
u_int32_t sensor_b_position = 0;
bool sen_a_high = false;
bool sen_a_low = false;
bool sen_b_high = false;
bool sen_b_low = false;

// SENSOR FUNCTIONS ----  TEST
bool sensorA()                                             // setup a true/false function to check if the sensors have input or not
{
  if (!digitalRead(ind_sensor_a) && !sen_a_high)            // if we have input from sensor a...
  {
    sen_a_high = true;
    sen_a_low = false;
    Serial.println("SensorA active");                      // 2. inform via Serial
    return true;                                           // 4. let the system know that the stepper is there
  }
  else if (digitalRead(ind_sensor_a) && !sen_a_low)      // if we don't have input from sensor a
  {
    sen_a_low = true;
    sen_a_high = false;
    Serial.println("SensorA not active");                  // 1. inform via Serial
    return false;                                          // 3. let the system know that the stepper is NOT there
  }
}
bool sensorB()                                             // setup a true/false function to check if the sensors have input or not
{
  if (!digitalRead(ind_sensor_b) && !sen_b_high)           // if we have input from sensor a...
  {
    sen_b_high = true;
    sen_b_low = false;
    Serial.println("SensorB active");                      // 2. inform via Serial
    return true;                                           // 4. let the system know that the stepper is there
  }
  else if (digitalRead(ind_sensor_b) && !sen_b_low)        // if we don't have input from sensor a
  {
    sen_b_low = true;
    sen_b_high = false;
    Serial.println("SensorB not active");                  // 1. inform via Serial
    return false;                                          // 3. let the system know that the stepper is NOT there
  }
}



void setup()
{
  Serial.begin(115200);

  // make sure that the inputs are set
  pinMode(32, INPUT_PULLUP);
  pinMode(33, INPUT_PULLUP);

}


void loop()
{

  sensorA();                                                 // continously check for sensor input
  sensorB();                                                 // continously check for sensor input

}