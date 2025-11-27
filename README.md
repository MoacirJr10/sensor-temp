# Estação IoT com Wemos D1 Mini

Este projeto implementa uma **estação de monitoramento ambiental completa** utilizando o microcontrolador **Wemos D1 Mini (ESP8266)**, sensor **DHT11** para temperatura e umidade, **display LCD 20x4 I2C** e **módulo RTC DS1302** para data e hora em tempo real.

O sistema possui **sincronização automática via WiFi/NTP** na inicialização, exibe temperatura, umidade, sensação térmica, nível de conforto e data/hora, com emojis animados indicando o estado climático.

---

## Funcionalidades

- Sincronização automática de data/hora via WiFi e servidor NTP
- Exibe temperatura em °C com símbolo customizado
- Mostra umidade relativa com barra gráfica visual animada
- Calcula e exibe sensação térmica automaticamente
- Níveis de conforto: IDEAL, FRIO, QUENTE, ÚMIDO ou SECO
- Emojis animados de 2 colunas para cada estado climático
- Relógio em tempo real (RTC) mantém hora mesmo sem internet
- Telas de inicialização informativas durante boot
- Desliga WiFi após sincronização para economia de energia
- Atualização automática dos dados a cada 2 segundos
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

## Configuração Inicial

### 1. Configurar WiFi

No código, localize as linhas:
```cpp
const char* ssid = "SEU_WIFI_AQUI";
const char* password = "SUA_SENHA_AQUI";
```

Substitua pelos dados da sua rede WiFi.

### 2. Primeira Execução

Na primeira vez que ligar o sistema:
- O dispositivo tentará conectar ao WiFi (timeout de 10 segundos)
- Se conectar, sincronizará automaticamente via servidor NTP
- A hora será gravada no RTC DS1302
- WiFi será desligado para economizar energia
- Sistema funcionará normalmente apenas com o RTC

### 3. Ajuste Manual do RTC (Opcional)

Se não tiver WiFi disponível ou quiser configurar manualmente:

1. No código, localize a função `setup()`
2. Descomente e ajuste a linha:
```cpp
RtcDateTime manual(2024, 11, 27, 16, 00, 00);
//                 ano, mês, dia, hora, min, seg
rtc.SetDateTime(manual);
```
3. Faça o upload do código
4. Comente novamente a linha e faça novo upload

A bateria CR2032 manterá a hora mesmo sem alimentação.

---

## Sequência de Inicialização

O sistema exibe telas informativas durante o boot:

### 1. Tela Inicial
```
     Estacao IoT
       Iniciando...
```

### 2. Conectando WiFi
```
  Conectando WiFi
Aguarde...
```
*(Animação de pontos durante conexão)*

### 3. Sincronização
```
  WiFi Conectado!
   Sincronizando
```

### 4. Confirmação
```
   Sincronizado!
   Via Internet
```
*Ou em caso de falha:*
```
   WiFi Falhou
   Usando RTC
```

### 5. Carregando Sensores
```
   Carregando
    Sensores...
```

### 6. Tela Normal
```
Temp:  9.5°C  14:35
Sens: 10.3°C  27/11
Umid: 60% [||||..]
Status: IDEAL    ❤️
```

---

## Layout do Display

```
Temp: 25.5°C  14:35
Sens: 26.3°C  27/11
Umid: 60% [||||..]
Status: IDEAL    ❤️
```

### Linha 0
- Temperatura atual com símbolo de grau (posições 0-12)
- Hora no formato HH:MM (posições 14-18)

### Linha 1
- Sensação térmica calculada (posições 0-12)
- Data no formato DD/MM (posições 14-18)

### Linha 2
- Umidade percentual (posições 0-9)
- Barra gráfica visual de 6 blocos (posições 11-18)

### Linha 3
- Status do conforto térmico (posições 0-17)
- Emoji animado (2 colunas) representando o estado (posições 18-19)

---

## Emojis Animados

Cada condição climática possui um emoji animado de 2 frames que alternam a cada 500ms:

- **IDEAL:** Coração pulsante (aumenta e diminui)
- **QUENTE:** Fogo crepitante (chamas dançantes)
- **FRIO:** Floco de neve girando (cristal rotacionando)
- **ÚMIDO:** Gotas de água caindo (movimento vertical)
- **SECO:** Sol radiante (raios alternados)

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

## Intervalos de Atualização

- **Sensor DHT11:** A cada 2 segundos
- **Animação dos emojis:** A cada 500ms
- **Data e hora:** A cada 10 segundos
- **Sincronização NTP:** Apenas na inicialização

---

## Recursos Técnicos

### Sincronização de Hora
- Utiliza servidor NTP `pool.ntp.org`
- Fuso horário: UTC-3 (Brasília)
- Timeout de conexão WiFi: 10 segundos
- WiFi desligado após sincronização

### Gerenciamento de Memória
- Caracteres customizados para emojis (8 slots)
- Atualização inteligente: redesenha apenas quando há mudança
- Formatação com sprintf para consistência de layout

### Tratamento de Erros
- Detecção de falhas consecutivas do DHT11
- Mensagem de erro após 3 tentativas falhas
- Reinicialização automática após 40s sem leitura válida
- Fallback para RTC em caso de falha WiFi/NTP

---

## Possíveis Expansões

- Servidor web local para visualização remota dos dados
- Envio de dados para plataformas IoT (ThingSpeak, Blynk)
- Integração com Home Assistant via MQTT
- Adição de sensores extras (pressão, luminosidade)
- Gravação de logs históricos em cartão SD
- Alarmes programáveis para condições críticas
- Gráficos de tendência de temperatura/umidade
- Controle de dispositivos baseado em condições

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
- Verifique se a bateria CR2032 está instalada e carregada
- Confirme conexões CLK, DAT e RST
- Execute a configuração inicial do RTC

### WiFi não conecta
- Verifique SSID e senha no código
- Certifique-se que a rede é 2.4GHz (ESP8266 não suporta 5GHz)
- Aproxime o dispositivo do roteador

### Data/hora ficam piscando
- Problema de formatação corrigido na versão atual
- Verifique se está usando a versão mais recente do código

### Display mostra caracteres estranhos
- Verifique conexões I2C
- Confirme endereço 0x27 no código
- Reinicie o sistema
- Teste com sketch I2C Scanner para confirmar endereço

---

## Estrutura do Código

O código está organizado nas seguintes seções:

- **Includes e configurações:** Bibliotecas e definições de pinos
- **Configuração WiFi/NTP:** Credenciais e servidor de tempo
- **Variáveis globais:** Controle de tempo e estados
- **Bitmaps dos emojis:** Caracteres customizados 8x8
- **Funções auxiliares:** Animação, conforto, barra gráfica
- **Funções de rede:** Conexão WiFi e sincronização NTP
- **Setup:** Inicialização com telas de feedback
- **Loop principal:** Atualização contínua de dados

---

## Consumo de Memória

Valores aproximados após compilação:

- **RAM:** ~29KB / 80KB (36%)
- **Flash:** ~262KB / 1MB (25%)
- **IRAM:** ~28KB / 32KB (93%)

O código está otimizado para deixar espaço para expansões futuras.

---

## Autor

**Moacir Pereira**  
Projeto pessoal de automação IoT e monitoramento ambiental.  
Atualizado em: Novembro/2024

---

## Licença

Este projeto é de código aberto sob a licença **MIT**.  
Sinta-se livre para usar, modificar e compartilhar.

---

## Agradecimentos

- Adafruit pela biblioteca DHT
- Makuna pela biblioteca RTC
- Comunidade ESP8266/Arduino
