#include <Time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define MENU_HOME           1
#define MENU_SESSION_SEL    2
#define MENU_SESSION        3
#define MENU_SESSION_CONFIG 4
#define MENU_NEW_SESSION    5
#define MENU_TRACK_SEL      6
#define MENU_TRACK_CONFIG   7
#define MENU_SETTINGS       8

#define NEW_SESSION       1
#define EXISTING_SESSION  2
#define SETTINGS          3

#define SESSION_OPEN      1
#define SESSION_DELETE    2

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define LEFT                  8
#define CENTER                4
#define RIGHT                 2
#define FAR_RIGHT             1
#define IS_SEL_LEFT(n)       (!!(n & LEFT))
#define IS_SEL_CENTER(n)     (!!(n & CENTER))
#define IS_SEL_RIGHT(n)      (!!(n & RIGHT))
#define IS_SEL_FAR_RIGHT(n)  (!!(n & FAR_RIGHT))

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

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
  int ID;
  bool empty;
  bool mute;
  int length;
  char filename[];
  //file* file_ptr;
};

struct session {
  int iD;
  int length;
  track tracklist[];
  date date_created;
  date last_modified;
};

struct globalConfig {
  int bpm;
  int volume;
  bool play_rec;  // indicates what pressing the play/rec button will do
};

// Global Variables
const int leftButton = 16;    // the number of the left button pin
const int rightButton = 14;   // the number of the right button pin
const int selectButton = 15;  // the number of the select button pin
const int backButton = 23;    // the number of the back button pin
const int clickButton = 2;    // the number of the click button pin
const int hapticButton = 2;   // the number of the haptic button pin
const int playrecButton = 22; // the number of the play/rec button pin

const int clickLED = 13;      // the number of the click LED

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

IntervalTimer beatTimer;

struct globalConfig options = {85, 50, true};

int menu_id; // current state of the menu

//int num_sessions; // number of existing sessions

//session sessions[num_sessions]; // list of existing sessions

int selected_home_option;
int selected_setting;
int selected_session;
int selected_session_config_option;
int selected_track;
int selected_track_option;

int current_session_id;
int current_track_id;

bool session_playing;
bool track_playing;

bool click_enable = false;
bool haptic_enable = false;

bool play_rec_pressed_flag;
bool right_pressed_flag;
bool left_pressed_flag;
bool select_pressed_flag;
bool back_pressed_flag;

// SETUP AND LOOP FUNCTIONS
void setup() {
  Serial.begin(9600);
  // initialize pins as inputs:
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(backButton, INPUT_PULLUP);
  pinMode(clickButton, INPUT_PULLUP);
  pinMode(hapticButton, INPUT_PULLUP);
  pinMode(playrecButton, INPUT_PULLUP);
  pinMode(clickLED,OUTPUT);

  // initialize beat timer
  beatTimer.begin(sendBeat, (60*pow(10,6))/options.bpm);

  // initialize ISRs
  attachInterrupt(digitalPinToInterrupt(leftButton), leftButton_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rightButton), rightButton_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(selectButton), selectButton_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(backButton), backButton_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(clickButton), clickButton_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(hapticButton), hapticButton_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(playrecButton), playrecButton_ISR, RISING);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Serial.println("Program Started");

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  display.display();
}

void loop() {
  drawScreen();
  switch (menu_id) {
    case (MENU_HOME):
      // MENU NAVIGATION
      if (right_pressed_flag) {
        selected_home_option += 1;
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        selected_home_option -= 1;
        left_pressed_flag = false;
      }
      if (select_pressed_flag) {
        // either create new session, show existing sessions, or go to settings
        if (selected_home_option == NEW_SESSION) {
          newSession();
        }
        else if (selected_home_option == EXISTING_SESSION) {
          menu_id = MENU_SESSION_SEL;
        }
        else if (selected_home_option == SETTINGS) {
          menu_id = MENU_SETTINGS;
          select_pressed_flag = false;
        }
      }
      // OTHER FUNCTIONALITY
      if (play_rec_pressed_flag) {
        // quick record pressed - new session with new track
        newQuickSession();
        play_rec_pressed_flag = false;
      }
      break;

    case (MENU_SETTINGS):
      // MENU NAVIGATION
      if (right_pressed_flag) {
        selected_setting += 1;
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        selected_setting -= 1;
        left_pressed_flag = false;
      }
      if (back_pressed_flag) {
        menu_id = MENU_HOME;
        selected_home_option = 2;
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

    case (MENU_SESSION_SEL):
      // MENU NAVIGATION
      if (right_pressed_flag) {
        selected_session += 1;
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        selected_session -= 1;
        left_pressed_flag = false;
      }
      if (back_pressed_flag) {
        menu_id = MENU_HOME;
        selected_home_option = 2;
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

    case (MENU_SESSION_CONFIG):
      // MENU NAVIGATION
      if (right_pressed_flag) {
        selected_session_config_option += 1;
        right_pressed_flag = false;
      }
      if (left_pressed_flag) {
        selected_session_config_option -= 1;
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

    case (MENU_TRACK_SEL):
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

    case (MENU_TRACK_CONFIG):
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

// CORE FUNCTIONS
void drawScreen() {
  
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
void sendBeat() {
  // blink LED
  digitalWrite(clickLED,HIGH);
  // conditionally send click
  // conditionally send pulse to haptic
}
void changeBPM() {
  // update options.bpm with new value
  beatTimer.update((60*pow(10,6))/options.bpm);
}
void changeVol() {
  // update options.volume with new value
}
void leftButton_ISR() {
  Serial.println("Left button pressed");
  left_pressed_flag = true;
}
void rightButton_ISR() {
  Serial.println("Right button pressed");
  right_pressed_flag = true;
}
void selectButton_ISR() {
  Serial.println("Select button pressed");
  select_pressed_flag = true;
}
void backButton_ISR() {
  Serial.println("Back button pressed");
  back_pressed_flag = true;
}
void clickButton_ISR() {
  Serial.println("Click button pressed");
  click_enable = !click_enable;
}
void hapticButton_ISR() {
  Serial.println("Haptic button pressed");
  haptic_enable = !haptic_enable;
}
void playrecButton_ISR() {
  Serial.println("Play/rec button pressed");
  play_rec_pressed_flag = true;
}
