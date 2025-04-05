// PROJECT   : KTANE Central Unit
// PURPOSE   : The central unit for a tabletop recreation of the game Keep Talking and Nobody Explodes
// COURSE    : ICS3U-E
// AUTHOR    : N. Willis
// DATE      : 2025 04 06
// MCU       : 328P (Standalone)
// STATUS    : Working
// REFERENCE : https://keeptalkinggame.com/

#include <Wire.h>              // I2C Library
#include <Adafruit_GFX.h>      // GFX for OLED
#include <Adafruit_SSD1306.h>  // Driver library for OLED

//#define SERIALENABLE  //Uncomment if serial is required

#define SCREEN_WIDTH 128                                                   // Width of OLED screen
#define SCREEN_HEIGHT 32                                                   // Height of screen
#define OLED_RESET -1                                                      // Defualt value if no other reset pin used
#define SCREEN_ADDRESS 0x3C                                                // I2C Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Initiate Display

#define MASTER_ADDRESS 0x01  // I2C address of the motherboard MPU

#define TIMER_SR_DATA 12   // Data pin for the timer SR
#define TIMER_SR_LATCH 11  // Latch pin
#define TIMER_SR_CLK 10    // Clock pin

#define LABELS_SR_DATA 13   // Data for the LED board's SR
#define LABELS_SR_LATCH 16  // Latch
#define LABELS_SR_CLK 17    // Clock

#define D4 9  // Common anode for digit 4
#define D3 8  // digit 3
#define D2 7  // digit 2
#define D1 6  // digit 1

#define startButton 5     // Start button input pin
#define timeUpButton 3    // Increase time by 30 (pin 3 for interrupt)
#define timeDownButton 2  // Decrease time by 30 (pin 2 for interrupt)

#define strike1LED 0
#define strike2LED 1

bool startButtonState = 0;  // State of the start button

uint16_t startTime = 360;  // The default timer (6 mins)
uint16_t startStamp = 0;   // Stores the game start time
bool gameStarted = false;  // Is the game started?
bool gameOver = false;

uint8_t digits[10] = { ~B11111100, ~B01100000, ~B11011010, ~B11110010, ~B01100110, ~B10110110, ~B00111110, ~B11100000, ~B11111110, ~B11100110 };  // Binary values for digits on 7-seg display

uint8_t moduleData[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // vowel, even, frk, car, bat 40-59, bat 60-79, timer1, timer4, timer5, strikes
uint8_t moduleDataSize = sizeof(moduleData);                // Size of moduleData

uint8_t moduleAddresses[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t numModules = 0;  // Total number of plugged in modues, determined on game start
uint8_t numModulesCompleted = 0;
uint8_t MIN_I2C_ADDRESS = 0x02;  // minimum module address
uint8_t MAX_I2C_ADDRESS = 0xB;   // maximum module address

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 100;  // 50ms debounce time

void setup() {
#ifdef SERIALENABLE
  Serial.begin(9600);  // Begin serial monitor only if it is enabled
#endif
  randomSeed(analogRead(A0));  // A disconnected analog pin has a lot of interference

  gameStarted = false;  // The game has not started
  gameOver = false;

  pinMode(TIMER_SR_DATA, OUTPUT);  // All Shift Register pins are outputs
  pinMode(TIMER_SR_LATCH, OUTPUT);
  pinMode(TIMER_SR_CLK, OUTPUT);

  pinMode(LABELS_SR_DATA, OUTPUT);
  pinMode(LABELS_SR_LATCH, OUTPUT);
  pinMode(LABELS_SR_CLK, OUTPUT);

  pinMode(D4, OUTPUT);  // All digit common anodes are outputs
  pinMode(D3, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D1, OUTPUT);

  pinMode(startButton, INPUT);  // All buttons are inputs
  pinMode(timeUpButton, INPUT);
  pinMode(timeDownButton, INPUT);

  pinMode(strike1LED, OUTPUT);  // The strike LEDs are inputs and should start off
  pinMode(strike2LED, OUTPUT);
  digitalWrite(strike1LED, LOW);
  digitalWrite(strike2LED, LOW);

  digitalWrite(LABELS_SR_LATCH, LOW);
  shiftOut(LABELS_SR_DATA, LABELS_SR_CLK, MSBFIRST, 0);  // Clear labels
  digitalWrite(LABELS_SR_LATCH, HIGH);

  attachInterrupt(digitalPinToInterrupt(timeUpButton), ISR_startTimeUp, RISING);      // Attach interrupt to time up button
  attachInterrupt(digitalPinToInterrupt(timeDownButton), ISR_startTimeDown, RISING);  // Attach interrupt to time down button

  Wire.begin(MASTER_ADDRESS);      // Begin I2C
  Wire.onRequest(sendModuleData);  // To be called when a module requests data

  while (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {  // Begin communicaiton with the display
    delay(100);                                                   // Wait 100ms before retrying
  }

  display.setTextSize(2);       // Font size
  display.setTextColor(WHITE);  // Color
  display.setCursor(0, 0);      // Set cursor to the corner
  display.clearDisplay();       // Reset display
  display.display();            // Display the empty screen
}

void ISR_startTimeUp() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay) {
    lastInterruptTime = currentTime;
  }

  if (digitalRead(timeUpButton) && !gameStarted && startTime < 900) {  // Max out at 15 minute timer
    startTime += 30;                                                   // Add 30s
  }
}

void ISR_startTimeDown() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay) {
    lastInterruptTime = currentTime;
  }

  if (digitalRead(timeDownButton) && !gameStarted && startTime > 30) {  // Minimum is 30s
    startTime -= 30;                                                    // Subtract 30s
  }
}

void sendModuleData() {
  Wire.write(moduleData, 10);
}

void displayDigit(uint8_t digitPin, uint8_t value, bool decimalPoint) {
  digitalWrite(digitPin, HIGH);       // Select a pin via the common anodes
  digitalWrite(TIMER_SR_LATCH, LOW);  // Latch low
  if (decimalPoint) {
    shiftOut(TIMER_SR_DATA, TIMER_SR_CLK, LSBFIRST, digits[value] - 1);  // Subtract 1 to add decimal point
  } else {
    shiftOut(TIMER_SR_DATA, TIMER_SR_CLK, LSBFIRST, digits[value]);  // No decimal point, grab from digits array
  }
  digitalWrite(TIMER_SR_LATCH, HIGH);  // Latch high
  digitalWrite(digitPin, LOW);         // Deselect the common anode
  delayMicroseconds(250);              // Delay to give time to register
}

void displayOther(uint8_t digitPin, uint8_t value, bool decimalPoint) {
  digitalWrite(digitPin, HIGH);       // Select a pin via the common anodes
  digitalWrite(TIMER_SR_LATCH, LOW);  // Latch low
  if (decimalPoint) {
    shiftOut(TIMER_SR_DATA, TIMER_SR_CLK, LSBFIRST, value - 1);  // Subtract 1 to add decimal point
  } else {
    shiftOut(TIMER_SR_DATA, TIMER_SR_CLK, LSBFIRST, value);  // No decimal point, grab from digits array
  }
  digitalWrite(TIMER_SR_LATCH, HIGH);  // Latch high
  digitalWrite(digitPin, LOW);         // Deselect the common anode
  delayMicroseconds(250);              // Delay to give time to register
}

String generateSerialNumber() {         // Returns a string
  String serialNumber;                  // Stores the SN
  for (uint8_t i = 0; i < 6; i++) {     // Repeat for each digit of the SN
    char ch;                            // Temporarily stores each digit
    if (random(0, 2) == 1 || i == 5) {  // Last digit is always a number, the other 5 are alphanumeric
      ch = random(48, 58);              // Random character, 0-10
      if (i == 5) {
        moduleData[0] = !(ch % 2);
      }
    } else {
      do {                         // Do/While loop ensures this is run at least once
        ch = random(65, 91);       // Random character, A-Z
        switch (ch) {              // The switch statement checks for multiple cases of a variable
          case 65:                 // If the character is A,
          case 69:                 // E,
          case 73:                 // I,
          case 85:                 // or U,
            moduleData[1] = true;  // Then the SN contains a vowel
        }
      } while (ch == 79 || ch == 89);  // O can be confused with 0, and Y causes vowel confusion
    }
    serialNumber += ch;  // Add the char to the SN and repeat!
  }
  return (serialNumber);  // Return the SN
}

void moduleScan() {
  for (uint8_t address = MIN_I2C_ADDRESS; address <= MAX_I2C_ADDRESS; address++) {  // Check each possible I2C "slot"
    Wire.beginTransmission(address);                                                // Begin transmission
    for (uint8_t i = 0; i <= 9; i++) {
      Wire.write(moduleData[i]);
    }
    uint8_t error = Wire.endTransmission();  // Is there a module there?

    if (error == 0) {  // If there is a module there
      numModules++;    // Add 1 to the number of modules
      for (uint8_t i = 0; i < 10; i++) {
        if (moduleAddresses[i] == 0) {
          moduleAddresses[i] = address;
          break;
        }
      }
    }
  }
}

void gameSetup() {
  uint8_t labeledLEDs = 0;           // Reset labeled LEDs
  for (uint8_t i = 0; i < 8; i++) {  // cycle through the 8 LEDs
    bool coinFlip = random(0, 2);    // 50/50 chance for each LED to be on
    if (coinFlip) {
      labeledLEDs += (1 << i);
    }
  }

  if ((labeledLEDs >> 7) % 2 == 1) {  // Check for FRK
    moduleData[2] = true;
  }
  if ((labeledLEDs >> 6) % 2 == 1) {  // Check for CAR
    moduleData[3] = true;
  }

  digitalWrite(LABELS_SR_LATCH, LOW);
  shiftOut(LABELS_SR_DATA, LABELS_SR_CLK, LSBFIRST, labeledLEDs);  // Update LEDs
  digitalWrite(LABELS_SR_LATCH, HIGH);

  display.print("SN:");
  display.println(generateSerialNumber());  // Print out the randomly generated serial number
  display.print("BATT:");
  char batteryString[4];
  uint8_t battery = random(1, 101);      // Battery will be random between 1 and 100%
  if (battery >= 40 && battery <= 59) {  // Is battery between 40 and 59?
    moduleData[4] = true;
  } else if (battery >= 60 && battery <= 79) {  // Is battery between 60 and 79?
    moduleData[5] = true;
  }
  itoa(battery, batteryString, 10);  // Convert the integer to string
  display.print(batteryString);      // Print the battery
  display.println('%');              // With a % sign
  display.display();                 // Update display
  delay(10);

  moduleScan();  // Scan for number of modules
}

bool containsDigit(uint8_t num, uint8_t digit) {
  return (num / 10 == digit || num % 10 == digit);  // Does a given 2 digit number contain a given digit?
}

void modulePolling() {
  for (uint8_t i = 0; i < numModules; i++) {  // For all present modules...
    Wire.requestFrom(moduleAddresses[i], 1);  // Request data from the module
    while (!Wire.available())                 // If there is data to be sent
      ;
    uint8_t moduleUpdate = Wire.read();  // Read the data
    switch (moduleUpdate) {              // What does the data say?
      case 1:
        moduleData[9]++;  // +1 Strike
        if (moduleData[9] == 1) {
          digitalWrite(strike1LED, HIGH);  // Strike 1 LED on
        } else if (moduleData[9] == 2) {
          digitalWrite(strike2LED, HIGH);  // Strike 2 LED on
        } else if (moduleData[9] == 3) {
          loseGame();  // You're out!
        }
        Serial.print("Strike ");
        Serial.println(moduleData[9]);
        break;
      case 2:
        numModulesCompleted++;  // The module was completed
        Serial.print("Modules Completed: ");
        Serial.print(numModulesCompleted);
        Serial.print("/");
        Serial.println(numModules);

        if (numModulesCompleted == numModules) {  // Check if that was the last module
          winGame();
          Serial.println("You Win!");
        }
        break;
      default:
        break;
    }
  }
}

void loseGame() {
  while (true) {
    displayOther(D4, ~B10011110, false);  // E
    displayOther(D1, ~B10110110, false);  // S
    displayOther(D2, ~B11111100, false);  // O
    displayOther(D3, ~B00011100, false);  // L
  }
}

void winGame() {
  while (true) {
    displayOther(D4, ~B11101100, false);  // N
    displayOther(D1, ~B01100000, false);  // I
    displayOther(D2, ~B01111000, false);  // W
    displayOther(D3, ~B00111100, false);  // W
  }
}

void loop() {
  while (startButtonState == 0) {                 // Before the game starts...
    startButtonState = digitalRead(startButton);  // Test for a change in the start button's state

    uint8_t startMinutes = startTime / 60;  // Convert into minutes
    uint8_t startSeconds = startTime % 60;  // And seconds

    displayDigit(D4, startSeconds % 10, false);  // Each digit of the timer on its respective 7 segment digit
    displayDigit(D1, startSeconds / 10, false);
    displayDigit(D2, startMinutes % 10, true);
    displayDigit(D3, startMinutes / 10, false);

    if (containsDigit(startMinutes, 1) || containsDigit(startSeconds, 1)) {  // Does timer contain a 1 in any position?
      moduleData[6] = true;
    }
    if (containsDigit(startMinutes, 4) || containsDigit(startSeconds, 4)) {  // Does timer contain a 4 in any position?
      moduleData[7] = true;
    }
    if (containsDigit(startMinutes, 5) || containsDigit(startSeconds, 5)) {  // Does timer contain a 5 in any position?
      moduleData[8] = true;
    }

    if (startButtonState == HIGH) {
      gameSetup();                   // Run the game setup
      startStamp = millis() / 1000;  // Take a timestamp
      gameStarted = true;            // The game has started
      break;                         // Break out of the while loop
    }
  }

  uint16_t currentTime = startTime + startStamp - (millis() / 1000);  // Get the amount of time since the start button was pressed

  uint8_t currentMinutes = currentTime / 60;  // Separate current time into minutes
  uint8_t currentSeconds = currentTime % 60;  // And seconds

  if (containsDigit(currentMinutes, 1) || containsDigit(currentSeconds, 1)) {  // Does timer contain a 1 in any position?
    moduleData[6] = true;
  }
  if (containsDigit(currentMinutes, 4) || containsDigit(currentSeconds, 4)) {  // Does timer contain a 4 in any position?
    moduleData[7] = true;
  }
  if (containsDigit(currentMinutes, 5) || containsDigit(currentSeconds, 5)) {  // Does timer contain a 5 in any position?
    moduleData[8] = true;
  }

  displayDigit(D4, currentSeconds % 10, false);  // Each digit of the timer on its respective 7 segment digit
  displayDigit(D1, currentSeconds / 10, false);
  displayDigit(D2, currentMinutes % 10, true);
  displayDigit(D3, currentMinutes / 10, false);

  modulePolling();  // Check if a module needs data
}