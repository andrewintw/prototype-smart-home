#define PIROUT_PIN  D15

int sensor_data = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIROUT_PIN, INPUT);

  Serial.println("\nPIR initializing...(60sec)");
  delay(1 * 1000);
}

void loop() {
  sensor_data = digitalRead(PIROUT_PIN);
  if (sensor_data == HIGH)
  {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }
  delay(500);
}
