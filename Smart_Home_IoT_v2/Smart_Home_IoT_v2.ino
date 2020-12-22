#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "DHT.h"
#define DHTTYPE DHT11

const int PIRPin                = D15;
const int DHTpin                = D14;
const int RedLEDpin             = D13;
const int GrnLEDpin             = D12;
const int BluLEDpin             = D11;
const int PhotoPin              = A0;
const int onBoardLEDin          = D5;

DHT dht(DHTpin, DHTTYPE);

#define LED_ON                  LOW
#define LED_OFF                 HIGH
#define PIR_DETECTED            HIGH
#define PIR_NONE                LOW

#define LIGHT_UP_THRESHOLD      30
#define HTTP_GET_INTERVAL       (20 * 1000)

#ifndef STASSID
#define STASSID                 "andrew"
#define STAPSK                  "1234567890"
#endif

const char* ssid                = STASSID;
const char* password            = STAPSK;

const char* host                = "maker.ifttt.com";
const uint16_t port             = 80;

unsigned short current_pir_stat = LOW;
unsigned short last_pir_stat    = LOW;
unsigned long detect_count      = 0;
unsigned long no_detect_count   = 0;

unsigned long send_data_time    = 0;

ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

typedef enum {
  OFF,
  ON,
  BLINK_INIT,
  BLINK_SENT,
} led_state_t;

void espLedCtrl(led_state_t st)
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
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    case BLINK_SENT:
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      break;
  }
}


void initDevIO()
{
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  delay(100);

  pinMode(LED_BUILTIN,  OUTPUT);
  pinMode(onBoardLEDin, OUTPUT);

  pinMode(RedLEDpin,   OUTPUT);
  pinMode(GrnLEDpin,   OUTPUT);
  pinMode(BluLEDpin,   OUTPUT);

  digitalWrite(RedLEDpin, HIGH);
  digitalWrite(GrnLEDpin, HIGH);
  digitalWrite(BluLEDpin, HIGH);

  pinMode(PIRPin,   INPUT);
  pinMode(PhotoPin, INPUT);

  Serial.println();
}


void initSensors()
{
  dht.begin();
}


void initWiFi()
{
  unsigned int i = 0;

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
  WiFiMulti.addAP("-andrlin", "1234567890");

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    i++;

    if (i % 2 == 0) {
      espLedCtrl(OFF);
    } else {
      espLedCtrl(ON);
    }

    if (i >= ((1 << (sizeof(uint16) * 8)) - 1)) {
      i = 0;
    }
  }

  Serial.printf("\nWiFi connected to: %s\n", WiFi.SSID().c_str());
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  espLedCtrl(ON);
}


void setup() {
  initDevIO();
  initSensors();
  initWiFi();

  espLedCtrl(OFF);
}

void lightControl(int state) {
  digitalWrite(RedLEDpin, state);
  digitalWrite(GrnLEDpin, state);
  digitalWrite(BluLEDpin, state);
}

void sendDataToCloud(int lux, int pir, float hum, float tmp)
{
#if 0
  if (((millis() - send_data_time) < HTTP_GET_INTERVAL)) {
    Serial.println("Ignore");
    return;
  }
#endif

  espLedCtrl(ON);
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

  espLedCtrl(BLINK_SENT);
  send_data_time = millis();
}

void showEnvInfo(int lux, int pir, float hum, float tmp) {

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("o "));
  } else {
    Serial.print(F("x "));
  }

  Serial.print(lux);
  Serial.print(F("lux p"));
  Serial.print(pir == PIR_DETECTED);
  Serial.print(F(" ("));
  Serial.print(detect_count);
  Serial.print(F("/"));
  Serial.print(no_detect_count);
  Serial.print(F(")_"));
  Serial.print(last_pir_stat);
  Serial.print(F(" "));
  Serial.print(hum);
  Serial.print(F("% "));
  Serial.print(tmp);
  Serial.print(F("C "));
}

void loop() {
  int lux = (((float)analogRead(PhotoPin)) / 1024) * 100;
  int pir = digitalRead(PIRPin);
  float hum = dht.readHumidity();
  float tmp = dht.readTemperature();


  if (pir == PIR_DETECTED) {
    espLedCtrl(ON);
    detect_count++;
    if (lux <= LIGHT_UP_THRESHOLD) {
      lightControl(LED_ON);
    } else {
      lightControl(LED_OFF);
    }
  } else {
    espLedCtrl(OFF);
    no_detect_count++;
    lightControl(LED_OFF);
  }

  if (detect_count > 10 && detect_count > no_detect_count) {
    current_pir_stat = PIR_DETECTED;
    detect_count = 0;
    no_detect_count = 0;
  }

  if (no_detect_count > 10 && no_detect_count > detect_count) {
    current_pir_stat = (1 - PIR_DETECTED);
    detect_count = 0;
    no_detect_count = 0;
  }

  showEnvInfo(lux, pir, hum, tmp);

  if (current_pir_stat != last_pir_stat) {
    last_pir_stat = current_pir_stat;

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(F("-->>"));
      sendDataToCloud(lux, pir, hum, tmp);
    }
  }

  Serial.println();

  if (isnan(hum) || isnan(tmp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  delay(1000);
}
