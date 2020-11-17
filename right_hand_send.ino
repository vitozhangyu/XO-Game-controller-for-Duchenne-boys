//
// Right hand sender inside the right controller
// This sender is used to send the raw data of the controller to the right hand receiver via nRF024L01+.
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
int vx, vy;
int i, s, d, a;
int code = 0;
int interact, switchWeapon, drop, attack;
int b = 0;

RF24 rf24(8, 10); // CE, CSN
const byte addr[] = "1Node"; // adress name in String


void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    while (1);
  }

  rf24.begin();
  rf24.setChannel(83);       // set channel
  rf24.openWritingPipe(addr); // set pipe
  rf24.setPALevel(RF24_PA_MIN);   // set power amplifier level to low to save energy, since required working distance is short
  rf24.setDataRate(RF24_250KBPS); // set KBPs
  rf24.stopListening();       // stop listening to send data

  pinMode(A0, INPUT); //interact
  pinMode(A1, INPUT); //switch weapon
  pinMode(A2, INPUT); //drop
  pinMode(A3, INPUT); //attack
  pinMode(5, OUTPUT); //vibrate - vibration motor 1
  pinMode(6, OUTPUT); //motor - vibration motor 2
}

void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  vx = (gx + 300) / 200 - 1;
  vy = -(gz - 100) / 200 ; // modify raw gx and gz (gyro data in X and Z axis).

  interact = analogRead(A2); // button for interact, read from pressure sensor 0
  switchWeapon = analogRead(A0); // button for switch weapon, read from pressure sensor 1
  drop = analogRead(A1); // button for drop, read from pressure sensor 2
  attack = analogRead(A3); // squeeze button for attack, read from pressure sensor 3

  translate(); //translate clicked pressure sensors into a 4 digits integer code
  Serial.println(drop);
  rf24.write(&vx, sizeof(vx));  // send vx to right hand receiver
  rf24.write(&vy, sizeof(vy));  // send vy to right hand receiver
  rf24.write(&code, sizeof(code));  // send code to right hand receiver
  vibrate(); // vibration motor 1 vibrates when 'attack' is pressed
  motor(); // vibration motor 2 vibrates once when 'interact, switch weapon & drop' is pressed
  delay(10);
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
  if (interact > 550) { // pressing threshold, can be changed based on user's situation
    Serial.println("interact");
    i = 1; //when pressed, change to 1
  } else {
    i = 0; //not pressed, change to 0
  }

  if (switchWeapon > 550) {
    Serial.println("switch");
    s = 1;
  } else {
    s = 0;
  }

  if (drop > 50) {
    Serial.println("drop");
    d = 1;
  } else {
    d = 0;
  }

  if (attack > 400) {
    Serial.println("attack");
    a = 1;
  } else {
    a = 0;
  }

  code = i * 1000 + s * 100 + d * 10 + a; //add up as a 4 digit code
  //Serial.println(code);
}

// Vibration motor 1 vibrates continuously as long as 'attack' is pressed
void vibrate() {
  if (attack > 50) {
    //Serial.println("viberate");
    digitalWrite(6, HIGH); //vibration motor for squeezing vibrates
  }
  else {
    digitalWrite(6, LOW); //vibration motor for squeezing stops
  }
}

// Vibration motor 2 only vibrates for 80ms once when 'interact, switchWeapon & drop' is pressed
void motor() {
  if ((interact >= 550 or switchWeapon >= 550 or drop >= 50) && (b == 0) ) {
    digitalWrite(6, HIGH); //vibration motor for buttons vibrates
    //Serial.println("motor");
    delay(80); //vibrates for 80ms
    digitalWrite(6, LOW);
    b = 1; // when one of the controls is pressed, b change from 0 to 1
  }

  if (interact < 550 && switchWeapon < 550 && drop < 50 && (b == 1)) {
    digitalWrite(6, LOW); //vibration motor for buttons stops
    b = 0; //if not, b changes to 0
  }
}
