#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RIGHT_BUTTON    14
#define SELECT_BUTTON   15
#define LEFT_BUTTON     16
#define BACK_BUTTON     23
#define PLAY_REC        22 

#define RIGHT_ARROW   16
#define LEFT_ARROW    17


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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void setup() {
  // put your setup code here, to run once:
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(SELECT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON, INPUT_PULLUP);
  pinMode(PLAY_REC, INPUT_PULLDOWN);

  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  display.display();
}

//bool selected_item
//
//enum Selected {
//  LEFT,
//  CENTER,
//  RIGHT
//};

void boxed_text(String text, int x_pos, int y_pos, bool selected) {
  if(!selected) display.setTextColor(WHITE, BLACK);
  else display.setTextColor(BLACK,WHITE);
  int len = text.length();
  display.setCursor(x_pos, y_pos);
  display.println(text);
  display.drawRect(x_pos-2, y_pos-2, len*7, 12, WHITE);
  display.setTextColor(WHITE, BLACK);
}

void header(uint16_t vol, uint16_t bpm) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  //                x,y
  display.setCursor(2,2);
  display.cp437(true);

  String BPM = "BPM:" + String(bpm);
  display.println(BPM);

  String VOL = "Vol:" + String(vol);
  display.setCursor(48,2);
  display.println(VOL);
  
}

void finish_header(bool Rec_or_Play) {
  if(Rec_or_Play) {
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

void draw_level(String Name) {
  display.setCursor(0,11);
  display.println(Name);
}

void draw_Home(uint16_t vol, uint16_t bpm, uint8_t highlight) {
  header(vol, bpm);
  boxed_text("New", 10, display.height() / 2 + 10, IS_SEL_LEFT(highlight));
  boxed_text("Files", 45, display.height() / 2 + 10, IS_SEL_CENTER(highlight));
  boxed_text("Prefs", 90, display.height() / 2 + 10, IS_SEL_RIGHT(highlight));
  display.setCursor(100,2);
  display.println("REC");
  display.setCursor(122,2);
  display.write(RIGHT_ARROW);
  display.drawLine(0, 9, 128, 9, WHITE);
  draw_level("Home");
  finish_header(true);
  display.display();
}

void draw_Session_Config(uint16_t vol, uint16_t bpm, int session_number, bool sel_open) {
  header(vol, bpm);
  
  String sess = "S" + String(session_number); 
  boxed_text(sess, display.width() / 2 - 10, display.height() / 2 - 4, false);
  boxed_text("Open", 25, display.height() / 2 + 10, sel_open);
  int posn_x = 67; 
  if(sess.length() > 2) posn_x = 74;
  boxed_text("Delete", posn_x, display.height() / 2 + 10, !sel_open);
  finish_header(false);
  display.display();
}

void draw_Session_Select(uint16_t vol, uint16_t bpm, uint8_t left_session, uint8_t center_session, uint8_t right_session, uint8_t highlight, bool left_arrow, bool right_arrow) {
  header(vol, bpm);
  draw_level("Session Select");
  String sess_l = "S" + String(left_session); 
  String sess_c = "S" + String(center_session); 
  String sess_r = "S" + String(right_session); 
  boxed_text(sess_l, 20, display.height() / 2 - 4, IS_SEL_LEFT(highlight));
  boxed_text(sess_c, 60, display.height() / 2 - 4, IS_SEL_CENTER(highlight));
  boxed_text(sess_r, 100, display.height() / 2 - 4, IS_SEL_RIGHT(highlight));
  if(left_arrow) {
    display.setCursor(2,display.height() / 2 - 4);
    display.write(LEFT_ARROW);
  }
  if(right_arrow) {
    display.setCursor(122,display.height() / 2 - 4);
    display.write(RIGHT_ARROW);
  }
  
  
  finish_header(false);
  display.display();
}

void draw_Preferences(uint16_t vol, uint16_t bpm, uint8_t highlight) {
      header(vol, bpm);
      draw_level("Preferences");
      boxed_text("Sound", 12, display.height() / 2 + 10, IS_SEL_LEFT(highlight));
      boxed_text("Reset", 51, display.height() / 2 + 10, IS_SEL_CENTER(highlight));
      boxed_text("Loop", 91, display.height() / 2 + 10, IS_SEL_RIGHT(highlight));
      finish_header(false);
      display.display();
}

void draw_Track_Select(uint16_t vol, uint16_t bpm, uint8_t existing_tracks, uint8_t highlight) {
      header(vol, bpm);
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
      
      finish_header(false);
      display.display();
}

void draw_Track_Options(uint16_t vol, uint16_t bpm, uint8_t track_no, uint8_t highlight) {
      header(vol, bpm);
      draw_level("Track Options");
      String track = "T" + String(track_no);
      boxed_text(track, display.width() / 2 - 10, display.height() - 25, false);
      boxed_text("Mute", 25, display.height() - 10, IS_SEL_LEFT(highlight));
      int posn_x = 67; 
      if(track.length() > 2) posn_x = 74;
      boxed_text("Delete", posn_x, display.height() - 10, IS_SEL_RIGHT(highlight));
      finish_header(false);
      display.display();
}

void draw_Storage_Limit() {
      display.clearDisplay();
      display.setCursor(display.width()/2-20, 0);
      display.println("Warning!");
      display.setCursor(2,13);
      display.println("Storage Limit Reached\nLess than 100MB free.\nDelete or export\ncontent.");
      display.drawRect(0, 11, 128, 50, WHITE);
      display.display();
}

void loop() {
//  draw_Home(80,120, LEFT);
//    draw_Session_Config(80,120, 1, true);
//    draw_Session_Select(80,120,0,1,2,CENTER, false, true);
//    draw_Preferences(100,40, RIGHT);
    draw_Track_Select(100, 40, (LEFT | FAR_RIGHT), (LEFT|FAR_RIGHT));
//    draw_Track_Options(12, 252, 4, RIGHT);
//    draw_Storage_Limit();
}
