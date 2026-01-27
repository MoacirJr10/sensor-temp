#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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
unsigned long ultimaLeitura = 0;
unsigned long ultimaAnimacao = 0;
byte frame = 0;

// --- Ícones com Movimento ---
byte grau[8]    = {6,9,9,6,0,0,0,0};
byte wifi_ico[8]= {0,0,14,17,4,4,0,0};

byte coracaoA[8] = {0,10,31,31,31,14,4,0};   // Coração cheio
byte coracaoB[8] = {0,10,21,17,17,10,4,0};   // Coração batendo (vazio)

byte fogoA[8] = {4,10,10,21,21,31,31,14};
byte fogoB[8] = {0,4,10,10,21,21,31,14};

byte neveA[8] = {21,10,31,10,21,0,0,0};
byte neveB[8] = {0,21,10,31,10,21,0,0};

byte gotaA[8] = {4,4,14,14,31,31,31,14};
byte gotaB[8] = {0,4,4,14,14,31,31,31};

void carregarIcones(byte f) {
  lcd.createChar(0, f ? coracaoA : coracaoB);
  lcd.createChar(1, f ? fogoA : fogoB);
  lcd.createChar(2, f ? neveA : neveB);
  lcd.createChar(3, f ? gotaA : gotaB);
  lcd.createChar(5, wifi_ico);
  lcd.createChar(7, grau);
}

void setup() {
  lcd.init();
  lcd.backlight();
  carregarIcones(0);

  lcd.setCursor(0, 0); lcd.print("INICIALIZANDO...");

  WiFiManager wm;
  wm.setConfigPortalTimeout(120);
  if (!wm.autoConnect("ESTACAO_AQUARIO")) {
    lcd.setCursor(0, 1); lcd.print("MODO OFFLINE");
    delay(2000);
  }

  rtc.Begin();
  if (!rtc.IsDateTimeValid()) rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));

  dht.begin();
  sensors.begin();
  lcd.clear();
}

void loop() {
  unsigned long agora = millis();

  if (agora - ultimaLeitura >= 2000) {
    ultimaLeitura = agora;

    sensors.requestTemperatures();
    float tAgua = sensors.getTempCByIndex(0);
    float tAr = dht.readTemperature();
    float h = dht.readHumidity();
    float stAr = dht.computeHeatIndex(tAr, h, false);

    // Linha 0: Ar e Hora
    lcd.setCursor(0, 0); lcd.print("Ar  : ");
    lcd.print(tAr, 1); lcd.write(7); lcd.print("C");

    // Linha 1: Aqua e Data
    lcd.setCursor(0, 1); lcd.print("Aqua: ");
    if(tAgua > -50) { lcd.print(tAgua, 1); lcd.write(7); lcd.print("C"); }
    else { lcd.print("ERR"); }

    // Linha 2: Umidade e ST (Afastado)
    lcd.setCursor(0, 2); lcd.print("Umid: ");
    lcd.print((int)h); lcd.print("%");
    if(h > 75) lcd.write(3); else lcd.print(" ");

    lcd.setCursor(12, 2); lcd.print("ST:");
    lcd.print(stAr, 0); lcd.write(7);

    // Linha 3: Status + TEXTO + EMOJI (À DIREITA)
    lcd.setCursor(0, 3);
    lcd.print("Status: ");
    if(tAr > 29) {
      lcd.print("QUENTE "); lcd.write(1);
    } else if(tAr < 18) {
      lcd.print("FRIO   "); lcd.write(2);
    } else {
      lcd.print("BOM    "); lcd.write(0);
    }

    // Relógio e Data
    RtcDateTime now = rtc.GetDateTime();
    char dStr[6], hStr[6];
    snprintf(dStr, sizeof(dStr), "%02d/%02d", now.Day(), now.Month());
    snprintf(hStr, sizeof(hStr), "%02d:%02d", now.Hour(), now.Minute());
    lcd.setCursor(15, 0); lcd.print(hStr);
    lcd.setCursor(15, 1); lcd.print(dStr);
  }

  // Animação (Troca de desenho para simular movimento)
  if (agora - ultimaAnimacao >= 600) {
    ultimaAnimacao = agora;
    frame = !frame;
    carregarIcones(frame);

    // WiFi no canto
    lcd.setCursor(18, 3);
    if(WiFi.status() == WL_CONNECTED) lcd.write(5); else lcd.print("X");
  }
}