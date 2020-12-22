#include "DHT.h"
#define DHTTYPE DHT11

const int PIRPin    = D15;
const int DHTpin    = D14;
const int RedLEDpin = D13;
const int GrnLEDpin = D12;
const int BluLEDpin = D11;
const int PhotoPin  = A0;

DHT dht(DHTpin, DHTTYPE);

#define LED_ON          LOW
#define LED_OFF         HIGH
#define PIR_DETECTED    HIGH
#define PIR_NONE        LOW

//-------------------------------
#define LIGHT_UP_THRESHOLD  300

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(RedLEDpin, OUTPUT);
  pinMode(GrnLEDpin, OUTPUT);
  pinMode(BluLEDpin, OUTPUT);

  digitalWrite(RedLEDpin, HIGH);
  digitalWrite(GrnLEDpin, HIGH);
  digitalWrite(BluLEDpin, HIGH);

  pinMode(PIRPin, INPUT);
  pinMode(PhotoPin, INPUT);


  Serial.println("\nPIR initializing...");
  delay(1 * 1000);
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

void loop() {
  int pir = digitalRead(PIRPin);
  float hum = dht.readHumidity();
  float tmp = dht.readTemperature();
  int lux = analogRead(PhotoPin);

  if (pir == PIR_DETECTED) {
    if (lux <= LIGHT_UP_THRESHOLD) {
      lightControl(LED_ON);
    }
  } else {
    lightControl(LED_OFF);
  }

  if (isnan(hum) || isnan(tmp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  showEnvInfo(lux, pir, hum, tmp);

  delay(500);
}
