// PROJECT  : RPN Calculator
// PURPOSE  : Introduction to writing a local Stack class
// COURSE   : ICS3U-E
// AUTHOR   : N. Willis
// DATE     : 2025 03 23
// MCU      : Arduino Nano (328P)
// STATUS   : Working
// REFERENCE: https://docs.arduino.cc/learn/contributions/arduino-creating-library-guide/

#include "Stack.h"          // Locally-implemented library containing a float Stack class
#include <LiquidCrystal.h>  // LCD control library

#define DURATION 300  // Delay for key input
#define KEYPADIN A5   // Analog input pin for one-wire keypad

const int rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 4;  // Connection pins for LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                 // Define the LCD

uint16_t thresholds[] = { 55, 58, 62, 66, 75, 81, 88, 97, 116, 132, 152, 179, 255, 341, 512, 1024 };  // The thresholds between each button's ADC value
char keys[] = { '+', 'E', '.', '0', '-', '3', '2', '1', '*', '6', '5', '4', '/', '9', '8', '7' };     // The input data for each key

Stack stack;  // Instance of the int8_t Stack class

char operand[32];            // Stores the current operand
float answer;                // Stores the answer
char strAnswer[32];          // Stores the answer as a string
bool answerPrinted = false;  // Has the answer been printed?

void setup() {
  lcd.begin(16, 2);  // Begin the LCD class
  splashScreen();    // Show the startup screen
}

void splashScreen() {
  lcd.clear();                  // Clear the screen
  lcd.setCursor(0, 0);          // Top left corner
  lcd.print("RPN Calculator");  // Startup text
  delay(3000);                  // Let it be seen
  lcd.clear();                  // Clear
  lcd.setCursor(0, 0);          // Top left corner
}

char getKey() {
  uint16_t value = 0;  // ADC value

  while ((value = analogRead(KEYPADIN)) == 0)  // Wait for a key ot be pressed
    ;
  delay(DURATION);                     // Delay for input to register
  uint8_t index = 0;                   // Index in the threshold array
  while (value > thresholds[index]) {  // Check all the indexes
    index++;
  }
  return keys[index];  // Return the correct input
}

void processInput(char ch) {  // Directs to another function based on the type of input
  switch (ch) {
    case '+':
    case '-':
    case '*':
    case '/':                         // Check if input is an operation
      if (stack.top < 1) {            // If there are less than 2 stack elements
        errorMessage("Stack Empty");  // Error message
      } else {
        answer = evaluate(ch);             // Call the evaluate function
        if (answer == INFINITY) {          // Check for dividing by 0
          errorMessage("Div By 0 Error");  // Error message
        } else {
          stack.push(answer);  // Push the answer on the stack
          displayAns(answer);  // Display the answer on the LCD
        }
      }
      break;
    case 'E':                             // Enter case
      if (operand[0] == '\0') {           // Check if there is an input
        errorMessage("No Input");         // Error message
      } else if (stack.isFull()) {        // Check for a full stack
        errorMessage("Stack Full, RST");  // Error message
      } else {
        stack.push(atof(operand));                   // Push the operand on the stack
        for (int i = 0; i < sizeof(operand); i++) {  // Clear the operand array
          operand[i] = '\0';
        }
        lcd.clear();          // Clear the LCD
        lcd.setCursor(0, 0);  // Reset the cursor
      }
      break;
    default: append(ch);  // If it's a digit or decimal, append it to the operand
  }
}

void append(char ch) {                              // Append function
  for (uint8_t i = 0; i <= sizeof(operand); i++) {  // Checks for the first empty operand array index
    if (operand[i] == '\0') {
      operand[i] = ch;
      operand[i + 1] = '\0';
      break;
    }
  }
  if (answerPrinted == true) {  // If there is an answer onscreen, clear it
    answerPrinted = false;
    lcd.clear();
  }
  lcd.print(ch);  // Print the digit onscreen
}

float evaluate(char op) {  // Evaluate function
  switch (op) {
    case '+': return stack.pop() + stack.pop(); break;
    case '-': return 0 - stack.pop() + stack.pop(); break;  // Topmost on the stack is subtracted from second-to-top
    case '*': return stack.pop() * stack.pop(); break;
    case '/': return 1 / stack.pop() * stack.pop(); break;  // Topmost must be the divisor
    default: return 0;
  }
}

void displayAns(float answer) {                 // Display answer
  dtostrf(answer, 0, 4, strAnswer);             // Convert to string with 4 decimal places
  lcd.setCursor(16 - strLength(strAnswer), 1);  // Bottom right align
  lcd.print(strAnswer);                         // Print it
  answerPrinted = true;
}

void errorMessage(char msg[]) {           // Generic function for error messages
  lcd.clear();                            // Clear
  lcd.setCursor(16 - strLength(msg), 1);  // Bottom right align
  lcd.print(msg);                         // Print it
  answerPrinted = true;
}

uint8_t strLength(char str[]) {  // Checks the length of a string
  uint8_t count = 0;             // Increases until a null char is found
  while (str[count] != '\0') {
    count++;
  }
  return count;
}

void loop() {
  processInput(getKey());  // Get the input and process it
}
