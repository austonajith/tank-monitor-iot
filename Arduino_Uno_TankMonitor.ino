#include <SoftwareSerial.h>

// --- SoftwareSerial setup (RX=0, TX=1 but only RX is used) ---
SoftwareSerial espSerial(0, 1);  // Connect ESP32 TX → UNO Pin 0

// --- Tank LED pins ---
int tank1LEDs[5] = { 4, 5, 6, 7, 8 };
int tank2LEDs[5] = { 9, 10, 11, 12, 13 };

int tank1Level = 0;
int tank2Level = 0;

void setup() {
  Serial.begin(9600);     // For debugging on Serial Monitor
  espSerial.begin(9600);  // Must match ESP32 Serial.begin(9600)
  Serial.println("UNO ready. Listening to ESP32...");

  for (int i = 0; i < 5; i++) {
    pinMode(tank1LEDs[i], OUTPUT);
    pinMode(tank2LEDs[i], OUTPUT);
  }
}

void loop() {
  if (espSerial.available()) {
    String line = espSerial.readStringUntil('\n');
    line.trim();
    Serial.print("From ESP32: ");
    Serial.println(line);

    // Expected format from ESP32: Tank1:30|Tank2:70
    int t1Index = line.indexOf("Tank1:");
    int t2Index = line.indexOf("|Tank2:");
    if (t1Index >= 0 && t2Index > 0) {
      int t1PercentIndex = line.indexOf('%', t1Index);
      int t2Index = line.indexOf("Tank2:");
      int t2PercentIndex = line.indexOf('%', t2Index);

      tank1Level = line.substring(t1Index + 6, t1PercentIndex).toInt();
      tank2Level = line.substring(t2Index + 6, t2PercentIndex).toInt();

      Serial.print("Parsed -> Tank1: ");
      Serial.print(tank1Level);
      Serial.print("%, Tank2: ");
      Serial.print(tank2Level);
      Serial.println("%");

      updateTankLEDs();
    }
  }
}

// --- Update LEDs based on tank levels ---
void updateTankLEDs() {
  int tank1On = round(tank1Level * 5.0 / 100);
  int tank2On = round(tank2Level * 5.0 / 100);

  Serial.print("tank1 LEDs: ");
  Serial.print(tank1On);
  Serial.print(" | tank2 LEDs: ");
  Serial.println(tank2On);

  // --- Handle Tank 1 ---
  if (tank1Level == 0) {
    // Blink lowest LED (index 0)
    digitalWrite(tank1LEDs[0], millis() / 500 % 2 ? HIGH : LOW);
    for (int i = 1; i < 5; i++) digitalWrite(tank1LEDs[i], LOW);
  } else {
    for (int i = 0; i < 5; i++) {
      digitalWrite(tank1LEDs[i], (i < tank1On) ? HIGH : LOW);
    }
  }

  // --- Handle Tank 2 ---
  if (tank2Level == 0) {
    // Blink lowest LED (index 0)
    digitalWrite(tank2LEDs[0], millis() / 500 % 2 ? HIGH : LOW);
    for (int i = 1; i < 5; i++) digitalWrite(tank2LEDs[i], LOW);
  } else {
    for (int i = 0; i < 5; i++) {
      digitalWrite(tank2LEDs[i], (i < tank2On) ? HIGH : LOW);
    }
  }
}
