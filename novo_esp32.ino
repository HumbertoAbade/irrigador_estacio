#include <WiFi.h>
#include <PubSubClient.h>

// =====================================
// WIFI
// =====================================

const char* ssid = "POCO F3";
const char* password = "caio roberto";

// =====================================
// THINGSBOARD
// =====================================

const char* mqtt_server = "thingsboard.cloud";

const int mqtt_port = 1883;

const char* token =
"JiJ16FvQ6h4pigpvshQw";

// =====================================
// MQTT
// =====================================

WiFiClient espClient;

PubSubClient client(espClient);

// =====================================
// PINOS
// =====================================

int pinoSolo = 34;

int pinoLuz = 35;

int pinoTemperatura = 32;

int pinoBomba = 26;

// =====================================
// RELÉ INVERTIDO
// =====================================

// LOW = LIGA
// HIGH = DESLIGA

#define BOMBA_ON LOW
#define BOMBA_OFF HIGH

// =====================================
// VARIÁVEIS
// =====================================

bool modoAutomatico = true;

bool bombaLigada = false;

int contadorBomba = 0;

int limiteSeco = 20;

// =====================================
// CALLBACK RPC
// =====================================

void callback(char* topic,
              byte* payload,
              unsigned int length) {

  String mensagem = "";

  for (int i = 0; i < length; i++) {

    mensagem += (char)payload[i];
  }

  Serial.println("====================");

  Serial.println("RPC RECEBIDO:");

  Serial.println(mensagem);

  // =====================================
  // BOTÃO LIGAR BOMBA
  // =====================================

  if (mensagem.indexOf("ligar") >= 0) {

    modoAutomatico = false;

    bombaLigada = true;

    digitalWrite(
      pinoBomba,
      BOMBA_ON
    );

    Serial.println(
      "BOMBA LIGADA"
    );
  }

  // =====================================
  // BOTÃO DESLIGAR BOMBA
  // =====================================

  if (mensagem.indexOf("desligar") >= 0) {

    modoAutomatico = false;

    bombaLigada = false;

    digitalWrite(
      pinoBomba,
      BOMBA_OFF
    );

    Serial.println(
      "BOMBA DESLIGADA"
    );
  }

  // =====================================
  // SWITCH MODO
  // =====================================

  if (mensagem.indexOf("setModo") >= 0) {

    // =================================
    // MANUAL
    // =================================

    if (
      mensagem.indexOf("manual")
      >= 0
    ) {

      modoAutomatico = false;

      Serial.println(
        "MODO MANUAL"
      );
    }

    // =================================
    // AUTOMÁTICO
    // =================================

    if (
      mensagem.indexOf("automatico")
      >= 0
    ) {

      modoAutomatico = true;

      bombaLigada = false;

      digitalWrite(
        pinoBomba,
        BOMBA_OFF
      );

      Serial.println(
        "MODO AUTOMATICO"
      );
    }
  }

  Serial.println("====================");
}

// =====================================
// WIFI
// =====================================

void conectarWiFi() {

  WiFi.begin(
    ssid,
    password
  );

  Serial.println(
    "Conectando WiFi..."
  );

  while (
    WiFi.status()
    != WL_CONNECTED
  ) {

    delay(1000);

    Serial.print(".");
  }

  Serial.println("");

  Serial.println(
    "WiFi conectado!"
  );

  Serial.print("IP: ");

  Serial.println(
    WiFi.localIP()
  );
}

// =====================================
// MQTT
// =====================================

void conectarMQTT() {

  while (!client.connected()) {

    Serial.println(
      "Conectando MQTT..."
    );

    if (

      client.connect(
        "ESP32_Irrigacao",
        token,
        NULL
      )

    ) {

      Serial.println(
        "MQTT conectado!"
      );

      client.subscribe(
        "v1/devices/me/rpc/request/+"
      );

      Serial.println(
        "RPC INSCRITO!"
      );
    }

    else {

      Serial.print(
        "Erro MQTT: "
      );

      Serial.println(
        client.state()
      );

      delay(2000);
    }
  }
}

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  analogReadResolution(12);

  // =====================================
  // MELHORIA ADC ESP32
  // =====================================

  analogSetAttenuation(
    ADC_11db
  );

  pinMode(
    pinoBomba,
    OUTPUT
  );

  digitalWrite(
    pinoBomba,
    BOMBA_OFF
  );

  conectarWiFi();

  client.setServer(
    mqtt_server,
    mqtt_port
  );

  client.setCallback(
    callback
  );

  Serial.println(
    "SISTEMA INICIADO"
  );
}

// =====================================
// LOOP
// =====================================

void loop() {

  // =====================================
  // MQTT
  // =====================================

  if (!client.connected()) {

    conectarMQTT();
  }

  client.loop();

  // =====================================
  // LEITURAS
  // =====================================

  // ---------- SOLO ----------

  int leituraSolo =
    analogRead(pinoSolo);

  // ---------- LUZ ----------

  int leituraLuz = 0;

  for(int i = 0; i < 20; i++){

    leituraLuz += analogRead(
      pinoLuz
    );

    delay(2);
  }

  leituraLuz /= 20;

  // ---------- TEMPERATURA ----------

  int leituraTemp = 0;

  for(int i = 0; i < 20; i++){

    leituraTemp += analogRead(
      pinoTemperatura
    );

    delay(2);
  }

  leituraTemp /= 20;

  // =====================================
  // UMIDADE
  // =====================================

  int umidade = map(
    leituraSolo,
    3200,
    1200,
    0,
    100
  );

  umidade = constrain(
    umidade,
    0,
    100
  );

  // =====================================
  // LUZ
  // =====================================

  int luz = map(
    leituraLuz,
    0,
    5500,
    0,
    100
  );

  luz = constrain(
    luz,
    0,
    100
  );

  // =====================================
  // TEMPERATURA
  // =====================================

  float tensao =
    (leituraTemp * 3.3)
    / 4095.0;

  float temperatura =
    (tensao * 100.0)+13;

  // =====================================
  // AUTOMÁTICO
  // =====================================

  if (modoAutomatico) {

    if (umidade < limiteSeco) {

      bombaLigada = true;

      digitalWrite(
        pinoBomba,
        BOMBA_ON
      );

      contadorBomba++;

      Serial.println(
        "IRRIGANDO..."
      );

      delay(5000);

      digitalWrite(
        pinoBomba,
        BOMBA_OFF
      );

      bombaLigada = false;
    }

    else {

      bombaLigada = false;

      digitalWrite(
        pinoBomba,
        BOMBA_OFF
      );
    }
  }

  // =====================================
  // MANUAL
  // =====================================

  else {

    if (bombaLigada) {

      digitalWrite(
        pinoBomba,
        BOMBA_ON
      );
    }

    else {

      digitalWrite(
        pinoBomba,
        BOMBA_OFF
      );
    }
  }

  // =====================================
  // JSON TELEMETRIA
  // =====================================

  String payload = "{";

  payload += "\"umidade\":";
  payload += String(umidade);
  payload += ",";

  payload += "\"luminosidade\":";
  payload += String(luz);
  payload += ",";

  payload += "\"temperatura\":";
  payload += String(temperatura);
  payload += ",";

  payload += "\"regas\":";
  payload += String(contadorBomba);
  payload += ",";

  payload += "\"modo\":\"";

  if (modoAutomatico) {

    payload += "automatico";
  }

  else {

    payload += "manual";
  }

  payload += "\",";

  payload += "\"bomba\":";

  if (bombaLigada) {

    payload += "true";
  }

  else {

    payload += "false";
  }

  payload += "}";

  // =====================================
  // ENVIAR TELEMETRIA
  // =====================================

  client.publish(
    "v1/devices/me/telemetry",
    payload.c_str()
  );

  // =====================================
  // DEBUG SERIAL
  // =====================================

  Serial.println("====================");

  Serial.print("Modo: ");

  if (modoAutomatico) {

    Serial.println("AUTOMATICO");
  }

  else {

    Serial.println("MANUAL");
  }

  Serial.print("ADC Luz: ");

  Serial.println(leituraLuz);

  Serial.print("ADC Temp: ");

  Serial.println(leituraTemp);

  Serial.print("Umidade: ");

  Serial.print(umidade);

  Serial.println("%");

  Serial.print("Luz: ");

  Serial.print(luz);

  Serial.println("%");

  Serial.print("Temperatura: ");

  Serial.print(temperatura);

  Serial.println(" C");

  Serial.print("Bomba: ");

  if (bombaLigada) {

    Serial.println("ON");
  }

  else {

    Serial.println("OFF");
  }

  Serial.println(payload);

  delay(3000);
}