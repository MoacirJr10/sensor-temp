#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <ESP8266WiFi.h>
#include <time.h>

// --- Configurações WiFi/NTP ---
const char* ssid = "SEU_WIFI_AQUI";
const char* password = "SUA_SENHA_AQUI";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600; // UTC-3 (Brasília)
const int daylightOffset_sec = 0;

// --- Configurações Hardware ---
#define DHTPIN D4            
#define DHTTYPE DHT11

// Pinos do DS1302: IO, SCLK, CE
ThreeWire myWire(D6, D5, D7); // DAT=D6, CLK=D5, RST=D7
RtcDS1302<ThreeWire> rtc(myWire);

LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(DHTPIN, DHTTYPE);

#define INTERVALO_LEITURA 2000UL
#define INTERVALO_ANIMACAO 500UL
#define INTERVALO_DATA 10000UL

// --- Variáveis ---
unsigned long ultimaLeitura = 0;
unsigned long ultimaBoaLeitura = 0;
unsigned long ultimaAnimacao = 0;
unsigned long ultimaAtualizacaoData = 0;
float tempAnterior = -999.0;
float umidAnterior = -999.0;
int errosConsecutivos = 0;
byte frameAtual = 0;

enum Conforto { IDEAL, FRIO, QUENTE, UMIDO, SECO, BOM };
Conforto confAtual = BOM;

// --- BITMAPS ANIMADOS (2 frames por estado, 2 chars cada) ---
// IDEAL - Coração pulsante
byte ideal_esq_1[8] = {B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000};
byte ideal_dir_1[8] = {B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000};
byte ideal_esq_2[8] = {B00000, B01010, B11111, B11111, B01110, B00100, B00000, B00000};
byte ideal_dir_2[8] = {B00000, B01010, B11111, B11111, B01110, B00100, B00000, B00000};

// QUENTE - Fogo animado
byte quente_esq_1[8] = {B00100, B01010, B01010, B10001, B10001, B11011, B01110, B00100};
byte quente_dir_1[8] = {B00100, B01010, B01010, B10001, B10001, B11011, B01110, B00100};
byte quente_esq_2[8] = {B01000, B10100, B10100, B01010, B01010, B10101, B11011, B01110};
byte quente_dir_2[8] = {B00010, B00101, B00101, B01010, B01010, B10101, B11011, B01110};

// FRIO - Cristal de gelo/floco de neve
byte frio_esq_1[8] = {B00100, B10101, B01110, B11111, B01110, B10101, B00100, B00000};
byte frio_dir_1[8] = {B00100, B10101, B01110, B11111, B01110, B10101, B00100, B00000};
byte frio_esq_2[8] = {B10001, B01010, B00100, B11111, B00100, B01010, B10001, B00000};
byte frio_dir_2[8] = {B10001, B01010, B00100, B11111, B00100, B01010, B10001, B00000};

// ÚMIDO - Gotas de água caindo
byte umido_esq_1[8] = {B00100, B00100, B01110, B01110, B11111, B11111, B01110, B00000};
byte umido_dir_1[8] = {B01000, B01000, B11100, B11100, B11110, B11110, B01100, B00000};
byte umido_esq_2[8] = {B00000, B00100, B00100, B01110, B01110, B11111, B11111, B01110};
byte umido_dir_2[8] = {B00000, B01000, B01000, B11100, B11100, B11110, B11110, B01100};

// SECO - Sol brilhante
byte seco_esq_1[8] = {B10101, B01000, B00100, B11111, B11111, B00100, B01000, B10101};
byte seco_dir_1[8] = {B10101, B00010, B00100, B11111, B11111, B00100, B00010, B10101};
byte seco_esq_2[8] = {B01010, B10001, B00100, B11111, B11111, B00100, B10001, B01010};
byte seco_dir_2[8] = {B01010, B10001, B00100, B11111, B11111, B00100, B10001, B01010};

// Grau Célsius
byte char_grau[8] = {B00110, B01001, B01001, B00110, B00000, B00000, B00000, B00000};

// --- Funções ---
void criarPersonagens() {
  lcd.createChar(7, char_grau);
}

void carregarAnimacao(Conforto c) {
  switch(c) {
    case IDEAL:
      lcd.createChar(0, ideal_esq_1);
      lcd.createChar(1, ideal_dir_1);
      lcd.createChar(2, ideal_esq_2);
      lcd.createChar(3, ideal_dir_2);
      break;
    case QUENTE:
      lcd.createChar(0, quente_esq_1);
      lcd.createChar(1, quente_dir_1);
      lcd.createChar(2, quente_esq_2);
      lcd.createChar(3, quente_dir_2);
      break;
    case FRIO:
      lcd.createChar(0, frio_esq_1);
      lcd.createChar(1, frio_dir_1);
      lcd.createChar(2, frio_esq_2);
      lcd.createChar(3, frio_dir_2);
      break;
    case UMIDO:
      lcd.createChar(0, umido_esq_1);
      lcd.createChar(1, umido_dir_1);
      lcd.createChar(2, umido_esq_2);
      lcd.createChar(3, umido_dir_2);
      break;
    case SECO:
      lcd.createChar(0, seco_esq_1);
      lcd.createChar(1, seco_dir_1);
      lcd.createChar(2, seco_esq_2);
      lcd.createChar(3, seco_dir_2);
      break;
    default:
      lcd.createChar(0, ideal_esq_1);
      lcd.createChar(1, ideal_dir_1);
      lcd.createChar(2, ideal_esq_2);
      lcd.createChar(3, ideal_dir_2);
      break;
  }
}

Conforto avaliarConforto(float t, float h) {
  if (t < 16.0) return FRIO;
  if (t > 29.0) return QUENTE;
  if (h < 30.0) return SECO;
  if (h > 75.0) return UMIDO;
  if (t >= 18.0 && t <= 26.0 && h >= 40.0 && h <= 60.0) return IDEAL;
  return BOM;
}

void atualizarRodape(Conforto c) {
  static Conforto confAnterior = BOM;
  
  if (c != confAnterior) {
    carregarAnimacao(c);
    confAnterior = c;
    frameAtual = 0;
  }
  
  lcd.setCursor(0, 3);
  lcd.print(F("Status: "));
  
  switch(c) {
    case IDEAL:  
      lcd.print(F("IDEAL    ")); 
      break;
    case QUENTE: 
      lcd.print(F("QUENTE   ")); 
      break;
    case FRIO:   
      lcd.print(F("FRIO     ")); 
      break;
    case UMIDO:  
      lcd.print(F("MTO UMIDO")); 
      break;
    case SECO:   
      lcd.print(F("MTO SECO ")); 
      break;
    default:     
      lcd.print(F("NORMAL   ")); 
      break;
  }

  lcd.setCursor(18, 3);
  if (frameAtual == 0) {
    lcd.write(0);
    lcd.write(1);
  } else {
    lcd.write(2);
    lcd.write(3);
  }
}

void desenhaBarraCompacta(float h) {
  lcd.setCursor(11, 2); 
  lcd.print(F("["));
  
  int totalBlocos = 6;
  int preenchidos = map((int)h, 0, 100, 0, totalBlocos);
  
  for(int i=0; i<totalBlocos; i++) {
    if(i < preenchidos) lcd.write(255); 
    else lcd.print(F("."));
  }
  lcd.print(F("]"));
}

void atualizarDataHora() {
  if (!rtc.IsDateTimeValid()) {
    lcd.setCursor(14, 0);
    lcd.print(F("--:--"));
    lcd.setCursor(14, 1);
    lcd.print(F("--/--"));
    return;
  }
  
  RtcDateTime now = rtc.GetDateTime();
  
  // Linha 0: Hora (HH:MM) - posições 14-18
  lcd.setCursor(14, 0);
  char horaStr[6];
  sprintf(horaStr, "%02d:%02d", now.Hour(), now.Minute());
  lcd.print(horaStr);
  
  // Linha 1: Data (DD/MM) - posições 14-18
  lcd.setCursor(14, 1);
  char dataStr[6];
  sprintf(dataStr, "%02d/%02d", now.Day(), now.Month());
  lcd.print(dataStr);
}

bool connectToWiFi() {
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  int dots = 0;

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 10000) return false;
    
    lcd.setCursor(0, 1);
    lcd.print("Aguarde");
    for (int i = 0; i < dots; i++) {
      lcd.print(".");
    }
    for (int i = dots; i < 3; i++) {
      lcd.print(" ");
    }
    
    dots = (dots + 1) % 4;
    delay(500);
  }

  return true;
}

bool updateRTCfromNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(2000);

  time_t now = time(nullptr);
  if (now < 100000) return false;

  struct tm* timeinfo = localtime(&now);

  RtcDateTime ntpTime(
    timeinfo->tm_year + 1900,
    timeinfo->tm_mon + 1,
    timeinfo->tm_mday,
    timeinfo->tm_hour,
    timeinfo->tm_min,
    timeinfo->tm_sec
  );

  rtc.SetDateTime(ntpTime);
  return true;
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  
  // --- TELA DE INICIALIZAÇÃO ---
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("Estacao IoT");
  lcd.setCursor(5, 2);
  lcd.print("Iniciando...");
  delay(2000);

  // Inicializa RTC
  rtc.Begin();

  // --- CONECTANDO WIFI ---
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Conectando WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Aguarde...");

  if (connectToWiFi()) {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("WiFi Conectado!");
    lcd.setCursor(3, 2);
    lcd.print("Sincronizando");
    delay(1500);

    if (updateRTCfromNTP()) {
      Serial.println("RTC atualizado via NTP!");
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("Sincronizado!");
      lcd.setCursor(4, 2);
      lcd.print("Via Internet");
      delay(2000);
    } else {
      Serial.println("Falha NTP.");
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("Erro NTP");
      lcd.setCursor(3, 2);
      lcd.print("Usando RTC");
      delay(2000);
    }
  } else {
    Serial.println("WiFi falhou.");
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("WiFi Falhou");
    lcd.setCursor(3, 2);
    lcd.print("Usando RTC");
    delay(2000);
  }

  // Desconecta WiFi para economizar energia
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  // Inicializa DHT
  dht.begin();
  criarPersonagens();

  // --- CARREGANDO TELA PRINCIPAL ---
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Carregando");
  lcd.setCursor(6, 2);
  lcd.print("Sensores...");
  delay(1500);

  // Layout estático
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Temp:"));
  lcd.setCursor(0, 1); lcd.print(F("Sens:"));
  lcd.setCursor(0, 2); lcd.print(F("Umid:"));
  
  atualizarDataHora();
  carregarAnimacao(BOM);
  atualizarRodape(BOM);
}

void loop() {
  unsigned long atual = millis();

  // Atualização Data/Hora
  if (atual - ultimaAtualizacaoData >= INTERVALO_DATA) {
    ultimaAtualizacaoData = atual;
    atualizarDataHora();
  }

  // Animação do Emoji
  if (atual - ultimaAnimacao >= INTERVALO_ANIMACAO) {
    ultimaAnimacao = atual;
    frameAtual = (frameAtual == 0) ? 1 : 0;
    atualizarRodape(confAtual);
  }

  // Leitura do Sensor DHT11
  if (atual - ultimaLeitura >= INTERVALO_LEITURA) {
    ultimaLeitura = atual;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      errosConsecutivos++;
      if (errosConsecutivos > 3) {
        lcd.setCursor(6, 0); lcd.print(F("--.-"));
      }
      return;
    }
    
    errosConsecutivos = 0;
    ultimaBoaLeitura = atual;
    
    confAtual = avaliarConforto(t, h);
    float hic = dht.computeHeatIndex(t, h, false);

    // Linha 0: Temperatura (fixa em 6 chars: "99.9°C")
    if (fabs(t - tempAnterior) >= 0.1) {
      lcd.setCursor(6, 0);
      char tempStr[7];
      sprintf(tempStr, "%4.1f", t); // Formata com 4 chars antes do ponto
      lcd.print(tempStr);
      lcd.write(7);
      lcd.print(F("C"));
      tempAnterior = t;
    }

    // Linha 1: Sensação Térmica (fixa em 6 chars: "99.9°C")
    lcd.setCursor(6, 1);
    char hicStr[7];
    sprintf(hicStr, "%4.1f", hic);
    lcd.print(hicStr);
    lcd.write(7);
    lcd.print(F("C")); 

    // Linha 2: Umidade + Barra
    if (fabs(h - umidAnterior) >= 1.0) {
      lcd.setCursor(6, 2);
      lcd.print(h, 0); 
      lcd.print(F("%"));
      
      if(h < 100) lcd.print(" ");
      
      desenhaBarraCompacta(h);
      umidAnterior = h;
    }
  }

  // Reiniciar se travar
  if (millis() - ultimaBoaLeitura > 40000UL) ESP.restart();
}
