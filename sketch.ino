#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>

#define room1Relay 27
#define room2Relay 33
#define ledRoom1   25
#define ledRoom2   26
#define ledMerah   32
#define ledBiru    23
#define buzzer     4

#define pot1Pin    34
#define pot2Pin    35

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid = "Wokwi-GUEST";
const char* pass = "";

const String apiKey = "FKHL0012GO4G75F3";
const String server = "http://api.thingspeak.com/update";

float power1 = 0, power2 = 0;
float maxPower = 500;
bool c1State = true, c2State = true;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");

  pinMode(room1Relay, OUTPUT);
  pinMode(room2Relay, OUTPUT);
  pinMode(ledRoom1, OUTPUT);
  pinMode(ledRoom2, OUTPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(ledBiru, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(room1Relay, HIGH);
  digitalWrite(room2Relay, HIGH);
  digitalWrite(ledRoom1, HIGH);
  digitalWrite(ledRoom2, HIGH);
  digitalWrite(ledMerah, LOW);
  digitalWrite(ledBiru, HIGH);
  digitalWrite(buzzer, LOW);
}

void loop() {
  power1 = map(analogRead(pot1Pin), 0, 4095, 0, 1000);
  power2 = map(analogRead(pot2Pin), 0, 4095, 0, 1000);

  bool overload1 = power1 > maxPower;
  bool overload2 = power2 > maxPower;

  if (overload1 && c1State) {
    digitalWrite(room1Relay, LOW);
    digitalWrite(ledRoom1, LOW);
    c1State = false;
    alert_display("ROOM1 OVERLOAD");
    alert_buzzer();
  } else if (!overload1 && !c1State) {
    digitalWrite(room1Relay, HIGH);
    digitalWrite(ledRoom1, HIGH);
    c1State = true;
  }

  if (overload2 && c2State) {
    digitalWrite(room2Relay, LOW);
    digitalWrite(ledRoom2, LOW);
    c2State = false;
    alert_display("ROOM2 OVERLOAD");
    alert_buzzer();
  } else if (!overload2 && !c2State) {
    digitalWrite(room2Relay, HIGH);
    digitalWrite(ledRoom2, HIGH);
    c2State = true;
  }

  if (overload1 || overload2) {
    digitalWrite(ledMerah, HIGH);
    digitalWrite(ledBiru, LOW);
  } else {
    digitalWrite(ledMerah, LOW);
    digitalWrite(ledBiru, HIGH);
  }

  send_thingspeak(power1, power2);
  homedisplay();
  delay(2000);
}

void homedisplay() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("-ROOMS' POWER USAGE-");
  lcd.setCursor(0, 1); lcd.print("ROOM1: "); lcd.print(power1, 1); lcd.print("W");
  lcd.setCursor(0, 2); lcd.print("ROOM2: "); lcd.print(power2, 1); lcd.print("W");
  lcd.setCursor(0, 3); lcd.print("LIMIT: "); lcd.print(maxPower); lcd.print("W");
}

void alert_display(String msg) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("- OVERLOAD ALERT -");
  lcd.setCursor(0, 2); lcd.print(msg);
  lcd.setCursor(0, 3); lcd.print("CUTTING POWER...");
}

void alert_buzzer() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
    delay(250);
  }
}

void send_thingspeak(float p1, float p2) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = server + "?api_key=" + apiKey + "&field1=" + p1 + "&field2=" + p2;
    http.begin(url);
    int httpCode = http.GET();
    http.end();
  }
}
