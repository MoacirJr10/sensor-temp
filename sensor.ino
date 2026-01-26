#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <WiFiManager.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ==================== HARDWARE ====================
#define DHTPIN D4
#define DHTTYPE DHT11
#define ONE_WIRE_BUS D3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
ThreeWire myWire(D6, D5, D7);
RtcDS1302<ThreeWire> rtc(myWire);
LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(DHTPIN, DHTTYPE);

// ==================== VARIÁVEIS ====================
unsigned long ultimaLeitura = 0;
unsigned long ultimaAnimacao = 0, ultimaAtualizacaoData = 0;
float tempArAnt = -999.0, tempAguaAnt = -999.0, umidAnt = -999.0;
byte frameAtual = 0;

enum Conforto { IDEAL, FRIO, QUENTE, UMIDO, SECO, BOM };
Conforto confAtual = BOM;

// ==================== BITMAPS ====================
byte char_grau[8] = {B00110, B01001, B01001, B00110, B00000, B00000, B00000, B00000};
byte wifi_on[8]   = {B00000, B00000, B00111, B01000, B10011, B10100, B10101, B00000};
byte wifi_off[8]  = {B00000, B00000, B00001, B00010, B00100, B01000, B10000, B00000};

byte ideal_esq_1[8] = {B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000};
byte ideal_dir_1[8] = {B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000};
byte ideal_esq_2[8] = {B00000, B01010, B11111, B11111, B01110, B00100, B00000, B00000};
byte ideal_dir_2[8] = {B00000, B01010, B11111, B11111, B01110, B00100, B00000, B00000};

// ==================== FUNÇÕES ====================

Conforto avaliarConforto(float t, float h) {
  if (t < 16.0) return FRIO;
  if (t > 29.0) return QUENTE;
  if (h < 30.0) return SECO;
  if (h > 75.0) return UMIDO;
  return BOM;
}

void carregarAnimacao(Conforto c) {
  lcd.createChar(0, ideal_esq_1); lcd.createChar(1, ideal_dir_1);
  lcd.createChar(2, ideal_esq_2); lcd.createChar(3, ideal_dir_2);
}

void criarPersonagens() {
  lcd.createChar(7, char_grau);
  lcd.createChar(4, wifi_on);
  lcd.createChar(5, wifi_off);
}

void desenharIconeStatus() {
  lcd.setCursor(18, 3);
  lcd.write(frameAtual == 0 ? 0 : 2);
  lcd.write(frameAtual == 0 ? 1 : 3);

  lcd.setCursor(16, 3);
  lcd.write(WiFi.status() == WL_CONNECTED ? 4 : 5);
}

void atualizarDataHora() {
  if (!rtc.IsDateTimeValid()) return;
  RtcDateTime now = rtc.GetDateTime();
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d", now.Hour(), now.Minute());
  lcd.setCursor(14, 0); lcd.print(buf);
  snprintf(buf, sizeof(buf), "%02d/%02d", now.Day(), now.Month());
  lcd.setCursor(14, 1); lcd.print(buf);
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  lcd.init(); lcd.backlight();
  criarPersonagens();

  sensors.begin();
  dht.begin();
  rtc.Begin();

  lcd.setCursor(3, 1); lcd.print("INICIALIZANDO...");

  WiFiManager wm;
  // wm.resetSettings(); // Descomente para limpar o WiFi salvo se necessário

  wm.setConfigPortalTimeout(60);

  // Se não conectar em 60s, ele segue para o loop
  if (!wm.autoConnect("ESTACAO_AQUARIO")) {
    lcd.setCursor(0, 2); lcd.print("Offline Mode Ativo");
  } else {
    lcd.setCursor(0, 2); lcd.print("WiFi Conectado!   ");
    configTime(-3 * 3600, 0, "pool.ntp.org");
  }

  delay(2000);
  lcd.clear();

  lcd.setCursor(0, 0); lcd.print(F("Ar  :"));
  lcd.setCursor(0, 1); lcd.print(F("Aqua:"));
  lcd.setCursor(0, 2); lcd.print(F("Umid:"));
  lcd.setCursor(0, 3); lcd.print(F("Status:"));

  carregarAnimacao(BOM);
}

// ==================== LOOP ====================
void loop() {
  unsigned long agora = millis();

  // Se o WiFi cair, tenta reconectar silenciosamente em segundo plano
  if (WiFi.status() != WL_CONNECTED && (agora % 10000 == 0)) {
     WiFi.begin();
  }

  if (agora - ultimaAnimacao >= 500) {
    ultimaAnimacao = agora;
    frameAtual = 1 - frameAtual;
    desenharIconeStatus();
  }

  if (agora - ultimaAtualizacaoData >= 10000) {
    ultimaAtualizacaoData = agora;
    atualizarDataHora();
  }

  if (agora - ultimaLeitura >= 2000) {
    ultimaLeitura = agora;

    sensors.requestTemperatures();
    float tAgua = sensors.getTempCByIndex(0);
    float tAr = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(tAr)) {
      lcd.setCursor(6, 0); lcd.print(tAr, 1); lcd.write(7);
      confAtual = avaliarConforto(tAr, h);
      lcd.setCursor(8, 3);
      if(confAtual == QUENTE)      lcd.print("QUENTE  ");
      else if(confAtual == FRIO)   lcd.print("FRIO    ");
      else                         lcd.print("NORMAL  ");
    }

    if (!isnan(h)) {
      lcd.setCursor(6, 2); lcd.print((int)h); lcd.print("% ");
      lcd.setCursor(11, 2); lcd.print("[");
      int blocos = map(constrain(h, 0, 100), 0, 100, 0, 6);
      for(int i=0; i<6; i++) lcd.write(i < blocos ? 255 : '.');
      lcd.print("]");
    }

    lcd.setCursor(6, 1);
    if (tAgua != DEVICE_DISCONNECTED_C) {
      lcd.print(tAgua, 1); lcd.write(7); lcd.print("C ");
    } else {
      lcd.print("ERR  ");
    }
  }
}
