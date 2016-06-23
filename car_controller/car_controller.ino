#include <Servo.h>

Servo steering;  //create a servo object for the steering
Servo motor;  //create a servo object for the motor controller

String command; //Keep track of our command as it comes in
String value;
boolean LEDstatus = false; //Keep track of the LED status
float voltage;

const boolean debug = true; //Turn this OFF if you don't want return messages.  
                            //(the only thing it writes to the console is just the voltage at 2Hz)

const int dirPin = 6;
const int motorPin = 5;
const int steerPin = 7;
const int ledPin = 13;
const int voltSensePin = A3;
const int dirInitial = true;


void setup() {
  Serial.begin(9600); //Start the serial port at 9600 baud
  if (debug == true) Serial.println("Ready to receive commands from Terminator Tim.\n");
  pinMode(ledPin, OUTPUT); //set the LED as an output
  pinMode(dirPin, OUTPUT); //set the dir pin as an output
  pinMode(voltSensePin, INPUT); //voltage sensing pin
  digitalWrite(dirPin, dirInitial); //set the initial direction or it's floating (bad)
  steering.attach(steerPin);
  motor.attach(motorPin);
}

void loop() {
  if (millis() % 500 == 0) { //run every half a second
    
    voltage = map(analogRead(voltSensePin), 0, 1023, 0, 480);
    voltage = voltage/100;
    if (debug == true) Serial.print("Voltage: ");
    Serial.println(voltage, 3);
    
    LEDstatus = !LEDstatus; //Flip the status for next time this runs
    digitalWrite(ledPin, LEDstatus); //turn the LED on or off

  }

  while (Serial.available() > 0) { //If there is something in the serial buffer waiting to be read

    command = Serial.readStringUntil(',');
    Serial.read(); //skip the comma
    value = Serial.readStringUntil('\n');
    Serial.read(); //skip the new line
    
    if (debug == true) Serial.println("Set the " + command + " to " + value);
    
    
    if (command == "steer") {
      steering.write(value.toInt());
    }
    
    else if (command == "motor") {
      motor.write(value.toInt());
    }
    
    else if (command == "motor-dir") {
      digitalWrite(dirPin, value.toInt());
    }
    
    else {
      if (debug == true) Serial.println("Someone done goofed... (you sent something you weren't meant to!)");
    }
  }
}

