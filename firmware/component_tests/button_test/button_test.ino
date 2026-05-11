const int buttonPin = 33;

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.println("Button test started");
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    Serial.println("BUTTON PRESSED");
    delay(300);
  }
}
