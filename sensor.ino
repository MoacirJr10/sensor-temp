#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <ESP8266WiFi.h> 

// --- Configurações ---
// --- Configurações ---
#define DHTPIN D4            
#define DHTTYPE DHT11

// Pinos do DS1302
#define RTC_CLK D5  // GPIO14
#define RTC_DAT D6  // GPIO12
#define RTC_RST D7  // GPIO13

LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(DHTPIN, DHTTYPE);
ThreeWire myWire(RTC_DAT, RTC_CLK, RTC_RST); // DAT, CLK, RST
RtcDS1302<ThreeWire> rtc(myWire);

#define INTERVALO_LEITURA 2000UL
#define INTERVALO_ANIMACAO 500UL  // Troca de frame a cada 500ms
#define INTERVALO_DATA 10000UL    // Atualiza data/hora a cada 10 segundos

// --- Variáveis ---
unsigned long ultimaLeitura = 0;
unsigned long ultimaBoaLeitura = 0;
unsigned long ultimaAnimacao = 0;
unsigned long ultimaAtualizacaoData = 0;
float tempAnterior = -999.0;
float umidAnterior = -999.0;
int errosConsecutivos = 0;
byte frameAtual = 0;  // Controla qual frame da animação exibir

enum Conforto { IDEAL, FRIO, QUENTE, UMIDO, SECO, BOM };
Conforto confAtual = BOM;

// --- BITMAPS ANIMADOS (2 frames por estado, 2 chars cada) ---
// IDEAL - Coração pulsante
byte ideal_esq_1[8] = {B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000};
byte ideal_dir_1[8] = {B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000};
byte ideal_esq_2[8] = {B00000, B01010, B11111, B11111, B01110, B00100, B00000, B00000};
byte ideal_dir_2[8] = {B00000, B01010, B11111, B11111, B01110, B00100, B00000, B00000};

// QUENTE - Fogo animado 🔥
byte quente_esq_1[8] = {B00100, B01010, B01010, B10001, B10001, B11011, B01110, B00100};
byte quente_dir_1[8] = {B00100, B01010, B01010, B10001, B10001, B11011, B01110, B00100};
byte quente_esq_2[8] = {B01000, B10100, B10100, B01010, B01010, B10101, B11011, B01110};
byte quente_dir_2[8] = {B00010, B00101, B00101, B01010, B01010, B10101, B11011, B01110};

// FRIO - Cristal de gelo/floco de neve ❄️
byte frio_esq_1[8] = {B00100, B10101, B01110, B11111, B01110, B10101, B00100, B00000};
byte frio_dir_1[8] = {B00100, B10101, B01110, B11111, B01110, B10101, B00100, B00000};
byte frio_esq_2[8] = {B10001, B01010, B00100, B11111, B00100, B01010, B10001, B00000};
byte frio_dir_2[8] = {B10001, B01010, B00100, B11111, B00100, B01010, B10001, B00000};

// ÚMIDO - Gotas de água caindo 💧
byte umido_esq_1[8] = {B00100, B00100, B01110, B01110, B11111, B11111, B01110, B00000};
byte umido_dir_1[8] = {B01000, B01000, B11100, B11100, B11110, B11110, B01100, B00000};
byte umido_esq_2[8] = {B00000, B00100, B00100, B01110, B01110, B11111, B11111, B01110};
byte umido_dir_2[8] = {B00000, B01000, B01000, B11100, B11100, B11110, B11110, B01100};

// SECO - Sol brilhante ☀️
byte seco_esq_1[8] = {B10101, B01000, B00100, B11111, B11111, B00100, B01000, B10101};
byte seco_dir_1[8] = {B10101, B00010, B00100, B11111, B11111, B00100, B00010, B10101};
byte seco_esq_2[8] = {B01010, B10001, B00100, B11111, B11111, B00100, B10001, B01010};
byte seco_dir_2[8] = {B01010, B10001, B00100, B11111, B11111, B00100, B10001, B01010};

// Grau Célsius
byte char_grau[8] = {B00110, B01001, B01001, B00110, B00000, B00000, B00000, B00000};

// --- Funções ---
void criarPersonagens() {
  // Slots 0-1: Frame 1 (esquerda, direita)
  // Slots 2-3: Frame 2 (esquerda, direita)
  // Slots 4-5: Reserva para outros estados
  // Slot 7: Grau Celsius
  
  lcd.createChar(7, char_grau);
  // Os demais serão carregados dinamicamente conforme o estado
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
    default: // BOM - usa IDEAL
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

// Atualiza a Linha 3 inteira (Texto + Emoji 2x no canto)
void atualizarRodape(Conforto c) {
  static Conforto confAnterior = BOM;
  
  // Se mudou o estado, recarrega a animação
  if (c != confAnterior) {
    carregarAnimacao(c);
    confAnterior = c;
    frameAtual = 0;
  }
  
  lcd.setCursor(0, 3);
  lcd.print(F("Status: ")); // 8 chars
  
  // Imprime texto e preenche com espaços
  // Total disponível para texto: col 8 até 16 (9 chars)
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

  // Emoji animado nas colunas 18 e 19
  lcd.setCursor(18, 3);
  if (frameAtual == 0) {
    lcd.write(0); // Esquerda frame 1
    lcd.write(1); // Direita frame 1
  } else {
    lcd.write(2); // Esquerda frame 2
    lcd.write(3); // Direita frame 2
  }
}

// Barra super compacta: [||..]
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

void atualizarData() {
  if (!rtc.IsDateTimeValid()) {
    lcd.setCursor(13, 0);
    lcd.print(F("--:--"));
    lcd.setCursor(13, 1);
    lcd.print(F("--/--"));
    return;
  }
  
  RtcDateTime now = rtc.GetDateTime();
  
  // Linha 0: Hora (HH:MM) - posições 13-17 (5 chars)
  lcd.setCursor(13, 0);
  if(now.Hour() < 10) lcd.print("0");
  lcd.print(now.Hour());
  lcd.print(":");
  if(now.Minute() < 10) lcd.print("0");
  lcd.print(now.Minute());
  
  // Linha 1: Data (DD/MM) - posições 13-17 (5 chars)
  lcd.setCursor(13, 1);
  if(now.Day() < 10) lcd.print("0");
  lcd.print(now.Day());
  lcd.print("/");
  if(now.Month() < 10) lcd.print("0");
  lcd.print(now.Month());
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  criarPersonagens();
  dht.begin();
  
  // Inicializa RTC DS1302
  rtc.Begin();
  
  // Verifica se o RTC está funcionando
  if (!rtc.IsDateTimeValid()) {
    Serial.println(F("RTC perdeu energia ou nao configurado!"));
    
    // Descomente a linha abaixo para configurar data/hora pela primeira vez
    // Formato: ano, mes, dia, hora, minuto, segundo
    // RtcDateTime compiled = RtcDateTime(2024, 11, 27, 14, 30, 0);
    // rtc.SetDateTime(compiled);
  }
  
  // Desabilita proteção de escrita
  rtc.SetIsWriteProtected(false);
  
  // Habilita o RTC
  rtc.SetIsRunning(true);

  lcd.clear();
  // Layout Estático
  lcd.setCursor(0, 0); lcd.print(F("Temp:"));
  lcd.setCursor(0, 1); lcd.print(F("Sens:"));
  lcd.setCursor(0, 2); lcd.print(F("Umid:"));
  
  // Exibe data na linha 0
  atualizarData();
  
  // Carrega animação inicial e exibe status
  carregarAnimacao(BOM);
  atualizarRodape(BOM);
}

void loop() {
  unsigned long atual = millis();

  // Atualização da Data (a cada 1 minuto)
  if (atual - ultimaAtualizacaoData >= INTERVALO_DATA) {
    ultimaAtualizacaoData = atual;
    atualizarData();
  }

  // Animação do Emoji
  if (atual - ultimaAnimacao >= INTERVALO_ANIMACAO) {
    ultimaAnimacao = atual;
    frameAtual = (frameAtual == 0) ? 1 : 0;  // Alterna entre frame 0 e 1
    atualizarRodape(confAtual);  // Redesenha com novo frame
  }

  // Leitura do Sensor
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

    // --- ATUALIZAÇÃO DA TELA ---

    // Linha 0: Temperatura
    if (fabs(t - tempAnterior) >= 0.1) {
      lcd.setCursor(6, 0);
      lcd.print(t, 1);
      lcd.write(7);
      lcd.print(F("C   "));
      tempAnterior = t;
    }

    // Linha 1: Sensação Térmica
    lcd.setCursor(6, 1);
    lcd.print(hic, 1);
    lcd.write(7);
    lcd.print(F("C   ")); 

    // Linha 2: Umidade + Barra Gráfica
    if (fabs(h - umidAnterior) >= 1.0) {
      lcd.setCursor(6, 2);
      lcd.print(h, 0); 
      lcd.print(F("%"));
      
      if(h < 100) lcd.print(" ");
      
      desenhaBarraCompacta(h);
      umidAnterior = h;
    }
    
    // Linha 3: Status + Emoji (já atualizado pela animação)
  }

  // Reiniciar se travar
  if (millis() - ultimaBoaLeitura > 40000UL) ESP.restart();
}
