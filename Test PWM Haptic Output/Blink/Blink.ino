/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
#define led 13
#define led2 9
#define AnalogIN0 0

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT); 
  pinMode(led2,OUTPUT);
  pinMode(AnalogIN0, INPUT);  
  Serial.begin(9600);   
}

// the loop routine runs over and over again forever:
void loop() {
  int val = analogRead(AnalogIN0);
  Serial.print("Val: ");
  Serial.print(val);
  Serial.print("\n");
  int delay_length = map(val,0,1023,0,500);
  Serial.print("Delay: ");
  Serial.print(delay_length);
  Serial.print("\n");
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  analogWrite(led2, 0);
  delay(delay_length);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  analogWrite(led2, 255);
  delay(delay_length);               // wait for a second
}
