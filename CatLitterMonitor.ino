#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==== OLED display setup ====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==== Pin definitions ====
#define BEAM_PIN 4       // Break-beam sensor (receiver output)
#define RELAY_PIN 5      // Relay controlling the fan
#define BUZZER_PIN 2     // Active buzzer
#define MQ_PIN A1        // MQ135 gas sensor analog output

// ==== Variables ====
int litterCount = 0;        // Number of litter box uses detected
bool beamBroken = false;    // Current beam break status
bool lastBeamState = false; // Previous beam state

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 300; // Debounce time to avoid multiple triggers

// ==== Thresholds for air quality ====
int smellLow = 300;   // Fan turns ON above this level
int smellHigh = 450;  // Buzzer alarm above this level

void setup() {

  pinMode(BEAM_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Relay is LOW-triggered: HIGH = OFF
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  // ==== Initialize OLED ====
  Wire.begin(11, 12); // MKR1010 I2C pins (SDA=11, SCL=12)
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

  // === Read break-beam sensor ===
  bool currentBeam = (digitalRead(BEAM_PIN) == LOW);  
  // LOW means the beam is blocked (cat passing)

  // === Edge detection with debounce ===
  if (currentBeam != lastBeamState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Count one litter box use per entry
    if (currentBeam == true && beamBroken == false) {
      litterCount++;
      beamBroken = true;
    }
    if (currentBeam == false) {
      beamBroken = false;
    }
  }
  lastBeamState = currentBeam;

  // === Read air quality from MQ135 ===
  int mq = analogRead(MQ_PIN);

  // === Fan control via relay (LOW-triggered) ===
  if (mq > smellLow) {
    digitalWrite(RELAY_PIN, LOW);  // Fan ON
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Fan OFF
  }

  // === Alarm for strong odor ===
  if (mq > smellHigh) {
    tone(BUZZER_PIN, 2000);   // Buzzer alarm
  } else {
    noTone(BUZZER_PIN);
  }

  // ==== Update OLED display ====
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Uses Today: ");
  display.println(litterCount);

  display.print("Beam: ");
  display.println(currentBeam ? "BLOCKED" : "CLEAR");

  display.print("Smell: ");
  display.println(mq);

  display.print("Fan: ");
  display.println(digitalRead(RELAY_PIN) == LOW ? "ON" : "OFF");

  display.print("Alarm: ");
  display.println(mq > smellHigh ? "YES" : "NO");

  display.display();

  delay(100);
}
