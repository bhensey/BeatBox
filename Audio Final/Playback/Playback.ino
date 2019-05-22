#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdRaw           playSdRaw3;     //xy=201.00390625,220.00390625
AudioPlaySdRaw           playSdRaw2;     //xy=210.00390625,150.00390625
AudioPlaySdRaw           playSdRaw4;     //xy=218.00390625,289.00390625
AudioPlayMemory          playMem1;       //xy=231.25390625,356.50390434265137
AudioPlaySdRaw           playSdRaw1;     //xy=277.00390625,90.00390625
AudioMixer4              mixer1;         //xy=437.50390625,249.50391578674316
AudioMixer4              mixer2;         //xy=439.25392150878906,332.50392150878906
AudioMixer4              mixer3;         //xy=583.2538986206055,282.50390625
AudioOutputI2S           i2s1;           //xy=719.2539520263672,287.5039463043213
AudioConnection          patchCord1(playSdRaw3, 0, mixer1, 2);
AudioConnection          patchCord2(playSdRaw2, 0, mixer1, 1);
AudioConnection          patchCord3(playSdRaw4, 0, mixer1, 3);
AudioConnection          patchCord4(playMem1, 0, mixer2, 0);
AudioConnection          patchCord5(playSdRaw1, 0, mixer1, 0);
AudioConnection          patchCord6(mixer1, 0, mixer3, 0);
AudioConnection          patchCord7(mixer2, 0, mixer3, 1);
AudioConnection          patchCord8(mixer3, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=252,427
// GUItool: end automatically generated code

start timer
int playDuration = 1000/(bpm/60)*length
void loop() {
  if (timer > playDuration) {
    start(Playback)
  }
  
}

char track1[] = "SDTEST1.RAW";
char track2[] = "SDTEST2.RAW";
char track3[] = "SDTEST3.RAW";
char track4[] = "SDTEST4.RAW";

void startPlayback() {
  Serial.println("Start playing 1");
  playSdWav1.play(track1);
  Serial.println("Start playing 2");
  playSdWav2.play(track2);
  Serial.println("Start playing 3");
  playSdWav3.play(track3);
  Serial.println("Start playing 4");
  playSdWav4.play(track4);
}
