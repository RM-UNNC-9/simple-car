#include <Servo.h>
#include <arduino-timer.h>

const uint8_t track_port1 = A5;
const uint8_t track_port2 = A0;
const uint8_t servo_port1 = 10;
const uint8_t servo_port2 = 5;
const uint8_t echo_port = A2;
const uint8_t trig_port = 7;

auto timer = timer_create_default();  // set up the timer

volatile int track[2];  // 0 is the right one, 1 is the left one

volatile Servo servo1, servo2;  // 1 is the right one, 2 is the left one

void pin_init() {
  pinMode(track_port1, INPUT);  // set the port of greyscale sensor 1 (right)
  pinMode(track_port2, INPUT);  // set the port of greyscale sensor 2 (left)

  pinMode(trig_port, OUTPUT);  // set the triger port of HC_SR04
}

void servo_init() {
  servo1.attach(servo_port1);  // set the servo 1 (right)
  servo2.attach(servo_port2);  // set the servo 2 (left)
}

void track_read() {
  track[0] = digitalRead(
      track_port1);  // read the digital signal of greyscale sensor 1 (right)
  track[1] = digitalRead(
      track_port2);  // read the digital signal of greyscale sensor 2 (left)
}

void track_track() {  // adjust the direction
  track_read();

  if (track[0] == 0 && track[1] == 1) {  // 0-1 turn left
    servo1.writeMicroseconds(1480);
    servo2.writeMicroseconds(1140);
  } else if (track[0] == 1 && track[1] == 0) {  // 1-0 turn right
    servo1.writeMicroseconds(1975);
    servo2.writeMicroseconds(1515);
  } else {
    go_forward();
  }
}

float distance_test() {
  static const int SPEED_SOUND_20 = 343;  // the speed of sound in 20 dgree

  // emit ultrasound
  digitalWrite(trig_port, LOW);  // give trigger port a 2μs low level
  delayMicroseconds(2);
  digitalWrite(trig_port, HIGH);  // give trigger port a high level, at least 10μs
  delayMicroseconds(10);
  digitalWrite(trig_port, LOW);  // keep giving trigger port a low level

  float time =
      pulseIn(echo_port, HIGH);  // get the time of the high level on echo port,
                                 // and also the time bewteen emit and receive
  return time * SPEED_SOUND_20 / 20000;  // time*speeed/2, microseconds to cm
}

void spin_left() {
  servo1.writeMicroseconds(1215);
  servo2.writeMicroseconds(1270);
}

void spin_right() {
  servo1.writeMicroseconds(1815);
  servo2.writeMicroseconds(1860);
}

void go_forward() {
  servo1.writeMicroseconds(1565);
  servo2.writeMicroseconds(1465);
}

void go_back() {
  servo1.writeMicroseconds(1465);
  servo2.writeMicroseconds(1565);
}

void brake() {
  servo1.writeMicroseconds(1515);
  servo2.writeMicroseconds(1515);
}

void obstacle_avoid() {
  float distance = distance_test();  // meausure the distance ahead
  if (distance <
      20)  // if the distance is smaller than 20cm, start the avoiding procedure
  {
    if (distance < 6) {  // if the distance is too short, go back
      go_back();
      delay(300);
      brake();
    }

    while (distance < 30)  // judge again whether there is an obstacle, if so,
                           // turn the direction and continue to judge
    {
      spin_right();  // try right firstly
      delay(700);
      brake();

      distance = distance_test();  // meausure the distance ahead

      if (distance < 30) {  // if there is still an obstacle, try left
        spin_left();
        delay(1400);
        brake();

        distance = distance_test();  // meausure the distance ahead
      }
    }

    // bypass the obstacle
    go_forward();
    delay(3000);
    brake();

    spin_left();
    delay(600);
    brake();

    go_forward();
    delay(6000);
    brake();

    spin_left();
    delay(450);
    brake();

    go_forward();
    track_read();
    while (!(track[0] || track[1]))
      track_read();  // go forward until meeting the track
    delay(300);
    brake();

    spin_right();
    delay(450);
    brake();
  }
}

void setup() {
  Serial.begin(9600);  // initalize the serial port with baud rate 9600

  pin_init();
  servo_init();

  timer.every(10, track_track);     // call track_track every 10ms
  timer.every(25, obstacle_avoid);  // call obstacle_avoid every 25ms
}

void loop() {
  // put your main code here, to run repeatedly:
  timer.tick<void>();  // tick the timer
}
