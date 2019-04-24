/* Input-side (button) Arduino code */
#include "SoftwareSerial.h"
#include <SPI.h>

#define LED 13

IntervalTimer beatTimer;

//int count = 0;
// RX: Arduino pin 2, XBee pin DOUT.  TX:  Arduino pin 3, XBee pin DIN
SoftwareSerial XBee(33, 34);

void setup() {
  // put your setup code here, to run once:
  pinMode(LED,OUTPUT);
  XBee.begin(115200);
  beatTimer.begin(sendBeat(), 1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED,LOW);
  delay(1);
//  delay(1);
//  if(count == 1) 
//  {
//    XBee.write(1);
//    digitalWrite(LED,HIGH);
//    delay(1);
//    count = 0;
//  }
//  else XBee.write(0);
//  XBee.write(count);
//  count++;
}

void sendBeat() {
  digitalWrite(LED,HIGH);
  XBee.write(1);
}



