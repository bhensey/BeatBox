// Advanced Microcontroller-based Audio Workshop
//
// http://www.pjrc.com/store/audio_tutorial_kit.html
// https://hackaday.io/project/8292-microcontroller-audio-workshop-had-supercon-2015
// 
// Part 1-3: First "Hello World" program, play a music file
//
// WAV files for this and other Tutorials are here:
// http://www.pjrc.com/teensy/td_libs_AudioDataFiles.html

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Encoder.h>

AudioSynthSimpleDrum     drum1;
//AudioPlaySdWav           playSdWav1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(drum1, 0, i2s1, 0);
//AudioConnection          patchCord2(playSdWav1, 1, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Use these with the Teensy 3.5 & 3.6 SD card
//#define SDCARD_CS_PIN    BUILTIN_SDCARD
//#define SDCARD_MOSI_PIN  11  // not actually used
//#define SDCARD_SCK_PIN   13  // not actually used

// Use these for the SD+Wiz820 or other adaptors
//#define SDCARD_CS_PIN    4
//#define SDCARD_MOSI_PIN  11
//#define SDCARD_SCK_PIN   13

Encoder myEnc(0,2);

void encoderInterrupt() {
  Serial.println("Encoder Trigger");
}


void setup() {
  attachInterrupt(0, encoderInterrupt, FALLING);
  Serial.begin(9600);
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(1);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  delay(1000);

  drum1.frequency(1000);
  drum1.length(70);
  drum1.secondMix(0.5);
  drum1.pitchMod(0.5);
  
  myEnc.write(400);
}

long oldPosition = -999;

void loop() {
   long newPosition = myEnc.read();
  if (newPosition % 4 == 0 && newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition / 4);
  }

  drum1.noteOn();
  delay(newPosition);

  
  
  //if (playSdWav1.isPlaying() == false) {
    //Serial.println("Start playing");
    //playSdWav1.play("click.wav");
    //playSdWav1.play("SDTEST3.WAV");
    //delay(10); // wait for library to parse WAV info
  //}
  // do nothing while playing...

}
