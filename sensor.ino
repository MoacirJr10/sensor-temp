#include <Wire.h>
#include <LiquidCrystal_I2C.h>        // ← Versão do Frank de Brabander
#include <DHT.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <WiFiManager.h>

// ==================== HARDWARE ====================
#define DHTPIN D4
#define DHTTYPE DHT11
ThreeWire myWire(D6, D5, D7);                 // IO=D6, SCLK=D5, CE=D7
RtcDS1302<ThreeWire> rtc(myWire);
LiquidCrystal_I2C lcd(0x27, 20, 4);           // Endereço comum do LCD 20x4
DHT dht(DHTPIN, DHTTYPE);

// ==================== TEMPOS ====================
#define INTERVALO_LEITURA       2000UL
#define INTERVALO_ANIMACAO      500UL
#define INTERVALO_DATA          10000UL
#define TIMEOUT_RESTART         40000UL // Tempo limite para reiniciar se não houver leitura

// ==================== VARIÁVEIS ====================
unsigned long ultimaLeitura = 0, ultimaBoaLeitura = 0;
unsigned long ultimaAnimacao = 0, ultimaAtualizacaoData = 0;
float tempAnterior = -999.0, umidAnterior = -999.0;
int errosConsecutivos = 0;
byte frameAtual = 0; // 0 ou 1 para animação de conforto

enum Conforto { IDEAL, FRIO, QUENTE, UMIDO, SECO, BOM };
Conforto confAtual = BOM;

// ==================== BITMAPS (todos os seus) ====================
// IDEAL - Coração
byte ideal_esq_1[8] = {B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000};
byte ideal_dir_1[8] = {B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000};
byte ideal_esq_2[8] = {B00000, B01010, B11111, B11111, B01110, B00100, B00000, B00000};
byte ideal_dir_2[8] = {B00000, B01010, B11111, B11111, B01110, B00100, B00000, B00000};

// QUENTE - Fogo
byte quente_esq_1[8] = {B00100, B01010, B01010, B10001, B10001, B11011, B01110, B00100};
byte quente_dir_1[8] = {B00100, B01010, B01010, B10001, B10001, B11011, B01110, B00100};
byte quente_esq_2[8] = {B01000, B10100, B10100, B01010, B01010, B10101, B11011, B01110};
byte quente_dir_2[8] = {B00010, B00101, B00101, B01010, B01010, B10101, B11011, B01110};

// FRIO - Floco
byte frio_esq_1[8] = {B00100, B10101, B01110, B11111, B01110, B10101, B00100, B00000};
byte frio_dir_1[8] = {B00100, B10101, B01110, B11111, B01110, B10101, B00100, B00000};
byte frio_esq_2[8] = {B10001, B01010, B00100, B11111, B00100, B01010, B10001, B00000};
byte frio_dir_2[8] = {B10001, B01010, B00100, B11111, B00100, B01010, B10001, B00000};

// ÚMIDO - Gotas
byte umido_esq_1[8] = {B00100, B00100, B01110, B01110, B11111, B11111, B01110, B00000};
byte umido_dir_1[8] = {B01000, B01000, B11100, B11100, B11110, B11110, B01100, B00000};
byte umido_esq_2[8] = {B00000, B00100, B00100, B01110, B01110, B11111, B11111, B01110};
byte umido_dir_2[8] = {B00000, B01000, B01000, B11100, B11100, B11110, B11110, B01100};

// SECO - Sol
byte seco_esq_1[8] = {B10101, B01000, B00100, B11111, B11111, B00100, B01000, B10101};
byte seco_dir_1[8] = {B10101, B00010, B00100, B11111, B11111, B00100, B00010, B10101};
byte seco_esq_2[8] = {B01010, B10001, B00100, B11111, B11111, B00100, B10001, B01010};
byte seco_dir_2[8] = {B01010, B10001, B00100, B11111, B11111, B00100, B10001, B01010};

// Grau Celsius (índice 7)
byte char_grau[8] = {B00110, B01001, B01001, B00110, B00000, B00000, B00000, B00000};

// ==================== FUNÇÕES ====================
void criarPersonagens() {
  lcd.createChar(7, char_grau); // índice 7 para °
}

void carregarAnimacao(Conforto c) {
  switch(c) {
    case IDEAL:   lcd.createChar(0, ideal_esq_1); lcd.createChar(1, ideal_dir_1); lcd.createChar(2, ideal_esq_2); lcd.createChar(3, ideal_dir_2); break;
    case QUENTE:  lcd.createChar(0, quente_esq_1); lcd.createChar(1, quente_dir_1); lcd.createChar(2, quente_esq_2); lcd.createChar(3, quente_dir_2); break;
    case FRIO:    lcd.createChar(0, frio_esq_1);   lcd.createChar(1, frio_dir_1);   lcd.createChar(2, frio_esq_2);   lcd.createChar(3, frio_dir_2);   break;
    case UMIDO:   lcd.createChar(0, umido_esq_1);  lcd.createChar(1, umido_dir_1);  lcd.createChar(2, umido_esq_2);  lcd.createChar(3, umido_dir_2);  break;
    case SECO:    lcd.createChar(0, seco_esq_1);   lcd.createChar(1, seco_dir_1);   lcd.createChar(2, seco_esq_2);   lcd.createChar(3, seco_dir_2);   break;
    default:      lcd.createChar(0, ideal_esq_1); lcd.createChar(1, ideal_dir_1); lcd.createChar(2, ideal_esq_2); lcd.createChar(3, ideal_dir_2); break;
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

// Atualiza apenas o texto de status (não inclui ícone)
void atualizarStatusTexto(Conforto c) {
  static Conforto confAnterior = BOM;

  // Se mudou o conforto, recarrega os bitmaps
  if (c != confAnterior) {
    carregarAnimacao(c);
    confAnterior = c;
    frameAtual = 0;
  }
  
  // Escreve o texto do status (colunas 8 a 17)
  lcd.setCursor(8, 3);
  switch(c) {
    case IDEAL:  lcd.print(F("IDEAL     ")); break;
    case QUENTE: lcd.print(F("QUENTE    ")); break;
    case FRIO:   lcd.print(F("FRIO      ")); break;
    case UMIDO:  lcd.print(F("MTO UMID  ")); break;
    case SECO:   lcd.print(F("MTO SECO  ")); break;
    default:     lcd.print(F("NORMAL    ")); break;
  }
}

// Desenha apenas o ícone do status (colunas 18-19)
void desenharIconeStatus() {
  lcd.setCursor(18, 3);
  lcd.write(frameAtual == 0 ? 0 : 2);
  lcd.write(frameAtual == 0 ? 1 : 3);
}

// BARRA DE UMIDADE (compacta) — usa 6 blocos
void desenhaBarraCompacta(float h) {
  lcd.setCursor(11, 2);
  lcd.print(F("["));
  int blocos = 6;
  int preenchidos = map(constrain((int)h, 0, 100), 0, 100, 0, blocos);
  for (int i = 0; i < blocos; i++) {
    if (i < preenchidos) lcd.write(255);
    else lcd.print(".");
  }
  lcd.print(F("]"));
}

void atualizarDataHora() {
  // Limpa área de data/hora (colunas 14..19 nas linhas 0 e 1)
  lcd.setCursor(14, 0); lcd.print(F("      "));
  lcd.setCursor(14, 1); lcd.print(F("      "));

  if (!rtc.IsDateTimeValid()) {
    lcd.setCursor(14, 0); lcd.print(F("--:--"));
    lcd.setCursor(14, 1); lcd.print(F("--/--"));
    return;
  }
  RtcDateTime now = rtc.GetDateTime();
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d", now.Hour(), now.Minute());
  lcd.setCursor(14, 0); lcd.print(buf);
  snprintf(buf, sizeof(buf), "%02d/%02d", now.Day(), now.Month());
  lcd.setCursor(14, 1); lcd.print(buf);
}

// Mostra estado na linha 3 (colunas 0-7) - "Status:"
void mostrarEstadoStatus() {
  lcd.setCursor(0, 3);
  if (errosConsecutivos > 0) {
    // Exibe erro de leitura
    lcd.print(F("Status:"));
  } else if (millis() - ultimaBoaLeitura > 5000UL) {
    // Sem leitura recente
    lcd.print(F("Status:"));
  } else {
    // Leitura OK
    lcd.print(F("Status:"));
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  lcd.init(); lcd.backlight(); lcd.clear();
  lcd.setCursor(4, 1); lcd.print("Estacao IoT");
  lcd.setCursor(7, 2); lcd.print("vFinal");
  delay(1200);

  WiFiManager wm;
  // wm.resetSettings(); // ← descomente só para limpar tudo

  wm.setAPCallback([](WiFiManager*) { telaPrimeiraVez(); });
  lcd.clear(); lcd.setCursor(0, 1); lcd.print("Conectando WiFi...");

  if (!wm.autoConnect("ESTACAO-IOT", "12345678")) {
    lcd.clear(); lcd.setCursor(3, 1); lcd.print("WiFi Falhou");
    lcd.setCursor(1, 2); lcd.print("Reiniciando..."); delay(3000); ESP.restart();
  }

  lcd.clear(); lcd.setCursor(1, 1); lcd.print("WiFi Conectado!");
  lcd.setCursor(3, 2); lcd.print(WiFi.SSID()); delay(1200);

  lcd.setCursor(0, 3); lcd.print("Hora via net... ");
  lcd.print(sincronizarNTP() ? "OK" : "OFF");
  WiFi.disconnect(true); WiFi.mode(WIFI_OFF); delay(600);

  rtc.Begin(); dht.begin();
  criarPersonagens(); carregarAnimacao(BOM);

  lcd.clear();
  // Campos fixos - escreve apenas UMA VEZ no setup
  lcd.setCursor(0, 0); lcd.print(F("Temp:"));
  lcd.setCursor(0, 1); lcd.print(F("Sens:"));
  lcd.setCursor(0, 2); lcd.print(F("Umid:"));
  lcd.setCursor(0, 3); lcd.print(F("Status:"));
  
  // Inicializa as áreas fixas
  atualizarDataHora();
  mostrarEstadoStatus();
  atualizarStatusTexto(BOM);
  desenharIconeStatus();
  
  // Mostra valores iniciais mais próximos dos labels
  lcd.setCursor(6, 0); lcd.print(F("--.- "));
  lcd.write(7); // Símbolo de grau
  
  lcd.setCursor(6, 1); lcd.print(F("--.- "));
  lcd.write(7); // Símbolo de grau
  
  lcd.setCursor(6, 2); lcd.print(F("--%  "));
  lcd.setCursor(11, 2); lcd.print(F("[......]"));
}

void telaPrimeiraVez() {
  lcd.clear();
  lcd.setCursor(1, 0); lcd.print("CONFIGURE O WIFI");
  lcd.setCursor(0, 1); lcd.print("1- Conecte-se a:");
  lcd.setCursor(3, 2); lcd.print("ESTACAO-IOT");
  lcd.setCursor(0, 3); lcd.print("2- Abra 192.168.4.1");
  delay(12000);
}

bool sincronizarNTP() {
  configTime(-3 * 3600, 0, "pool.ntp.org");
  time_t agora = time(nullptr);
  int tent = 0;
  while (agora < 100000 && tent++ < 40) { delay(500); agora = time(nullptr); }
  if (agora < 100000) return false;
  struct tm timeinfo;
  localtime_r(&agora, &timeinfo);
  RtcDateTime ntpTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  rtc.SetDateTime(ntpTime);
  return true;
}

// ==================== LOOP PERFEITO ====================
void loop() {
  unsigned long agora = millis();

  // Animação de ícone de status (toggle frame) — a cada INTERVALO_ANIMACAO
  if (agora - ultimaAnimacao >= INTERVALO_ANIMACAO) {
    ultimaAnimacao = agora;
    frameAtual = 1 - frameAtual;
    // Atualiza apenas o ícone (colunas 18-19)
    desenharIconeStatus();
  }

  // Atualização de Data/Hora
  if (agora - ultimaAtualizacaoData >= INTERVALO_DATA) {
    ultimaAtualizacaoData = agora;
    atualizarDataHora();
  }

  // Leitura do DHT
  if (agora - ultimaLeitura >= INTERVALO_LEITURA) {
    ultimaLeitura = agora;
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      // erro na leitura
      errosConsecutivos++;
      
      // Atualiza estado do status
      mostrarEstadoStatus();
      
      // Exibe valores de erro mais próximos dos labels
      lcd.setCursor(6, 0); lcd.print(F("--.- "));
      lcd.write(7); // Símbolo de grau
      
      lcd.setCursor(6, 1); lcd.print(F("--.- "));
      lcd.write(7); // Símbolo de grau
      
      lcd.setCursor(6, 2); lcd.print(F("--%  "));
      lcd.setCursor(11, 2); lcd.print(F("[......]"));
      
      // Atualiza texto do status com estado neutro
      confAtual = BOM;
      atualizarStatusTexto(confAtual);
      desenharIconeStatus();
      return; // sai desta iteração
    }

    // Leitura OK
    errosConsecutivos = 0;
    ultimaBoaLeitura = agora;
    
    // Atualiza estado do status
    mostrarEstadoStatus();
    
    confAtual = avaliarConforto(t, h);
    float sensacao = dht.computeHeatIndex(t, h, false);

    // Temperatura (apenas reescreve quando alteração significativa)
    if (fabs(t - tempAnterior) >= 0.1 || tempAnterior == -999.0) {
      char buf[12];
      snprintf(buf, sizeof(buf), " %4.1f", t);
      // Limpa e escreve mais próximo do label
      lcd.setCursor(6, 0); lcd.print(F("     "));
      lcd.setCursor(6, 0); lcd.print(buf);
      lcd.write(7); // Símbolo de grau
      tempAnterior = t;
    }

    // Sensação térmica (sempre atualiza)
    {
      char buf[12];
      snprintf(buf, sizeof(buf), " %4.1f", sensacao);
      lcd.setCursor(6, 1); lcd.print(F("     "));
      lcd.setCursor(6, 1); lcd.print(buf);
      lcd.write(7); // Símbolo de grau
    }

    // Umidade (atualiza quando muda 1% ou inicial)
    if (fabs(h - umidAnterior) >= 1.0 || umidAnterior == -999.0) {
      char buf[6];
      snprintf(buf, sizeof(buf), "%3d%%", (int)h);
      lcd.setCursor(6, 2); lcd.print(F("     "));
      lcd.setCursor(6, 2); lcd.print(buf);
      desenhaBarraCompacta(h);
      umidAnterior = h;
    }
    
    // Atualiza o texto e ícone do status
    atualizarStatusTexto(confAtual);
    desenharIconeStatus();
  }

  // Verifica timeout de leitura (reinicia o ESP8266)
  if (millis() - ultimaBoaLeitura > TIMEOUT_RESTART && ultimaBoaLeitura > 0) {
     lcd.clear(); lcd.setCursor(1, 1); lcd.print("Erro DHT persistente");
     lcd.setCursor(1, 2); lcd.print("Reiniciando...");
     delay(3000);
     ESP.restart();
  }
}
