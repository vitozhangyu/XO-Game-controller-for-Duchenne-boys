//
// Left hand receiver connected to the Xbox
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

const byte addr[] = "2Node"; // set address same as the sender's - 2Node
const byte pipe = 1; // pipe number
int x, vx, walk;
long zipcode;
int code;
int sp, i, sn, j;
void setup() {
  Serial.begin(9600);
  Mouse.begin();
  Keyboard.begin();
  rf24.begin();
  rf24.setChannel(85);  // set channel
  rf24.setPALevel(RF24_PA_MIN); // set power amplifier level to low to save energy, since required working distance is short
  rf24.setDataRate(RF24_250KBPS); // set KBPs
  rf24.openReadingPipe(pipe, addr);  // open the responded pipe and address
  rf24.startListening();  // start to receive sender's data
}

void loop() {
  if (rf24.available(&pipe)) { // confirm if the pipe is available for the sender

    //rf24.read(&vx, sizeof(vx)); // read gyro data
    rf24.read(&zipcode, sizeof(zipcode)); // read code data
    //rf24.read(&walk, sizeof(walk)); // read walk data
    walk = zipcode % 1000;
    code = zipcode / 1000;
    //slide(); // slide left/right
    sprint();
    inventory();
    sneak();
    jump();
    walkk(); //walk foreward/backward

    //Serial.println(x);
    Serial.println(walk);
    //delay(10);
  }
}

/* This slide function is trying to transfer the modified gyro raw data in z axis into movement controls.
   When user holds the left controller with his left hand, if he moves the controller to the right beyond
   threshold angular speed (what gyro tests), the receiver will control the Xbox to press 'a' to go left
   in game, then if he moves to the left, the character will stop in game, if then he moves to the left,
   the game character will go left.
*/
//void slide() {
//  if (vx == -1 or vx == 525 or vx == 536 ) { // Because the MPU6050 here used randomly return -1 of vx when it is totally still, thus here -1 is filtered out.
//    vx = 0;
//  } else {
//    vx = vx;
//  }
//
//  x = x + vx; // Because vx is modified based on raw gyroscope data, which is the angular speed of MPU6050. The speed will turn back to 0 when it is still, but x should not, thus x is accumulated by vx.
//
//  if (x > 600) { // Because x is accumulated by vx, thus it is necessary to give it a range, otherwise x will be too high or low.
//    x = 400; // set x back to 400, which is the threshold when it is over 600.
//  } else if (x < -600) {
//    x = -400;
//  } else {
//    x = x;
//  }
//
//  if (x > 399) { // threshold is 400, thus over 399
//    //Serial.println("left");
//    Keyboard.release(100); // when x is over 399, slide to left
//    Keyboard.press(97); // press a to go left
//  } else if (x < -399) { // when x is under -399, slide to right
//    //Serial.println("right");
//    Keyboard.release(97);
//    Keyboard.press(100); //press d to go right
//  } else { // if not, stay still
//    //Serial.println("no");
//    Keyboard.release(100); //release d
//    Keyboard.release(97); //release a
//  }
//  Serial.println(vx);
//}

void sprint() {
  sp = code / 1000; // if the first digit of the code is 1, which means 'sprint' is pressed, press key 17.
  if (sp == 1) {
    Keyboard.press(114); //key r
    Serial.println("sprint");
  } else {
    Keyboard.release(114);
  }
}

void inventory() {
  i = code / 100;
  if (i == 1 or i == 11) {
    Keyboard.press(101); //key e
    Serial.println("inventory");
  } else {
    Keyboard.release(101);
  }
}

void sneak () {
  sn = code / 10;
  if (sn == 1 or sn == 11 or sn == 101 or sn == 111) {
    Keyboard.press(111); //key o
    Serial.println("sneak");
  } else {
    Keyboard.release(111);
  }
}

void jump() {
  j = code % 10;
  if (j == 1) {
    Keyboard.press(32); // key space
    Serial.println("jump");
  } else {
    Keyboard.release(32);
  }
}

void walkk () {
  if (walk < 800 && walk > 0) { // reason is unknown (could be the delay) that 'walk' data repetitively return 0 to the receiver, thus 0 is filtered out
    Keyboard.release(115); //release key s
    Keyboard.press(119); //press key w to go foreward
    Serial.println("front");
  } else if ( walk > 900) {
    Keyboard.release(119); //release key w
    Keyboard.press(115); //press key s to go backward
    Serial.println("back");
  } else {
    Keyboard.release(115); //release key s
    Keyboard.release(119); //release key w
  }
  //Serial.println(walk);
}
