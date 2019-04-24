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

void boxed_text(String text, int x_pos, int y_pos, bool selected) {
  if(!selected) display.setTextColor(WHITE, BLACK);
  else display.setTextColor(BLACK,WHITE);
  int len = text.length();
  display.setCursor(x_pos, y_pos);
  display.println(text);
  display.drawRect(x_pos-2, y_pos-2, len*7, 12, WHITE);
//  display.display();
  display.setTextColor(WHITE, BLACK);
}

void basic_menu(short vol, short bpm) {
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
  
//  display.display();
}

void draw_level(String Name) {
  display.setCursor(0,11);
  display.println(Name);
}

void draw_home(bool highlight[3]) {
//  bool left = digitalRead(LEFT_BUTTON);
//  bool ok = digitalRead(SELECT_BUTTON);
//  bool right = digitalRead(RIGHT_BUTTON);
  
  boxed_text("New", 10, display.height() / 2 + 10, false);
  boxed_text("Files", 45, display.height() / 2 + 10, false);
  boxed_text("Prefs", 90, display.height() / 2 + 10, false);
  display.setCursor(100,2);
  display.println("REC");
  display.setCursor(122,2);
  display.write(RIGHT_ARROW);
  display.drawLine(0, 9, 128, 9, WHITE);
  draw_level("Home");

  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:
  bool back = digitalRead(PLAY_REC);
  if(back) digitalWrite(13,HIGH);
//  bool sel = digitalRead(SELECT_BUTTON);
//  if(sel) digitalWrite(13,HIGH);
//  bool right = digitalRead(RIGHT_BUTTON);
//  if(right) digitalWrite(13,HIGH);
  digitalWrite(13,LOW);
  basic_menu(80, 120);
  bool sel_arr[3] = {true, false, false};
  if(digitalRead(RIGHT_BUTTON)) {
    sel_arr[0] = false;
    sel_arr[1] = true;
  }
  draw_home(sel_arr);
}
