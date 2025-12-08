#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Pin definitions
#define BEAM_PIN 4       // Break-beam sensor (receiver output)
#define RELAY_PIN 5      // Relay controlling the fan (HIGH = ON for CW-019)
#define BUZZER_PIN 2     // Active buzzer output
#define MQ_PIN A1        // MQ135 gas sensor analog output

//Cat use detection (2-cycle entry+exit detection)
enum CatState {
  WAIT_ENTRY_BLOCK,
  WAIT_ENTRY_CLEAR,
  WAIT_EXIT_BLOCK,
  WAIT_EXIT_CLEAR
};

CatState catState = WAIT_ENTRY_BLOCK;

int litterCount = 0;
bool beamBlocked = false;
unsigned long debounceDelay = 120;
unsigned long lastChange = 0;

// Air quality thresholds 
int fanThreshold = 300;     // MQ > 300 → Fan ON
int alarmThreshold = 500;   // MQ > 500 → Fan + Buzzer ON

void setup() {

  pinMode(BEAM_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Relay OFF by default
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Initialize I2C
  Wire.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Cat Litter");
  display.display();
  delay(1500);
}

void updateCatCounter() {

  bool currentBlocked = (digitalRead(BEAM_PIN) == LOW); // LOW = beam blocked

  // Debounce
  if (currentBlocked != beamBlocked) {
    if (millis() - lastChange > debounceDelay) {

      beamBlocked = currentBlocked;
      lastChange = millis();

      // 2-Cycle State Machine 
      switch (catState) {

        // 1. Wait for first BLOCK (cat entering)
        case WAIT_ENTRY_BLOCK:
          if (beamBlocked) {
            catState = WAIT_ENTRY_CLEAR;
          }
          break;

        // 2. Wait for CLEAR (entry complete)
        case WAIT_ENTRY_CLEAR:
          if (!beamBlocked) {
            catState = WAIT_EXIT_BLOCK;
          }
          break;

        // 3. Wait for second BLOCK (cat exiting)
        case WAIT_EXIT_BLOCK:
          if (beamBlocked) {
            catState = WAIT_EXIT_CLEAR;
          }
          break;

        // 4. Wait for final CLEAR → full cycle done → count++
        case WAIT_EXIT_CLEAR:
          if (!beamBlocked) {
            litterCount++;  // One full toilet use detected
            catState = WAIT_ENTRY_BLOCK;  // Reset state machine
          }
          break;
      }
    }
  }
}

void loop() {

  //UPDATE CAT COUNTER 
  updateCatCounter();

  // READ MQ135 GAS SENSOR 
  int mq = analogRead(MQ_PIN);

  //  FAN CONTROL (HIGH = ON on your relay) 
  if (mq > fanThreshold) {
    digitalWrite(RELAY_PIN, HIGH);   // Fan ON
  } else {
    digitalWrite(RELAY_PIN, LOW);    // Fan OFF
  }

  // BUZZER ALARM CONTROL 
  if (mq > alarmThreshold) {
    tone(BUZZER_PIN, 2000);          // Alarm ON
  } else {
    noTone(BUZZER_PIN);              // Alarm OFF
  }

  //  OLED DISPLAY 
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Uses Today: ");
  display.println(litterCount);

  display.print("Beam: ");
  display.println(beamBlocked ? "BLOCKED" : "CLEAR");

  display.print("Smell: ");
  display.println(mq);

  display.print("Fan: ");
  display.println(digitalRead(RELAY_PIN) == HIGH ? "ON" : "OFF");

  display.print("Alarm: ");
  display.println(mq > alarmThreshold ? "YES" : "NO");

  display.display();

  delay(80);
}
