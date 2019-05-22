// Record sound as raw data to a SD card, and play it back.
//
// Requires the audio shield:
//   http://www.pjrc.com/store/teensy3_audio.html
//
// Three pushbuttons need to be connected:
//   Record Button: pin 0 to GND
//   Stop Button:   pin 1 to GND
//   Play Button:   pin 2 to GND
//
// This example code is in the public domain.

#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=105,63
AudioAnalyzePeak         peak1;          //xy=278,108
AudioRecordQueue         queue1;         //xy=281,63
AudioPlaySdRaw           playRaw1;       //xy=302,157
AudioOutputI2S           i2s1;           //xy=470,120
AudioConnection          patchCord1(i2s2, 0, queue1, 0);
AudioConnection          patchCord2(i2s2, 0, peak1, 0);
AudioConnection          patchCord3(playRaw1, 0, i2s1, 0);
AudioConnection          patchCord4(playRaw1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=265,212
// GUItool: end automatically generated code

// For a stereo recording version, see this forum thread:
// https://forum.pjrc.com/threads/46150?p=158388&viewfull=1#post158388

// A much more advanced sound recording and data logging project:
// https://github.com/WMXZ-EU/microSoundRecorder
// https://github.com/WMXZ-EU/microSoundRecorder/wiki/Hardware-setup
// https://forum.pjrc.com/threads/52175?p=185386&viewfull=1#post185386


// Bounce objects to easily and reliably read the buttons
Bounce buttonStop = Bounce(0, 8);


// which input on the audio shield will be used?
const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Remember which mode we're doing
int mode = 0;  // 0=stopped, 1=recording, 2=playing

// The file where data is recorded

void setup() {
  // Configure the pushbutton pins
  pinMode(0, INPUT_PULLUP);

  // Audio connections require memory, and the record queue
  // uses this memory to buffer incoming audio.
  AudioMemory(60);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.8);

  // Initialize the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  } 
  frec = SD.open("Dummy.raw", FILE_WRITE);
}


void loop() {
  // First, read the buttons
  buttonStop.update();
  // Respond to button presses
  if (buttonStop.fallingEdge()) {
    Serial.println("Stop Button Press");
    stopRecording(frec);
  }
  continueRecording(frec);
}



void startRecording(File frec) {
  Serial.println("startRecording");
  if (SD.exists(frec)) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove(frec);
  }
  
  if (frec) {
    queue1.begin();
    mode = 1;
  }
}

void continueRecording(File frec) {
  if (queue1.available() >= 2) {
    byte buffer[512];
    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer+256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    // write all 512 bytes to the SD card
    elapsedMicros usec = 0;
    frec.write(buffer, 512);
    // Uncomment these lines to see how long SD writes
    // are taking.  A pair of audio blocks arrives every
    // 5802 microseconds, so hopefully most of the writes
    // take well under 5802 us.  Some will take more, as
    // the SD library also must write to the FAT tables
    // and the SD card controller manages media erase and
    // wear leveling.  The queue1 object can buffer
    // approximately 301700 us of audio, to allow time
    // for occasional high SD card latency, as long as
    // the average write time is under 5802 us.
    //Serial.print("SD write, us=");
    //Serial.println(usec);
  }
}

void stopRecording(File frec) {
  Serial.println("stopRecording");
  queue1.end();
  if (mode == 1) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }
  mode = 0;
}

void deleteRecording(File frec) {
  Serial.println("deleteRecording");
  stopRecording()
}
