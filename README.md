# 🌡 Estação IoT com Wemos D1 Mini + DHT11 + LCD 20x4 I2C

Este projeto implementa uma **estação de monitoramento IoT** utilizando o microcontrolador **Wemos D1 Mini (ESP8266)**, o **sensor de temperatura e umidade DHT11** e um **display LCD 20x4 com módulo I2C**.

Ele exibe **temperatura, umidade, sensação térmica e nível de conforto térmico** diretamente no display, com tratamento de erros automáticos e atualização inteligente de dados.

---

##  Funcionalidades

-  Exibe:
    - Temperatura (°C)
    - Umidade relativa (%)
    - Sensação térmica calculada automaticamente
    - Nível de conforto: IDEAL, FRIO, QUENTE, ÚMIDO ou SECO
-  Atualização automática a cada 2 segundos
-  Detecção e aviso de falha no sensor DHT11
-  Cálculo otimizado: só atualiza quando há variação real de valores

---

##  Componentes Utilizados

| Componente | Quantidade | Observações |
|-------------|-------------|-------------|
| Wemos D1 Mini (ESP8266) | 1 | Microcontrolador Wi-Fi |
| Sensor DHT11 | 1 | Leitura de temperatura e umidade |
| Display LCD 20x4 com módulo I2C | 1 | Interface visual de 4 linhas |
| Jumpers macho-fêmea | 6 | Conexões entre componentes |
| Fonte 5V | 1 | Pode ser via USB do próprio Wemos |

---

## ⚡ Ligações dos Componentes

### 🔹 DHT11 → Wemos D1 Mini
| DHT11 | Wemos D1 Mini |
|--------|----------------|
| VCC    | 5V             |
| GND    | GND            |
| DATA   | D4             |

### 🔹 LCD I2C → Wemos D1 Mini
| LCD I2C | Wemos D1 Mini |
|----------|----------------|
| VCC      | 5V             |
| GND      | GND            |
| SDA      | D2 (GPIO4)     |
| SCL      | D1 (GPIO5)     |

---

##  Bibliotecas Necessárias

Antes de compilar, instale as seguintes bibliotecas no **Arduino IDE**:

1. **DHT sensor library** — por Adafruit  
   *(Gerenciar bibliotecas → procurar “DHT sensor library”)*
2. **Adafruit Unified Sensor**  
   *(Dependência do DHT)*
3. **LiquidCrystal I2C** — por Frank de Brabander ou Marco Schwartz

---

##  Configuração da IDE Arduino

- **Placa:** `LOLIN(WEMOS) D1 R2 & mini`
- **Velocidade de upload:** `115200`
- **Flash size:** `4MB (FS: 1MB OTA:~1019KB)`
- **Porta:** Selecione a porta COM correspondente ao seu Wemos D1 Mini

---

##  Como Usar

1. Conecte os componentes conforme o esquema acima.
2. Faça o upload do código para o Wemos D1 Mini.
3. Abra o **Monitor Serial** (115200 baud) para visualizar os logs.
4. Observe os valores atualizados no display LCD.

---

##  Código Principal

Arquivo: [`sensor.ino`]

O código faz:
- Leitura do DHT11 a cada 2 segundos;
- Exibição dinâmica no LCD;
- Cálculo de conforto térmico;
- Mensagens de erro se o sensor falhar.

---

##  Exemplo de Saída Serial

Temp: 25.3°C | Umid: 56.8%
Temp: 25.2°C | Umid: 56.9%
Temp: 25.2°C | Umid: 57.0%


---

##  Possíveis Expansões

- Envio dos dados via Wi-Fi (HTTP ou MQTT);
- Integração com **ThingSpeak**, **Home Assistant** ou **Blynk**;
- Adição de sensores como BMP180 (pressão) ou LDR (luminosidade);
- Gravação de logs em cartão SD;
- Alerta visual com LEDs ou buzzer quando temperatura ultrapassar limites.

---

##  Layout Simplificado

┌─────────────────┐
│ Temp: 24.6°C    │
│ Umid: 52.3%     │
│ Sens: 25.8°C    │
│ Conforto: IDEAL │
└─────────────────┘

---

##  Autor

**Moacir Pereira**  
 Projeto pessoal de automação IoT e monitoramento ambiental.  
 Atualizado em: Outubro/2025

---

##  Licença

Este projeto é de código aberto sob a licença **MIT**.  
Sinta-se livre para usar, modificar e compartilhar! 

---

