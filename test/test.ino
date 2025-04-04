#define A 5
#define A_bar 4
#define B 3
#define B_bar 2

void setup() {
  pinMode(A, OUTPUT);
  pinMode(A_bar, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(B_bar, OUTPUT);

  digitalWrite(A, LOW);
  digitalWrite(A_bar, HIGH);
}

void loop() {
}
