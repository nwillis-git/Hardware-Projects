// PROJECT: Snake
// PURPOSE: A snake game to serve as a test of the Pocket ACE
// COURSE: ICS4U-E
// AUTHOR: N. Willis
// DATE: 2025 01 20
// MCU: 328P
// STATUS: Not Working
// REFERENCE: https://en.wikipedia.org/wiki/Snake_(video_game_genre)

#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

#define LEFT_BTN 5
#define UP_BTN 2
#define DOWN_BTN 4
#define RIGHT_BTN 3

#define TICKTIME 250
#define GRIDCELLSIZE 8

U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/13, /* data=*/11, /* CS=*/10, /* reset=*/8);

struct posVector {
  uint8_t xPos;
  uint8_t yPos;
};
posVector snake[256] = { { 32, 0 }, { 24, 0 }, { 16, 0 }, { 8, 0 }, { 0, 0 } };
uint8_t snakeLength = 5;
posVector foodPos = { 64, 32 };

uint8_t inputs[4] = { LEFT_BTN, UP_BTN, DOWN_BTN, RIGHT_BTN };
int8_t direction = 2;
uint32_t stamp = 0;

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(inputs[i], INPUT);
  }
  randomSeed(analogRead(A0));
  foodPos = { random(0, 128) & ~7, random(0, 64) & ~7 };
}

int8_t checkInput() {
  for (uint8_t i = 0; i < 4; i++) {
    if (!digitalRead(inputs[i])) {
      switch (i) {
        case 0: return -2;
        case 1: return 1;
        case 2: return -1;
        case 3: return 2;
      }
    }
  }
  return 0;
}

void updateSnakePos() {
  if (direction != 0) {
    posVector nextPos = snake[0];
    for (uint8_t i = 1; i < snakeLength; i++) {
      posVector lastPos = snake[i];
      snake[i] = nextPos;
      nextPos = lastPos;
    }
  }

  switch (direction) {
    case -2:
      snake[0].xPos -= GRIDCELLSIZE;
      break;
    case -1:
      snake[0].yPos += GRIDCELLSIZE;
      break;
    case 1:
      snake[0].yPos -= GRIDCELLSIZE;
      break;
    case 2:
      snake[0].xPos += GRIDCELLSIZE;
      break;
    default:
      break;
  }
}

void foodCheck() {
  bool foodValid = true;
  if (snake[0].xPos == foodPos.xPos && snake[0].yPos == foodPos.yPos) {
    do { 
      foodPos.xPos = random(0, 128) & ~7;
      foodPos.yPos = random(0, 64) & ~7;
      for (uint8_t i = 0; i < snakeLength; i++) {
        if (foodPos.xPos == snake[i].xPos && foodPos.yPos == snake[i].yPos) {
          foodValid = false;
          break;
        }
        foodValid = true;
      }
    } while (!foodValid);
    snake[snakeLength].xPos = snake[snakeLength - 1].xPos;
    snake[snakeLength].yPos = snake[snakeLength - 1].yPos;
    snakeLength++;
  }
}

void loseCheck() {
  if (snake[0].xPos< 0 | snake[0].xPos > 127 | snake[0].yPos< 0 | snake[0].yPos > 63) {
    lose();
  }
  for (uint8_t i = 1; i < snakeLength; i++) {
    if (snake[i].xPos == snake[0].xPos && snake[i].yPos == snake[0].yPos) {
      lose();
    }
  }
}

void lose() {
  snake[0] = { 32, 0 };
  snake[1] = { 24, 0 };
  snake[2] = { 16, 0 };
  snake[3] = { 8, 0 };
  snake[4] = { 0, 0 };
  snakeLength = 5;
  direction = 2;
}

void gameTick() {
  updateSnakePos();
  foodCheck();
  loseCheck();
}


void loop(void) {
  if (millis() - stamp >= TICKTIME) {
    stamp += TICKTIME;
    gameTick();
  }

  int8_t input = checkInput();
  if (input != 0 && input != -direction) {
    direction = input;
  }

  u8g2.firstPage();
  do {
    for (uint8_t i = 0; i < snakeLength; i++) {
      u8g2.drawBox(snake[i].xPos, snake[i].yPos, GRIDCELLSIZE, GRIDCELLSIZE);
    }
    u8g2.drawBox(foodPos.xPos, foodPos.yPos, GRIDCELLSIZE, GRIDCELLSIZE);
  } while (u8g2.nextPage());
}