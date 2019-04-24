#include <Time.h>

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
const int leftButton = 2;     // the number of the left button pin
const int rightButton = 2;    // the number of the right button pin
const int selectButton = 2;   // the number of the select button pin
const int backButton = 2;     // the number of the back button pin
const int clickButton = 2;    // the number of the click button pin
const int hapticButton = 2;   // the number of the haptic button pin
const int playrecButton = 2;  // the number of the play/rec button pin

const int clickLED = 13;      // the number of the click LED

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

bool click_enable;
bool haptic_enable;

bool play_rec_pressed_flag;
bool right_pressed_flag;
bool left_pressed_flag;
bool select_pressed_flag;
bool back_pressed_flag;

// SETUP AND LOOP FUNCTIONS
void setup() {
  // initialize pins as inputs:
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);
  pinMode(selectButton, INPUT);
  pinMode(backButton, INPUT);
  pinMode(clickButton, INPUT);
  pinMode(hapticButton, INPUT);
  pinMode(playrecButton, INPUT);
  pinMode(clickLED,OUTPUT);

  // initialize beat timer
  beatTimer.begin(sendBeat, (60*pow(10,6))/options.bpm);

  // initialize ISRs
  attachInterrupt(digitalPinToInterrupt(leftButton), leftButton_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rightButton), rightButton_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(selectButton), selectButton_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(backButton), backButton_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(clickButton), clickButton_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(hapticButton), hapticButton_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(playrecButton), playrecButton_ISR, CHANGE);
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
