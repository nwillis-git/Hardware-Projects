#ifndef Stack_h
#define Stack_h

#include "Arduino.h"

class Stack {

public:
  Stack();
  // Stack(uint8_t size);
  void push(float data);
  float pop();
  boolean isEmpty();
  boolean isFull();
  float peek();
  int8_t top;
private:
  uint8_t _size = 20;
  float _stack[20];
};

#endif