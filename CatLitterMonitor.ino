#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin definitions
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

//Air quality thresholds
int fanThreshold = 300;     // MQ > 300 → Fan ON
int alarmThreshold = 500;   // MQ > 500 → Fan + Buzzer ON

void setup() {

  pinMode(BEAM_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Relay OFF by default
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Serial for debug output
  Serial.begin(9600);

  // Initialize I2C
  Wire.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 20);
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

      //2-Cycle State Machine 
      switch (catState) {

        case WAIT_ENTRY_BLOCK:
          if (beamBlocked) catState = WAIT_ENTRY_CLEAR;
          break;

        case WAIT_ENTRY_CLEAR:
          if (!beamBlocked) catState = WAIT_EXIT_BLOCK;
          break;

        case WAIT_EXIT_BLOCK:
          if (beamBlocked) catState = WAIT_EXIT_CLEAR;
          break;

        case WAIT_EXIT_CLEAR:
          if (!beamBlocked) {
            litterCount++;  
            catState = WAIT_ENTRY_BLOCK;  // reset machine
          }
          break;
      }
    }
  }
}

void loop() {

  updateCatCounter();

  // READ MQ135 GAS SENSOR 
  int mq = analogRead(MQ_PIN);

  // FAN CONTROL (HIGH = ON) 
  if (mq > fanThreshold) {
    digitalWrite(RELAY_PIN, HIGH);   // Fan ON
  } else {
    digitalWrite(RELAY_PIN, LOW);    // Fan OFF
  }

  // BUZZER ALARM CONTROL
  if (mq > alarmThreshold) {
    tone(BUZZER_PIN, 2000);          
  } else {
    noTone(BUZZER_PIN);              
  }

  // SERIAL OUTPUT (full debug info)
  Serial.print("Cat Uses: ");
  Serial.print(litterCount);

  Serial.print(" | Beam: ");
  Serial.print(beamBlocked ? "BLOCKED" : "CLEAR");

  Serial.print(" | Smell: ");
  Serial.print(mq);

  Serial.print(" | Fan: ");
  Serial.print(digitalRead(RELAY_PIN) == HIGH ? "ON" : "OFF");

  Serial.print(" | Alarm: ");
  Serial.println(mq > alarmThreshold ? "YES" : "NO");

  // OLED DISPLAY (only the count)
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("Today: ");
  display.println(litterCount);
  display.display();

  delay(100);
}
