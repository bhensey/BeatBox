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

// TO DO
// 1. Impliment getSessionOverview 
// 2. Impliment createTrack() and muteTrack()
// 3. 
//

struct Track
{
  bool trackExists = 0;
  bool trackMute = 0;
  char trackFilepath[50] = "no filepath";
};

class Session 
{
  public:
    Session();
    Session(char* Name, int Bpm, int Length);
    char* sessionName;
    int sessionBpm;
    int sessionLength;
    Track trackList[4];

    // Helper functions
    void updateMetadata();
    void deleteSession();
    void createTrack(int trackNum);
    void muteTrack(int trackNum);
    
    
    // Debugging functions
    void showFilepath() {Serial.println(_sessionFilepath);};
    void showMetaFilepath() {Serial.println(_metaFilepath);};
    
  private:
    char _sessionFilepath[50];
    char _metaFilepath[50];
};


class FileClass
{
  public:
  int getSessionOverview(Session *sessionArray); // Takes a preallocated empty session array, returns size and populates it
  Session getSession(char* sessionNum); // Takes a session number, returns a session object
  
  // Helper functions

  private:

};



//
// Define FileClass Methods
//

int FileClass::getSessionOverview(Session *sessionArray) {
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
    Session session = getSession(entry.name());
    Serial.print("Session Name: ");
    Serial.println(session.sessionName);
    sessionArray[numSessions] =session;
    numSessions += 1;
    entry.close();
  }
  
  return numSessions;
}

Session FileClass::getSession(char* sessionNum) {
  char sessionFilepath [50];
  char tmpFilepath [50];
  strcpy(sessionFilepath, "Session/");
  strcat(sessionFilepath, sessionNum);
  strcat(sessionFilepath, "/meta");
  if (SD.exists(sessionFilepath)) {
    Serial.printf("Getting session %s\n", sessionNum);
  } else {
    Serial.printf("Error: Cannot get session %s\n", sessionNum);
  }
  File metaFile = SD.open(sessionFilepath);
  char sessionName[50]; 
  metaFile.readStringUntil('\n').trim().toCharArray(sessionName, 50);
  int sessionBpm = metaFile.readStringUntil('\n').trim().toInt(); 
  int sessionLength = metaFile.readStringUntil('\n').trim().toInt(); 
  Session session = Session(sessionName, sessionBpm, sessionLength);
  // Update track properties
  for (int i = 0; i < 4; i++) {
    if (SD.exists(session.trackList[i].trackFilepath)) {
      Serial.printf("Track %d exists\n", i);
      session.trackList[i].trackExists = 1;
      session.trackList[i].trackMute = metaFile.readStringUntil('\n').trim().toInt(); 
      } else{
        Serial.printf("Track %d does not exist\n", i);
        int throwaway = metaFile.readStringUntil('\n').trim().toInt(); 
      }
    } 
  metaFile.close();
  return session;
}


//
// Define Session Methods
//

Session::Session() {
  sessionName = "empty";
  sessionBpm = 0;
  sessionLength = 0;
}

Session::Session(char* Name, int Bpm, int Length) {
  char tmpFilepath [50];
  sessionName = Name;
  sessionBpm = Bpm;
  sessionLength = Length;
  strcpy(_sessionFilepath, "Session/");
  strcat(_sessionFilepath, Name);
  strcat(_sessionFilepath, "/");
  strcpy(_metaFilepath, _sessionFilepath);
  strcat(_metaFilepath, "meta"); 
  // Generate track filepaths
  char buff[4];
  for (int i = 0; i < 4; i++) {
      strcpy(tmpFilepath, _sessionFilepath);
      strcat(tmpFilepath, "track_");
      strcat(tmpFilepath, itoa(i, buff, 4));
      strcat(tmpFilepath, ".raw");
      strcpy(trackList[i].trackFilepath, tmpFilepath);
      trackList[i].trackMute = 1;
    } 
  if (SD.exists(_sessionFilepath)) {
    Serial.printf("Getting directory %s\n", _sessionFilepath);
  } else {
    Serial.printf("Making directory %s\n", _sessionFilepath);
    SD.mkdir(_sessionFilepath);
    updateMetadata();
  }
  if (!SD.exists(_sessionFilepath)) {
    Serial.printf("Error creating %s\n", _sessionFilepath);
  } 
}

void Session::deleteSession() {
  File dir = SD.open(_sessionFilepath);
  char tmpFilepath[50];
  // Delete all files in folder
  while (true) {
    File entry = dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    strcpy(tmpFilepath, _sessionFilepath);
    strcat(tmpFilepath, entry.name());
    SD.remove(tmpFilepath);
    Serial.printf("Removed file %s\n", tmpFilepath);
  }
  SD.rmdir(_sessionFilepath);
  if (SD.exists(_sessionFilepath)){
    Serial.printf("ERROR: The session %s was not removed\n", _sessionFilepath);
  }
  else {
    Serial.printf("Directory %s is removed\n", _sessionFilepath);
  }
}

void Session::updateMetadata() {
  if (SD.exists(_metaFilepath)) {
    SD.remove(_metaFilepath);
    Serial.printf("Updating metadata for %s\n", sessionName);
  } else {
    Serial.printf("Initializing metadata for %s\n", sessionName);
  }
  File dataFile = SD.open(_metaFilepath, FILE_WRITE);
  if (dataFile) {
    dataFile.println(String(sessionName)); // Name
    dataFile.println(String(sessionBpm)); // BPM
    dataFile.println(String(sessionLength)); // Length
    dataFile.println(String(trackList[0].trackMute)); // Track mute options
    dataFile.println(String(trackList[1].trackMute));
    dataFile.println(String(trackList[2].trackMute));
    dataFile.println(String(trackList[3].trackMute));
    dataFile.close();
  } else {
    Serial.println("Error in updateMetadata");
  } 
}

//
// DEBUGGING CODE
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
      Serial.println("\t\t");
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

  FileClass fileSystem = FileClass();
  Session session = Session("4", 85, 16);
  session.sessionLength = 17;
  File testTrack = SD.open("Session/4/track_3.raw", FILE_WRITE);
  testTrack.close();
  session.updateMetadata();
  Session sessionSD = fileSystem.getSession("4");
  SD.remove("Session/4/track_3.raw");
  Serial.println(sessionSD.sessionLength);
  session.deleteSession();
  Serial.println("Printing Directory:");
  printDirectory(SD.open("Session/"), 1);
  Session sessionArray[99];
  int numSessions = fileSystem.getSessionOverview(sessionArray);
  for (int i=0; i<numSessions; i++) {
    Serial.println(sessionArray[i].sessionName);
    Serial.println(sessionArray[i].sessionBpm);
    Serial.println(sessionArray[i].sessionLength);
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
