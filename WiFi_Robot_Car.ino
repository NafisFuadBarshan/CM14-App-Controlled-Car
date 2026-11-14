/******************************************************************
 *  CM14 App-Controlled Car  -  WiFi Robot Remote Control Mode
 *  Board   : NodeMCU ESP8266 (ESP-12E) + NodeMCU Motor Shield (L293D)
 *  Course  : CSE 316 - Peripheral & Interfacing Lab, UAP
 *
 *  Control : HTTP GET  http://<car-ip>/?State=<char>
 *            Compatible with the "WiFi RC Car" style apps.
 *
 *  Commands
 *    F forward        B backward       L left        R right
 *    G forward-left   I forward-right  H back-left   J back-right
 *    S stop           V horn           W light ON    w light OFF
 *    0..9 speed steps (330..960)       q full speed (1023)
 ******************************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

/* ---------------- USER SETTINGS ---------------- */
const char* STA_SSID     = "";   // your home/hotspot WiFi name (leave empty to force AP mode)
const char* STA_PASSWORD = "";   // your WiFi password
const char* AP_PASSWORD  = "";   // optional AP password (empty = open, otherwise min 8 chars)
const char* OTA_PASSWORD = "";   // optional OTA password (empty = none)

const unsigned long WIFI_TIMEOUT_MS = 10000;  // time to try STA connection
const unsigned long FAILSAFE_MS     = 0;      // stop motors if no command for X ms (0 = disabled)
/* ------------------------------------------------ */

// Motor driver pins (NodeMCU Motor Shield)
const int PWM_A = D1;   // Motor A speed (left side)
const int PWM_B = D2;   // Motor B speed (right side)
const int DIR_A = D3;   // Motor A direction
const int DIR_B = D4;   // Motor B direction

// Peripherals
const int buzPin     = D5;  // active buzzer
const int ledPin     = D8;  // bright headlight LEDs
const int wifiLedPin = D0;  // status LED (ON = connected in STA mode)

// Speed
const int SPEED_MIN   = 330;
const int SPEED_MAX   = 1023;
const int speed_Coeff = 3;  // divider for the inner wheel when curving
int SPEED = SPEED_MAX;

String command;
unsigned long lastCommandMillis = 0;
bool motorsRunning = false;

ESP8266WebServer server(80);

/* ---------- Motor helpers ---------- */
void drive(bool dirA, bool dirB, int pwmA, int pwmB) {
  digitalWrite(DIR_A, dirA ? HIGH : LOW);
  digitalWrite(DIR_B, dirB ? HIGH : LOW);
  analogWrite(PWM_A, pwmA);
  analogWrite(PWM_B, pwmB);
  motorsRunning = (pwmA > 0 || pwmB > 0);
}

void Forward()       { drive(HIGH, HIGH, SPEED, SPEED); }
void Backward()      { drive(LOW,  LOW,  SPEED, SPEED); }
void TurnRight()     { drive(LOW,  HIGH, SPEED, SPEED); }
void TurnLeft()      { drive(HIGH, LOW,  SPEED, SPEED); }
void ForwardLeft()   { drive(HIGH, HIGH, SPEED, SPEED / speed_Coeff); }
void BackwardLeft()  { drive(LOW,  LOW,  SPEED, SPEED / speed_Coeff); }
void ForwardRight()  { drive(HIGH, HIGH, SPEED / speed_Coeff, SPEED); }
void BackwardRight() { drive(LOW,  LOW,  SPEED / speed_Coeff, SPEED); }
void Stop()          { drive(LOW,  LOW,  0, 0); }

/* ---------- Peripherals ---------- */
void BeepHorn() {
  digitalWrite(buzPin, HIGH);
  delay(150);
  digitalWrite(buzPin, LOW);
  delay(80);
}
void TurnLightOn()  { digitalWrite(ledPin, HIGH); }
void TurnLightOff() { digitalWrite(ledPin, LOW);  }

/* ---------- Command parser ---------- */
void handleCommand(const String& cmd) {
  if (cmd.length() != 1) return;
  char c = cmd.charAt(0);

  switch (c) {
    case 'F': Forward();       break;
    case 'B': Backward();      break;
    case 'R': TurnRight();     break;
    case 'L': TurnLeft();      break;
    case 'G': ForwardLeft();   break;
    case 'H': BackwardLeft();  break;
    case 'I': ForwardRight();  break;
    case 'J': BackwardRight(); break;
    case 'S': Stop();          break;
    case 'V': BeepHorn();      break;
    case 'W': TurnLightOn();   break;
    case 'w': TurnLightOff();  break;
    case '0': SPEED = 330;  break;
    case '1': SPEED = 400;  break;
    case '2': SPEED = 470;  break;
    case '3': SPEED = 540;  break;
    case '4': SPEED = 610;  break;
    case '5': SPEED = 680;  break;
    case '6': SPEED = 750;  break;
    case '7': SPEED = 820;  break;
    case '8': SPEED = 890;  break;
    case '9': SPEED = 960;  break;
    case 'q': SPEED = 1023; break;
    default: return;
  }
}

/* ---------- HTTP handlers ---------- */
void HTTP_handleRoot() {
  if (server.hasArg("State")) {
    command = server.arg("State");
    Serial.println(command);
    handleCommand(command);
    lastCommandMillis = millis();
  }
  server.send(200, "text/html", "");
}

/* ---------- Setup ---------- */
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("WiFi Robot Remote Control Mode");
  Serial.println("--------------------------------------");

  pinMode(buzPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(wifiLedPin, OUTPUT);
  digitalWrite(buzPin, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(wifiLedPin, HIGH);   // OFF (active-low)

  pinMode(PWM_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(DIR_A, OUTPUT);
  pinMode(DIR_B, OUTPUT);
  Stop();

  // Hostname from chip ID (last 4 hex digits)
  String chip_id = String(ESP.getChipId(), HEX);
  if (chip_id.length() > 4) chip_id = chip_id.substring(chip_id.length() - 4);
  String hostname = "wificar-" + chip_id;
  Serial.println("Hostname: " + hostname);

  // Try STA mode first
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  if (strlen(STA_SSID) > 0) {
    WiFi.begin(STA_SSID, STA_PASSWORD);
    Serial.print("Connecting to: ");
    Serial.println(STA_SSID);

    unsigned long startMillis = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startMillis <= WIFI_TIMEOUT_MS) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi-STA-Mode");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(wifiLedPin, LOW);   // LED ON
  } else {
    // Fallback: create own access point
    WiFi.mode(WIFI_AP);
    if (strlen(AP_PASSWORD) >= 8) WiFi.softAP(hostname.c_str(), AP_PASSWORD);
    else                          WiFi.softAP(hostname.c_str());
    Serial.println("WiFi-AP-Mode");
    Serial.print("AP SSID: ");
    Serial.println(hostname);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    digitalWrite(wifiLedPin, HIGH);  // LED OFF
  }
  delay(1000);

  server.on("/", HTTP_handleRoot);
  server.onNotFound(HTTP_handleRoot);
  server.begin();

  ArduinoOTA.setHostname(hostname.c_str());
  if (strlen(OTA_PASSWORD) > 0) ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  Serial.println("HTTP server started");
}

/* ---------- Main loop ---------- */
void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  // Optional safety: stop if the controller disappears
  if (FAILSAFE_MS > 0 && motorsRunning && (millis() - lastCommandMillis > FAILSAFE_MS)) {
    Stop();
    Serial.println("Failsafe: no command, motors stopped");
  }
}
