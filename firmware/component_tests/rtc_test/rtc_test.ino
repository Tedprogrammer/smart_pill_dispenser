#include <ThreeWire.h>
#include <RtcDS1302.h>

// DS1302 wiring: DAT, CLK, RST
ThreeWire myWire(19, 18, 5);
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  Serial.begin(115200);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC time invalid. Setting time from computer.");
    Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not running. Starting it.");
    Rtc.SetIsRunning(true);
  }

  Serial.println("RTC test started");
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();

  Serial.print("Time: ");
  printTwoDigits(now.Hour());
  Serial.print(":");
  printTwoDigits(now.Minute());
  Serial.print(":");
  printTwoDigits(now.Second());

  Serial.print(" Date: ");
  Serial.print(now.Day());
  Serial.print("/");
  Serial.print(now.Month());
  Serial.print("/");
  Serial.println(now.Year());

  delay(1000);
}

void printTwoDigits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}
