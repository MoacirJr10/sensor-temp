#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>

// --- Hardware ---
#define DHTPIN D4
#define DHTTYPE DHT11
#define ONE_WIRE_BUS D3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
ThreeWire myWire(D6, D5, D7);
RtcDS1302<ThreeWire> rtc(myWire);
LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(DHTPIN, DHTTYPE);

// --- Variáveis ---
unsigned long ultimaLeitura  = 0;
unsigned long ultimaAnimacao = 0;
byte frame = 0;

// --- Ícones ---
byte grau[8] = {6,9,9,6,0,0,0,0};
byte wifiA[8] = {0,0,14,17,4,0,0,0};
byte wifiB[8] = {0,14,17,4,14,0,0,0};

byte coracaoA[8] = {0,10,31,31,31,14,4,0};
byte coracaoB[8] = {0,10,21,17,17,10,4,0};
byte fogoA[8]    = {4,10,10,21,21,31,31,14};
byte fogoB[8]    = {0,4,10,10,21,21,31,14};
byte neveA[8]    = {21,10,31,10,21,0,0,0};
byte neveB[8]    = {0,21,10,31,10,21,0,0};
byte gotaA[8]    = {4,4,14,14,31,31,31,14};
byte gotaB[8]    = {0,4,4,14,14,31,31,31};

// --- Ícones ---
void carregarIcones(byte f) {
  lcd.createChar(0, f ? coracaoA : coracaoB);
  lcd.createChar(1, f ? fogoA    : fogoB);
  lcd.createChar(2, f ? neveA    : neveB);
  lcd.createChar(3, f ? gotaA    : gotaB);
  lcd.createChar(4, f ? wifiA    : wifiB);
  lcd.createChar(7, grau);
}

// --- Sincroniza RTC via NTP ---
void sincronizarRTCviaWiFi() {
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  time_t agora = time(nullptr);
  int tentativas = 0;

  while (agora < 100000 && tentativas < 20) {
    delay(500);
    agora = time(nullptr);
    tentativas++;
  }

  if (agora > 100000) {
    struct tm *t = localtime(&agora);
    rtc.SetDateTime(RtcDateTime(
      t->tm_year + 1900,
      t->tm_mon + 1,
      t->tm_mday,
      t->tm_hour,
      t->tm_min,
      t->tm_sec
    ));
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  carregarIcones(0);

  lcd.setCursor(0, 0);
  lcd.print("INICIALIZANDO...");

  // --- WiFi ---
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);
  delay(300);

  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  wm.setBreakAfterConfig(true);
  wm.setDebugOutput(false);
  wm.autoConnect("ESTACAO_AQUARIO");

  // --- RTC ---
  rtc.Begin();

  if (WiFi.status() == WL_CONNECTED) {
    sincronizarRTCviaWiFi(); // 🕒 AJUSTE PERFEITO
  }

  if (!rtc.GetIsRunning()) {
    rtc.SetIsRunning(true);
  }

  dht.begin();
  sensors.begin();
  lcd.clear();
}

// --- LOOP ---
void loop() {
  unsigned long agora = millis();

  if (agora - ultimaLeitura >= 2000) {
    ultimaLeitura = agora;

    sensors.requestTemperatures();
    float tAgua = sensors.getTempCByIndex(0);
    float tAr   = dht.readTemperature();
    float h     = dht.readHumidity();
    float stAr  = dht.computeHeatIndex(tAr, h, false);

    lcd.setCursor(0, 0);
    lcd.print("Ar  : ");
    lcd.print(tAr, 1); lcd.write(7); lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Aqua: ");
    if (tAgua > -50) {
      lcd.print(tAgua, 1); lcd.write(7); lcd.print("C");
    } else lcd.print("ERR");

    lcd.setCursor(0, 2);
    lcd.print("Umid: ");
    lcd.print((int)h); lcd.print("%");
    if (h > 75) lcd.write(3); else lcd.print(" ");

    lcd.setCursor(12, 2);
    lcd.print("ST:");
    lcd.print(stAr, 0); lcd.write(7);

    lcd.setCursor(0, 3);
    lcd.print("Status: ");
    if (tAr > 29) {
      lcd.print("QUENTE "); lcd.write(1);
    } else if (tAr < 18) {
      lcd.print("FRIO   "); lcd.write(2);
    } else {
      lcd.print("BOM    "); lcd.write(0);
    }

    RtcDateTime now = rtc.GetDateTime();
    char hStr[6], dStr[6];
    snprintf(hStr, sizeof(hStr), "%02d:%02d", now.Hour(), now.Minute());
    snprintf(dStr, sizeof(dStr), "%02d/%02d", now.Day(), now.Month());

    lcd.setCursor(15, 0); lcd.print(hStr);
    lcd.setCursor(15, 1); lcd.print(dStr);
  }

  if (agora - ultimaAnimacao >= 600) {
    ultimaAnimacao = agora;
    frame = !frame;
    carregarIcones(frame);

    lcd.setCursor(18, 3);
    if (WiFi.status() == WL_CONNECTED) lcd.write(4);
    else lcd.print("X");
  }
}
