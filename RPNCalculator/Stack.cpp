#include "Arduino.h"
#include "Stack.h"

Stack::Stack() {
  top = -1;  // The top starts at -1 so that the first element is at position 0
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
    _stack[++top] = data;  // Push the data if the stack is not full
  }
}

float Stack::pop() {
  if (!isEmpty()) {
    return _stack[top--];  // Pop the top element if there is one
  }
}

float Stack::peek() {
  return _stack[top];  // Return the top element without changing it
}

boolean Stack::isEmpty() {
  return top == -1;  // True if the stack is empty
}

boolean Stack::isFull() {
  return top == _size - 1;  // True if the stack is full
}