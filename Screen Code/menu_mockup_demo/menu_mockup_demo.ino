/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x32 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include "SoftwareSerial.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define RIGHT_ARROW   16
#define LEFT_ARROW    17
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

  SoftwareSerial XBee(33, 34);
  uint8_t count = 255;
   char vol = 100;
  char bpm = 40;
void setup() {
  pinMode(13,OUTPUT);
  pinMode(35,OUTPUT);
  XBee.begin(115200);
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  
 
  
  // Draw a single pixel in white
  
//  display.drawPixel(10, 10, WHITE);
//
//  // Show the display buffer on the screen. You MUST call display() after
//  // drawing commands to make them visible on screen!
  display.display();
//  delay(2000);
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...

//  testdrawline();      // Draw many lines
//
//  testdrawrect();      // Draw rectangles (outlines)
//
//  testfillrect();      // Draw rectangles (filled)
//
//  testdrawcircle();    // Draw circles (outlines)
//
//  testfillcircle();    // Draw circles (filled)
//
//  testdrawroundrect(); // Draw rounded rectangles (outlines)
//
//  testfillroundrect(); // Draw rounded rectangles (filled)
//
//  testdrawtriangle();  // Draw triangles (outlines)
//
//  testfilltriangle();  // Draw triangles (filled)
//
//  testdrawchar();      // Draw characters of the default font
//
//  testdrawstyles();    // Draw 'stylized' characters
//
//  testscrolltext();    // Draw scrolling text

//  testdrawbitmap();    // Draw a small bitmap image

//  // Invert and restore display, pausing in-between
//  display.invertDisplay(true);
//  delay(1000);
//  display.invertDisplay(false);
//  delay(1000);
//
//  testanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT); // Animate bitmaps
}

void loop() {
  digitalWrite(13, HIGH);   // set the LED on
  delay(100);                  // wait for a second
  digitalWrite(13, LOW);    // set the LED off
  delay(100);
  basic_menu(vol, bpm);
  bpm++;
  vol--;
}

void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
    delay(1);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE);
    display.display();
    delay(1);
  }

  delay(2000); // Pause for 2 seconds
}

void testdrawrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(2000);
}

void testfillrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=3) {
    // The INVERSE color is used so rectangles alternate white/black
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, INVERSE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(2000);
}

void testdrawcircle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfillcircle(void) {
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=3) {
    // The INVERSE color is used so circles alternate white/black
    display.fillCircle(display.width() / 2, display.height() / 2, i, INVERSE);
    display.display(); // Update screen with each newly-drawn circle
    delay(1);
  }

  delay(2000);
}

//char idx = 0;
short cnt = 0;
short i = 0;

long demo_cnt = 0;
short nums[3] = {1, 2, 3};

void draw_level(short idx) {
  String levels[8] = { "Home", "Session Select", "Session Config", "New Session", "Recording", "Track View", "Track Options", "Prefernces" };
  display.setCursor(0,11);
  display.println(levels[idx]);
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

void Session_Config(int session_number, bool sel_open) {
  String sess = "S" + String(session_number); 
  boxed_text(sess, display.width() / 2 - 10, display.height() / 2 - 4, false);
  boxed_text("Open", 25, display.height() / 2 + 10, sel_open);
  int posn_x = 67; 
  if(sess.length() > 2) posn_x = 74;
  boxed_text("Delete", posn_x, display.height() / 2 + 10, !sel_open);
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

  
  draw_level(i);
  i++;
  if(i > 7) i =0;

  
  if(demo_cnt < 10) {
    display.setCursor(100,2);
  display.println("REC");
  }
  else {
    display.setCursor(97,2);
  display.println("Play");
  }

  display.drawLine(0, 9, 128, 9, WHITE);
  display.setCursor(122,2);
  display.write(RIGHT_ARROW);
  
  cnt++;
  if (cnt > 3) {
    for(short i=0; i < 3; i++) {
      if(nums[i] < 100) {
         nums[i]++;
      }
      else nums[i] = 0;
    }
      
      
  }
  char idx = cnt % 3;
  
  bool sel[3] = {false, false, false};
  sel[idx] = true;
  
  String text0 = "S" + String(nums[0]);
  String text1 = "S" + String(nums[1]);
  String text2 = "S" + String(nums[2]);
    //session select
    if(demo_cnt < 25) {
      boxed_text(text0, 20, display.height() / 2 - 4,sel[0]);
      boxed_text(text1, 60, display.height() / 2 - 4, sel[1]);
      boxed_text(text2, 100, display.height() / 2 - 4, sel[2]);
      display.setCursor(2,display.height() / 2 - 4);
      display.write(LEFT_ARROW);
      display.setCursor(122,display.height() / 2 - 4);
      display.write(RIGHT_ARROW);
    }
    //home
    else if(demo_cnt < 50) {
      boxed_text("New", 10, display.height() / 2 + 10, sel[2]);
      boxed_text("Files", 45, display.height() / 2 + 10, sel[1]);
      boxed_text("Prefs", 90, display.height() / 2 + 10, sel[0]);
    }
    else if(demo_cnt < 75) {
      //preferences
      boxed_text("Sound", 12, display.height() / 2 + 10, sel[2]);
      boxed_text("Reset", 51, display.height() / 2 + 10, sel[1]);
      boxed_text("Loop", 91, display.height() / 2 + 10, sel[0]);
    }

    else if(demo_cnt < 100) {
      //Session config
      Session_Config(nums[0], sel[0]);
    }
    else if(demo_cnt < 125) {
      //track select
      boxed_text("T1", 10, display.height() - 20, sel[0]);
      boxed_text(" + ", 40, display.height() - 20, sel[1]);
      boxed_text(" + ", 70, display.height() - 20, sel[2]);
      boxed_text(" + ", 100, display.height() - 20, false);
    }
    else if(demo_cnt < 150) {
      //track options
      String track = "T2";//
      boxed_text(track, display.width() / 2 - 10, display.height() - 25, false);
      boxed_text("Mute", 25, display.height() - 10, sel[0]);
      int posn_x = 67; 
      if(track.length() > 2) posn_x = 74;
      boxed_text("Delete", posn_x, display.height() - 10, !sel[0]);
    }
    else if(demo_cnt < 175) {
      //Storage Limit
      display.clearDisplay();
      display.setCursor(display.width()/2-20, 0);
      display.println("Warning!");
      display.setCursor(2,13);
      display.println("Storage Limit Reached\nLess than 100MB free.\nDelete or export\ncontent.");
      display.drawRect(0, 11, 128, 50, WHITE);
    }
    demo_cnt++;
    demo_cnt = demo_cnt % 175;
//    

    

    
    
//  display.setCursor(4, display.height() / 2 - 4);
//  display.println("Sess 1");
//
//  display.setCursor(48, display.height() / 2 - 4);
//  display.println("Sess 2");
//
//  display.setCursor(92, display.height() / 2 - 4);
//  display.println("Sess 3");
  
//  display.setCursor(100,2);
//  display.println("REC");
//  display.drawLine(0, 9, 128, 9, WHITE);
//  display.setCursor(120,2);
//  display.write(RIGHT_ARROW);
  display.display();
}

void testdrawroundrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfillroundrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-2; i+=2) {
    // The INVERSE color is used so round-rects alternate white/black
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, INVERSE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testdrawtriangle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(
      display.width()/2  , display.height()/2-i,
      display.width()/2-i, display.height()/2+i,
      display.width()/2+i, display.height()/2+i, WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfilltriangle(void) {
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=5) {
    // The INVERSE color is used so triangles alternate white/black
    display.fillTriangle(
      display.width()/2  , display.height()/2-i,
      display.width()/2-i, display.height()/2+i,
      display.width()/2+i, display.height()/2+i, INVERSE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(2, 2);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
//  display.write(14);
  display.println(F("Session 1"));
  display.drawRect(0,0,85 , 11, WHITE);

  display.setCursor(2, 14);
  display.println(F("Session 2"));
  display.drawRect(13,12,85 , 23, WHITE);

    display.setCursor(2, 26);
    display.println(F("Session 3"));
  display.drawRect(22,24,85 , 35, WHITE);
//  display.drawLine(0, 9, 30, 9, WHITE);
//  for(int i=10; i < 20; i++) {
//    display.write(i);
//    delay(1000);
//    display.clearDisplay();
//    display.println(F("Hello, world!"));
//  }
  

//  for(int16_t i=0; i<256; i++) {
//    if(i == '\n') display.write(' ');
//    else          display.write(i);
//  }

  display.display();
  delay(2000);
}

void testdrawstyles(void) {

  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));

  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.println(3.141592);

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  delay(2000);
}

void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println(F("scroll"));
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
}

void testdrawbitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(1000);
}

#define XPOS   0 // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

void testanimate(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  int8_t f, icons[NUMFLAKES][3];

  // Initialize 'snowflake' positions
  for(f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
    icons[f][YPOS]   = -LOGO_HEIGHT;
    icons[f][DELTAY] = random(1, 6);
    Serial.print(F("x: "));
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(F(" y: "));
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(F(" dy: "));
    Serial.println(icons[f][DELTAY], DEC);
  }

  for(;;) { // Loop forever...
    display.clearDisplay(); // Clear the display buffer

    // Draw each snowflake:
    for(f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
//      analogWrite(35,f*10);
XBee.write(count);
  count--;
    }

    for(int i=0; i < 255; i++)
    {
      analogWrite(35,i);
      delay(50);
    }

    display.display(); // Show the display buffer on the screen
    delay(200);        // Pause for 1/10 second

    // Then update coordinates of each flake...
    for(f=0; f< NUMFLAKES; f++) {
      icons[f][YPOS] += icons[f][DELTAY];
      // If snowflake is off the bottom of the screen...
      if (icons[f][YPOS] >= display.height()) {
        // Reinitialize to a random position, just off the top
        icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
        icons[f][YPOS]   = -LOGO_HEIGHT;
        icons[f][DELTAY] = random(1, 6);
      }
    }
  }
}
