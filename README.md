# Car Controller
This project provides firmware that runs on an arduino.  This firmware with coupled with an arduino provides a serial (UART) interface for outputting PWM signals.  Something like a raspberry has lots of processing power and I/O but can't reliably run servos and PWM outputs.

By sending the steering and motor outputs over UART to a real time microcontroller reliability can be ensured.  (I have tested it on an Arduino nano)  Another feature is the firmware will report the voltage of a battery which is great for remote monitoring and safety.  (you can monitor a single cell from a LiPo without any extra hardware when using a 5v micro controller!)

# Setup
It's fairly straight forward to use.  The PWM output and voltage monitoring pins are in configurable variables at the start of the program.  Specify which pins you're using, bearing in mind the voltage monitor must be on an analog input.  If the batteries maximum voltage is less than your microcontroller's operating voltage no extra hardware is needed.  

# Usage
You can send it three commands, "motor,x" to control motor output, "steer,x" to control steering output and "motor-dir,x" (x is 0 or 1) for the motor direction pin.  If the debug flag is set to true it outpus nicely formatted messages to the serial console every time you change an ouput and the battery voltage at 2Hz.  

# License
As with all my projects this is released under a GNU GPLv3 or later license in the hope it will be useful.
