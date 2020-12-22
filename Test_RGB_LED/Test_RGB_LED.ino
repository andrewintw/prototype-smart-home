const int RedLEDpin = D13;
const int GrnLEDpin = D12;
const int BluLEDpin = D11;


void setup() {
  pinMode(RedLEDpin, OUTPUT);
  pinMode(GrnLEDpin, OUTPUT);
  pinMode(BluLEDpin, OUTPUT);

  digitalWrite(RedLEDpin, HIGH);
  digitalWrite(GrnLEDpin, HIGH);
  digitalWrite(BluLEDpin, HIGH);
}

void loop() {

  digitalWrite(RedLEDpin, LOW);
  delay(1000);
  digitalWrite(RedLEDpin, HIGH);
  digitalWrite(GrnLEDpin, LOW);
  delay(1000);
  digitalWrite(GrnLEDpin, HIGH);
  digitalWrite(BluLEDpin, LOW);
  delay(1000);
  digitalWrite(BluLEDpin, HIGH);

}
