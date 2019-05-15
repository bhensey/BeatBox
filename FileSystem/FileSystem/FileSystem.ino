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

struct Session {
  String sessionName;
  int sessionBpm;
  int sessionLength;
  String trackOne;
  String trackTwo;
  String trackThree;
  String trackFour;
  
  
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
int getSessionOverview(Session *sessionArray) {
  // Scan SD card and return a pointer to an array of every existing session in order
  Serial.println("Running getSessionOverview...");
  int numSessions = 0;
  File dir = SD.open("Sessions");
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files  
      break;
      }
    Session newSession = getSession(entry.name());
    Serial.println("Session Name: " + newSession.sessionName);
    sessionArray[numSessions] = newSession;
    
    numSessions += 1;
    entry.close();
  }
  
  return numSessions;
}

//
// READ LINE FROM FILE AND RETURN STRING
//
String readLine(File readFile) {
  String output = "";
  int fileChar = readFile.read();
    while (fileChar != -1 && fileChar != 44) {
      output += char(fileChar);
      fileChar = readFile.read();
    }  
  return output;
}

//
// GET METADATA
//
Session getSession(String sessionName) {
  char filename[50];
  String fileString;
  fileString = "Sessions/";
  fileString += sessionName;
  fileString += "/meta";
  fileString.toCharArray(filename,50);
  File metaFile = SD.open(filename);
  Session newSession = {
    sessionName : readLine(metaFile),      // Name
    sessionBpm : readLine(metaFile).toInt(),  // BPM
    sessionLength : readLine(metaFile).toInt(),   // Length
    trackOne: readLine(metaFile), 
    trackTwo: readLine(metaFile),
    trackThree: readLine(metaFile),
    trackFour: readLine(metaFile)
  };
  metaFile.close();
  
  return newSession;
  
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
 writeMetadata(filename, sessionNumber);
}

void writeMetadata(char* filename, int sessionNumber) {
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.print("Session_" + String(sessionNumber) +','); // Name
    dataFile.print("85,"); // BPM
    dataFile.print("16,"); // Length
    dataFile.print("1.RAW,2.RAW,3.RAW,4.RAW"); // Four Tracks
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
   deleteSession(3);
   deleteSession(5);
   deleteSession(23);
    createSession(3);
    createSession(5);
    createSession(23);

    root = SD.open("Sessions");
    Session sessionArray[99];
    int numSessions = getSessionOverview(sessionArray);
    for (int i=0; i<numSessions; i++) {
      Serial.println(sessionArray[i].sessionName);
      Serial.println(sessionArray[i].sessionBpm);
      Serial.println(sessionArray[i].sessionLength);
      Serial.println(sessionArray[i].trackOne);
    }
    root.rewindDirectory();
    root.close();
    
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
