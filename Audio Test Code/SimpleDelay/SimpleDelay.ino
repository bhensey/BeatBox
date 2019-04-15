// Advanced Microcontroller-based Audio Workshop
//
// http://www.pjrc.com/store/audio_tutorial_kit.html
// https://hackaday.io/project/8292-microcontroller-audio-workshop-had-supercon-2015
// 
// Part 2-5: Simple Delay


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=116,197
AudioEffectDelay         delay1;         //xy=336,130
AudioOutputI2S           i2s1;           //xy=509,197
AudioConnection          patchCord1(i2s2, 0, delay1, 0);
AudioConnection          patchCord2(delay1, 0, i2s1, 0);
AudioConnection          patchCord3(delay1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=323,337
// GUItool: end automatically generated code


void setup() {
  Serial.begin(9600);
  AudioMemory(160);
  sgtl5000_1.enable();
  sgtl5000_1.volume(.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(36);
  delay1.delay(0, 1000);
  //delay1.delay(1, 400);
  //delay1.delay(2, 400);
  //delay1.delay(3, 400);
  //delay1.delay(4, 400);
  //delay1.delay(5, 400);
  //delay1.delay(6, 400);
  //delay1.delay(7, 400);
  delay(1000);
}

void loop() {
  // do nothing
}
