const int onBoardLEDin = D5; // or 14
const int ModuleLEDpin = D9; // or LED_BUILTIN or 2


// LED_BUILTIN is D9

void setup() {
  pinMode(ModuleLEDpin, OUTPUT);
  pinMode(onBoardLEDin, OUTPUT);
}

void loop() {
  digitalWrite(ModuleLEDpin, HIGH);
  digitalWrite(onBoardLEDin, HIGH);
  
  delay(500);
  
  digitalWrite(ModuleLEDpin, LOW);
  digitalWrite(onBoardLEDin, LOW);
  
  delay(500);
}
