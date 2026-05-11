const int buzzerPin = 32;

void setup() {
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  tone(buzzerPin, 2000);
  delay(300);

  noTone(buzzerPin);
  delay(700);
}
