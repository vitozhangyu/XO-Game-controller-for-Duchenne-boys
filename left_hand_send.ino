//
// Left hand sender inside the left controller
// This sender is used to send the raw data of the controller to the left hand receiver via nRF024L01+.
//
// References:
// MPU6050: https://create.arduino.cc/projecthub/Gabry295/head-mouse-with-mpu6050-and-arduino-micro-d42878
// nRF24L01: https://swf.com.tw/?p=1044
// nRF24L01: https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/

#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <SPI.h>
#include "RF24.h"

MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz;
int vx;
int x ;
int sp, i, sn, j;
long code;
int sprint, inventory, sneak, jump, walk;
int c = 0;
RF24 rf24(8, 10); // CE, CSN
const byte addr[] = "2Node"; // name address as 2Node


void setup() {
  Serial.begin(9600);

  Wire.begin();
//  mpu.initialize();
//  if (!mpu.testConnection()) {
//    while (1);
//  }

  rf24.begin();
  rf24.setChannel(85);       // set channel
  rf24.openWritingPipe(addr); // set pipe
  rf24.setPALevel(RF24_PA_MIN);   // set power amplifier level to low to energy, since required working distance is short
  rf24.setDataRate(RF24_250KBPS); // set KBPs
  rf24.stopListening();       // stop listening to send data

  pinMode(A0, INPUT); // sprint
  pinMode(A1, INPUT); // inventory
  pinMode(A2, INPUT); // sneak
  pinMode(A3, INPUT); // jump
  pinMode(A4, INPUT); // walk
  pinMode(5, OUTPUT); // vibrate - vibration motor 1
  pinMode(6, OUTPUT); // motor - vibration motor 2
}

void loop() {
  //  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); // get acceleration and gyro data
  //  vx = -(gz - 100) / 200; // modify raw gz (gyro data in Z axis), this piece is referred to https://create.arduino.cc/projecthub/Gabry295/head-mouse-with-mpu6050-and-arduino-micro-d42878
  //  if (0< vx < 20) {
  //    vx = 0;
  //  } else {
  //    vx = vx;
  //  }
  // Serial.println(vx);

  sprint = analogRead(A2); // button for sprint, read from pressure sensor 0
  inventory = analogRead(A0); // button for inventory, read from pressure sensor 1
  sneak = analogRead(A1); // button for sneak, read from pressure sensor 2
  jump = analogRead(A3); // squeeze button for jump, read from pressure sensor 3
  walk = analogRead(A4); // button for walk, read from potentiometer

  translate(); //translate clicked pressure sensors into a 4 digits integer code
  long zipcode = code * 1000 + walk;
  Serial.println(code);
  Serial.println(zipcode);
  //rf24.write(&vx, sizeof(vx));  // send vx to left hand receiver
  rf24.write(&zipcode, sizeof(zipcode));  // send code to left hand receiver
  //rf24.write(&walk, sizeof(walk)); // send potentiometer data to left hand receiver

  vibrate(); // vibration motor 1 vibrates when 'jump' is pressed
  motor(); // vibration motor 2 vibrates once when 'sprint, inventory & sneak' is pressed
  //delay(10);
}

/* By our test on nRF024L01+, it can only send data 3 times in one loop, 3 rf24.write() can be called,
   and each data can be well received by the receiver, if more than 3 rf24.write() is called, the rest
   data will be ignored by the receiver, the receiver can only receiver 3 data from one sender in one
   loop. The reason is unknown, because from our research, one nRF024L01+ can pair with 6 others.
   This is why here a translate function is called. If one of the pressure sensor is pressed, its
   relative digit will change from 0 to 1, and times relative digits. All digits add up to a code to
   send to the receiver.
*/
void translate () {
  if (sprint > 600) { // pressing threshold, can be changed based on user's situation
    Serial.println("sprint");
    sp = 1; //when pressed, change to 1
  } else {
    sp = 0; //not pressed, change to 0
  }

  if (inventory > 600) {
    Serial.println("inventory");
    i = 1;
  } else {
    i = 0;
  }

  if (sneak > 600) {
    Serial.println("sneak");
    sn = 1;
  } else {
    sn = 0;
  }

  if (jump > 30) {
    Serial.println("jump");
    j = 1;
  } else {
    j = 0;
  }

  code = sp * 1000 + i * 100 + sn * 10 + j ; //add up as a 4 digit code
  //Serial.println(code);
}

// Vibration motor 1 vibrates continuously as long as 'jump' is pressed
void vibrate() {
  if (jump > 30) {
    //Serial.println("vibrate");
    digitalWrite(5, HIGH); //vibration motor for squeezing vibrates
  }
  else {
    digitalWrite(5, LOW); //vibration motor for squeezing stops
  }
}

// Vibration motor 2 only vibrates for 80ms once when 'sprint, inventory & sneak' is pressed
void motor() {
  if ((sprint >= 600 or inventory >= 600 or sneak >= 600) && (c == 0) ) {
    digitalWrite(6, HIGH); //vibration motor for buttons vibrates
    //Serial.println("motor");
    delay(80); //vibrates for 80ms
    digitalWrite(6, LOW);
    c = 1; // when one of the controls is pressed, c change from 0 to 1
  }

  if (sprint < 600 && inventory < 600 && sneak < 600 && (c == 1)) {
    digitalWrite(6, LOW); //vibration motor for buttons stops
    c = 0; // if not, c change to 0
  }
}
