// PROJECT: CharlieClock
// PURPOSE: Create a clock to learn about charlieplexing and to get back into the rhythm of ACES projects
// COURSE: ICS4U-E
// AUTHOR: N. Willis
// DATE: 2025 10 04
// MCU: ATMega328p
// STATUS: Working
// REFERENCE: http://darcy.rsgc.on.ca/ACES/TEI4M/CharlieClock/V2DataModel.txt

#include <Wire.h>  // For I2C Communitcaiton

#define DS1307_ADDR 0x68  // DS1307 Address

// CharlieClock Out Pins
#define P1 12
#define P2 11
#define P3 10
#define P4 9
#define P5 8
#define P6 7
#define P7 6
#define P8 5
#define P9 4
#define P10 A1
#define P11 A2
#define P12 A3

#define AMPMPIN 2  // PM Indicator

#define TIMESETSNOOZE 3  // Double-purpose button
#define MINUTESET A6     // Debounced
#define HOURSET A7       // Debounced

#define ALMTIMESWITCH 13  // Also toggles alarm on/off
#define ALARMPIN A0       // Buzzer Out

struct LED {  // Template for one LED on the clock
  uint8_t anode;
  uint8_t cathode;
};

uint8_t cathodePlaceholder;  // Holds the currently enabled cathode
uint8_t anodePlaceholder;    // And the anode

// Hour LEDS
LED hours[] = {
  { P9, P1 }, { P10, P1 }, { P11, P1 }, { P12, P1 }, { P12, P11 }, { P5, P1 }, { P4, P1 }, { P3, P1 }, { P2, P1 }, { P6, P1 }, { P7, P1 }, { P8, P1 }
};
// Minute LEDS
LED minutes[] = {
  { P8, P2 }, { P9, P2 }, { P10, P2 }, { P11, P2 }, { P12, P2 }, { P1, P12 }, { P2, P12 }, { P3, P12 }, { P4, P12 }, { P5, P12 }, { P6, P12 }, { P7, P12 }, { P8, P12 }, { P9, P12 }, { P10, P12 }, { P11, P12 }, { P10, P11 }, { P9, P11 }, { P8, P11 }, { P7, P11 }, { P6, P11 }, { P5, P11 }, { P4, P11 }, { P3, P11 }, { P2, P11 }, { P1, P11 }, { P12, P10 }, { P11, P10 }, { P9, P10 }, { P8, P10 }, { P7, P10 }, { P6, P10 }, { P5, P10 }, { P4, P10 }, { P3, P10 }, { P2, P10 }, { P1, P10 }, { P7, P4 }, { P6, P4 }, { P5, P4 }, { P3, P4 }, { P2, P4 }, { P1, P4 }, { P1, P3 }, { P2, P3 }, { P4, P3 }, { P5, P3 }, { P6, P3 }, { P7, P3 }, { P8, P3 }, { P9, P3 }, { P10, P3 }, { P11, P3 }, { P12, P3 }, { P1, P2 }, { P3, P2 }, { P4, P2 }, { P5, P2 }, { P6, P2 }, { P7, P2 }
};
// Second LEDS
LED seconds[] = {
  { P1, P5 }, { P2, P5 }, { P3, P5 }, { P4, P5 }, { P6, P5 }, { P7, P5 }, { P8, P5 }, { P9, P5 }, { P10, P5 }, { P11, P5 }, { P12, P5 }, { P8, P4 }, { P9, P4 }, { P10, P4 }, { P11, P4 }, { P12, P4 }, { P12, P9 }, { P11, P9 }, { P10, P9 }, { P8, P9 }, { P7, P9 }, { P6, P9 }, { P5, P9 }, { P4, P9 }, { P3, P9 }, { P2, P9 }, { P1, P9 }, { P12, P8 }, { P11, P8 }, { P10, P8 }, { P9, P8 }, { P7, P8 }, { P6, P8 }, { P5, P8 }, { P4, P8 }, { P3, P8 }, { P2, P8 }, { P1, P8 }, { P12, P7 }, { P11, P7 }, { P10, P7 }, { P9, P7 }, { P8, P7 }, { P6, P7 }, { P5, P7 }, { P4, P7 }, { P3, P7 }, { P2, P7 }, { P1, P7 }, { P1, P6 }, { P2, P6 }, { P3, P6 }, { P4, P6 }, { P5, P6 }, { P7, P6 }, { P8, P6 }, { P9, P6 }, { P10, P6 }, { P11, P6 }, { P12, P6 }
};

uint8_t sec;  // Current RTC Seconds
uint8_t min;  // Minutes
uint8_t hr;   // Hours
bool pm;      // Is it afternoon?

uint8_t almMin = 0;  // Stores the alarm's minutes
uint8_t almHr = 7;   // And hours
bool almPM = false;  // Wake up in the afternoon, if you really want to

bool settingTime = false;  // Switches between the two main modes

bool almFlag = false;     // Alarm is currently active
bool almBuffer = true;    // Alarm has already been active at this time
bool snoozeFlag = false;  // Snoozing
uint32_t snoozeStamp;     // Timestamp to count snooze time from

void setup() {
  Wire.begin();
  attachInterrupt(digitalPinToInterrupt(TIMESETSNOOZE), ISR_TimeSetSnooze, FALLING);  // Would be a waste to check every frame
  pinMode(ALARMPIN, OUTPUT);
  pinMode(AMPMPIN, OUTPUT);

  Wire.beginTransmission(DS1307_ADDR);  // Start interfacing with RTC
  Wire.write(0x00);                     // Start at seconds register
  Wire.write(decToBcd(0) & 0x7F);       // Seconds = 0, Clock Halt bit cleared
  Wire.write(decToBcd(0));              // Minutes = 0
  Wire.write(decToBcd(0));              // Hours = 0 (24h mode)
  Wire.endTransmission();
}

void ISR_TimeSetSnooze() {
  settingTime = true;  // Self explanatory
}

uint8_t bcdToDec(byte val) {
  return (val / 16 * 10) + (val % 16);  // Convert BCD to decimal
}

uint8_t decToBcd(uint8_t val) {
  return ((val / 10 * 16) + (val % 10));  // Convert Decimal to BCD
}

void LEDON(uint8_t timeUnit, uint8_t LEDNUM) {
  pinMode(cathodePlaceholder, INPUT);  // Reset the last active anode/cathode
  pinMode(anodePlaceholder, INPUT);

  switch (timeUnit) {
    case 0:                                          // Seconds
      cathodePlaceholder = seconds[LEDNUM].cathode;  // Ready the correct cathode
      anodePlaceholder = seconds[LEDNUM].anode;      // And anode
      break;
    case 1:  // Minutes
      cathodePlaceholder = minutes[LEDNUM].cathode;
      anodePlaceholder = minutes[LEDNUM].anode;
      break;
    case 2:  // Hours
      cathodePlaceholder = hours[LEDNUM].cathode;
      anodePlaceholder = hours[LEDNUM].anode;
      break;
  }

  pinMode(cathodePlaceholder, OUTPUT);
  pinMode(anodePlaceholder, OUTPUT);
  digitalWrite(anodePlaceholder, HIGH);  // Light up the LED!

  delay(1);  // POV Delay
}

void setAndAccel(uint8_t &value, uint8_t cap, uint8_t pin, uint16_t interval) {  // Accelerate the longer you hold a button
  while (analogRead(pin) < 512) {                                                // I ran out of digital I/O pins
    uint32_t timeStamp = millis();                                               // Save the current time in ms
    value = value >= cap ? 0 : value + 1;                                        // Increase value by 1, loop back to 0 if at max
    while (millis() - interval < timeStamp && analogRead(pin) < 512) {           // Wait in this loop for interval
      if (!digitalRead(ALMTIMESWITCH)) {                                         // Setting time
        displayTime();                                                           //
      } else {                                                                   // Setting alarm
        displayAlarm();
      }
    }
    if (interval > 100) {  // Max speed
      interval *= 0.9;     // Increase speed exponentially
    }
  }
}

void timeSet() {
  while (!digitalRead(TIMESETSNOOZE)) {  // Do this until the button is released
    uint16_t interval = 500;
    if (!digitalRead(ALMTIMESWITCH)) {            // Setting time
      setAndAccel(min, 59, MINUTESET, interval);  // Minute button check
      setAndAccel(hr, 23, HOURSET, interval);     // Hour button check
      displayTime();                              // Admire the time
    } else {                                      // Alarm setting
      setAndAccel(almMin, 59, MINUTESET, interval);
      setAndAccel(almHr, 23, HOURSET, interval);
      displayAlarm();
    }
  }
  writeRTC(sec, min, hr);  // Write the newly set time to the RTC
  settingTime = false;     // Done!
}

void getRTC() {
  Wire.beginTransmission(DS1307_ADDR);
  Wire.write(0x00);  // Position at seconds register
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDR, 3);  // Read seconds, minutes, hours
  if (Wire.available() >= 3) {
    sec = bcdToDec(Wire.read() & 0x7F);  // Mask Clock Halt bit
    min = bcdToDec(Wire.read());
    hr = bcdToDec(Wire.read() & 0x3F);  // 24-hour mode
    if (hr == 24) {
      hr = 0;  // Cap hours
    }
    if (almMin == min && almHr == hr) {                // Check for alarm
      if (!almBuffer && digitalRead(ALMTIMESWITCH)) {  // If the alarm is on and hasn't just been turned off & on
        pinMode(cathodePlaceholder, INPUT);            // Stop showing time
        pinMode(anodePlaceholder, INPUT);              //
        almFlag = true;                                // Sound the alarm!
      }
    } else {
      almBuffer = false;  // If it's not alarm time anymore, ready it for the next time
    }
  }
}

void writeRTC(uint8_t second, uint8_t minute, uint8_t hour) {
  Wire.beginTransmission(DS1307_ADDR);
  Wire.write(0x00);              // Start at register 0
  Wire.write(decToBcd(second));  // Seconds
  Wire.write(decToBcd(minute));  // Minutes
  Wire.write(decToBcd(hour));    // Hours
  Wire.endTransmission();
}

void displayTime() {
  LEDON(0, sec);
  LEDON(1, min);
  if (hr < 12) {
    LEDON(2, hr);
  } else {
    LEDON(2, hr - 12);  // Convert from 24hr to 12hr
  }
  if (hr > 11) {  // AM/PM Check
    digitalWrite(AMPMPIN, LOW);
  } else {
    digitalWrite(AMPMPIN, HIGH);
  }
}

void displayAlarm() {
  LEDON(1, almMin);  // Show minutes
  if (almHr < 12) {
    LEDON(2, almHr);  // Show hours
  } else {
    LEDON(2, almHr - 12);  // Convert from 24hr to 12hr
  }
  if (almHr > 11) {  // AM/PM Check
    digitalWrite(AMPMPIN, LOW);
  } else {
    digitalWrite(AMPMPIN, HIGH);
  }
}

void alarm() {
  uint32_t timeStamp = millis();                // Save the time in ms
  while (millis() - 500 < timeStamp) {          // Loop for 500 ms
    uint64_t timeStampMicros = micros();        // Save the time in µs
    digitalWrite(ALARMPIN, HIGH);               // Alarm requires PWM signal
    while (micros() - 500 < timeStampMicros) {  // Wait 500 µs and check for snooze
      if (!digitalRead(TIMESETSNOOZE)) {
        snoozeFlag = true;
        snoozeStamp = millis();
      }
    }
    timeStampMicros = micros();                 // Save the new time in µs
    digitalWrite(ALARMPIN, LOW);                // Alternate alarm off
    while (micros() - 500 < timeStampMicros) {  // Check for 500 µs
      if (!digitalRead(TIMESETSNOOZE)) {
        snoozeFlag = true;
        snoozeStamp = millis();
      }
    }
  }
  timeStamp = millis();                 // Now turn the alarm off for 500
  while (millis() - 500 < timeStamp) {  // No one wants an alarm that's a constant tone
    if (!digitalRead(TIMESETSNOOZE)) {  // CHeck for snooze
      snoozeFlag = true;
      snoozeStamp = millis();
    }
    if (!digitalRead(ALMTIMESWITCH)) {  // Check for alarm off
      almFlag = false;
      almBuffer = true;  // Don't let the alarm turn back on in the same minute
    }
  }
}

void loop() {
  if (settingTime) {
    timeSet();
  } else if (almFlag && !snoozeFlag) {
    alarm();
  } else {
    displayTime();
    getRTC();
  }

  if (snoozeFlag) {
    if (snoozeStamp + 540000 < millis()) { // Snooze for the standard 9 minutes
      snoozeFlag = false;
    }
  }
}