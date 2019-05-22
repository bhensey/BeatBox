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


class Session 
{
  public:
    Session(char* Name, int Bpm, int Length);
    char* sessionName;
    int sessionBpm;
    int sessionLength;

    // Helper functions
    void updateMetadata();
    void deleteSession();
    
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
  
  String _readLine(File readFile); // Takes a file and outputs the next line
  char* _sessionPath(char* sessionNumber); 
};



//
// Define FileClass Methods
//

//
// Define Session Methods
//

Session::Session(char* Name, int Bpm, int Length) {
  sessionName = Name;
  sessionBpm = Bpm;
  sessionLength = Length;
  strcpy(_sessionFilepath, "Session/");
  strcat(_sessionFilepath, Name);
  strcat(_sessionFilepath, "/");
  strcpy(_metaFilepath, _sessionFilepath);
  strcat(_metaFilepath, "meta");
  SD.mkdir(_sessionFilepath);
  if (SD.exists(_sessionFilepath)) {
    Serial.printf("%s is created \n", _sessionFilepath);
  } else {
    Serial.printf("Error creating %s\n", _sessionFilepath);
  }
  updateMetadata();
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
    Serial.printf("Initializing metadata for %s\n", sessionName);
  } else {
    Serial.printf("Updating metadata for %s\n", sessionName);
  }
  File dataFile = SD.open(_metaFilepath, FILE_WRITE);
  if (dataFile) {
    dataFile.println(sessionName); // Name
    dataFile.println(sessionBpm); // BPM
    dataFile.println(sessionLength); // Length
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
  FileClass fileSystem = FileClass();
  Session session = Session("4", 85, 16);
  session.deleteSession();
  Serial.println("Printing Directory:");
  printDirectory(SD.open("Session/"), 1);
}

void loop() {
  // put your main code here, to run repeatedly:

}
