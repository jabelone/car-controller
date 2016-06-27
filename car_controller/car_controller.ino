#include <Servo.h>
Servo steering;

//Settings
const boolean debug = true; //Turn this OFF if you don't want return messages.
const int dirInitial = true; // Set the initial direction; forwards/backwards depends on setup
const int modeThreshold = 1200;
const int killThreshold = 1600;
const int noSignalThreshold = 500;
const int reverseThreshold = 1340;

//Define pin numbers
const int inSteer = 11; // Pin connected to steering output on R/C receiver
const int inMotor = 9; // Pin connected to motor output on R/C receiver
const int inMode = 10; // Pin connected to mode output on R/C receiver
const int dirPin = 6;  // Pin connected to direction on motor controller
const int motorPin = 5; // Pin that outputs the PWM signal to the motor controller
const int steerPin = 7; // Pin that outputs the PWM signal to the steering servo
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

void setup() {
  Serial.begin(57600); //Start the serial port at 9600 baud
  if (debug == true) Serial.println("Ready to receive commands...\n");

  //Set the correct pin modes
  pinMode(ledPin, OUTPUT); //set the LED as an output
  pinMode(dirPin, OUTPUT); //set the dir pin as an output
  pinMode(motorPin, OUTPUT);
  pinMode(voltSensePin, INPUT); //voltage sensing pin
  pinMode(inMode, INPUT); //voltage sensing pin
  pinMode(inMotor, INPUT); //voltage sensing pin
  pinMode(inSteer, INPUT); //voltage sensing pin

  steering.attach(steerPin);

  //Set the default outputs
  digitalWrite(dirPin, dirInitial); //set the initial direction

  straightAndStop();
}

void loop() {
  rxMode = pulseIn(inMode, HIGH, 20000); // Read the mode output on R/C receiver

  if (millis() % 500 == 0) { //don't spam the console

    voltage = map(analogRead(voltSensePin), 0, 1023, 0, 480);
    voltage = voltage/100;
    if (debug == true) Serial.print("Voltage: ");
    Serial.println(voltage, 3);

    LEDstatus = !LEDstatus; //Flip the status for next time this runs
    digitalWrite(ledPin, LEDstatus); //turn the LED on or off
  }

  if (rxMode < noSignalThreshold) {
    straightAndStop();
  }

  else if (rxMode > killThreshold) { //Kill outputs
    straightAndStop();
    Serial.println("Killed all outputs.");
  }

  else if (rxMode < modeThreshold) {  //Manual mode

    if (mode == true) { // If we just came from auto control
      straightAndStop(); // Set the motor power to zero and straighten wheels
      Serial.println("Entered manual control mode.");
      mode = false; // Now we're in manual mode
    }

    rxSteer = map(pulseIn(inSteer, HIGH, 20000), 900, 1900, 50, 150); // Read the steering output on R/C receiver
    rxMotor = pulseIn(inMotor, HIGH, 20000);
    
    if (rxMotor < reverseThreshold) {
      digitalWrite(dirPin, false);
      rxMotor = map(rxMotor, reverseThreshold, 900, 0, 200); // Read the motor output on R/C receiver
    }

    else {
      digitalWrite(dirPin, true);
      rxMotor = map(rxMotor, reverseThreshold, 1900, 0, 200); // Read the motor output on R/C receiver
    }



    if (millis() % 20 == 0) {
      if (debug == true) {
        Serial.print("rxSteer: "); // Read the mode output on R/C receiver);
        Serial.print(rxSteer);
        Serial.print("  rxMotor: ");
        Serial.println(rxMotor);
        Serial.print("  rxMode: ");
        Serial.println(rxMode);
      }
    }

    analogWrite(motorPin, rxMotor);
    steering.write(rxSteer);
  }

  else { //Autonomous control mode
    if (mode == false) { // If we just came from manual control
      straightAndStop(); // Set the motor power to zero and straighten wheels
      Serial.println("Entered autonomous control mode.");
    }

    mode = true; // Now we're in auto mode

      while (Serial.available() > 0) { //If there is something in the serial buffer waiting to be read
      //Serial.println(Serial.readStringUntil('\n'));
      command = Serial.readStringUntil(',');
      value = Serial.readStringUntil('\n');
      Serial.read(); //skip the new line

      if (debug == true) Serial.println("Set the " + command + " to " + value.toInt());

      if (command == "steer") {
        steering.write(value.toInt());
      }

      else if (command == "motor") {
        analogWrite(motorPin, value.toInt());
      }

      else if (command == "motor-dir") {
        digitalWrite(dirPin, value.toInt());
      }

      else {
        if (debug == true) Serial.println("Someone done goofed... (you sent something you weren't meant to!)");
      }
    }
  }
}

void straightAndStop() {
  analogWrite(motorPin, 0);
  steering.write(100);
}



