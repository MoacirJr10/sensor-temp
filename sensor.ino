#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN D4
#define DHTTYPE DHT11
#define ERRO_MAX 3           // Número de erros consecutivos antes de alertar
#define INTERVALO_LEITURA 2000  // Intervalo entre leituras (ms)

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Variáveis para controle
unsigned long ultimaLeitura = 0;
int errosConsecutivos = 0;
float tempAnterior = -999;
float umidAnterior = -999;
bool erroExibido = false;  // Controla se mensagem de erro está na tela

// Caractere customizado para grau (°)
byte grau[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

void setup() {
  Serial.begin(115200);  // Para debug

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, grau);  // Criar caractere customizado

  dht.begin();

  // Mensagem inicial
  lcd.setCursor(0, 0);
  lcd.print(F("Estacao IoT"));
  lcd.setCursor(0, 1);
  lcd.print(F("Iniciando..."));
  delay(2000);

  // Limpar e preparar layout fixo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Temp:"));
  lcd.setCursor(0, 1);
  lcd.print(F("Umid:"));
}

void loop() {
  unsigned long agora = millis();

  // Controle de intervalo de leitura
  if (agora - ultimaLeitura < INTERVALO_LEITURA) {
    return;
  }
  ultimaLeitura = agora;

  // Leitura dos sensores
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Tratamento de erro
  if (isnan(h) || isnan(t)) {
    errosConsecutivos++;
    Serial.println(F("Erro na leitura do DHT11"));

    if (errosConsecutivos >= ERRO_MAX && !erroExibido) {
      lcd.setCursor(0, 2);
      lcd.print(F("ERRO: Sensor DHT11!"));
      lcd.setCursor(0, 3);
      lcd.print(F("Verifique conexao  "));
      erroExibido = true;
    }
    return;
  }

  // Reset contador de erros e limpar mensagem se estava com erro
  if (errosConsecutivos >= ERRO_MAX && erroExibido) {
    lcd.setCursor(0, 2);
    lcd.print(F("                    "));
    lcd.setCursor(0, 3);
    lcd.print(F("                    "));
    erroExibido = false;
  }
  errosConsecutivos = 0;

  // Atualizar temperatura apenas se mudou
  if (abs(t - tempAnterior) > 0.1) {
    lcd.setCursor(6, 0);
    lcd.print(F("              ")); // Limpar área
    lcd.setCursor(6, 0);
    lcd.print(t, 1);  // 1 casa decimal
    lcd.write(byte(0));  // Símbolo de grau
    lcd.print(F("C"));
    tempAnterior = t;
  }

  // Atualizar umidade apenas se mudou
  if (abs(h - umidAnterior) > 0.1) {
    lcd.setCursor(6, 1);
    lcd.print(F("              ")); // Limpar área
    lcd.setCursor(6, 1);
    lcd.print(h, 1);  // 1 casa decimal
    lcd.print(F(" %"));
    umidAnterior = h;
  }

  // Indicador de sensação térmica (linha 2)
  lcd.setCursor(0, 2);
  float hic = dht.computeHeatIndex(t, h, false);
  lcd.print(F("Sens:"));
  lcd.print(hic, 1);
  lcd.write(byte(0));
  lcd.print(F("C "));

  // Conforto térmico (linha 3)
  lcd.setCursor(0, 3);
  if (t >= 18 && t <= 24 && h >= 40 && h <= 60) {
    lcd.print(F("Conforto: IDEAL   "));
  } else if (t > 30) {
    lcd.print(F("Conforto: QUENTE  "));
  } else if (t < 15) {
    lcd.print(F("Conforto: FRIO    "));
  } else if (h > 70) {
    lcd.print(F("Conforto: UMIDO   "));
  } else if (h < 30) {
    lcd.print(F("Conforto: SECO    "));
  } else {
    lcd.print(F("Conforto: BOM     "));
  }

  // Debug serial
  Serial.print(F("Temp: "));
  Serial.print(t);
  Serial.print(F("°C | Umid: "));
  Serial.print(h);
  Serial.println(F("%"));
}