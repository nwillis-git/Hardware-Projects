#include "Arduino.h"
#include "Stack.h"

Stack::Stack() {
  _top = -1;  // The top starts at -1 so that the first element is at position 0
}

void Stack::push(float data) {
  if (!isFull()) {
    _stack[++_top] = data;  // Push the data if the stack is not full
  }
}

float Stack::pop() {
  if (!isEmpty()) {
    return _stack[_top--];  // Pop the top element if there is one
  }
}

float Stack::peek() {
  return _stack[_top];  // Return the top element without changing it
}

boolean Stack::isEmpty() {
  return _top == -1;  // True if the stack is empty
}

boolean Stack::isFull() {
  return _top == _size - 1;  // True if the stack is full
}
