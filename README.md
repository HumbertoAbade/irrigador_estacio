# Sistema de Irrigação Automatizado com Monitoramento IoT Residencial

Este repositório contém o firmware embarcado em C++/Arduino desenvolvido para o **Sistema de Irrigação Automatizado com Monitoramento IoT**, um projeto de extensão acadêmica apresentado à **Universidade Estácio de Sá** (Supercampus Maracanã, Rio de Janeiro). 

O sistema utiliza um microcontrolador **ESP32** para gerenciar de forma 100% autônoma o microambiente botânico de um vaso de plantas, mitigando o desgaste por oxidação galvânica de sensores e eliminando resets indutivos de potência por meio de isolamento elétrico e um algoritmo local de controle por histerese. Os dados coletados são transmitidos em tempo real para a nuvem através do protocolo MQTT e persistidos na plataforma **ThingsBoard Cloud**.

---

## 📌 Funcionalidades e Características Técnicas

* **Controle de Borda Autônomo:** Toda a lógica de verificação de umidade e acionamento da bomba roda localmente no chip dual-core do ESP32, garantindo a sobrevivência e rega da planta mesmo em cenários de queda de conexão Wi-Fi.
* **Algoritmo de Histerese Temporal:** Evita transbordamentos do vaso por meio de pulsos de rega cirúrgicos (intermitência ativa de 5 segundos de irrigação seguidos por 10 segundos de infiltração capilar no substrato).
* **Imunidade à Oxidação Galvânica:** O sensor de umidade capacitivo opera em intervalos programados, mitigando a passagem constante de corrente e aumentando expressivamente a vida útil dos eletrodos em solo úmido.
* **Lógica Inversa de Segurança:** O módulo relé opera em nível lógico invertido configurado em firmware (`BOMBA_OFF = HIGH`), impedindo o acionamento acidental ou contínuo da bomba d'água durante resets elétricos do circuito.
* **Telemetria Avançada e RPC:** Comunicação criptografada e leve via protocolo **MQTT (porta 1883)** com encapsulamento de payloads em formato estruturado **JSON**, permitindo tanto a transmissão de dados quanto a recepção de comandos remotos (*Remote Procedure Call*) para alternar entre modo Automático e Manual via painel web.

---

## 🗺️ Arquitetura do Sistema e Mapeamento de Pinos

O circuito físico divide-se estritamente entre a **Malha de Sinal** (3.3V nativos do ESP32 para amostragem limpa e sem ruídos) e a **Malha de Potência** (alimentação externa e isolamento por relé optoisolado para a bomba), conectando-se conforme a pinagem abaixo:

| Componente | Tipo de Sinal | Pino ESP32 (GPIO) | Função no Sistema |
| :--- | :--- | :--- | :--- |
| **Sensor de Umidade Solo (Capacitivo V1.2)** | Input Analógico | `GPIO34` | Leitura da variação de capacitância do solo (calibrado de 0% a 100%). |
| **Sensor de Temperatura (LM35)** | Input Analógico | `GPIO32` | Monitoramento térmico linear do microclima ambiente (10mV/°C). |
| **Sensor de Luminosidade (LDR 5mm)** | Input Analógico | `GPIO35` | Medição de incidência de luz solar direta sobre o ecossistema. |
| **Módulo Relé Optoisolado (Bomba DC)** | Output Digital | `GPIO26` | Acionamento por lógica inversa da microbomba submersível de rega. |

---

## 💾 Estrutura do Payload JSON

A telemetria é despachada periodicamente para o broker da plataforma ThingsBoard sob o tópico `v1/devices/me/telemetry` com a seguinte estrutura de dados:

```json
{
  "umidade": 45,
  "luz": 72,
  "temperatura": 24.50,
  "regas": 14,
  "modo": "automatico",
  "bomba": false
}
