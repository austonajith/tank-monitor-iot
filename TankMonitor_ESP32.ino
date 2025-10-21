#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

Preferences prefs;
WebServer server(80);

// ---------- Tank Config ----------
int tank1Pins[5] = {32, 33, 25, 26, 27};
int tank2Pins[5] = {14, 12, 13, 4, 5};
int tank1Level = 0, tank2Level = 0;
#define BUTTON_PIN 0
#define LED_BUILTIN 2
#define LONG_PRESS_TIME 3000

// ---------- WiFi Config ----------
String ssid, password;
bool apMode = false;
bool wifiEnabled = false;
unsigned long buttonPressStart = 0;
bool buttonWasPressed = false;
unsigned long lastBlinkTime = 0;
bool ledState = false;

// ---------- Function Prototypes ----------
void readTankLevels();
void startAPMode();
void startClientMode();
void handleRoot();
void handleData();
void handleConfig();
void handleSave();
void handleDeleteWifi();
int calculateLevel(int pins[5]);

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);
  Serial.println("\nAJI Tank Smart WiFi Monitor Starting...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Set up probes
  for (int i = 0; i < 5; i++) {
    pinMode(tank1Pins[i], INPUT_PULLUP);
    pinMode(tank2Pins[i], INPUT_PULLUP);
  }

  // Load saved WiFi credentials
  prefs.begin("wifi", false);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("pass", "");
  prefs.end();

  Serial.println("Press and hold BOOT for 3 seconds to enable Wi-Fi...");
}

// ---------- LOOP ----------
void loop() {
  readTankLevels();
  Serial.printf("Tank1: %d%% | Tank2: %d%%\n", tank1Level, tank2Level);

  // ---- WiFi/OTA operations if enabled ----
  if (wifiEnabled) {
    server.handleClient();
    if (!apMode && WiFi.status() == WL_CONNECTED) {
      ArduinoOTA.handle();
    }
  }

  // ---- Button Handling ----
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!buttonWasPressed) {
      buttonWasPressed = true;
      buttonPressStart = millis();
    } 
    else if (millis() - buttonPressStart >= LONG_PRESS_TIME) {
      // Long press -> enable WiFi if not already enabled
      if (!wifiEnabled) {
        Serial.println("Long press detected! Enabling Wi-Fi...");
        wifiEnabled = true;

        if (ssid == "" || password == "") {
          Serial.println("No Wi-Fi credentials saved.");
          startAPMode();
        } else {
          startClientMode();
        }
      } 
      else {
        // If already enabled -> erase Wi-Fi and reboot
        Serial.println("Long press detected! Deleting Wi-Fi credentials...");
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_BUILTIN, HIGH);
          delay(200);
          digitalWrite(LED_BUILTIN, LOW);
          delay(200);
        }

        prefs.begin("wifi", false);
        prefs.remove("ssid");
        prefs.remove("pass");
        prefs.end();

        Serial.println("Wi-Fi credentials deleted! Rebooting...");
        delay(500);
        ESP.restart();
      }
    }
  } else {
    buttonWasPressed = false;
  }

  // ---- LED Behavior ----
  if (wifiEnabled) {
    if (apMode) {
      if (millis() - lastBlinkTime > 1000) {
        lastBlinkTime = millis();
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
      }
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);  // LED off when Wi-Fi disabled
  }

  delay(1000);
}

// ---------- Tank Reading ----------
void readTankLevels() {
  tank1Level = calculateLevel(tank1Pins);
  tank2Level = calculateLevel(tank2Pins);
}

int calculateLevel(int pins[5]) {
  int highest = -1;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(pins[i]) == LOW) highest = i;
  }
  switch (highest) {
    case 0: return 10;
    case 1: return 30;
    case 2: return 50;
    case 3: return 70;
    case 4: return 100;
    default: return 0;
  }
}

// ---------- WiFi Client Mode ----------
void startClientMode() {
  apMode = false;
  Serial.printf("Connecting to WiFi: %s\n", ssid.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // ---------- OTA SETUP ----------
    ArduinoOTA.setHostname("AJI-TankMonitor");
    ArduinoOTA.onStart([]() { Serial.println("OTA Update Started..."); });
    ArduinoOTA.onEnd([]() { Serial.println("\nOTA Update Complete!"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]\n", error);
    });
    ArduinoOTA.begin();
    Serial.println("OTA Ready");

    // ---------- Web Server ----------
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/delete_wifi", HTTP_POST, handleDeleteWifi);
    server.begin();
  } else {
    Serial.println("\nFailed to connect. Starting AP mode...");
    startAPMode();
  }
}

// ---------- WiFi Access Point Mode ----------
void startAPMode() {
  apMode = true;

  WiFi.mode(WIFI_AP);
  IPAddress local_IP(192,168,1,1);
  IPAddress gateway(192,168,1,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("AJI-TankMonitor");

  Serial.println("Started Access Point mode.");
  Serial.print("AP SSID: AJI-TankMonitor\nAP IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleConfig);
  server.on("/save", handleSave);
  server.begin();
}

// ---------- Web Handlers ----------
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='2'/>"
                "<title>AJI Dual Tank Monitor</title>"
                "<style>"
                "body{font-family:Arial;text-align:center;background:#f2f2f2;}"
                ".card{margin:20px auto;padding:20px;background:white;max-width:320px;"
                "border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.2);}"
                ".level{font-size:40px;color:#2196F3;}"
                "button{margin-top:20px;padding:10px 20px;border:none;border-radius:5px;"
                "background:#f44336;color:white;font-size:16px;cursor:pointer;}"
                "button:hover{background:#d32f2f;}"
                "</style></head><body>"
                "<h2>AJI Dual Water Tank Monitor</h2>"
                "<div class='card'><h3>Tank 1</h3><div class='level'>" + String(tank1Level) + "%</div></div>"
                "<div class='card'><h3>Tank 2</h3><div class='level'>" + String(tank2Level) + "%</div></div>"
                "<p>JSON Endpoint: <a href='/data'>/data</a></p>"
                "<form action='/delete_wifi' method='post'>"
                "<button type='submit'>Delete Wi-Fi Config</button>"
                "</form>"
                "</body></html>";
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{\"tank1\": " + String(tank1Level) + ", \"tank2\": " + String(tank2Level) + "}";
  server.send(200, "application/json", json);
}

void handleConfig() {
  String html = "<!DOCTYPE html><html><head><title>WiFi Setup</title></head><body>"
                "<h2>WiFi Configuration</h2>"
                "<form action='/save' method='get'>"
                "SSID:<br><input name='ssid'><br>"
                "Password:<br><input name='pass' type='password'><br><br>"
                "<input type='submit' value='Save'>"
                "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  String newSSID = server.arg("ssid");
  String newPASS = server.arg("pass");

  if (newSSID.length() > 0) {
    prefs.begin("wifi", false);
    prefs.putString("ssid", newSSID);
    prefs.putString("pass", newPASS);
    prefs.end();

    server.send(200, "text/html", "<html><body><h3>WiFi Saved! Rebooting...</h3></body></html>");
    delay(2000);
    ESP.restart();
  } else {
    server.send(200, "text/html", "<html><body><h3>Invalid SSID</h3></body></html>");
  }
}

void handleDeleteWifi() {
  prefs.begin("wifi", false);
  prefs.remove("ssid");
  prefs.remove("pass");
  prefs.end();

  server.send(200, "text/html",
              "<html><body><h3>Wi-Fi credentials deleted! Rebooting...</h3></body></html>");
  delay(2000);
  ESP.restart();
}
