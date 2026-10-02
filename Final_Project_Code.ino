/*
Created 11/28/2024 by Samuel Fong
Last Maintained 11/29/2024 by Samuel Fong
**Important** Analog values can be extremely finnicky, TODO for later
              otherwise, program works fine (mainly pot values and button analog reader).

*/

// include the library code:
#include <LiquidCrystal.h>
#include <string.h>
#include <Wire.h>
#include <RTClib.h>


int const potPin = A0;        // analog pin used to connect the potentiometer
int potVal;                   // variable to read the value from the analog pin
int const rtcPin1 = A4;       // Analog pin for clock
int const rtcPin2 = A5;       // Analog pin for clock
int const lcdContrast = A3;   // Analog pin to control the contrast of the lcd
int const buttons = A3;       // Analog pin to determine which button is being pressed
int const piezo = A2;         // Analog pin used as digital pin for the piezo/buzzer
bool inSetup = false;         // Check if program should be in setup mode based on which button is pushed
int alarmNum = -1;            // Tells program which alarm to change
int time;

// Initialize alarm values
int alarmValues[6] = {32, 48, 68, 55, 69, 60};

// Initialize alarm names
char* alarmNames[6] = {"Breakfast", "Lunch", "Dinner", "Call Grandson", "Feed Cats", "Take Medicine"};

// Initialize pins
int const LEDs[6] = {13, 10, 9, 8, 7, 6};

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// initialize rtc module
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  // set up the number of columns and rows on the LCD
  lcd.begin(16, 2);

  // Initialize analog pins
  pinMode(piezo, OUTPUT);
  digitalWrite(piezo, LOW);

  // Initialize digital pins
  for (auto pin : LEDs) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  // line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 0);

  // Check if rtc is running properly
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    // Set the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(2024, 12, 6, 18, 30));
    // Set the RTC to the manual date & time defined above
    //rtc.adjust(now);
  } else {
    //rtc.adjust(DateTime(2024, 12, 6, 18, 22));
    Serial.println("RTC is running with correct time.");
  }
  
}


void loop() {
  // print data for debugging and analytical purposes
  Serial.print(analogRead(buttons));
  Serial.print("\n");

  // Check if program should switch to setup mode
  if (inSetup) {
    delay(1000);
    int buttonVal = analogRead(buttons);

    // Adjust alarm time based off of potentiometer input & quit if any button is pressed
    while (buttonVal > 700) {
      potVal = analogRead(potPin);
      time = map(potVal, 0, 1023, 0, 96);
      Serial.print(buttonVal);
      Serial.print(" ");
      Serial.print(potVal);
      Serial.print("\n");
      lcd.clear();
      lcd.print(alarmNames[alarmNum-1]);
      lcd.setCursor(0, 1);
      lcd.print(String((int)time/4) + ":" + String((time%4)*15));
      lcd.setCursor(0, 0);
      delay(1500);
      buttonVal = analogRead(buttons);
    }
    alarmValues[alarmNum-1] = time;
    inSetup = false;
    alarmNum = -1;
    delay(500);
      
  } else {
    
    // Determine if a button is being pressed and which button is being pressed
    if (analogRead(buttons) < 700) {
      inSetup = true;
      int buttonVoltage = analogRead(buttons);
      if (buttonVoltage > 400 && buttonVoltage <= 700) {         // 6th button
        alarmNum = 6;

      } else if (buttonVoltage > 200 && buttonVoltage <= 400) {   // 5th button
        alarmNum = 5;

      } else if (buttonVoltage > 130 && buttonVoltage <= 200) {   // 4th button
        alarmNum = 4;

      } else if (buttonVoltage > 50 && buttonVoltage <= 130) {   // 3rd button
        alarmNum = 3;

      } else if (buttonVoltage > 25 && buttonVoltage <= 50) {   // 2nd button
        alarmNum = 2;


      } else if (buttonVoltage > 10 && buttonVoltage <= 25) {   // 1st button
        alarmNum = 1;

      } else {                                                    // Default behaviour (catch error maybe?) 
        alarmNum = -1;
      }
      delay(500);
    }

    // Trigger respective alarm if it is the right time
    DateTime now = rtc.now();
    for (int i = 0; i < 6; i++) {
      if (now.hour() == int(alarmValues[i]/4) && now.minute() == (alarmValues[i]%4)*15) {
        while (analogRead(buttons) > 700) {
          lcd.clear();
          digitalWrite(LEDs[i], HIGH);
          tone(piezo, 500);
          lcd.print(alarmNames[i]);
          delay(1000);
          digitalWrite(LEDs[i], LOW);
          noTone(piezo);
          lcd.print("");
          delay(1000);
        }
      }
    }
    lcd.clear();
    // arduino doesn't have std... very sad
    //string minute;
    //(now.minute < 10) ? minute = String(now.minute()): minute = "0" + String(now.minute());
    lcd.print(String(now.hour()) + ":" + String(now.minute()));
  }
  delay(150);
}

