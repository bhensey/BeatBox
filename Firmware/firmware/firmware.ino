#include <Time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
  bool mt;
  bool mute;
  int duration;
  char filename[];
  //file* file_ptr;
};

struct session {
  bool mt;
  int iD;
  int duration;
  track tracklist[];
  date date_created;
  date last_modified;
};

struct globalConfig {
  int bpm;
  int volume;
  bool play_rec;  // indicates what pressing the play/rec button will do (0=play, 1=rec)
};

enum SessionState {
  Valid,
  Deleted,
  End
};




// Global Variables
const int leftButton = 16;      // the number of the left button pin
const int rightButton = 14;     // the number of the right button pin
const int selectButton = 15;    // the number of the select button pin
const int backButton = 23;      // the number of the back button pin
const int clickButton = 21;     // the number of the click button pin
const int hapticButton = 20;    // the number of the haptic button pin
const int playrecButton = 22;   // the number of the play/rec button pin

const int beatLED = 13;         // the number of the beat indicator LED
const int clickLED = 1;         // the number of the click enable LED
const int hapticLED = 2;        // the number of the haptic enable LED
const int recordingLED = 0;     // the number of the recording indicator LED

static unsigned long last_interrupt_time = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

IntervalTimer beatTimer;

struct globalConfig statusBar = {120, 50, false};

int menu_id = 0; // current state of the menu

int num_sessions = 0; // number of existing sessions

SessionState sessions[100];

//session sessions[num_sessions]; // list of existing sessions

int selected_home_option = 0;
int selected_setting = 0;
int selected_session = 0;
int viewable_sessions[] = {1,2,3};
int selected_session_config_option = 0;
int selected_track = 0;
int selected_track_option = 0;

int current_session_id;
int current_track_id;

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

// SETUP AND LOOP FUNCTIONS
void setup() {
  Serial.begin(9600);
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

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();
}

void loop() {
  drawStatusBar();
  switch (menu_id) {
    case (MENU_HOME):
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
          selected_session++;
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

    case (MENU_SETTINGS):
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

    case (MENU_SESSION_SEL):
      drawSessionSelect(selected_session);
      // MENU NAVIGATION
      if (right_pressed_flag) {
        if (selected_session < num_sessions) {
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
        if (selected_session > 1) {
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

    case (MENU_SESSION_CONFIG):
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
  
  String VOL = "Vol:" + String(statusBar.volume);
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
    String sess_l = "S" + String(viewable_sessions[0]);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
  } else if (num_sessions == 2) {
    String sess_l = "S" + String(viewable_sessions[0]);
    String sess_c = "S" + String(viewable_sessions[1]);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
    boxed_text(sess_c, 60, display.height() / 2 - 4, IS_SEL_CENTER(highlight));
  } else {
    String sess_l = "S" + String(viewable_sessions[0]);
    String sess_c = "S" + String(viewable_sessions[1]);
    String sess_r = "S" + String(viewable_sessions[2]);
    boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
    boxed_text(sess_c, 60, display.height() / 2 - 4, IS_SEL_CENTER(highlight));
    boxed_text(sess_r, 100, display.height() / 2 - 4, IS_SEL_RIGHT(highlight));

  }
  if(viewable_sessions[0] > 1) { // left arrow
    display.setCursor(2,display.height() / 2 - 4);
    display.write(LEFT_ARROW);
    
  }
  if(viewable_sessions[2] < num_sessions) { // right arrow
    display.setCursor(122,display.height() / 2 - 4);
    display.write(RIGHT_ARROW);
  }

  centerText("ssssssssss", 40);
    
  display.display();
}



void drawSessionConfig(int session_number, bool highlight) {
  draw_level("Session Config");
  String sess = "S" + String(session_number); 
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
void newSession() {
  num_sessions++;

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
   Serial.println(message);
  }
  last_interrupt_time = interrupt_time;
}

void debounce_toggle(bool& flag, int pin, String message) {
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > INTERRUPT_THRESHOLD) {
    flag = !flag;
    if (flag) {
      digitalWrite(pin,HIGH);
      Serial.println(message + "->on");
    } else {
      digitalWrite(pin,LOW);
      Serial.println(message + "->off");
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
  // conditionally send pulse to haptic
}
void changeBPM() {
  // update statusBar.bpm with new value
//  beatTimer.update((60*pow(10,6))/statusBar.bpm);
}
void changeVol() {
  // update statusBar.volume with new value
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
