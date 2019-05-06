// Record sound as raw data to a SD card, and play it back.
//
// Requires the audio shield:
//   http://www.pjrc.com/store/teensy3_audio.html
//
// Three pushbuttons nee\\\\\\\d to be connected:
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

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Use these with the Teensy 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11 // not actually used
#define SDCARD_SCK_PIN   13 // not actually used

// CUSTOM FUNCTIONS (using global variables)

int session = 23;

struct SessionMeta {
  String name;
  int size;
};

struct SessionView {
  String name;
  //String lastModified;
};

struct Track {
  String name;
};


void getTrackFilepath(int trackNumber) {
  // Use global variable selected_session
  // session/track.wav
  String fileString = "Sessions/";
  fileString += session;
  fileString += "/";
  fileString += trackNumber;
  fileString += ".wav";
  char filename[50];
  fileString.toCharArray(filename, 50);
  Serial.println(filename);
}

bool fileExists(char dir[]) {
  if (SD.exists(dir)) {
    return 1;
  }
  else {
    return 0;
  } 
}

struct SessionMeta* getSessionOverview() {
  // Scan SD card and return a pointer to an array of every existing session in order
  struct SessionMeta sessionArray[99];
  int numSessions = 0;
  File dir = SD.open("Sessions");
  
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files  
      break;
      }
    //struct Session tempSession = {entry.name(),3}; 
    sessionArray[numSessions] = {entry.name(),entry.size()}; // Initialize session  
    numSessions += 1;
    entry.close();
  }
  for (int i = 0; i < numSessions; i++) {
    Serial.print(sessionArray[i].name);
    Serial.print(",");
    Serial.print(sessionArray[i].size);
    Serial.println();
  }
    Serial.println();
  
  return sessionArray;
}

void createSession(int sessionNumber) {
  String fileString = "Sessions/";
  fileString += sessionNumber;
  char filename[50];
  fileString.toCharArray(filename, 50);
  SD.mkdir(filename);
  if (fileExists(filename)) {
    Serial.printf("%s is created \n", filename);
  }
  else {
    Serial.printf("Error creating %s\n", filename);
  }
fileString += "/meta";
fileString.toCharArray(filename, 50);
 writeMetadata(filename);
}

void writeMetadata(char* filename) {
  File dataFile = SD.open(filename, FILE_WRITE);
  Serial.printf("Testing filename* is %s \n", filename);
  if (dataFile) {
    dataFile.println("Session 3"); // Name
    dataFile.println("11/2/2019"); // Date Created
    dataFile.println("11/5/2019"); // Last opened
    dataFile.println("85"); // BPM
    dataFile.println("8"); // Length
  }
  else {
    Serial.println("Error opening filename in writeMetadata");
  }
}

void deleteSession(int sessionNumber) {
  String fileString = "Sessions/";
  fileString += sessionNumber;
  String tmpString = fileString;
  char filename[50];
  fileString.toCharArray(filename, 50);
  File dir = SD.open(filename);
  // Delete all 
  while (true) {
    File entry = dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    tmpString += "/";
    tmpString += entry.name();
    tmpString.toCharArray(filename, 50);
    SD.remove(filename);
    Serial.println("Removed file");
  }
  fileString.toCharArray(filename, 50);
  SD.rmdir(filename);
  if (! fileExists(filename)){
    Serial.printf("Directory %s is removed\n", filename);
  }
  else {
    Serial.printf("ERROR: The session %s was not removed\n", filename);
  }
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files  
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}


File root;
void setup() {
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
  

   // Test Code
    createSession(3);
    createSession(5);
    createSession(23);
    deleteSession(23);

    root = SD.open("Sessions");
    root.rewindDirectory();
    root.close();
    getSessionOverview();
  
}
