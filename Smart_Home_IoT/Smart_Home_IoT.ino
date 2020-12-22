#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "DHT.h"
#define DHTTYPE DHT11

const int PIRPin        = D15;
const int DHTpin        = D14;
const int RedLEDpin     = D13;
const int GrnLEDpin     = D12;
const int BluLEDpin     = D11;
const int PhotoPin      = A0;

DHT dht(DHTpin, DHTTYPE);

#define LED_ON          LOW
#define LED_OFF         HIGH
#define PIR_DETECTED    HIGH
#define PIR_NONE        LOW

#define LIGHT_UP_THRESHOLD  30

#ifndef STASSID
#define STASSID         "andrew"
#define STAPSK          "1234567890"
#endif

const char* ssid        = STASSID;
const char* password    = STAPSK;

const char* host        = "maker.ifttt.com";
const uint16_t port     = 80;

unsigned int current_pir_stat        = 0;

ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

typedef enum {
  OFF,
  ON,
  BLINK_INIT,
  BLINK_SENT,
} led_state_t;

void ledCtrl(led_state_t st)
{
  switch (st)
  {
    case ON:
      digitalWrite(LED_BUILTIN, LOW); /* Active Low LED_BUILTIN */
      break;
    case OFF:
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    case BLINK_INIT:
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    case BLINK_SENT:
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN, LOW);
      delay(50);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN, LOW);
      break;
  }
}


void init_dev_io()
{
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  delay(100);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RedLEDpin,   OUTPUT);
  pinMode(GrnLEDpin,   OUTPUT);
  pinMode(BluLEDpin,   OUTPUT);

  digitalWrite(RedLEDpin, HIGH);
  digitalWrite(GrnLEDpin, HIGH);
  digitalWrite(BluLEDpin, HIGH);

  pinMode(PIRPin,   INPUT);
  pinMode(PhotoPin, INPUT);

  ledCtrl(BLINK_INIT);
  Serial.println();
}


void init_sensors()
{
  dht.begin();
}


void init_wifi()
{
  unsigned int i = 0;

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    i++;

    if (i % 2 == 0) {
      ledCtrl(OFF);
    } else {
      ledCtrl(ON);
    }

    if (i >= ((1 << (sizeof(uint16) * 8)) - 1)) {
      i = 0;
    }
  }

  Serial.printf("\nWiFi connected to: %s\n", WiFi.SSID().c_str());
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  ledCtrl(ON);
}


void setup() {
  init_dev_io();
  init_sensors();
  init_wifi();

  ledCtrl(OFF);
}

void lightControl(int state) {
  digitalWrite(RedLEDpin, state);
  digitalWrite(GrnLEDpin, state);
  digitalWrite(BluLEDpin, state);
}

void showEnvInfo(int lux, int pir, float hum, float tmp) {
  Serial.print(F("lux: "));
  Serial.print(lux);
  Serial.print(F(", pir: "));
  Serial.print(pir == PIR_DETECTED);
  Serial.print(F(", hum(%): "));
  Serial.print(hum);
  Serial.print(F(", tmp(C): "));
  Serial.println(tmp);
}

void send_data_to_cloud(int lux, int pir, float hum, float tmp)
{
  while (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
  }

  // https://www.urlencoder.org/
  String strEnvInfo = String("%E7%92%B0%E5%A2%83%E8%B3%87%E8%A8%8A"); // 環境資訊
  String strLux = String("%E7%85%A7%E5%BA%A6"); // 照度
  String strHum = String("%E6%BF%95%E5%BA%A6"); // 濕度
  String strTmp = String("%E6%BA%AB%E5%BA%A6"); // 溫度
  String value1 = strEnvInfo +
                  ":%20" + strLux + ":" + String(lux) + "%25" +
                  ",%20" + strHum + ":" + String(hum) + "%25" +
                  ",%20" + strTmp + ":" + String(tmp) + "%C2%B0C";

  String value2 = String("");
  if (pir == PIR_DETECTED) {
    value2 = String("%E6%9C%89%E4%BA%BA%E5%9C%A8%E6%88%BF%E9%96%93"); // 有人在房間
  } else {
    value2 = String("%E6%88%BF%E9%96%93%E6%B2%92%E4%BA%BA"); // 房間沒人
  }

  String value3 = String("");

  client.print("GET /trigger/i-smart-home/with/key/dl1STyaVAEygTVSG?value1=" + value1 + "&value2=" + value2 + "&value3=" + value3 +
               " HTTP/1.1\r\n" +
               "Host: maker.ifttt.com\r\n" +
               "Connection: close\r\n\r\n");

  String line = client.readStringUntil('\r');
  Serial.println(line);
  client.stop();
}

void loop() {
  int lux = (((float)analogRead(PhotoPin)) / 1024) * 100;
  int pir = digitalRead(PIRPin);
  float hum = dht.readHumidity();
  float tmp = dht.readTemperature();


  if (pir == PIR_DETECTED) {
    if (lux <= LIGHT_UP_THRESHOLD) {
      lightControl(LED_ON);
    }
  } else {
    lightControl(LED_OFF);
  }

  if (pir != current_pir_stat) {
    current_pir_stat = pir;
    send_data_to_cloud(lux, pir, hum, tmp);
  }

  if (isnan(hum) || isnan(tmp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  showEnvInfo(lux, pir, hum, tmp);

  delay(1000);
}
