#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// =========================
// LCD SETTINGS
// =========================

// LCD I2C address.
// Most modules use 0x27. Some use 0x3F.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ESP32 I2C pins for LCD.
const int LCD_SDA_PIN = 21;
const int LCD_SCL_PIN = 22;


// =========================
// RTC SETTINGS
// =========================

// DS1302 pins.
// Order required by library: DAT, CLK, RST
const int RTC_DAT_PIN = 19;
const int RTC_CLK_PIN = 18;
const int RTC_RST_PIN = 5;

ThreeWire rtcWire(RTC_DAT_PIN, RTC_CLK_PIN, RTC_RST_PIN);
RtcDS1302<ThreeWire> Rtc(rtcWire);


// =========================
// OUTPUT AND INPUT PINS
// =========================

// Seven LEDs represent seven days.
const int ledPins[7] = {
  13,  // Monday
  14,  // Tuesday
  16,  // Wednesday
  17,  // Thursday
  25,  // Friday
  26,  // Saturday
  27   // Sunday
};

// Short day names for LCD.
const char* dayNames[7] = {
  "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"
};

// Passive buzzer pin.
const int buzzerPin = 32;

// Confirm button pin.
// Button should connect GPIO33 to GND when pressed.
const int buttonPin = 33;


// =========================
// MEDICATION SCHEDULE
// =========================

// Each Dose stores one medication time.
struct Dose {
  int hour;          // Dose hour in 24 hour format
  int minute;        // Dose minute
  int tablets;       // Number of tablets to display
  const char* label; // Text shown on LCD
};

// For testing, set one time close to current RTC time.
// Example: if current time is 14:35, set first dose to 14:36.
Dose doses[] = {
  {8, 0, 2, "Morning"},
  {13, 0, 1, "Noon"},
  {20, 0, 2, "Night"}
};

// Total number of dose times.
const int doseCount = sizeof(doses) / sizeof(doses[0]);


// =========================
// ALARM STATE VARIABLES
// =========================

// True when medication alarm is currently active.
bool alarmActive = false;

// Stores which dose is currently alarming.
int activeDoseIndex = -1;

// Stores which day LED should be active.
int activeDayIndex = -1;

// This records whether medication was taken.
// First index is day: 0 Monday to 6 Sunday.
// Second index is dose number.
bool medicationTaken[7][10];

// Used to reset medicationTaken when date changes.
int lastKnownDay = -1;
int lastKnownMonth = -1;
int lastKnownYear = -1;


// =========================
// TIMING VARIABLES
// =========================

// When alarm started.
unsigned long alarmStartMillis = 0;

// LED stays solid for this duration before buzzer starts.
const unsigned long LED_ONLY_TIME = 5000;

// Used for buzzer timing.
unsigned long lastBuzzerMillis = 0;

// Buzzer beeps once per second.
const unsigned long BUZZER_INTERVAL = 1000;

// Buzzer beep duration.
const unsigned long BUZZER_ON_TIME = 180;

// True while buzzer is currently on.
bool buzzerIsOn = false;

// Used to avoid refreshing LCD too fast.
unsigned long lastLcdUpdate = 0;


// =========================
// SETUP
// =========================

void setup() {
  Serial.begin(115200);

  // Start LCD I2C communication.
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);

  // Start LCD.
  lcd.init();
  lcd.backlight();

  // Start RTC.
  Rtc.Begin();

  // Start RTC if not running.
  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  // Configure LED pins.
  for (int i = 0; i < 7; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // Configure buzzer.
  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);

  // Configure button with internal pullup.
  // Normal state is HIGH.
  // Pressed state is LOW.
  pinMode(buttonPin, INPUT_PULLUP);

  clearMedicationRecords();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Pill Box");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");

  delay(1500);
}


// =========================
// MAIN LOOP
// =========================

void loop() {
  RtcDateTime now = Rtc.GetDateTime();

  int todayIndex = convertRtcDayToIndex(now.DayOfWeek());

  resetRecordsIfNewDay(now);

  checkForDoseTime(now, todayIndex);

  checkButton();

  if (alarmActive) {
    runAlarm(now);
  } else {
    showNormalScreen(now, todayIndex);
  }
}


// =========================
// CHECK MEDICATION TIME
// =========================

void checkForDoseTime(RtcDateTime now, int todayIndex) {
  if (alarmActive) {
    return;
  }

  for (int i = 0; i < doseCount; i++) {
    bool timeMatches =
      now.Hour() == doses[i].hour &&
      now.Minute() == doses[i].minute;

    bool alreadyTaken =
      medicationTaken[todayIndex][i];

    if (timeMatches && !alreadyTaken) {
      startAlarm(todayIndex, i);
      return;
    }
  }
}


// =========================
// START ALARM
// =========================

void startAlarm(int dayIndex, int doseIndex) {
  alarmActive = true;
  activeDayIndex = dayIndex;
  activeDoseIndex = doseIndex;

  alarmStartMillis = millis();
  lastBuzzerMillis = 0;
  buzzerIsOn = false;

  allLedsOff();

  // First stage: LED turns on immediately.
  digitalWrite(ledPins[activeDayIndex], HIGH);

  Serial.println("Medication alarm started");
}


// =========================
// RUN ALARM
// =========================

void runAlarm(RtcDateTime now) {
  showAlarmScreen();

  unsigned long elapsed = millis() - alarmStartMillis;

  // First 5 seconds: LED only, no buzzer.
  if (elapsed < LED_ONLY_TIME) {
    digitalWrite(ledPins[activeDayIndex], HIGH);
    noTone(buzzerPin);
    return;
  }

  // After 5 seconds: LED remains on and buzzer beeps every second.
  digitalWrite(ledPins[activeDayIndex], HIGH);

  unsigned long currentMillis = millis();

  if (!buzzerIsOn && currentMillis - lastBuzzerMillis >= BUZZER_INTERVAL) {
    tone(buzzerPin, 2000);
    buzzerIsOn = true;
    lastBuzzerMillis = currentMillis;
  }

  if (buzzerIsOn && currentMillis - lastBuzzerMillis >= BUZZER_ON_TIME) {
    noTone(buzzerPin);
    buzzerIsOn = false;
  }
}


// =========================
// BUTTON HANDLING
// =========================

void checkButton() {
  if (digitalRead(buttonPin) == LOW) {
    delay(50);

    if (digitalRead(buttonPin) == LOW) {
      if (alarmActive) {
        confirmMedicationTaken();
      }

      while (digitalRead(buttonPin) == LOW) {
        delay(10);
      }
    }
  }
}


// =========================
// CONFIRM MEDICATION TAKEN
// =========================

void confirmMedicationTaken() {
  medicationTaken[activeDayIndex][activeDoseIndex] = true;

  alarmActive = false;

  allLedsOff();
  noTone(buzzerPin);

  Serial.print("Medication taken: ");
  Serial.print(dayNames[activeDayIndex]);
  Serial.print(" ");
  Serial.println(doses[activeDoseIndex].label);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dose Confirmed");
  lcd.setCursor(0, 1);
  lcd.print("Marked Taken");

  delay(2000);

  activeDayIndex = -1;
  activeDoseIndex = -1;
}


// =========================
// LCD NORMAL SCREEN
// =========================

void showNormalScreen(RtcDateTime now, int todayIndex) {
  if (millis() - lastLcdUpdate < 1000) {
    return;
  }

  lastLcdUpdate = millis();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(dayNames[todayIndex]);
  lcd.print(" ");

  printTwoDigitsToLcd(now.Hour());
  lcd.print(":");
  printTwoDigitsToLcd(now.Minute());
  lcd.print(":");
  printTwoDigitsToLcd(now.Second());

  lcd.setCursor(0, 1);
  lcd.print("Next ");
  lcd.print(doses[0].hour);
  lcd.print(":");
  printTwoDigitsToLcd(doses[0].minute);
  lcd.print(" ");
  lcd.print(doses[0].tablets);
  lcd.print(" tab");
}


// =========================
// LCD ALARM SCREEN
// =========================

void showAlarmScreen() {
  if (millis() - lastLcdUpdate < 500) {
    return;
  }

  lastLcdUpdate = millis();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(dayNames[activeDayIndex]);
  lcd.print(" ");
  lcd.print(doses[activeDoseIndex].label);

  lcd.setCursor(0, 1);
  lcd.print("Take ");
  lcd.print(doses[activeDoseIndex].tablets);
  lcd.print(" tablet");
}


// =========================
// RESET DAILY RECORDS
// =========================

void resetRecordsIfNewDay(RtcDateTime now) {
  if (
    now.Day() != lastKnownDay ||
    now.Month() != lastKnownMonth ||
    now.Year() != lastKnownYear
  ) {
    clearMedicationRecords();

    lastKnownDay = now.Day();
    lastKnownMonth = now.Month();
    lastKnownYear = now.Year();

    Serial.println("New day detected. Medication records cleared.");
  }
}


// =========================
// HELPER FUNCTIONS
// =========================

void clearMedicationRecords() {
  for (int day = 0; day < 7; day++) {
    for (int dose = 0; dose < 10; dose++) {
      medicationTaken[day][dose] = false;
    }
  }
}

void allLedsOff() {
  for (int i = 0; i < 7; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

int convertRtcDayToIndex(uint8_t rtcDay) {
  // Makuna library normally uses:
  // Sunday = 0
  // Monday = 1
  // Tuesday = 2
  // ...
  // Saturday = 6

  if (rtcDay == 0) {
    return 6; // Sunday becomes index 6
  }

  return rtcDay - 1; // Monday becomes index 0
}

void printTwoDigitsToLcd(int number) {
  if (number < 10) {
    lcd.print("0");
  }

  lcd.print(number);
}
