Funcionalidades
Sincronização automática de data/hora via WiFi e NTP.
Exibição de temperatura (°C) com símbolo customizado.
Umidade relativa com barra gráfica.
Cálculo de sensação térmica.
Detecção de níveis de conforto: IDEAL, FRIO, QUENTE, ÚMIDO, SECO.
Relógio em tempo real (RTC) mantém hora mesmo sem internet.
Configuração WiFi via Portal de Configuração (WiFiManager).
Desliga WiFi após sincronização para economia de energia.
Atualização dos dados a cada 2 segundos.
Detecção e tratamento de falhas do sensor.
Reinicialização automática em caso de travamento.

Componentes Utilizados:
| Componente              | Quantidade | Observações                |
| ----------------------- | ---------- | -------------------------- |
| Wemos D1 Mini (ESP8266) | 1          | Microcontrolador Wi-Fi     |
| Sensor DHT11            | 1          | Temperatura e umidade      |
| LCD 20x4 I2C            | 1          | Endereço padrão 0x27       |
| RTC DS1302              | 1          | Relógio em tempo real      |
| Bateria CR2032          | 1          | Para o módulo RTC          |
| Jumpers                 | 10-12      | Conexões entre componentes |
| Fonte 5V                | 1          | Via USB do Wemos           |

Ligações dos Componentes:
| DHT11 | Wemos D1 Mini |
| ----- | ------------- |
| VCC   | 5V            |
| GND   | GND           |
| DATA  | D4 (GPIO2)    |

LCD I2C → Wemos D1 Mini
| LCD I2C | Wemos D1 Mini |
| ------- | ------------- |
| VCC     | 5V            |
| GND     | GND           |
| SDA     | D2 (GPIO4)    |
| SCL     | D1 (GPIO5)    |

RTC DS1302 → Wemos D1 Mini
| DS1302 | Wemos D1 Mini |
| ------ | ------------- |
| VCC    | 5V            |
| GND    | GND           |
| CLK    | D5 (GPIO14)   |
| DAT    | D6 (GPIO12)   |
| RST    | D7 (GPIO13)   |

Bibliotecas Necessárias
Instalar via Gerenciador de Bibliotecas do Arduino IDE:
DHT sensor library por Adafruit
Adafruit Unified Sensor (dependência do DHT)
LiquidCrystal I2C por Frank de Brabander
Rtc by Makuna (para DS1302/DS3231)
WiFiManager por tzapu

Configuração da IDE Arduino
Placa: LOLIN(WEMOS) D1 R2 & mini
Velocidade de upload: 115200
Flash size: 4MB (FS: 1MB OTA: ~1019KB)
Porta: Selecione a porta COM do Wemos

Configuração Inicial:

1. Primeira Execução
Ao ligar, cria um ponto de acesso WiFi ESTACAO-IOT (senha: 12345678)
Conecte-se e acesse 192.168.4.1 no navegador
Selecione sua rede WiFi e senha
Sistema sincroniza via NTP e grava a hora no RTC
WiFi será desligado para economia de energia
Funcionamento normal usando apenas o RTC

2. Ajuste Manual do RTC (Opcional)

No código, dentro da função setup(), descomente:
// Para configuração manual, ajuste abaixo:
// RtcDateTime manual(2024, 11, 27, 16, 00, 00);
// rtc.SetDateTime(manual);

Faça upload, depois comente novamente e reenvie o código
Bateria CR2032 mantém a hora mesmo sem alimentação
Sequência de Inicialização
Tela Inicial: Estacao IoT vFinal
Conectando WiFi: Conectando WiFi...
Portal de Configuração (se necessário):
CONFIGURE O WIFI
1- Conecte-se a: ESTACAO-IOT
2- Abra 192.168.4.1

Sincronização: 
WiFi Conectado!

[Nome da rede]

Hora via NTP: Hora via net... OK

Tela Normal:

Temp:  25.5°   14:35

Sens:  26.3°   27/11

Umid: 65% [■■■■..]

Status: IDEAL ❤️

Layout do Display
| Linha | Conteúdo                                                           |
| ----- | ------------------------------------------------------------------ |
| 0     | Temperatura + °C (0-12) / Hora HH:MM (14-18)                       |
| 1     | Sensação térmica (0-12) / Data DD/MM (14-18)                       |
| 2     | Umidade % (0-9) + barra gráfica de 6 blocos (11-18)                |
| 3     | "Status:" fixo (0-6) / Conforto térmico (8-17) / Indicador (18-19) |


Critérios de Conforto Térmico
| Condição | Temperatura  | Umidade | Texto Exibido |
| -------- | ------------ | ------- | ------------- |
| FRIO     | <16°C        | -       | FRIO          |
| QUENTE   | >29°C        | -       | QUENTE        |
| SECO     | -            | <30%    | MTO SECO      |
| ÚMIDO    | -            | >75%    | MTO UMID      |
| IDEAL    | 18-26°C      | 40-60%  | IDEAL         |
| NORMAL   | Demais casos | -       | NORMAL        |


Intervalos de Atualização
Sensor DHT11: a cada 2 segundos
Data e hora: a cada 10 segundos
Animação do status: a cada 500ms
Sincronização NTP: apenas na inicialização
 Recursos Técnicos
Sincronização NTP (pool.ntp.org, UTC-3, timeout 20s)
WiFi desligado após sincronização
Atualização inteligente do display (evita flicker)
Formatação consistente via snprintf()
Detecção de falhas no DHT11 (ERRO DHT → reinício automático)
Fallback para RTC se WiFi/NTP falhar
WiFiManager permite configurar sem recompilar
Possíveis Expansões
Servidor web para visualização remota
Envio de dados para ThingSpeak, Blynk
Integração Home Assistant via MQTT
Sensores extras: pressão, luminosidade, CO₂
Logs históricos em cartão SD
Alarmes programáveis
Gráficos de tendências
Controle de dispositivos baseado em condições
Solução de Problemas
Display não acende: verifique SDA/SCL, endereço I2C e contraste
DHT11 erro: aguarde 2s entre leituras, verifique D4
RTC não mantém hora: confira bateria CR2032 e conexões CLK/DAT/RST
Portal WiFi não aparece: use wm.resetSettings() ou pressione RESET
Data/hora piscando: use versão mais recente do código
Caracteres estranhos no LCD: verifique endereço I2C e conexões
Estrutura do Código
Includes e configurações
Configuração WiFi/NTP (WiFiManager)
Variáveis globais
Bitmaps customizados
Funções auxiliares (conforto, barra gráfica)
Funções de rede (WiFi, NTP)
Setup (inicialização com feedback)
Loop principal (atualizações e animações)

Consumo de Memória
RAM: ~30KB / 80KB (38%)
Flash: ~270KB / 1MB (26%)
IRAM: ~29KB / 32KB (91%)

Autor
Moacir Pereira
Projeto pessoal de automação IoT e monitoramento ambiental
Atualizado em: Novembro/2024

Licença
Código aberto sob MIT License. Use, modifique e compartilhe livremente.
Agradecimentos
Adafruit (DHT)
Makuna (RTC)
tzapu (WiFiManager)
Comunidade ESP8266/Arduino

Suporte
Verifique a seção de problemas e soluções
Consulte issues no repositório
Contato via comunidade ESP8266
