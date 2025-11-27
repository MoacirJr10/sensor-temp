# Estação IoT com Wemos D1 Mini

Este projeto implementa uma **estação de monitoramento ambiental** utilizando o microcontrolador **Wemos D1 Mini (ESP8266)**, sensor **DHT11** para temperatura e umidade, **display LCD 20x4 I2C** e **módulo RTC DS1302** para data e hora.

O sistema exibe temperatura, umidade, sensação térmica, nível de conforto e data/hora em tempo real, com emojis animados indicando o estado climático.

---

## Funcionalidades

- Exibe temperatura em °C com símbolo customizado
- Mostra umidade relativa com barra gráfica visual
- Calcula e exibe sensação térmica automaticamente
- Níveis de conforto: IDEAL, FRIO, QUENTE, ÚMIDO ou SECO
- Emojis animados de 2 colunas para cada estado climático
- Data e hora em tempo real via RTC DS1302
- Atualização automática a cada 2 segundos
- Detecção e tratamento de falhas no sensor
- Sistema de reinicialização automática em caso de travamento

---

## Componentes Utilizados

| Componente | Quantidade | Observações |
|------------|------------|-------------|
| Wemos D1 Mini (ESP8266) | 1 | Microcontrolador Wi-Fi |
| Sensor DHT11 | 1 | Temperatura e umidade |
| Display LCD 20x4 I2C | 1 | Endereço padrão 0x27 |
| Módulo RTC DS1302 | 1 | Relógio em tempo real |
| Bateria CR2032 | 1 | Para o módulo RTC |
| Jumpers | 10-12 | Conexões entre componentes |
| Fonte 5V | 1 | Via USB do Wemos |

---

## Ligações dos Componentes

### DHT11 → Wemos D1 Mini
| DHT11 | Wemos D1 Mini |
|-------|---------------|
| VCC   | 5V            |
| GND   | GND           |
| DATA  | D4 (GPIO2)    |

### LCD I2C → Wemos D1 Mini
| LCD I2C | Wemos D1 Mini |
|---------|---------------|
| VCC     | 5V            |
| GND     | GND           |
| SDA     | D2 (GPIO4)    |
| SCL     | D1 (GPIO5)    |

### RTC DS1302 → Wemos D1 Mini
| DS1302 | Wemos D1 Mini |
|--------|---------------|
| VCC    | 5V            |
| GND    | GND           |
| CLK    | D5 (GPIO14)   |
| DAT    | D6 (GPIO12)   |
| RST    | D7 (GPIO13)   |

---

## Bibliotecas Necessárias

Instale as seguintes bibliotecas no **Arduino IDE** através do Gerenciador de Bibliotecas:

1. **DHT sensor library** por Adafruit
2. **Adafruit Unified Sensor** (dependência do DHT)
3. **LiquidCrystal I2C** por Frank de Brabander
4. **Rtc by Makuna** (para DS1302/DS3231)

---

## Configuração da IDE Arduino

- **Placa:** LOLIN(WEMOS) D1 R2 & mini
- **Velocidade de upload:** 115200
- **Flash size:** 4MB (FS: 1MB OTA:~1019KB)
- **Porta:** Selecione a porta COM do seu Wemos

---

## Configuração Inicial do RTC

### Primeira vez - Acertar data e hora:

1. No código, localize a função `setup()`
2. Descomente a linha de configuração do RTC:
```cpp
RtcDateTime compiled = RtcDateTime(2024, 11, 27, 15, 30, 0);
//                                 ano, mês, dia, hora, min, seg
rtc.SetDateTime(compiled);
```
3. Ajuste para a data e hora atuais
4. Faça o upload do código
5. Comente novamente as linhas e faça novo upload

A bateria CR2032 manterá a hora mesmo sem alimentação.

---

## Layout do Display

```
Temp: 25.5°C  14:35
Sens: 26.3°C  27/11
Umid: 60% [||||..]
Status: IDEAL    ❤️
```

### Linha 0
- Temperatura atual com símbolo de grau
- Hora no formato HH:MM

### Linha 1
- Sensação térmica calculada
- Data no formato DD/MM

### Linha 2
- Umidade percentual
- Barra gráfica visual de 6 blocos

### Linha 3
- Status do conforto térmico
- Emoji animado (2 colunas) representando o estado

---

## Emojis Animados

Cada condição climática possui um emoji animado de 2 frames:

- **IDEAL:** Coração pulsante
- **QUENTE:** Fogo crepitante
- **FRIO:** Floco de neve girando
- **ÚMIDO:** Gotas de água caindo
- **SECO:** Sol radiante

Os emojis alternam frames a cada 500ms para criar efeito de animação.

---

## Critérios de Conforto Térmico

| Condição | Temperatura | Umidade |
|----------|-------------|---------|
| FRIO     | < 16°C      | -       |
| QUENTE   | > 29°C      | -       |
| SECO     | -           | < 30%   |
| ÚMIDO    | -           | > 75%   |
| IDEAL    | 18-26°C     | 40-60%  |
| NORMAL   | Demais casos | -      |

---

## Código Principal

O código implementa:

- Sistema de leitura do DHT11 a cada 2 segundos
- Atualização inteligente: só redesenha se houver mudança
- Animação dos emojis independente das leituras
- Atualização de data/hora a cada 10 segundos
- Tratamento de erros consecutivos do sensor
- Reinicialização automática após 40 segundos sem leitura válida
- Caracteres customizados para emojis e símbolo de grau

---

## Possíveis Expansões

- Integração Wi-Fi para envio de dados (MQTT/HTTP)
- Conexão com Home Assistant ou Blynk
- Adição de sensores de pressão atmosférica (BMP180/BME280)
- Sensor de luminosidade (LDR ou BH1750)
- Gravação de logs históricos em cartão SD
- Alarmes sonoros ou visuais para condições críticas
- Gráficos de tendência de temperatura/umidade
- Display OLED adicional para informações extras

---

## Solução de Problemas

### Display não acende
- Verifique conexões SDA/SCL
- Confirme endereço I2C (geralmente 0x27 ou 0x3F)
- Ajuste o contraste com o potenciômetro do módulo I2C

### Sensor DHT11 retorna erro
- Verifique conexão no pino D4
- Aguarde 2 segundos entre leituras
- DHT11 tem precisão ±2°C e ±5% umidade

### RTC não mantém hora
- Verifique se a bateria CR2032 está instalada
- Confirme conexões CLK, DAT e RST
- Execute a configuração inicial do RTC

### Display mostra caracteres estranhos
- Verifique conexões I2C
- Confirme endereço 0x27 no código
- Reinicie o sistema

---

## Autor

**Moacir Pereira**  
Projeto pessoal de automação IoT e monitoramento ambiental.  
Atualizado em: Novembro/2024

---

## Licença

Este projeto é de código aberto sob a licença **MIT**.  
Sinta-se livre para usar, modificar e compartilhar.
