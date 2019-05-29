#include <TimeLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Audio.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#include "SoftwareSerial.h"
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AudioSampleKick.h"         // http://www.freesound.org/people/DWSD/sounds/171104/
#include "AudioSampleMetronome.h"

// GUItool: begin automatically generated code
AudioPlaySdRaw           playSdRaw3;     //xy=201.00390625,220.00390625
AudioPlaySdRaw           playSdRaw2;     //xy=210.00390625,150.00390625
AudioPlaySdRaw           playSdRaw4;     //xy=218.00390625,289.00390625
AudioPlayMemory          playMem1;       //xy=231.25390625,356.50390434265137
AudioPlaySdRaw           playSdRaw1;     //xy=277.00390625,90.00390625
AudioInputI2S            i2s2;           //xy=361.75389862060547,531.0039129257202
AudioMixer4              mixer1;         //xy=437.50390625,249.50391578674316
AudioMixer4              mixer2;         //xy=439.25392150878906,332.50392150878906
AudioRecordQueue         queue1;         //xy=538.75390625,540.50390625
AudioMixer4              mixer3;         //xy=583.2538986206055,282.50390625
AudioOutputI2S           i2s1;           //xy=719.2539520263672,287.5039463043213
AudioConnection          patchCord1(playSdRaw3, 0, mixer1, 2);
AudioConnection          patchCord2(playSdRaw2, 0, mixer1, 1);
AudioConnection          patchCord3(playSdRaw4, 0, mixer1, 3);
AudioConnection          patchCord4(playMem1, 0, mixer2, 0);
AudioConnection          patchCord5(playSdRaw1, 0, mixer1, 0);
AudioConnection          patchCord6(i2s2, 0, queue1, 0);
AudioConnection          patchCord7(mixer1, 0, mixer3, 0);
AudioConnection          patchCord8(mixer2, 0, mixer3, 1);
AudioConnection          patchCord9(mixer3, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=252,427
// GUItool: end automatically generated code

// Use these with the Teensy Audio Shield
//#define SDCARD_CS_PIN    10
//#define SDCARD_MOSI_PIN  7
//#define SDCARD_SCK_PIN   14

// Use these with the Teensy 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11 // not actually used
#define SDCARD_SCK_PIN   13 // not actually used

const int myInput = AUDIO_INPUT_LINEIN;
int mode = 0;

#define MENU_HOME                   0
#define MENU_SESSION_SEL            1
#define MENU_SESSION                2
#define MENU_SESSION_CONFIG         3
#define MENU_NEW_SESSION            4
#define MENU_TRACK_SEL              5
#define MENU_TRACK_CONFIG           6
#define MENU_SETTINGS               7
#define ARE_YOU_SURE                8
#define MENU_SET_LOOP_LENGTH  9
#define ERROR_MENU                  10
#define MENU_SET_DEF_BPM            11
#define MENU_SET_DEF_LOOP_LEN       12

#define NEW_SESSION       0
#define EXISTING_SESSION  1
#define SETTINGS          2

#define SESSION_OPEN      0
#define SESSION_DELETE    1

#define TRACK_MUTE      0
#define TRACK_DELETE    1
#define BEAT_SOUND        0
#define DEFAULT_BPM       1
#define DEFAULT_LOOP_LEN  2

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

#define MIN_VOL     0
#define MAX_VOL     100
#define MIN_BPM     40
#define MAX_BPM     220

#define MAX_SESSIONS  99

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

struct Track {
  bool mute;
  String trackName;
};

struct Session {
  String sessionName;
  int sessionNum;
  int sessionBPM;
  int sessionLength;  // in beats
  String trackOne;
  String trackTwo;
  String trackThree;
  String trackFour;
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

const int beatLED = 3;         // the number of the beat indicator LED // DO NOT ENABLE, INTERFERES WITH AUDIO INTERFACE
const int clickLED = 1;         // the number of the click enable LED
const int hapticLED = 2;        // the number of the haptic enable LED
const int recordingLED = 0;     // the number of the recording indicator LED

static unsigned long last_interrupt_time = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

IntervalTimer beatTimer;

Encoder BPM_Enc(BPMPin1, BPMPin2);
Encoder vol_Enc(volPin1, volPin2);

SoftwareSerial XBee(33, 34);

File frec;

uint8_t bpm = 85;
uint8_t vol = 50;
struct globalConfig statusBar = {bpm, vol, false};

int menu_id = 0; // current state of the menu

int num_sessions; // number of existing sessions

Session sessions[MAX_SESSIONS]; // empty list of sessions

// menu selection global variables
int selected_home_option = 0;
int selected_setting = 0;
int selected_session = 0;
int viewable_sessions[] = {0, 1, 2};
int selected_session_config_option = 0;
int selected_track = 0;
int selected_track_option = 0;

bool recording;
bool playing;
bool pendingRecording = false;

int recording_count = 0;

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

volatile unsigned int VOL_encoderPos = statusBar.vol;  // a counter for the dial
static bool VOL_rotating = false;                   // debounce management

volatile unsigned int BPM_encoderPos = statusBar.bpm;  // a counter for the dial
static bool BPM_rotating = false;                   // debounce management

// interrupt service routine vars
bool VOL_A_set = false;
bool VOL_B_set = false;

bool BPM_A_set = false;
bool BPM_B_set = false;

bool are_you_sure = false;

uint16_t session_length = 16;

uint16_t lead_in_beats = 4;

File root;


// SETUP AND LOOP FUNCTIONS
void setup() {
  Serial.begin(9600);
  while (!Serial);

  AudioMemory(60); // Memory for all audio funcitons especially recording buffer
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
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
  mixer2.gain(0, 1);

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

  pinMode(beatLED, OUTPUT);
  pinMode(clickLED, OUTPUT);
  pinMode(hapticLED, OUTPUT);
  pinMode(recordingLED, OUTPUT);

  // initialize beat timer
  beatTimer.begin(sendBeat, 0.5 * (60 * pow(10, 6)) / statusBar.bpm);

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


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();

  XBee.begin(115200);

  deleteAll(); // deletes all sessions on the card (for testing purposes)
//  createSession(2,16);
  
  // get session information from SD card
  num_sessions = getSessionOverview(sessions); // populates list of sessions, returns the number of sessions

  root = SD.open("Sessions");
  for (int i = 0; i < num_sessions; i++) {
    Serial.println(sessions[i].sessionName);
    Serial.println(sessions[i].sessionBPM);
    Serial.println(sessions[i].sessionLength);
    Serial.println(sessions[i].trackOne);
    Serial.println(sessions[i].trackTwo);
    Serial.println(sessions[i].trackThree);
    Serial.println(sessions[i].trackFour);
  }
  root.rewindDirectory();
  root.close();

}

void loop() {
  handleStatus();
  updateDisplay();

//  if(recording && (recording_count > lead_in_beats) && !pendingRecording) {
//    //Serial.println("Continue Recording");
//    continueRecording();
//  }
}

// DISPLAY FUNCTIONS
void drawStatusBar() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  //                x,y
  display.setCursor(2, 2);
  display.cp437(true);

  String BPM = "BPM:" + String(statusBar.bpm);
  display.println(BPM);

  String VOL = "Vol:" + String(statusBar.vol);
  display.setCursor(48, 2);
  display.println(VOL);

  if (!statusBar.play_rec) {
    display.setCursor(100, 2);
    display.println("REC");
  } else {
    display.setCursor(97, 2);
    display.println("Play");
  }

  display.setCursor(122, 2);
  display.write(RIGHT_ARROW);
  display.drawLine(0, 9, 128, 9, WHITE);
}

void boxed_text(String text, int x_pos, int y_pos, bool selected) {
  if (!selected) display.setTextColor(WHITE, BLACK);
  else display.setTextColor(BLACK, WHITE);
  int len = text.length();
  display.setCursor(x_pos, y_pos);
  display.println(text);
  display.drawRect(x_pos - 2, y_pos - 2, len * 7, 12, WHITE);
  display.setTextColor(WHITE, BLACK);
}

void draw_level(String Name) {
  display.setCursor(0, 11);
  display.println(Name);
}

void drawHome(uint8_t selected) {
  uint8_t highlight = 0x8 >> selected;
  draw_level("Home");
  boxed_text("New", 10, display.height() / 2 + 10, IS_SEL_LEFT(highlight));
  boxed_text("Files", 45, display.height() / 2 + 10, IS_SEL_CENTER(highlight));
  boxed_text("Prefs", 90, display.height() / 2 + 10, IS_SEL_RIGHT(highlight));
  display.display();
}

void centerText(String text, int y_pos) {
  int len = text.length();
  int num_px = len * 6;
  display.setCursor(SCREEN_WIDTH / 2 - num_px / 2, y_pos);
  display.println(text);
}

void drawSettings(uint8_t selected) {
  uint8_t highlight = 0x8 >> selected;
  draw_level("Preferences");
  boxed_text("Sound", 12, display.height() / 2 + 10, IS_SEL_LEFT(highlight));
  boxed_text("BPM", 57, display.height() / 2 + 10, IS_SEL_CENTER(highlight));
  boxed_text("Loop", 91, display.height() / 2 + 10, IS_SEL_RIGHT(highlight));
  display.display();
}

void drawSessionSelect(uint8_t selected) {
  uint8_t highlight = 0x8 >> (selected - viewable_sessions[0]);
  draw_level("Session Select");
  if (num_sessions == 0) {
    // no sessions
  } else if (num_sessions == 1) {
    String sess_l = "S" + String(sessions[viewable_sessions[0]].sessionNum);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
    centerText(sessions[selected_session].sessionName, 40);
  } else if (num_sessions == 2) {
    String sess_l = "S" + String(sessions[viewable_sessions[0]].sessionNum);
    String sess_c = "S" + String(sessions[viewable_sessions[1]].sessionNum);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
    boxed_text(sess_c, 60, display.height() / 2 - 4, IS_SEL_CENTER(highlight));
    centerText(sessions[selected_session].sessionName, 40);
  } else {
    String sess_l = "S" + String(sessions[viewable_sessions[0]].sessionNum);
    String sess_c = "S" + String(sessions[viewable_sessions[1]].sessionNum);
    String sess_r = "S" + String(sessions[viewable_sessions[2]].sessionNum);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
    boxed_text(sess_c, 60, display.height() / 2 - 4, IS_SEL_CENTER(highlight));
    boxed_text(sess_r, 100, display.height() / 2 - 4, IS_SEL_RIGHT(highlight));
    centerText(sessions[selected_session].sessionName, 40);
  }
  if (viewable_sessions[0] > 0) { // left arrow
    display.setCursor(2, display.height() / 2 - 4);
    display.write(LEFT_ARROW);
  }
  if (viewable_sessions[2] < num_sessions - 1) { // right arrow
    display.setCursor(122, display.height() / 2 - 4);
    display.write(RIGHT_ARROW);
  }

  display.display();
}

void drawSessionConfig(int session_number, bool highlight) {
  draw_level("Session Config");
  String sess = "S" + String(session_number);
  boxed_text(sess, display.width() / 2 - 10, display.height() / 2 - 4, false);
  boxed_text("Open", 25, display.height() / 2 + 10, !highlight);
  int posn_x = 67;
  if (sess.length() > 2) posn_x = 74;
  boxed_text("Delete", posn_x, display.height() / 2 + 10, highlight);
  display.display();
}

void drawTrackSelect(uint8_t existing_tracks, uint8_t muted_tracks, uint8_t selected) {
  uint8_t highlight = 0x8 >> selected;
  draw_level("Session " + String(sessions[selected_session].sessionNum));
  display.setCursor(0, 20);
  display.println("Track Select");
  String track_dne = " + ";
  if (IS_SEL_LEFT(existing_tracks)) {
    boxed_text("T1", 10, display.height() - 20, IS_SEL_LEFT(highlight));
    if (IS_SEL_LEFT(muted_tracks)) {
      // display track 1 mute indication
      display.setCursor(13, display.height() - 8);
      display.println("M");
    }
  } else {
    boxed_text(track_dne, 10, display.height() - 20, IS_SEL_LEFT(highlight));
  }

  if (IS_SEL_CENTER(existing_tracks)) {
    boxed_text("T2", 40, display.height() - 20, IS_SEL_CENTER(highlight));
    if (IS_SEL_CENTER(muted_tracks)) {
      // display track 2 mute indication
      display.setCursor(43, display.height() - 8);
      display.println("M");
    }
  } else {
    boxed_text(track_dne, 40, display.height() - 20, IS_SEL_CENTER(highlight));
  }

  if (IS_SEL_RIGHT(existing_tracks)) {
    boxed_text("T3", 70, display.height() - 20, IS_SEL_RIGHT(highlight));
    if (IS_SEL_RIGHT(muted_tracks)) {
      // display track 3 mute indication
      display.setCursor(73, display.height() - 8);
      display.println("M");
    }
  } else {
    boxed_text(track_dne, 70, display.height() - 20, IS_SEL_RIGHT(highlight));
  }

  if (IS_SEL_FAR_RIGHT(existing_tracks)) {
    boxed_text("T4", 100, display.height() - 20, IS_SEL_FAR_RIGHT(highlight));
    if (IS_SEL_FAR_RIGHT(muted_tracks)) {
      // display track 4 mute indication
      display.setCursor(103, display.height() - 8);
      display.println("M");
    }
  } else {
    boxed_text(track_dne, 100, display.height() - 20, IS_SEL_FAR_RIGHT(highlight));
  }

  display.display();
}

void drawTrackOptions(uint8_t track_no, bool highlight) {
  if (recording) {
    if (recording_count > lead_in_beats) {
      centerText("Recording", 15);
      centerText("Any button to cancel", 25);
    } else {
      centerText("Counting in...", 20);
    }
  } else {
    draw_level("Track Options");
  }
  String track = "T" + String(track_no);
  boxed_text(track, display.width() / 2 - 10, display.height() - 25, false);
  boxed_text("Mute", 25, display.height() - 10, !highlight);
  int posn_x = 67;
  if (track.length() > 2) posn_x = 74;
  boxed_text("Delete", posn_x, display.height() - 10, highlight);
  // if recording, display "recording"
  // if playing, display "playing"
  if (playing) {
    centerText("Playing", 18);
  }
  display.display();
}

void drawStorageLimit(String text) {
  display.clearDisplay();
  display.setCursor(display.width() / 2 - 20, 0);
  display.println("Warning!");
  display.setCursor(2, 13);
  display.println(text);
//  display.println("Storage Limit Reached\nLess than 100MB free.\nDelete or export\ncontent.");
  display.drawRect(0, 11, 128, 50, WHITE);
  display.display();
}

void drawSelectOption(String sel_option, String text) {
    drawStatusBar();
    centerText(text, display.height() / 2 - 12);
    display.setCursor(2, display.height() / 2 + 8);
    display.write(LEFT_ARROW);

    int len = sel_option.length();
    int num_px = len*6;
    centerText(sel_option, display.height() / 2 + 8);
    display.drawLine(SCREEN_WIDTH/2 - num_px/2-4, display.height() / 2 + 16, SCREEN_WIDTH/2 - num_px/2+len*7 , display.height() / 2 + 16, WHITE);
    boxed_text("Ok", 60, display.height() / 2 + 24, true);
    display.setCursor(122, display.height() / 2 + 8);
    display.write(RIGHT_ARROW);
    display.display();
}

void areYouSure(bool yes) {
  drawStatusBar();
  centerText("Are you sure?", 32);
  boxed_text("Yes", 25, 50, yes);
  boxed_text("No", 90, 50, !yes);
  display.display();
}

void updateDisplay() {
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
          menu_id = MENU_SET_LOOP_LENGTH;
        }
        else if (selected_home_option == EXISTING_SESSION) {
          menu_id = MENU_SESSION_SEL;
          Serial.println("Selected session: " + String(selected_session));
        }
        else if (selected_home_option == SETTINGS) {
          menu_id = MENU_SETTINGS;
        }
        select_pressed_flag = false;
      }
      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new session with new track
        int newSessionNum = findNewSession();
        if (newSessionNum > 0) {
            createSession(newSessionNum, session_length);
            recording = true;
            pendingRecording = true;
            menu_id = MENU_TRACK_CONFIG;
            selected_track_option = TRACK_MUTE;
          } else {
            menu_id = ERROR_MENU;
          } 
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_SET_LOOP_LENGTH):
      drawSelectOption(String(session_length), "Loop Length (Beats)");
       if (select_pressed_flag) {
        int newSessionNum = findNewSession();
          if (newSessionNum > 0) {
            createSession(newSessionNum, session_length);
            menu_id = MENU_TRACK_CONFIG;
            selected_track_option = TRACK_MUTE;
          } else {
            menu_id = ERROR_MENU;
          }
          select_pressed_flag = false;
       }

       if(left_pressed_flag) {
        if(session_length > 1) {
          session_length--;
        } else session_length = 1;
        left_pressed_flag = false;
       }

        if(right_pressed_flag) {
        session_length++;
        right_pressed_flag = false;
       }

       if(back_pressed_flag) {
        menu_id = MENU_HOME;
        back_pressed_flag = false;
       }
      break;

    case (MENU_SET_DEF_BPM):
      drawSelectOption(String(statusBar.bpm), "Set Default BPM");
      if (select_pressed_flag) {
        menu_id = MENU_SETTINGS;
        select_pressed_flag = false;
      }
      if (left_pressed_flag) {
        BPM_encoderPos--;
        left_pressed_flag = false;
      }
      if (right_pressed_flag) {
        BPM_encoderPos++;
        right_pressed_flag = false;
      }
      if(back_pressed_flag) {
        menu_id = MENU_SETTINGS;
        back_pressed_flag = false;
      }
      break;

    case (MENU_SET_DEF_LOOP_LEN):
      drawSelectOption(String(session_length), "Default Loop Length");
      if (select_pressed_flag) {
        menu_id = MENU_SETTINGS;
        select_pressed_flag = false;
      }
      if (left_pressed_flag) {
        if(session_length > 1) {
          session_length--;
        }
        left_pressed_flag = false;
      }
      if (right_pressed_flag) {
        session_length++;
        right_pressed_flag = false;
      }
      if(back_pressed_flag) {
        menu_id = MENU_SETTINGS;
        back_pressed_flag = false;
      }
      break;

    case (ERROR_MENU):
      drawStorageLimit("Session Limit Reached!\nDelete or export content!");
      if(left_pressed_flag) {
        menu_id = MENU_HOME;
        left_pressed_flag = false;
       }
       if(right_pressed_flag) {
         menu_id = MENU_HOME;
        right_pressed_flag = false;
       }
       if(back_pressed_flag) {
        menu_id = MENU_HOME;
        back_pressed_flag = false;
       }
       if(select_pressed_flag) {
        menu_id = MENU_HOME;
        select_pressed_flag = false;
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
        enterSetting(selected_setting);
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new session with new track
        int newSessionNum = findNewSession();
        if (newSessionNum > 0) {
            createSession(newSessionNum, session_length);
            recording = true;
            pendingRecording = true;
            menu_id = MENU_TRACK_CONFIG;
            selected_track_option = TRACK_MUTE;
          } else {
            menu_id = ERROR_MENU;
          } 
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_SESSION_SEL):                               // SESSION SELECT
      drawSessionSelect(selected_session);
      // MENU NAVIGATION
      if (right_pressed_flag) {
        if (selected_session < num_sessions - 1) {
          selected_session++;
          Serial.println("Selected session: " + String(selected_session));
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
          Serial.println("Selected session: " + String(selected_session));
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
        selected_home_option = EXISTING_SESSION;
        back_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // enter session options
        if (num_sessions != 0) {
          menu_id = MENU_SESSION_CONFIG;
          selected_session_config_option = SESSION_OPEN;
        }
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new session with new track
        int newSessionNum = findNewSession();
        if (newSessionNum > 0) {
            createSession(newSessionNum, session_length);
            recording = true;
            pendingRecording = true;
            menu_id = MENU_TRACK_CONFIG;
            selected_track_option = TRACK_MUTE;
          } else {
            menu_id = ERROR_MENU;
          } 
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_SESSION_CONFIG):                               // SESSION CONFIG
      drawSessionConfig(sessions[selected_session].sessionNum, selected_session_config_option);
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
          menu_id = MENU_TRACK_SEL;
          statusBar.bpm = sessions[selected_session].sessionBPM;
          changeBPM();
          session_length = sessions[selected_session].sessionLength;
          selected_track = 0;
        }
        if (selected_session_config_option == SESSION_DELETE) {
          menu_id = ARE_YOU_SURE;
//          deleteSession(sessions[selected_session].sessionNum);
//          menu_id = MENU_SESSION_SEL;
        }
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new track in current session
        //enterSession(sessions[selected_session-1]);
        //newTrack(sessions[selected_session-1]);
        menu_id = MENU_TRACK_CONFIG;
        selected_track_option = TRACK_MUTE;
        play_rec_pressed_flag = false;
      }
      break;

    case (ARE_YOU_SURE): {
      areYouSure(are_you_sure);
      if (select_pressed_flag && !are_you_sure) {
        menu_id = MENU_SESSION_SEL;
        select_pressed_flag = false;
      }
      if (select_pressed_flag && are_you_sure) {
        menu_id = MENU_SESSION_SEL;
        deleteSession(sessions[selected_session].sessionNum);
        select_pressed_flag = false;
        are_you_sure = false;
      }
      
      if (back_pressed_flag) {
        menu_id = MENU_SESSION_SEL;
        back_pressed_flag = false;
      }
      if (left_pressed_flag) {
        are_you_sure = true;
        left_pressed_flag = false; 
      }
      if (right_pressed_flag) {
        are_you_sure = false;
        right_pressed_flag = false;
      }
      break;
    }

    case (MENU_TRACK_SEL):                               // TRACK SELECT
      drawTrackSelect(15, 15, selected_track);
      // MENU NAVIGATION
      if (right_pressed_flag) {
        if (selected_track < 3) {
          selected_track++;
        }
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        if (selected_track > 0) {
          selected_track--;
        }
        left_pressed_flag = false;
      }
      if (back_pressed_flag) {
        menu_id = MENU_SESSION_CONFIG;
        selected_session_config_option = SESSION_OPEN;
        back_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // either new track or existing track
        menu_id = MENU_TRACK_CONFIG;
        selected_track_option = TRACK_MUTE;
        select_pressed_flag = false;
      }

      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // play button pressed - play/pause session
        if (!playing) {
          playSession();
        } else {
          pauseSession();
        }
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_TRACK_CONFIG):                               // TRACK CONFIG
      drawTrackOptions(selected_track+1, selected_track_option);
      if (!recording) {             // not recording, so normal behavior
        // MENU NAVIGATION
        if (right_pressed_flag) {
          selected_track_option = TRACK_DELETE;
          right_pressed_flag = false;
        }
        if (left_pressed_flag) {
          selected_track_option = TRACK_MUTE;
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
          recording = true;
          pendingRecording = true;
          // rec/play button pressed - record or play/pause track
          /*
          if (track_exists(sessions[selected_session], selected_track)) {
            // there's a track there, so play/pause it
            if (!playing) {
              //playTrack();
            } else {
              //pauseTrack();
            }
          } else {
            // there's no track there, so start recording
            //startRecording();
            // turn on recording LED
            recording = true;
            pendingRecording = true;
          }*/
  
          play_rec_pressed_flag = false;
        }
        break;
    } else {                // recording, so don't accept input
      // cancel if any button is pressed
      // stop recording, turn off LED, delete newly created track
      if (right_pressed_flag) {
          recording = false;
          recording_count = 0;
          digitalWrite(recordingLED, LOW);
          right_pressed_flag = false;
        }
        if (left_pressed_flag) {
          recording = false;
          recording_count = 0;
          digitalWrite(recordingLED, LOW);
          left_pressed_flag = false;
        }
        if (back_pressed_flag) {
          recording = false;
          recording_count = 0;
          digitalWrite(recordingLED, LOW);
          back_pressed_flag = false;
        }
        if (select_pressed_flag) {
          recording = false;
          recording_count = 0;
          digitalWrite(recordingLED, LOW);
          select_pressed_flag = false;
        }
        if (play_rec_pressed_flag) {
          recording = false;
          recording_count = 0;
          digitalWrite(recordingLED, LOW);
          play_rec_pressed_flag = false;
        }
    }
  }
}


// File System Functions

//
// HELPER FUNCTIONS
//
void getTrackFilepath(int trackNumber) {
  // Use global variable selected_session
  // session/track.raw
  String fileString = "Sessions/";
  fileString += selected_session;
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

void deleteAll() {
  for (int i = -2; i < MAX_SESSIONS; i++) {
    deleteSession(i + 1);
  }
}

int findIndex(int value)
{
  int index = 0;
  while (index < num_sessions && sessions[index].sessionNum != value) ++index;
  Serial.println("Index: " + String(index));
  return index;
}

int compare(const void *a, const void *b) {
  return (((struct Session *)a)->sessionNum - ((struct Session *)b)->sessionNum);
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

  if (numSessions > 1) {
    qsort(sessionArray, numSessions, sizeof(struct Session), compare);
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
  fileString.toCharArray(filename, 50);
  File metaFile = SD.open(filename);
  Session newSession = {
    sessionName : readLine(metaFile),      // Name
    sessionNum : readLine(metaFile).toInt(),  // Number
    sessionBPM : readLine(metaFile).toInt(),  // BPM
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
void createSession(int sessionNum, uint16_t session_length) {
  // Creating file structure of new session
  String fileString = "Sessions/";
  fileString += sessionNum;
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
  writeMetadata(filename, sessionNum, session_length);

  // Update global data
  num_sessions = getSessionOverview(sessions);
  selected_session = findIndex(sessionNum);
  selected_track = 0;
  if (num_sessions >= 3) {
    if (selected_session > viewable_sessions[2] || viewable_sessions[2] >= num_sessions) {
      viewable_sessions[0] = selected_session - 2;
      viewable_sessions[1] = selected_session - 1;
      viewable_sessions[2] = selected_session;
    } else if (selected_session < viewable_sessions[0]) {
      viewable_sessions[0] = selected_session;
      viewable_sessions[1] = selected_session + 1;
      viewable_sessions[2] = selected_session + 2;
    }
  }
}

void writeMetadata(char* filename, int sessionNum, uint16_t session_length) {
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.print("Session_" + String(sessionNum) + ','); // Name
    dataFile.print(String(sessionNum) + ","); // Number
    dataFile.print("85,"); // BPM
    dataFile.print(String(session_length) + ","); // Length
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
void deleteSession(int sessionNum) {
  String fileString = "Sessions/";
  fileString += sessionNum;
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
  if (! fileExists(filename)) {
    Serial.printf("Directory %s is removed\n", filename);
  }
  else {
    Serial.printf("ERROR: The session %s was not removed\n", filename);
  }

  // Update global data
  num_sessions = getSessionOverview(sessions);
  if (selected_session > 0) {
    selected_session--;
  }
  if (num_sessions == 0) {
    selected_session = 0;
  }
  if (num_sessions >= 3) {
    if (selected_session > viewable_sessions[2] || viewable_sessions[2] >= num_sessions) {
      viewable_sessions[0] = selected_session - 2;
      viewable_sessions[1] = selected_session - 1;
      viewable_sessions[2] = selected_session;
    } else if (selected_session < viewable_sessions[0]) {
      viewable_sessions[0] = selected_session;
      viewable_sessions[1] = selected_session + 1;
      viewable_sessions[2] = selected_session + 2;
    }
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


// CORE FUNCTIONS
int findNewSession() {
  int newSessionNum = -1;
  if (num_sessions < MAX_SESSIONS) {
    if (num_sessions == 0 || (num_sessions > 0 && sessions[0].sessionNum > 1)) {
      // new session is before all existing sessions
      newSessionNum = 1;
    } else if (num_sessions == 1) {
      newSessionNum = sessions[0].sessionNum + 1;
    } else {
      for (int i = 0; i < num_sessions; i++) {
        if (i == num_sessions - 1 || sessions[i].sessionNum + 1 < sessions[i+1].sessionNum) {
          newSessionNum = sessions[i].sessionNum + 1;
          break;
        }
      }
    }
  }
  Serial.println("New Session: " + String(newSessionNum));
  return newSessionNum;
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
  Serial.println("startRecording");
  if (SD.exists(frec.name())) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    Serial.println("Removing " + String(frec.name()));
    SD.remove(frec.name());
  }

  frec = SD.open("DUMMY.RAW", FILE_WRITE);
  
  if (frec) {
    queue1.begin();
    mode = 1;
  }
}

void stopRecording() {
  Serial.println("stopRecording");
  queue1.end();
  if (mode == 1) {
    while (queue1.available() > 0) {
      Serial.println("Waiting");
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    Serial.println("Done waiting");
    frec.close();
  }
  mode = 0;
}


void continueRecording() {
  Serial.println("Continue Recording");
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
    Serial.print("SD write, us=");
    Serial.println(usec);
  }
}

void deleteRecording() {
  Serial.println("deleteRecording");
  stopRecording();
  SD.remove(frec.name());
}

void enterSetting(int selected_setting) {
  switch(selected_setting) {
    case(BEAT_SOUND): {
      break;
    }
    case(DEFAULT_BPM): {
      menu_id = MENU_SET_DEF_BPM;
      break;
    }
    case(DEFAULT_LOOP_LEN): {
      menu_id = MENU_SET_DEF_LOOP_LEN;
      break;
    }
    default: {
      Serial.println("ERROR");
    }
  }
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
  if (interrupt_time - last_interrupt_time > INTERRUPT_THRESHOLD) {
    flag = !flag;
    if (flag) {
      digitalWrite(pin, HIGH);
      //Serial.println(message + "->on");
    } else {
      digitalWrite(pin, LOW);
      //Serial.println(message + "->off");
    }
  }
  last_interrupt_time = interrupt_time;
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
  if (VOL_encoderPos > 201) VOL_encoderPos = MIN_VOL;
  if (VOL_encoderPos > MAX_VOL) VOL_encoderPos = MAX_VOL;

  if (BPM_encoderPos < MIN_BPM) BPM_encoderPos = MIN_BPM;
  if (BPM_encoderPos > MAX_BPM) BPM_encoderPos = MAX_BPM;
  statusBar.vol = VOL_encoderPos;
  changeVol();

  if (!recording) {
    statusBar.bpm = BPM_encoderPos;
    changeBPM();
  }
}

void sendBeat() {
  // blink LED
  if (beat_LED_enable) {
    digitalWrite(beatLED, LOW);
    beat_LED_enable = false;
  } else {
    digitalWrite(beatLED, HIGH);
    beat_LED_enable = true;
  }
  // conditionally send click
  if (click_enable && beat_LED_enable) {
    playMem1.play(AudioSampleMetronome);
  }
  // conditionally send pulse to haptic
  if(haptic_enable && beat_LED_enable) {
    XBee.write(1);
  } else {//if(haptic_enable) {
    XBee.write(2);
  }
  // increment counter if recording
  if(recording) {
    Serial.println("Recording Count : " + String(recording_count));
    if (recording_count > (lead_in_beats + session_length)) {
      // stop recording
      stopRecording();
      Serial.println("Recording Done");
      playSdRaw1.play("DUMMY.RAW");
      Serial.println("Playing recording");
      recording = false;
      recording_count = 0;
      digitalWrite(recordingLED, LOW);
    } else if (recording_count > lead_in_beats) {
      digitalWrite(recordingLED, HIGH);
      // start recording
      if(pendingRecording) {
        frec = SD.open("DUMMY.RAW", FILE_WRITE);
        startRecording();
        pendingRecording = false;
//        onlyRecording();
      } else {
        // continue recording
        continueRecording();
      }
    } else {
      if (beat_LED_enable) {
        digitalWrite(recordingLED, HIGH);
      } else {
        digitalWrite(recordingLED, LOW);
      }
    }
    if (beat_LED_enable) {
      recording_count++;
    }
  }
}

void onlyRecording() {
//  cli();
  Serial.println("Only recording");
  int startTimer = millis();
  int endTimer = startTimer;
  while ((endTimer - startTimer) < 4000) {
    continueRecording();
    endTimer = millis();
  }
  Serial.println("Done continuing");
  stopRecording();
  playSdRaw1.play("DUMMY.RAW");
  Serial.println("Playing recording");
//  sei();
}

void changeBPM() {
  beatTimer.update((0.5 * 60 * pow(10, 6)) / statusBar.bpm);
}
void changeVol() {
  float newVol = float(statusBar.vol) / MAX_VOL;
  mixer3.gain(0, newVol * 2 / 3); // Track Mixer
  mixer3.gain(1, newVol); // Metronome Mixer
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
