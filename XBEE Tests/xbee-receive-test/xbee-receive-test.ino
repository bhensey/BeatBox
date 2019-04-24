/* Output-side (LED) Arduino code */
#include "SoftwareSerial.h"
// RX: Arduino pin 2, XBee pin DOUT.  TX:  Arduino pin 3, XBee pin DIN
//SoftwareSeria
//XBee(0, 1); //change these for Teensy LC
int LED = 13;

#define HWSERIAL Serial1
#define HAPTIC 9
#define LED 13

#define GIVEBEAT 1

void setup() {
  Serial.begin(9600);
  HWSERIAL.begin(115200);
  pinMode(HAPTIC,OUTPUT);
  pinMode(LED,OUTPUT);
}

void loop() {
  int incomingByte;
//  digitalWrite(LED,HIGH);
//  delay(100);
//  digitalWrite(LED,LOW);
//  delay(100);
//  if (Serial.available() > 0) {
   
//    else Serial.println(incomingByte);
//  }
  if (HWSERIAL.available() > 0) {
    incomingByte = HWSERIAL.read();
    if(incomingByte == GIVEBEAT) {
      Serial.print("Calling Beat");
//      beat();
      beat2();
    }
    Serial.print("UART received: ");
    Serial.println(incomingByte, DEC);
                HWSERIAL.print("UART received:");
                HWSERIAL.println(incomingByte, DEC);
  }
}

void beat() {
  digitalWrite(LED,HIGH);
  analogWrite(HAPTIC,255);
  delay(100);
  analogWrite(HAPTIC,100);
  digitalWrite(LED,LOW);
  delay(100);
   analogWrite(HAPTIC,0);
  digitalWrite(LED,LOW);
  delay(500);
}

void beat2() {
  digitalWrite(LED,HIGH);
  analogWrite(HAPTIC,255);
  delay(33);
  analogWrite(HAPTIC,170);
  delay(50);
  analogWrite(HAPTIC,50);
  delay(17);
  analogWrite(HAPTIC,0);
  delay(600);
  digitalWrite(LED,LOW);
}

