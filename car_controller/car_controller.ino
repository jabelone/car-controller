#include <Servo.h>
Servo steering;
Servo throttle;

//Settings
const boolean debug = true; //Turn this OFF if you don't want debug messages.
const boolean inputsOut = true; //Turn this OFF if you don't want the inputs to be sent via uart.
const boolean voltageOut = false; //Turn this ON if you want votlage messages.
const int dirInitial = true; // Set the initial direction; forwards/backwards depends on setup
const int modeThreshold = 1200;
const int killThreshold = 1600;
const int noSignalThreshold = 500;

//Define pin numbers
const int inSteer = 2; // Pin connected to steering output on R/C receiver
const int inMotor = 3; // Pin connected to motor output on R/C receiver
const int inMode = 4; // Pin connected to mode output on R/C receiver
const int motorPin = 6; // Pin that outputs the PWM signal to the motor controller
const int steerPin = 5; // Pin that outputs the PWM signal to the steering servo
const int ledPin = 13; // Status LED
const int voltSensePin = A3; // The voltage sensing pin

String command; //Keep track of our command as it comes in
String value;
boolean LEDstatus = false; //Keep track of the LED status
boolean mode;
float voltage;
int rxSteer;
int rxMotor;
int rxMode = 0;

void straightAndStop() {
  throttle.write(90);
  steering.write(90);
}

void setup() {
  Serial.begin(57600); //Start the serial port at 57600 baud
  if (debug) Serial.println("Ready to receive commands...\n");

  //Set the correct pin modes
  pinMode(ledPin, OUTPUT); //set the LED as an output
  pinMode(motorPin, OUTPUT);
  pinMode(voltSensePin, INPUT); //voltage sensing pin
  pinMode(inMode, INPUT);
  pinMode(inMotor, INPUT);
  pinMode(inSteer, INPUT);

  steering.attach(steerPin);
  throttle.attach(motorPin);

  straightAndStop();
  delay(1000);
}

void loop() {
  rxMode = pulseIn(inMode, HIGH, 20000); // Read the mode output on R/C receiver
  delay(10);

  if (voltageOut) {
    voltage = map(analogRead(voltSensePin), 0, 1023, 0, 480);
    voltage = voltage / 100;
    Serial.print("voltage,");
    Serial.println(voltage, 3);
  }

  LEDstatus = !LEDstatus; //Flip the status for next time this runs
  digitalWrite(ledPin, LEDstatus); //turn the LED on or off


  if (rxMode > killThreshold) {
    straightAndStop();
    Serial.println("error");

  }

  else if ((rxMode < modeThreshold) && (rxMode > 900)) {  //Manual mode
    if (mode == true) { // If we just came from auto control
      straightAndStop(); // Set the motor power to zero and straighten wheels
      Serial.println("Entered manual control mode.");
      mode = false; // Now we're in manual mode
    }
    int rxSteerRaw = pulseIn(inSteer, HIGH, 20000);
    delay(10);
    int rxMotorRaw = pulseIn(inMotor, HIGH, 20000);
    rxSteer = map(rxSteerRaw, 1000, 2000, 40, 150); // Read the steering output on R/C receiver
    rxMotor = map(rxMotorRaw, 1000, 2000, 40, 150); // Read the motor output on R/C receiver

    if (inputsOut) {
      while (Serial.available() > 0) {
        Serial.read();
        Serial.print(rxSteer);
        Serial.print(",");
        Serial.println(rxMotor);
      }

      if (debug) {
        Serial.print("rawRxMotor,");
        Serial.println(rxMotorRaw);
        Serial.print("rawRxSteer,");
        Serial.println(rxSteerRaw);
        Serial.println();
      }
    }

    throttle.write(rxMotor);
    steering.write(rxSteer);
  }

  else { //Autonomous control mode
    if (mode == false) { // If we just came from manual control
      straightAndStop(); // Set the motor power to zero and straighten wheels
      Serial.println("Entered autonomous control mode.");
    }

    mode = true; // Now we're in auto mode

    while (Serial.available() > 0) { //If there is something in the serial buffer waiting to be read

      command = Serial.readStringUntil(',');
      //Serial.read(); //skip the comma
      value = Serial.readStringUntil('\n');
      Serial.read(); //skip the new line

      if (debug == true) Serial.println("Set the " + command + " to " + value);


      if (command == "steer") {
        steering.write(value.toInt());
      }

      else if (command == "motor") {
        throttle.write(value.toInt());
      }

      else {
        if (debug == true) Serial.println("Someone done goofed... (you sent something you weren't meant to!)");
      }
    }
  }
}
