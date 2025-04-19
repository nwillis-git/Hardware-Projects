#include "Arduino.h"
#include "Stack.h"

Stack::Stack() {
  top = -1;
}
/*
Stack::Stack(uint8_t size) {
  _top = -1;
  _size = size;
  _stack[_size];
}
*/

void Stack::push(float data) {
  if (!isFull()) {
    _stack[++top] = data;
  }
}

float Stack::pop() {
  if (!isEmpty()) {
    return _stack[top--];
  }
}

float Stack::peek() {
  return _stack[top];
}

boolean Stack::isEmpty() {
  return top == -1;
}

boolean Stack::isFull() {
  return top == _size - 1;
}