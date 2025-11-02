// PROJECT:   AT28C16 EEPROM Burner
// PURPOSE:   Simple reading and writing for the EEPROM used for CHUMP
// COURSE:    ICS4U-E
// AUTHOR:    N. Willis
// DATE:      2025 01 02
// MCU:       328P (Nano)
// STATUS:    Working
// REFERENCE: http://darcy.rsgc.on.ca/ACES/Datasheets/AT28C16.pdf

#define Addr0 A3  // Only 4 Address bits are needed (16-line programs)
#define Addr1 A2
#define Addr2 A1
#define Addr3 A0

#define IO0 2  // const LSB
#define IO1 3
#define IO2 4
#define IO3 5  // const MSB
#define IO4 6  // opcode LSB
#define IO5 7
#define IO6 8
#define IO7 9  // opcode MSB

#define OE 10  // Manipulated for read or write mode
#define WE 11

uint8_t addrPins[4] = { Addr0, Addr1, Addr2, Addr3 };            // Used to address registers of EEPROM
uint8_t IOPins[8] = { IO0, IO1, IO2, IO3, IO4, IO5, IO6, IO7 };  // Access to registers

uint8_t bitshiftProgram[16] = {
  0b00000001,  // LOAD 1
  0b01100000,  // STORETO 0
  0b10000000,  // READ 0
  0b00000000,  // LOAD IT
  0b10000000,  // READ 0
  0b00110000,  // ADD IT
  0b11101001,  // IFZERO 0
  0b11000010   // GOTO 1
};

uint8_t feinbergExample[16] = {
  0b10000000,  // READ 2
  0b00010000,  // LOAD IT
  0b00100001,  // ADD 1
  0b01100010,  // STORETO 2
  0b11000000   // GOTO 0
}

uint8_t FourBitTo7Seg[16] = {
  0b11111100,  // 0
  0b01100000,  // 1
  0b11011010,  // 2
  0b11110010,  // 3
  0b01100110,  // 4
  0b10110110,  // 5
  0b10111110,  // 6
  0b11100000,  // 7
  0b11111110,  // 8
  0b11100110,  // 9
  0b11101110,  // A
  0b00111110,  // B
  0b10011100,  // C
  0b01111010,  // D
  0b10011110,  // E
  0b10001110   // F
};

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  pinMode(OE, OUTPUT);
  pinMode(WE, OUTPUT);
  digitalWrite(WE, HIGH);
  for (uint8_t i = 0; i < sizeof(addrPins); i++) {
    pinMode(addrPins[i], OUTPUT);  // Set all address pins as output
  }

  write16(0, FourBitTo7Seg);  // Write the lookup table to an EEPROM
  read16(0);                  // Verify it's there
}

void setAddr(uint8_t addr) {
  for (uint8_t i = 0; i <= 3; i++) {
    uint8_t bitVal = (addr >> i) & 1;   // Shift the bit to LSB position and mask
    digitalWrite(addrPins[i], bitVal);  // Set it if 1
  }
}

uint8_t readAddr(uint8_t addr) {
  digitalWrite(OE, LOW);  // Configure as read mode
  setAddr(addr);          // Select a register

  uint8_t readVal = 0;  // Stores the result

  for (uint8_t i = 0; i < sizeof(IOPins); i++) {
    pinMode(IOPins[i], INPUT);  // Set as input to read

    if (digitalRead(IOPins[i])) {
      readVal |= (1 << i);  // Append a 1
    } else {
      readVal &= ~(1 << i);  // or a 0
    }
  }

  return readVal;
}

void writeAddr(uint8_t addr, uint8_t value) {
  digitalWrite(OE, HIGH);  // Configure as write mode
  setAddr(addr);           // Select a register

  for (uint8_t i = 0; i < sizeof(IOPins); i++) {
    pinMode(IOPins[i], OUTPUT);                 // Set as output to write
    digitalWrite(IOPins[i], (value >> i) & 1);  // Write each bit
  }

  digitalWrite(WE, LOW);  // Pulse low to write it
  delay(1);
  digitalWrite(WE, HIGH);
}

void write16(uint8_t startAddr, uint8_t program[16]) {
  for (uint8_t i = 0; i < 16; i++) {
    writeAddr(startAddr + i, program[i]);  // Call writeAddr for all 16 addresses
    delay(50);
  }
}

void binPrint(uint8_t value, uint8_t length) {   // Prints an integer as binary
  for (uint8_t bit = length; bit >= 1; bit--) {  // Count down to print MSB first
    if ((value >> bit - 1) & 1) {                // Check if the i-th bit is set
      Serial.print('1');                         // Print 1
    } else {                                     //
      Serial.print('0');                         // or 0
    }
  }
}

void read16(uint8_t startAddr) {
  for (uint8_t addr = startAddr; addr < startAddr + 16; addr++) {
    binPrint(addr, 4);            // Print the 4-bit address
    Serial.print("\t:\t");        // Align with a colon to read easily
    binPrint(readAddr(addr), 8);  // Print the contents of the current address
    Serial.println();             // New line
  }
}

void loop() {
}