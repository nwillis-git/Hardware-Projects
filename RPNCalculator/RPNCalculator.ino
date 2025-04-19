// PROJECT  : RPN Calculator
// PURPOSE  : Introduction to writing a local Stack class
// COURSE   : ICS3U-E
// AUTHOR   : C. D'Arcy
// DATE     : 2025 03 23
// MCU      : Arduino Nano (328P)
// STATUS   : Working
// REFERENCE: https://docs.arduino.cc/learn/contributions/arduino-creating-library-guide/

#include "Stack.h"  //Locally-implemented library containing a int8_t Stack class
#include <LiquidCrystal.h>

#define DURATION 300
#define KEYPADIN A5

const int rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

uint16_t thresholds[] = { 55, 58, 62, 66, 75, 81, 88, 97, 116, 132, 152, 179, 255, 341, 512, 1024 };
char keys[] = { '+', 'E', '.', '0', '-', '3', '2', '1', '*', '6', '5', '4', '/', '9', '8', '7' };

Stack stack;  //instance of the int8_t Stack class

char operand[32];
float answer;
char strAnswer[32];
bool answerPrinted = false;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
}

char getKey() {
  uint16_t value = 0;

  while ((value = analogRead(KEYPADIN)) == 0)
    ;
  delay(DURATION);
  uint8_t index = 0;
  while (value > thresholds[index]) {
    index++;
  }
  return keys[index];
}

void processInput(char ch) {
  switch (ch) {
    case '+':
    case '-':
    case '*':
    case '/':
      if (stack.top < 1) {
        errorMessage("Stack Empty");
      } else {
        answer = evaluate(ch);
        if (answer == INFINITY) {
          errorMessage("Div By 0 Error");
        } else {
          stack.push(answer);
          displayAns(answer);
        }
      }
      break;
    case 'E':
      if (operand[0] == '\0') {
        errorMessage("No Input");
      } else if (stack.isFull()) {
        errorMessage("Stack Full, RST");
      } else {
        stack.push(atof(operand));
        for (int i = 0; i < sizeof(operand); i++) {
          operand[i] = '\0';
        }
        lcd.clear();
        lcd.setCursor(0, 0);
      }
      break;
    default: append(ch);
  }
}

void append(char ch) {
  for (uint8_t i = 0; i <= sizeof(operand); i++) {
    if (operand[i] == '\0') {
      operand[i] = ch;
      operand[i + 1] = '\0';
      break;
    }
  }
  if (answerPrinted == true) {
    answerPrinted = false;
    lcd.clear();
  }
  lcd.print(ch);
}

float evaluate(char op) {
  switch (op) {
    case '+': return stack.pop() + stack.pop(); break;
    case '-': return 0 - stack.pop() + stack.pop(); break;
    case '*': return stack.pop() * stack.pop(); break;
    case '/': return 1 / stack.pop() * stack.pop(); break;  // Topmost must be the divisor
    default: return 0;
  }
}

void displayAns(float answer) {
  dtostrf(answer, 0, 4, strAnswer);
  lcd.setCursor(16 - strLength(strAnswer), 1);
  lcd.print(strAnswer);
  answerPrinted = true;
}

void errorMessage(char msg[]) {
  lcd.clear();
  lcd.setCursor(16 - strLength(msg), 1);
  lcd.print(msg);
  answerPrinted = true;
}

uint8_t strLength(char str[]) {
  uint8_t count = 0;
  while (str[count] != '\0') {
    count++;
  }
  return count;
}

void loop() {
  processInput(getKey());
}