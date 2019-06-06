#include <Audio.h>
#include <SD.h>

struct Track
{
  bool trackExists = 0;
  bool trackMute = 0;
  char trackFilepath[50] = "no filepath";
};

class Session 
{
  public:
    Session(int num, int BPM, int len);
    Session() {};
    int sessionNum;
    int sessionBPM;
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
  Session getSession(int sessionNum); // Takes a session number, returns a session object
  
  // Helper functions

  private:

};


//
// DEBUGGING CODE
//
void printDirectory(File dir, int numTabs);

//
// HELPER FUNCTIONS
//
void getTrackFilepath(int trackNumber);

bool fileExists(char dir[]);

void deleteAll();

int findIndex(int value);

int compare(const void *a, const void *b);

void updateSessions(int sessionNum);

// CORE FUNCTIONS
int findNewSession();

void newTrack() ;

void playSession();

void pauseSession();

void playTrack() ;

void pauseTrack();

void muteTrack();

void eraseTrack();

void startRecording();

void stopRecording();

void continueRecording();

void deleteRecording();

void enterSetting(int selected_setting);

// Interrupt Service Routines
void debounce_normal(bool &flag, String message);

void debounce_toggle(bool& flag, int pin, String message);

// Interrupt on A changing state
void changeVOL_DOWN();

// Interrupt on B changing state, same as A above
void changeVOL_UP();

// Interrupt on A changing state
void changeBPM_DOWN() ;

// Interrupt on B changing state, same as A above
void changeBPM_UP();

void handleStatus();

void sendBeat();

void onlyRecording();

void changeBPM();

void changeVol();

void leftButton_ISR();

void rightButton_ISR();

void selectButton_ISR();

void backButton_ISR();

void playrecButton_ISR();

void clickButton_ISR();

void hapticButton_ISR();
