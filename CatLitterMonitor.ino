#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==== Pin definitions ====
#define BEAM_PIN 4       // Break-beam sensor (receiver output)
#define RELAY_PIN 5      // Relay controlling the fan (HIGH = ON for CW-019)
#define BUZZER_PIN 2     // Active buzzer output
#define MQ_PIN A1        // MQ135 gas sensor analog output

// ==== Cat use detection variables ====
int litterCount = 0;          // Number of litter box uses
bool beamBlocked = false;     // Whether the IR beam is currently blocked
bool catInside = false;       // Whether the cat is inside the litter box
unsigned long debounceDelay = 150; 
unsigned long lastChange = 0;

// ==== Air quality thresholds ====
int fanThreshold = 300;     // MQ > 300 → Fan ON
int alarmThreshold = 500;   // MQ > 500 → Fan + Buzzer ON

void setup() {

  pinMode(BEAM_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Relay OFF by default (LOW = OFF for your module)
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Initialize I2C
  Wire.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  // Startup screen
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Cat Litter");
  display.display();
  delay(1500);
}

void loop() {

  // ========= READ BREAK-BEAM SENSOR =========
  bool currentBlocked = (digitalRead(BEAM_PIN) == LOW);  // LOW = beam blocked

  // Detect changes with debounce
  if (currentBlocked != beamBlocked) {
    if (millis() - lastChange > debounceDelay) {
      
      beamBlocked = currentBlocked;
      lastChange = millis();

      // ========= CAT ENTRY/EXIT LOGIC =========
      if (beamBlocked && !catInside) {
        // Cat ENTERS the litter box
        catInside = true;
      }

      if (!beamBlocked && catInside) {
        // Cat EXITS → Count as one use
        litterCount++;
        catInside = false;
      }
    }
  }

  // ========= READ MQ135 GAS SENSOR =========
  int mq = analogRead(MQ_PIN);

  // ========= FAN CONTROL (HIGH = ON on YOUR RELAY) =========
  if (mq > fanThreshold) {
    digitalWrite(RELAY_PIN, HIGH);   // Fan ON
  } else {
    digitalWrite(RELAY_PIN, LOW);    // Fan OFF
  }

  // ========= BUZZER ALARM CONTROL =========
  if (mq > alarmThreshold) {
    tone(BUZZER_PIN, 2000);          // Alarm ON
  } else {
    noTone(BUZZER_PIN);              // Alarm OFF
  }

  // ========= OLED DISPLAY =========
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Uses Today: ");
  display.println(litterCount);

  display.print("Beam: ");
  display.println(beamBlocked ? "BLOCKED" : "CLEAR");

  display.print("Cat Inside: ");
  display.println(catInside ? "YES" : "NO");

  display.print("Smell: ");
  display.println(mq);

  display.print("Fan: ");
  display.println(digitalRead(RELAY_PIN) == HIGH ? "ON" : "OFF");

  display.print("Alarm: ");
  display.println(mq > alarmThreshold ? "YES" : "NO");

  display.display();

  delay(80);
}
