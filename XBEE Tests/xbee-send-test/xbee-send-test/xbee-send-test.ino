/* Input-side (button) Arduino code */
#include "SoftwareSerial.h"
#include <SD.h>
#include <SPI.h>

#define LED 13

uint8_t count = 255;
// RX: Arduino pin 2, XBee pin DOUT.  TX:  Arduino pin 3, XBee pin DIN
SoftwareSerial XBee(33, 34);

void setup() {
  // put your setup code here, to run once:
  pinMode(LED,OUTPUT);
  XBee.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  XBee.write(count);
  count--;
}
