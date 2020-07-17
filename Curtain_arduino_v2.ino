// a/hr4988 has been limited to 0.3A using onboard pot.
// this (current) is smaller than l293N but still a lot of waste.
// ****do something with enable pin on the driver module.
// enable pin on the a4988 is also not used now
// need to see if that can be used to keep driver off when not in use
// code currently on arduino 2020-07-01
// interfaces between esp8266 mqtt client and a4988 stepper driver
// pin 8 reads signal input from esp8266
// pin 2 tells stepper the rotation direction
// pin 3 is the pulse generator to move the stepper.
// 5v supply/GND ref to driver is from arduino
// arduino is getting 12v supply from random adapter
// 12v is also given to ld33v > esp8266
// curtain rolls up/down in 7 rotations (manually counted)
// 7 rotation count is hard coded for now
// single digital pin can't communicate up/down and rotations
// ****no soft serial to/from esp8266 is being used (but really should be)
//  to communicate more complex actions through mqtt

// many of the ****s have been addressed

/* Simple Stepper Motor Control Exaple Code

   by Dejan Nedelkovski, www.HowToMechatronics.com

*/

#include <SoftwareSerial.h>

SoftwareSerial SSerial(6, 7);


// defines pins numbers
const int stepPin = 3;
const int dirPin = 2;
const int sleepPin = 9;
const int resetPin = 10;
const int STEPS_PER_REV = 200;

int curtain_roll_count = 0;
String curtain_roll_count_str;
String sserial_input;


// Functions declarations

void curtain_up();
void curtain_down();

void setup() {
  // debugging

  Serial.begin(9600);

  // debugging

  // software serial for communication with esp8266

  SSerial.begin(9600);

  // Sets the two pins as Outputs
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(sleepPin, OUTPUT);
  delay(2);
  digitalWrite(resetPin, HIGH);
  digitalWrite(sleepPin, HIGH);
  delay(2);
}
void loop() {
  if (SSerial.available()) {
    sserial_input = SSerial.readString();
    Serial.println(sserial_input);

    if (sserial_input.indexOf("curtain_up") > -1) {
      if (sserial_input.indexOf("default") > -1) {
        curtain_up(28);
      }
      else if (sserial_input.indexOf("-r") > -1) {
        curtain_roll_count_str = sserial_input.substring(sserial_input.indexOf("-r") + 3);
        if (curtain_roll_count_str.toFloat()) {
          curtain_roll_count = curtain_roll_count_str.toFloat();
          if ((curtain_roll_count <= 28) and (curtain_roll_count > 0)) {
            curtain_up(curtain_roll_count);
          }
        }
      }
      else {
        ;
      }
    }
    else if (sserial_input.indexOf("curtain_down") > -1) {
      if (sserial_input.indexOf("default") > -1) {
        curtain_down(28);
      }
      else if (sserial_input.indexOf("-r") > -1) {
        curtain_roll_count_str = sserial_input.substring(sserial_input.indexOf("-r") + 3);
        if (curtain_roll_count_str.toFloat()) {
          curtain_roll_count = curtain_roll_count_str.toFloat();
          if ((curtain_roll_count <= 28) and (curtain_roll_count > 0)) {
            curtain_down(curtain_roll_count);
          }
        }
      }
      else {
        ;
      }
    }
    else {
      ;
    }

  }

}


void curtain_up(float roll_count) {
  Serial.println("curtain_up");



  // wake device and wait 5ms
  digitalWrite(sleepPin, HIGH);
  delayMicroseconds(5);

  // Enables the motor to move in a particular direction
  digitalWrite(dirPin, HIGH);

  // Makes 400 pulses for making two full cycle rotation
  for (int x = 0; x < (STEPS_PER_REV * roll_count); x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(700);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(700);
  }

  // back to sleep
  digitalWrite(sleepPin, LOW);
}


void curtain_down(float roll_count) {
  Serial.println("curtain_down");

  // wake device and wait 5ms
  digitalWrite(sleepPin, HIGH);
  delayMicroseconds(5);

  // Enables the motor to move in a particular direction
  digitalWrite(dirPin, LOW);

  // Makes 400 pulses for making two full cycle rotation
  for (int x = 0; x < (STEPS_PER_REV * roll_count); x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(700);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(700);
  }
  
  // back to sleep
  digitalWrite(sleepPin, LOW);
}
