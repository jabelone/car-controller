#include <Servo.h>
Servo steering;
Servo throttle;

//Settings
const boolean debug = true; //Turn this OFF if you don't want debug messages.
const boolean inputsOut = true; //Turn this OFF if you don't want the inputs to be sent via uart.
const boolean voltageOut = false; //Turn this ON if you want votlage messages.
const int dirInitial = true; // Set the initial direction; forwards/backwards depends on setup
const int modeThreshold = 1200;  //Less than this value will result in manual mode
const int killThreshold = 1600;  //Between modeThreshold and this will kill outputs
                                 //Above killThreshold will result in autonomous mode
const int noSignalThreshold = 500; //

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
int rxMode = 0; //Start in manual mode

//This function is called everytime the code wants the car to zero throttle and steering
//Change the values to suit your needs
void straightAndStop() {
  throttle.write(90);
  steering.write(90);
}

void setup() {
  Serial.begin(57600);  //Start serial @ 57600 baud

  //Set the correct pin modes
  pinMode(ledPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(voltSensePin, INPUT);
  pinMode(inMode, INPUT);
  pinMode(inMotor, INPUT);
  pinMode(inSteer, INPUT);

  //Setup our servos
  steering.attach(steerPin);
  throttle.attach(motorPin);

  //Begin with zero motor/steering
  straightAndStop();
  if (debug) Serial.println("Ready to receive commands...\n");
  delay(1000);
}

void loop() {
  rxMode = pulseIn(inMode, HIGH, 20000); // Read the mode output on R/C receiver
  delay(10); //Without this (intentional blocking) delay the arduino won't read the next pulseIn properly

  if (voltageOut) { //If we're reporting a voltage
    voltage = map(analogRead(voltSensePin), 0, 1023, 0, 480); //Read and map it
    voltage = voltage / 100; //Scale it
    Serial.print("voltage,"); //Print it
    Serial.println(voltage, 3);
  }

  LEDstatus = !LEDstatus; //Flip the status for next time this runs and write it
  digitalWrite(ledPin, LEDstatus);


  if (rxMode > killThreshold) {  //If we hit the I done goofed switch zero motor/steering
    straightAndStop();
    Serial.println("error");  //This means if recording data with suiron it will pause
                              //Makes sure you don't get a heap of "bad" data

  }

  else if ((rxMode < modeThreshold) && (rxMode > 900)) {  //Manual mode
    if (mode == true) { // If we just came from auto control,  zero motor/steering
      straightAndStop();
      Serial.println("Entered manual control mode.");
      mode = false; // Now we're in manual mode
    }
    int rxSteerRaw = pulseIn(inSteer, HIGH, 20000);  //Read our steering/motor values
    delay(10); //Without this (intentional blocking) delay the arduino won't read the second pulseIn properly
    int rxMotorRaw = pulseIn(inMotor, HIGH, 20000);
    rxSteer = map(rxSteerRaw, 1000, 2000, 40, 150); // Read the steering output on R/C receiver
    rxMotor = map(rxMotorRaw, 1000, 2000, 40, 150); // Read the motor output on R/C receiver

    if (inputsOut) { //If we're outputting control values
      while (Serial.available() > 0) {  //Everytime we get a character via serial read it then send values
        Serial.read();
        Serial.print(rxSteer);
        Serial.print(",");
        Serial.println(rxMotor);
      }

      if (debug) {  //If we're debugging print the raw inputs (before scaling/mapping)
        Serial.print("rawRxMotor,");
        Serial.println(rxMotorRaw);
        Serial.print("rawRxSteer,");
        Serial.println(rxSteerRaw);
        Serial.println();
      }
    }

    throttle.write(rxMotor);  //Set the motor and steering values
    steering.write(rxSteer);
  }

  else { //Autonomous control mode
    if (mode == false) { // If we just came from manual control
      straightAndStop(); // Set the motor power to zero and straighten wheels
      Serial.println("Entered autonomous control mode.");
    }

    mode = true; // Now we're in auto mode

    while (Serial.available() > 0) { //If there is something in the serial buffer waiting to be read

      command = Serial.readStringUntil(','); //Read all of the command
      value = Serial.readStringUntil('\n'); //Read all of the value to set
      Serial.read(); //skip the new line

      if (debug == true) Serial.println("Set the " + command + " to " + value);  //If debugging, print out commands

      if (command == "steer") {  //If we get a steering command then set the steering to it
        steering.write(value.toInt());
      }

      else if (command == "motor") {  //If we get a motor command then set the motor to it
        throttle.write(value.toInt());
      }

      else {  //If it was any other command we weren't expecting it
        if (debug == true) Serial.println("Someone done goofed... (you sent something you weren't meant to!)");
      }
    }
  }
}
