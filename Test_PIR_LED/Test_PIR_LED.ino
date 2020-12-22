const int RedLEDpin = D13;
const int GrnLEDpin = D12;
const int BluLEDpin = D11;

#define PIROUT_PIN  D15

int sensor_data = 0;

void setup() {
  Serial.begin(115200);
  pinMode(RedLEDpin, OUTPUT);
  pinMode(GrnLEDpin, OUTPUT);
  pinMode(BluLEDpin, OUTPUT);

  digitalWrite(RedLEDpin, HIGH);
  digitalWrite(GrnLEDpin, HIGH);
  digitalWrite(BluLEDpin, HIGH);

  pinMode(PIROUT_PIN, INPUT);

  Serial.println("\nPIR initializing...(60sec)");
  delay(1 * 1000);
}

void loop() {
  sensor_data = digitalRead(PIROUT_PIN);
  if (sensor_data == HIGH)
  {
    Serial.println("ON");
    digitalWrite(RedLEDpin, LOW);
    digitalWrite(GrnLEDpin, LOW);
    digitalWrite(BluLEDpin, LOW);
  } else {
    Serial.println("OFF");
    digitalWrite(RedLEDpin, HIGH);
    digitalWrite(GrnLEDpin, HIGH);
    digitalWrite(BluLEDpin, HIGH);
  }
  delay(500);
}
