#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// DS1302 wiring: DAT, CLK, RST
ThreeWire myWire(19, 18, 5);
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  Rtc.Begin();

  if (!Rtc.IsDateTimeValid()) {
    Rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
  }

  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RTC + LCD TEST");
  delay(1500);
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Time ");
  printTwoDigitsLCD(now.Hour());
  lcd.print(":");
  printTwoDigitsLCD(now.Minute());
  lcd.print(":");
  printTwoDigitsLCD(now.Second());

  lcd.setCursor(0, 1);
  lcd.print("Date ");
  lcd.print(now.Day());
  lcd.print("/");
  lcd.print(now.Month());
  lcd.print("/");
  lcd.print(now.Year());

  delay(1000);
}

void printTwoDigitsLCD(int number) {
  if (number < 10) {
    lcd.print("0");
  }
  lcd.print(number);
}
