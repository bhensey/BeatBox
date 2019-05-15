/* Output-side (LED) Arduino code */
#include "SoftwareSerial.h"

#define HAPTIC 10
#define LED    13
// RX: Arduino pin 2, XBee pin DOUT.  TX:  Arduino pin 3, XBee pin DIN
SoftwareSerial XBee(0, 1); //change these for Teensy LC



//IntervalTimer beatTimer;
bool beat_LED_enable = false;
int oldBeat = 85;

void setup()
{
  // Baud rate MUST match XBee settings (as set in XCTU)
  XBee.begin(115200);
  Serial.begin(115200);

//  beatTimer.begin(sendBeat, 0.5*(60*pow(10,6))/oldBeat);
  pinMode(LED, OUTPUT);
  pinMode(HAPTIC, OUTPUT);
}



void loop()
{
  if (XBee.available())  
  { 
    int curr_bpm = XBee.read();
    if(curr_bpm == 1) {
      sendBeat();
    } else if(curr_bpm == 2) {
      turnOff();
    }
    Serial.println(curr_bpm);
    delay(1);
  }

}

void turnOff() {
  if (beat_LED_enable) {
     digitalWrite(LED,LOW);
     analogWrite(HAPTIC, 0);
     beat_LED_enable = false;
  }
}


void sendBeat() {
     digitalWrite(LED,HIGH);
     analogWrite(HAPTIC, 255);
     delay(1);
     analogWrite(HAPTIC, 125);
     delay(1);
     analogWrite(HAPTIC,0);
     beat_LED_enable = true;
}
