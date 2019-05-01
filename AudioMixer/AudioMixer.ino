
#include "AudioSampleKick.h"         // http://www.freesound.org/people/DWSD/sounds/171104/
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlayMemory          playMem1;       //xy=231.25390625,356.50390434265137
AudioPlaySdWav           playSdWav3;     //xy=233.25390625,249.50390148162842
AudioPlaySdWav           playSdWav2;     //xy=234.25391006469727,199.50391674041748
AudioPlaySdWav           playSdWav4;     //xy=234.2539176940918,301.50392723083496
AudioPlaySdWav           playSdWav1;     //xy=235.25390625,149.50390625
AudioMixer4              mixer1;         //xy=437.50390625,249.50391578674316
AudioMixer4              mixer2;         //xy=439.25392150878906,332.50392150878906
AudioMixer4              mixer3;         //xy=583.2538986206055,282.50390625
AudioOutputI2S           i2s1;           //xy=719.2539520263672,287.5039463043213
AudioConnection          patchCord1(playMem1, 0, mixer2, 0);
AudioConnection          patchCord2(playSdWav3, 0, mixer1, 2);
AudioConnection          patchCord3(playSdWav2, 0, mixer1, 1);
AudioConnection          patchCord4(playSdWav4, 0, mixer1, 3);
AudioConnection          patchCord5(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord6(mixer1, 0, mixer3, 0);
AudioConnection          patchCord7(mixer2, 0, mixer3, 1);
AudioConnection          patchCord8(mixer3, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=252,427
// GUItool: end automatically generated code



// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

IntervalTimer bpmTimer;

void playBeat() {
  Serial.println("Play Beat");
  playMem1.play(AudioSampleKick);
  //bpmTimer.update();
}

void setup() {
  Serial.begin(9600);
  bpmTimer.begin(playBeat, 1000000); // Bpm Timer
  AudioMemory(60); // Memory for all audio funcitons especially recording buffer
  sgtl5000_1.enable();
  sgtl5000_1.volume(1);

  // Initialize the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  // Track Mixer
  mixer1.gain(0, 0.5); // Track 1
  mixer1.gain(1, 0.5); // Track 2
  mixer1.gain(2, 0.5); // Track 3
  mixer1.gain(3, 0.5); // Track 4
  
  // Metronome Mixer
  mixer2.gain(0,1);

  //Track and Metronome Mixer
  mixer3.gain(0, 0); // Track Mixer
  mixer3.gain(1, 0); // Metronome Mixer
  
  delay(1000);
}

char track1[] = "SDTEST1.WAV";
char track2[] = "SDTEST2.WAV";
char track3[] = "SDTEST3.WAV";
char track4[] = "SDTEST4.WAV";


void loop() {
  if (playSdWav1.isPlaying() == false) {
    Serial.println("Start playing 1");
    playSdWav1.play(track1);
    delay(10); // wait for library to parse WAV info
  }
  if (playSdWav2.isPlaying() == false) {
    Serial.println("Start playing 2");
    playSdWav2.play(track2);
    delay(10); // wait for library to parse WAV info
  }
  if (playSdWav3.isPlaying() == false) {
    Serial.println("Start playing 3");
    playSdWav3.play(track3);
    delay(10); // wait for library to parse WAV info
  }
  if (playSdWav4.isPlaying() == false) {
    Serial.println("Start playing 4");
    playSdWav4.play(track4);
    delay(10); // wait for library to parse WAV info
  }
}
