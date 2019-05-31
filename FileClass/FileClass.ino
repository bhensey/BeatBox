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

struct Track
{
  bool trackExists = 0;
  bool trackMute = 0;
  char trackFilepath[50] = "no filepath";
};

class Session 
{
  public:
    Session(int Num, int Bpm, int Length);
    int sessionNum;
    int sessionBpm;
    int sessionLength;
    Track trackList[4];

    // Helper functions
    void updateMetadata();
    void deleteSession();
    File createTrack(int trackNum);
    void deleteTrack(int trackNum);
    void muteTrack(int trackNum);
    void unmuteTrack(int trackNum);
    
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
  int getSessionOverview(Session **sessionArray); // Takes a preallocated empty session array, returns size and populates it
  Session* getSession(int sessionNum); // Takes a session number, returns a session object
  
  // Helper functions

  private:

};



//
// Define FileClass Methods
//

int FileClass::getSessionOverview(Session **sessionArray) {
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
    Serial.print("Session Name: ");
    Serial.println(entry.name());
    sessionArray[numSessions] = getSession(String(entry.name()).toInt());
    numSessions += 1;
    entry.close();
    Serial.println(sessionArray[numSessions-1]->sessionNum);
  }
  
  return numSessions;
}

Session* FileClass::getSession(int Num) {
  char sessionFilepath [50];
  char tmpFilepath [50];
  char buff [50];
  String(Num).toCharArray(buff, 50);
  strcpy(sessionFilepath, "Sessions/");
  strcat(sessionFilepath, buff);
  strcat(sessionFilepath, "/meta");
  if (SD.exists(sessionFilepath)) {
    Serial.printf("Getting session %d\n", Num);
  } else {
    Serial.printf("Error: Cannot get session %d\n", Num);
  }
  File metaFile = SD.open(sessionFilepath);
  int sessionNum = metaFile.readStringUntil('\n').trim().toInt();
  int sessionBpm = metaFile.readStringUntil('\n').trim().toInt(); 
  int sessionLength = metaFile.readStringUntil('\n').trim().toInt(); 
  Session* session = new Session(sessionNum, sessionBpm, sessionLength);
  // Update track properties
  for (int i = 0; i < 4; i++) {
    if (SD.exists(session->trackList[i].trackFilepath)) {
      Serial.printf("Session %s, Track %d exists\n", sessionNum, i);
      session->trackList[i].trackExists = 1;
      session->trackList[i].trackMute = metaFile.readStringUntil('\n').trim().toInt(); 
      } else{
        //Serial.printf("Track %d does not exist\n", i);
        int throwaway = metaFile.readStringUntil('\n').trim().toInt(); 
      }
    } 
  metaFile.close();
  return session;
}


//
// Define Session Methods
//

Session::Session(int Num, int Bpm, int Length) {
  char tmpFilepath [50];
  char buff1 [50];
  String(Num).toCharArray(buff1, 50);
  sessionNum = Num;
  sessionBpm = Bpm;
  sessionLength = Length;
  strcpy(_sessionFilepath, "Sessions/");
  strcat(_sessionFilepath, buff1);
  strcat(_sessionFilepath, "/");
  strcpy(_metaFilepath, _sessionFilepath);
  strcat(_metaFilepath, "meta"); 
  // Generate track filepaths
  char buff2[4];
  for (int i = 0; i < 4; i++) {
      strcpy(tmpFilepath, _sessionFilepath);
      strcat(tmpFilepath, "track_");
      strcat(tmpFilepath, itoa(i, buff2, 4));
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
    Serial.printf("Updating metadata for %d\n", sessionNum);
  } else {
    Serial.printf("Initializing metadata for %d\n", sessionNum);
  }
  File dataFile = SD.open(_metaFilepath, FILE_WRITE);
  if (dataFile) {
    dataFile.println(String(sessionNum)); // Name
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

File Session::createTrack(int trackNum) {
  Serial.printf("Creating track %d\n", trackNum);
  if (trackNum > 3) {
    Serial.printf("Track must be between 0 and 3");
  }
  trackList[trackNum].trackExists = 1;
  return SD.open(trackList[trackNum].trackFilepath, FILE_WRITE);
}

void Session::deleteTrack(int trackNum){
  Serial.printf("Deleting track %d\n", trackNum);
  trackList[trackNum].trackExists = 0;
  SD.remove(trackList[trackNum].trackFilepath);
  updateMetadata();
}

void Session::muteTrack(int trackNum){
  Serial.printf("Muting track %d\n", trackNum);
  trackList[trackNum].trackMute = 1;
  updateMetadata();
}

void Session::unmuteTrack(int trackNum) {
  Serial.printf("Unmuting track %d\n", trackNum);
  trackList[trackNum].trackMute = 0;
  updateMetadata();
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
  Session* sessionTest = new Session(12, 13, 14);
  Serial.println(sessionTest->sessionNum);

  
  Session session3 = Session(3, 85, 16);
  Session session4 = Session(4, 85, 16);
  Session session5 = Session(5, 85, 16);
  Session session24 = Session(24, 85, 16);
  File testTrack = session4.createTrack(3);
  testTrack.close();
  session4.deleteTrack(3);
  Session* sessionSD = fileSystem.getSession(4);
  Serial.println("SessionSD Length:");
  Serial.println(sessionSD->sessionLength);
  session4.deleteSession();
  Serial.println("Printing Directory:");
  printDirectory(SD.open("Sessions/"), 1);
  Session *sessionArray[99];
  int numSessions = fileSystem.getSessionOverview(sessionArray);
  Serial.println("There are " + String(numSessions) + " Sessions");
}

void loop() {
  // put your main code here, to run repeatedly:

}
