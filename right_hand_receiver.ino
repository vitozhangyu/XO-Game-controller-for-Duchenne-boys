//
// Right hand receiver connected to the Xbox
// This receiver is plugged to Xbox as a Keyboard/Mouse via USB cable. It receives sender's raw data
// and translate the data into controls of the game.
//
// References:
// MPU6050: https://create.arduino.cc/projecthub/Gabry295/head-mouse-with-mpu6050-and-arduino-micro-d42878
// nRF24L01: https://swf.com.tw/?p=1044
// nRF24L01: https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/

#include <Wire.h>
#include <I2Cdev.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <SPI.h>
#include "RF24.h"

RF24 rf24(8, 10); // CE, CSN

const byte addr[] = "1Node"; // set address same as the sender's - 1Node
const byte pipe = 1;  // pipe number
int vx = 0;
int vy = 0;
int code;
int i, s, d, a;
int c = 48;
void setup() {
  Serial.begin(9600);
  Mouse.begin();
  Keyboard.begin();
  rf24.begin();
  rf24.setChannel(83);  // set channel
  rf24.setPALevel(RF24_PA_MIN); // set power amplifier level to low to save energy, since required working distance is short
  rf24.setDataRate(RF24_250KBPS); // set KBPs
  rf24.openReadingPipe(pipe, addr);  // open the responded pipe and address
  rf24.startListening();  // start to receive sender's data
}

void loop() {
  if (rf24.available(&pipe)) { // confirm if the pipe is available for the sender

    rf24.read(&vx, sizeof(vx)); // read gyro data
    rf24.read(&vy, sizeof(vy));
    rf24.read(&code, sizeof(code)); // read code data
    // Mouse's moving distance and direction are given by vx and vy, which is used for aiming or adjusting player's perspective in the game
    // vx and vy is modified gyroscope data of X and Z axis, which is the angular speed of the gyro in X and Z axis, thus its speed and direction is mapped as mouse's moving distance and direction.
    Mouse.move(vx, vy);

    interact();
    switchWeapon();
    drop();
    attack();

    delay(10);
  }
}

void interact() {
  i = code / 1000;
  if (i == 1) { // if the first digit of the code is 1, which means 'interact' is pressed, right press mouse
    Mouse.press(MOUSE_RIGHT);
    Serial.println("interact");
  } else {
    Mouse.release(MOUSE_RIGHT);
  }
}

void switchWeapon() {
  s = code / 100;
  if (s == 1 or s == 11) {
    c = c + 1;
    if (c == 58) {
      c = 48;
    }
    Keyboard.press(c);
  } else {
    Keyboard.release(c);
  }
}

void drop () {
  d = code / 10;
  if (d == 1 or d == 11 or d == 101 or d == 111) {
    Keyboard.press(113); // key q
    Serial.println("drop");
  } else {
    Keyboard.release(113);
  }
}

void attack() {
  a = code % 10;
  if (a == 1) {
    Mouse.press(); // mouse left press
    Serial.println("attack");
  } else {
    Mouse.release();
  }
}
