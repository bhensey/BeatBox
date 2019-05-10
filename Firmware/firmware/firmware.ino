#include <Time.h>
#include <SPI.h>
#include <Wire.h>
#include <Audio.h>
#include <SD.h>
#include <SerialFlash.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AudioSampleKick.h"         // http://www.freesound.org/people/DWSD/sounds/171104/
#include "AudioSampleMetronome.h"

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

#define MENU_HOME           0
#define MENU_SESSION_SEL    1
#define MENU_SESSION        2
#define MENU_SESSION_CONFIG 3
#define MENU_NEW_SESSION    4
#define MENU_TRACK_SEL      5
#define MENU_TRACK_CONFIG   6
#define MENU_SETTINGS       7

#define NEW_SESSION       0
#define EXISTING_SESSION  1
#define SETTINGS          2

#define SESSION_OPEN      0
#define SESSION_DELETE    1

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define RIGHT_ARROW   16
#define LEFT_ARROW    17

#define LEFT                  8
#define CENTER                4
#define RIGHT                 2
#define FAR_RIGHT             1
#define IS_SEL_LEFT(n)       (!!(n & LEFT))
#define IS_SEL_CENTER(n)     (!!(n & CENTER))
#define IS_SEL_RIGHT(n)      (!!(n & RIGHT))
#define IS_SEL_FAR_RIGHT(n)  (!!(n & FAR_RIGHT))

#define INTERRUPT_THRESHOLD   200

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     5 // Reset pin # (or -1 if sharing Arduino reset pin)

// Data Structures
struct date {
  int hour;
  int minute;
  int second;
  int day;
  int month;
  int year;
};

struct track {
  int number;
  bool mt;
  bool mute;
  int duration;
  String filename;
};

struct session {
  String session_name;
  int duration;
  track tracklist[];
  date date_created;
  date last_modified;
};

struct sessionView {
  String session_name;
  //date last_modified;
};

struct globalConfig {
  int bpm;
  int vol;
  bool play_rec;  // indicates what pressing the play/rec button will do (0=play, 1=rec)
};

enum SessionState {
  Valid,
  Deleted,
  End
};


// Global Variables
const int leftButton = 16;      // the number of the left button pin
const int rightButton = 15;     // the number of the right button pin
const int selectButton = 14;    // the number of the select button pin
const int backButton = 35;      // the number of the back button pin
const int clickButton = 37;     // the number of the click button pin
const int hapticButton = 36;    // the number of the haptic button pin
const int playrecButton = 38;   // the number of the play/rec button pin

const int BPMPin1 = 31;         // the number of the right BPM encoder pin
const int BPMPin2 = 32;         // the number of the left BPM encoder pin
const int volPin1 = 29;         // the number of the right volume encoder pin
const int volPin2 = 30;         // the number of the left volume encoder pin

const int beatLED = 13;         // the number of the beat indicator LED
const int clickLED = 1;         // the number of the click enable LED
const int hapticLED = 2;        // the number of the haptic enable LED
const int recordingLED = 0;     // the number of the recording indicator LED

static unsigned long last_interrupt_time = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

IntervalTimer beatTimer;

Encoder BPM_Enc(BPMPin1,BPMPin2);
Encoder vol_Enc(volPin1,volPin2);

struct globalConfig statusBar = {85, 50, false};

int menu_id; // current state of the menu

int num_sessions; // number of existing sessions

sessionView sessions[99]; // empty list of sessions

session current_session; // current session we're in

// menu selection global variables
int selected_home_option = 0;
int selected_setting = 0;
int selected_session = 0;
int viewable_sessions[] = {0,1,2};
int selected_session_config_option = 0;
int selected_track = 0;
int selected_track_option = 0;

bool session_playing;
bool track_playing;

bool beat_LED_enable = false;
bool click_enable = false;
bool haptic_enable = false;

bool play_rec_pressed_flag = false;
bool right_pressed_flag = false;
bool left_pressed_flag = false;
bool select_pressed_flag = false;
bool back_pressed_flag = false;

char track1[] = "SDTEST1.WAV";
char track2[] = "SDTEST2.WAV";
char track3[] = "SDTEST3.WAV";
char track4[] = "SDTEST4.WAV";

long oldBPMPosition = -999;
long oldVolPosition = -999;
long newVolPosition;
long newBPMPosition;

volatile unsigned int VOL_encoderPos = 50;  // a counter for the dial
static boolean VOL_rotating = false;    // debounce management

volatile unsigned int BPM_encoderPos = 85;  // a counter for the dial
static boolean BPM_rotating = false;    // debounce management

// interrupt service routine vars
boolean VOL_A_set = false;
boolean VOL_B_set = false;

boolean BPM_A_set = false;
boolean BPM_B_set = false;


// SETUP AND LOOP FUNCTIONS
void setup() {
  //Serial.begin(9600);

  AudioMemory(60); // Memory for all audio funcitons especially recording buffer
  sgtl5000_1.enable();
  sgtl5000_1.volume(1);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      //Serial.println("Unable to access the SD card");
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
  mixer3.gain(0, 0.4); // Track Mixer
  mixer3.gain(1, 0.7); // Metronome Mixer
  
  // initialize pins as inputs:
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(backButton, INPUT_PULLUP);
  pinMode(clickButton, INPUT_PULLDOWN);
  pinMode(hapticButton, INPUT_PULLDOWN);
  pinMode(playrecButton, INPUT_PULLDOWN);
  
  pinMode(beatLED,OUTPUT);
  pinMode(clickLED,OUTPUT);
  pinMode(hapticLED,OUTPUT);
  pinMode(recordingLED,OUTPUT);

  // initialize beat timer
  beatTimer.begin(sendBeat, 0.5*(60*pow(10,6))/statusBar.bpm);
  
  // initialize ISRs
  attachInterrupt(digitalPinToInterrupt(leftButton), leftButton_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(rightButton), rightButton_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(selectButton), selectButton_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(backButton), backButton_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(clickButton), clickButton_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(hapticButton), hapticButton_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(playrecButton), playrecButton_ISR, FALLING);

  attachInterrupt(digitalPinToInterrupt(volPin1), changeVOL_UP,   CHANGE);
  attachInterrupt(digitalPinToInterrupt(volPin2), changeVOL_DOWN, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BPMPin1), changeBPM_UP,   CHANGE);
  attachInterrupt(digitalPinToInterrupt(BPMPin2), changeBPM_DOWN, CHANGE);


  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();

  // get session information from SD card
  num_sessions = getSessionOverview(sessions); // populates list of sessions, returns the number of sessions

  menu_id = 0;
}

// Interrupt on A changing state
void changeVOL_DOWN() {
  // debounce
  if ( VOL_rotating ) delay (1);  // wait a little until the bouncing is done

  // Test transition, did things really change?
  if ( digitalRead(volPin1) != VOL_A_set ) { // debounce once more
    VOL_A_set = !VOL_A_set;

    // adjust counter + if A leads B
    if ( VOL_A_set && !VOL_B_set )
      VOL_encoderPos += 1;

    VOL_rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void changeVOL_UP() {
  if ( VOL_rotating ) delay (1);
  if ( digitalRead(volPin2) != VOL_B_set ) {
    VOL_B_set = !VOL_B_set;
    //  adjust counter - 1 if B leads A
    if ( VOL_B_set && !VOL_A_set )
      VOL_encoderPos -= 1;

    VOL_rotating = false;
  }
}

// Interrupt on A changing state
void changeBPM_DOWN() {
  // debounce
  if ( BPM_rotating ) delay (1);  // wait a little until the bouncing is done

  // Test transition, did things really change?
  if ( digitalRead(BPMPin1) != BPM_A_set ) { // debounce once more
    BPM_A_set = !BPM_A_set;

    // adjust counter + if A leads B
    if ( BPM_A_set && !BPM_B_set )
      BPM_encoderPos += 1;

    BPM_rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void changeBPM_UP() {
  if ( BPM_rotating ) delay (1);
  if ( digitalRead(BPMPin2) != BPM_B_set ) {
    BPM_B_set = !BPM_B_set;
    //  adjust counter - 1 if B leads A
    if ( BPM_B_set && !BPM_A_set )
      BPM_encoderPos -= 1;

    BPM_rotating = false;
  }
}

void handleStatus() {
  if(VOL_encoderPos > 201) VOL_encoderPos = 0;
    if(VOL_encoderPos > 100) VOL_encoderPos = 100;
    
    if(BPM_encoderPos < 40) BPM_encoderPos = 40;
    if(BPM_encoderPos > 240) BPM_encoderPos = 240;
    statusBar.vol = VOL_encoderPos;
    changeVol();

    statusBar.bpm = BPM_encoderPos;
    changeBPM();
    
}

void loop() {

  handleStatus();
  drawStatusBar();

  switch (menu_id) {
    case (MENU_HOME):                               // HOME
      drawHome(selected_home_option);
      // MENU NAVIGATION
      if (right_pressed_flag) {
        if (selected_home_option < 2) {
          selected_home_option += 1;
        }
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        if (selected_home_option > 0) {
          selected_home_option -= 1;
        }
        left_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // either create new session, show existing sessions, or go to settings
        if (selected_home_option == NEW_SESSION) {  
          menu_id = MENU_SESSION_CONFIG;
          newSession();
          selected_session = num_sessions + 1; // this could be done in newSession() if we want
          // ADD A NEW SESSION TO MEMORY (TODO)
        }
        else if (selected_home_option == EXISTING_SESSION) {
          menu_id = MENU_SESSION_SEL;
        }
        else if (selected_home_option == SETTINGS) {
          menu_id = MENU_SETTINGS;
        }
        select_pressed_flag = false;
      }
      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new session with new track
        newQuickSession();
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_SETTINGS):                               // SETTINGS
      drawSettings(selected_setting);
      // MENU NAVIGATION
      if (right_pressed_flag) {
        if (selected_setting < 2) {
          selected_setting += 1;
        }
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        if (selected_setting > 0) {
          selected_setting -= 1;
        }
        left_pressed_flag = false;
      }
      if (back_pressed_flag) {
        menu_id = MENU_HOME;
        back_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // enter setting options
        //enterSetting(selected_setting);
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new session with new track
        newQuickSession();
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_SESSION_SEL):                               // SESSION SELECT
      drawSessionSelect(selected_session);
      // MENU NAVIGATION
      if (right_pressed_flag) {
        if (selected_session < num_sessions-1) {
          selected_session++;
        }
        if (selected_session > viewable_sessions[2]) {
          viewable_sessions[0]++;
          viewable_sessions[1]++;
          viewable_sessions[2]++;
        }
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        if (selected_session > 0) {
          selected_session--;
        }
        if (selected_session < viewable_sessions[0]) {
          viewable_sessions[0]--;
          viewable_sessions[1]--;
          viewable_sessions[2]--;
        }
        left_pressed_flag = false;
      }
      if (back_pressed_flag) {
        menu_id = MENU_HOME;
        back_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // enter session options
        menu_id = MENU_SESSION_CONFIG;
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new session with new track
        newQuickSession();
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_SESSION_CONFIG):                               // SESSION CONFIG
      drawSessionConfig(selected_session, selected_session_config_option);
      // MENU NAVIGATION
      if (right_pressed_flag) {
        if (selected_session_config_option < 1) {
          selected_session_config_option++;
        }
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        if (selected_session_config_option > 0) {
          selected_session_config_option--;
        }
        left_pressed_flag = false;
      }
      if (back_pressed_flag) {
        menu_id = MENU_SESSION_SEL;
        back_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // either open or delete
        if (selected_session_config_option == SESSION_OPEN) {
          //enterSession(sessions[selected_session-1]);
        }
        if (selected_session_config_option == SESSION_DELETE) {
          //deleteSession(sessions[selected_session-1]);
        }
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new track in current session
        //enterSession(sessions[selected_session-1]);
        //newTrack(sessions[selected_session-1]);
        menu_id = MENU_TRACK_CONFIG;
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_TRACK_SEL):                               // TRACK SELECT
      // MENU NAVIGATION
      if (right_pressed_flag) {
        selected_track += 1;
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        selected_track -= 1;
        left_pressed_flag = false;
      }
      if (back_pressed_flag) {
        menu_id = MENU_SESSION_CONFIG;
        selected_session_config_option = 1;
        back_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // either new track or existing track
        if (selected_track == 0) {
          //newTrack(sessions[selected_session-1]);
        } else {
          //enterTrack(sessions[selected_session-1]);
        }
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // play button pressed - play/pause session
        if (!session_playing) {
          playSession();
        } else {
          pauseSession();
        }
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_TRACK_CONFIG):                               // TRACK CONFIG
      // MENU NAVIGATION
      if (right_pressed_flag) {
        selected_track_option += 1;
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        selected_track_option -= 1;
        left_pressed_flag = false;
      }
      if (back_pressed_flag) {
        menu_id = MENU_TRACK_SEL;
        back_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // perform action
        if (selected_track_option == 1) {
          // mute track
          //muteTrack(selected_track);
        }
        if (selected_track_option == 2) {
          // erase track
          //eraseTrack(selected_track);
          // go back to track selection
          menu_id = MENU_TRACK_SEL;
          // (check for edge cases about which track to have selected upon return)
        }
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // play button pressed - play/pause track
        if (!track_playing) {
          playTrack();
        } else {
          pauseTrack();
        }
        play_rec_pressed_flag = false;
      }
      break;
  } 
}

// DISPLAY FUNCTIONS
void drawStatusBar() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  //                x,y
  display.setCursor(2,2);
  display.cp437(true);
  
  String BPM = "BPM:" + String(statusBar.bpm);
  display.println(BPM);
  
  String VOL = "Vol:" + String(statusBar.vol);
  display.setCursor(48,2);
  display.println(VOL);

  if(!statusBar.play_rec) {
    display.setCursor(100,2);
    display.println("REC");
  } else {
    display.setCursor(97,2);
    display.println("Play");
  }

  display.setCursor(122,2);
  display.write(RIGHT_ARROW);
  display.drawLine(0, 9, 128, 9, WHITE);
}

void boxed_text(String text, int x_pos, int y_pos, bool selected) {
  if(!selected) display.setTextColor(WHITE, BLACK);
  else display.setTextColor(BLACK,WHITE);
  int len = text.length();
  display.setCursor(x_pos, y_pos);
  display.println(text);
  display.drawRect(x_pos-2, y_pos-2, len*7, 12, WHITE);
  display.setTextColor(WHITE, BLACK);
}

void draw_level(String Name) {
  display.setCursor(0,11);
  display.println(Name);
}

void drawHome(uint8_t selected) {
  uint8_t highlight = 0x8>>selected;
  draw_level("Home");
  boxed_text("New", 10, display.height() / 2 + 10, IS_SEL_LEFT(highlight));
  boxed_text("Files", 45, display.height() / 2 + 10, IS_SEL_CENTER(highlight));
  boxed_text("Prefs", 90, display.height() / 2 + 10, IS_SEL_RIGHT(highlight));
  display.display();
}

void centerText(String text, int y_pos) {
  int len = text.length();
  int num_px = len*6;
  display.setCursor(SCREEN_WIDTH/2 - num_px/2,y_pos);
  display.println(text);
}

void drawSettings(uint8_t selected) {
  uint8_t highlight = 0x8>>selected;
  draw_level("Preferences");
  boxed_text("Sound", 12, display.height() / 2 + 10, IS_SEL_LEFT(highlight));
  boxed_text("Reset", 51, display.height() / 2 + 10, IS_SEL_CENTER(highlight));
  boxed_text("Loop", 91, display.height() / 2 + 10, IS_SEL_RIGHT(highlight));
  display.display();
}

void drawSessionSelect(uint8_t selected) {
  uint8_t highlight = 0x8>>(selected-viewable_sessions[0]);
  draw_level("Session Select");
  if (num_sessions == 0) {
    // no sessions
  } else if (num_sessions == 1) {
    String sess_l = "S" + String(viewable_sessions[0]+1);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
  } else if (num_sessions == 2) {
    String sess_l = "S" + String(viewable_sessions[0]+1);
    String sess_c = "S" + String(viewable_sessions[1]+1);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
    boxed_text(sess_c, 60, display.height() / 2 - 4, IS_SEL_CENTER(highlight));
  } else {
    String sess_l = "S" + String(viewable_sessions[0]+1);
    String sess_c = "S" + String(viewable_sessions[1]+1);
    String sess_r = "S" + String(viewable_sessions[2]+1);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
    boxed_text(sess_c, 60, display.height() / 2 - 4, IS_SEL_CENTER(highlight));
    boxed_text(sess_r, 100, display.height() / 2 - 4, IS_SEL_RIGHT(highlight));
  }
  if(viewable_sessions[0] > 0) { // left arrow
    display.setCursor(2,display.height() / 2 - 4);
    display.write(LEFT_ARROW);
    
  }
  if(viewable_sessions[2] < num_sessions-1) { // right arrow
    display.setCursor(122,display.height() / 2 - 4);
    display.write(RIGHT_ARROW);
  }

  centerText(sessions[selected_session].session_name, 40);
  display.display();
}

void drawSessionConfig(int session_number, bool highlight) {
  draw_level("Session Config");
  String sess = "S" + String(session_number+1); 
  boxed_text(sess, display.width() / 2 - 10, display.height() / 2 - 4, false);
  boxed_text("Open", 25, display.height() / 2 + 10, !highlight);
  int posn_x = 67;
  if(sess.length() > 2) posn_x = 74;
  boxed_text("Delete", posn_x, display.height() / 2 + 10, highlight);
  display.display();
}

void drawTrackSelect(uint8_t existing_tracks, uint8_t highlight) {
  draw_level("Track Select");
  String track_dne = " + ";
  if(IS_SEL_LEFT(existing_tracks)) {
    boxed_text("T1", 10, display.height() - 20, IS_SEL_LEFT(highlight));
  } else {
    boxed_text(track_dne, 10, display.height() - 20, IS_SEL_LEFT(highlight));
  }

  if(IS_SEL_CENTER(existing_tracks)) {
     boxed_text("T2", 40, display.height() - 20, IS_SEL_CENTER(highlight));
  } else {
     boxed_text(track_dne, 40, display.height() - 20, IS_SEL_CENTER(highlight));
  }

  if(IS_SEL_RIGHT(existing_tracks)) {
     boxed_text("T3", 70, display.height() - 20, IS_SEL_RIGHT(highlight));
  } else {
     boxed_text(track_dne, 70, display.height() - 20, IS_SEL_RIGHT(highlight));
  }

  if(IS_SEL_FAR_RIGHT(existing_tracks)) {
     boxed_text("T4", 100, display.height() - 20, IS_SEL_FAR_RIGHT(highlight));
  } else {
     boxed_text(track_dne, 100, display.height() - 20, IS_SEL_FAR_RIGHT(highlight));
  }
  
  display.display();
}

void drawTrackOptions(uint8_t track_no, uint8_t highlight) {
  draw_level("Track Options");
  String track = "T" + String(track_no);
  boxed_text(track, display.width() / 2 - 10, display.height() - 25, false);
  boxed_text("Mute", 25, display.height() - 10, IS_SEL_LEFT(highlight));
  int posn_x = 67; 
  if(track.length() > 2) posn_x = 74;
  boxed_text("Delete", posn_x, display.height() - 10, IS_SEL_RIGHT(highlight));
  display.display();
}

void drawStorageLimit() {
  display.clearDisplay();
  display.setCursor(display.width()/2-20, 0);
  display.println("Warning!");
  display.setCursor(2,13);
  display.println("Storage Limit Reached\nLess than 100MB free.\nDelete or export\ncontent.");
  display.drawRect(0, 11, 128, 50, WHITE);
  display.display();
}


// CORE FUNCTIONS
int getSessionOverview(sessionView sessions_list[]) {
  // currently creates a dummy list of 5 sessions
  String s = "Session ";
  String n;
  for (int i = 0; i < 5; i++) {
    n = s + (i+1);
    sessions_list[i] = {n};
  }
  return 5;
}
void newSession() {

}

void newQuickSession() {

}

void enterSession() {

}

void deleteSession() {

}

void newTrack() {

}

void playSession() {

}

void pauseSession() {

}

void playTrack() {

}

void pauseTrack() {

}

void muteTrack() {

}

void eraseTrack() {

}

void startRecording() {

}

void stopRecording() {

}

void enterSetting() {

}


// Interrupt Service Routines
void debounce_normal(bool &flag, String message) {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > INTERRUPT_THRESHOLD) 
  {
   flag = true;
   //Serial.println(message);
  }
  last_interrupt_time = interrupt_time;
}

void debounce_toggle(bool& flag, int pin, String message) {
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > INTERRUPT_THRESHOLD) {
    flag = !flag;
    if (flag) {
      digitalWrite(pin,HIGH);
      //Serial.println(message + "->on");
    } else {
      digitalWrite(pin,LOW);
      //Serial.println(message + "->off");
    }
  }
  last_interrupt_time = interrupt_time;
}

void sendBeat() {
  // blink LED
  if (beat_LED_enable) {
     digitalWrite(beatLED,LOW);
     beat_LED_enable = false;
  } else {
     digitalWrite(beatLED,HIGH);
     beat_LED_enable = true;
  }
  // conditionally send click
  if (click_enable && beat_LED_enable) {
    playMem1.play(AudioSampleMetronome);
  }
  // conditionally send pulse to haptic
}
void changeBPM() {
  // update statusBar.bpm with new value
  beatTimer.update((0.5*60*pow(10,6))/statusBar.bpm);
}
void changeVol() {
  // update statusBar.vol with new value
}
void leftButton_ISR() {
  debounce_normal(left_pressed_flag, "Left button pressed");
}
void rightButton_ISR() {
  debounce_normal(right_pressed_flag, "Right button pressed");
}
void selectButton_ISR() {
  debounce_normal(select_pressed_flag, "Select button pressed");
}
void backButton_ISR() {
  debounce_normal(back_pressed_flag, "Back button pressed");
}
void playrecButton_ISR() {
  debounce_normal(play_rec_pressed_flag, "Play/rec button pressed");
}
void clickButton_ISR() {
  debounce_toggle(click_enable, clickLED, "Click button pressed");
}
void hapticButton_ISR() {
  debounce_toggle(haptic_enable, hapticLED, "Haptic button pressed");
}
