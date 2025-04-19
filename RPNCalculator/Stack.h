#ifndef Stack_h  // Make sure the library is not included multiple times
#define Stack_h

#include "Arduino.h"  // Allows inclusion in the sketch

class Stack {  // Define the stack class

public:
  Stack();  // The main stack function
  // Stack(uint8_t size);
  void push(float data);  // Pushes data onto the stack
  float pop();            // Pops data from the stack
  boolean isEmpty();      // True if stack is empty
  boolean isFull();       // True if stack is full
  float peek();           // Returns top stack element
  int8_t top;             // Stores the position of the top element
private:
  uint8_t _size = 20;  // Size of the stack
  float _stack[20];    // The stack array, to hold the data
};

#endif