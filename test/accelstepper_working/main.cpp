#include <Arduino.h>
// Include the AccelStepper library:
#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define dirPin 4
#define stepPin 5
#define motorInterfaceType 1

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

void setup() {
  Serial.begin(9600);
  Serial.println("Start Test");
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed(15000);
  stepper.setAcceleration(1500);
}

void loop() {
  // Set the target position:
  stepper.moveTo(200000);
  // Run to target position with set speed and acceleration/deceleration:
  Serial.println("Run to position");
  Serial.println(stepper.distanceToGo());
  stepper.runToPosition();

  delay(1000);

  // Move back to zero:
  stepper.moveTo(0);
  Serial.println("Run to home");
  Serial.println(stepper.distanceToGo());  
  stepper.runToPosition();

  delay(1000);
}
