#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Use these with the Teensy Audio Shield
//#define SDCARD_CS_PIN    10
//#define SDCARD_MOSI_PIN  7
//#define SDCARD_SCK_PIN   14

// Use these with the Teensy 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11 // not actually used
#define SDCARD_SCK_PIN   13 // not actually used

//
// DATA AND STRUCTS
//

int session = 23;


// GIVES METADATA OF ALL SESSIONS
struct SessionMeta {
  String name;
  int size;
  
};

struct Session {
  String name;
  //String lastModified;
};

struct Track {
  String name;
};


//
// HELPER FUNCTIONS
//
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

//
// RETURN POINTER TO AN ARRAY OF EVERY EXISTING SESSION
//
int getSessionOverview(SessionMeta *sessionArray) {
  // Scan SD card and return a pointer to an array of every existing session in order
  Serial.println("Running getSessionOverview...");
  int numSessions = 0;
  char filename[50];
  String fileString;
  File dir = SD.open("Sessions");
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files  
      break;
      }

    fileString = "Sessions/";
    fileString += entry.name();
    fileString += "/meta";
    fileString.toCharArray(filename,50);
    sessionArray[numSessions] = getSessionMeta(SD.open(filename));
    //sessionArray[numSessions] = {entry.name(),entry.size()}; // Initialize session  
    numSessions += 1;
    entry.close();
  }
  
  return numSessions;
}

//
// GET METADATA
//
SessionMeta getSessionMeta(File metaFile) {
  Serial.println(metaFile.read());
  SessionMeta newSession = {
    metaFile.read(),
    metaFile.read() 
  };
  metaFile.close();
  return newSession;
  
}

//
// READ LINE FROM FILE AND RETURN STRING
//
String readLine() {
  return 
}


//
// INITIALIZE NEW TRACK
//
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
  if (dataFile) {
    dataFile.println("Session 3"); // Name
    dataFile.println("11/2/2019"); // Date Created
    dataFile.println("11/5/2019"); // Last opened
    dataFile.println("85"); // BPM
    dataFile.println("8"); // Length
    dataFile.close();
  }
  else {
    Serial.println("Error opening filename in writeMetadata");
  }
}

//
// DELETE SESSION FILES AND FOLDER
//
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

//
// DEBUGGING: PRINT FILE SYSTEM
//
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


//
// SETUP CODE
//
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
  

     //
     // TEST CODE
     //
    createSession(3);
    createSession(5);
    createSession(23);
    deleteSession(23);

    root = SD.open("Sessions");
    SessionMeta sessionArray[99];
    int numSessions = getSessionOverview(sessionArray);
    for (int i=0; i<numSessions; i++) {
      Serial.println(sessionArray[i].name);
    }
  
    root.rewindDirectory();
    root.close();
    
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
