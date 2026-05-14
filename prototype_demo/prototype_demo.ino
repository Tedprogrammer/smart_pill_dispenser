#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// LCD
const int LCD_SDA_PIN = 21;
const int LCD_SCL_PIN = 22;

// LEDs
const int ledPins[7] = {13, 14, 16, 17, 25, 26, 27};

// Buzzer and button
const int buzzerPin = 32;
const int buttonPin = 33;

// Demo timing
const unsigned long countdownSeconds = 20;
const unsigned long ledInterval = 3000;
const unsigned long buzzerInterval = 1000;
const unsigned long buzzerDuration = 250;

unsigned long countdownStartMillis = 0;
unsigned long lastLedMillis = 0;
unsigned long lastBuzzerMillis = 0;
unsigned long buzzerStartMillis = 0;

int currentLed = 0;

bool alarmActive = false;
bool medicationTaken = false;
bool buzzerOn = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  for (int i = 0; i < 7; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);

  pinMode(buttonPin, INPUT_PULLUP);

  countdownStartMillis = millis();

  lcd.setCursor(0, 0);
  lcd.print("Breakfast done");
  lcd.setCursor(0, 1);
  lcd.print("Lunch in 20 sec");
}

void loop() {
  runLedSequence();

  if (!alarmActive && !medicationTaken) {
    runCountdown();
  }

  if (alarmActive) {
    showAlarmScreen();
    runBuzzerAlarm();
    checkButton();
  }

  if (medicationTaken) {
    showTakenScreen();
  }
}

void runLedSequence() {
  if (millis() - lastLedMillis >= ledInterval) {
    lastLedMillis = millis();

    for (int i = 0; i < 7; i++) {
      digitalWrite(ledPins[i], LOW);
    }

    digitalWrite(ledPins[currentLed], HIGH);

    currentLed++;
    if (currentLed >= 7) {
      currentLed = 0;
    }
  }
}

void runCountdown() {
  unsigned long elapsed = (millis() - countdownStartMillis) / 1000;
  int remaining = countdownSeconds - elapsed;

  if (remaining > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Breakfast done  ");

    lcd.setCursor(0, 1);
    lcd.print("Lunch in ");
    lcd.print(remaining);
    lcd.print(" sec   ");
  } else {
    alarmActive = true;
    lastBuzzerMillis = millis();
    Serial.println("Lunch alarm active");
  }
}

void showAlarmScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Time for lunch ");

  lcd.setCursor(0, 1);
  lcd.print("Press button   ");
}

void runBuzzerAlarm() {
  unsigned long now = millis();

  if (!buzzerOn && now - lastBuzzerMillis >= buzzerInterval) {
    tone(buzzerPin, 2000);
    buzzerOn = true;
    buzzerStartMillis = now;
  }

  if (buzzerOn && now - buzzerStartMillis >= buzzerDuration) {
    noTone(buzzerPin);
    buzzerOn = false;
    lastBuzzerMillis = now;
  }
}

void checkButton() {
  if (digitalRead(buttonPin) == LOW) {
    delay(50);

    if (digitalRead(buttonPin) == LOW) {
      noTone(buzzerPin);
      buzzerOn = false;
      alarmActive = false;
      medicationTaken = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lunch pill taken");
      lcd.setCursor(0, 1);
      lcd.print("Next pill:1 hr");

      Serial.println("Lunch pill marked as taken");

      while (digitalRead(buttonPin) == LOW) {
        delay(10);
      }
    }
  }
}

void showTakenScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Lunch pill taken");

  lcd.setCursor(0, 1);
  lcd.print("Next pill: 1 hr");
}
