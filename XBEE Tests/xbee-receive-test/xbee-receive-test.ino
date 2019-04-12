/* Output-side (LED) Arduino code */
#include "SoftwareSerial.h"
// RX: Arduino pin 2, XBee pin DOUT.  TX:  Arduino pin 3, XBee pin DIN
SoftwareSerial XBee(2, 3); //change these for Teensy LC
int LED = 9;

void setup()
{
  // Baud rate MUST match XBee settings (as set in XCTU)
  XBee.begin(115200);
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
}

void loop()
{
  if (XBee.available())  
  { 
    int c = XBee.read();
//    Serial.print("Voltage:");
    Serial.print(c);
    Serial.print("\n");
    analogWrite(LED,c);
    delay(1);
  }
//  else Serial.print("Not available\n");

}
