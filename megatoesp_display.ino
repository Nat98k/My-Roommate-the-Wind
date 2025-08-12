//final iteration
// mega receives info from esp_display which receives info from esp_sensors
// mega sends manipulated light data to esp_display
#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial mySerial(10, 11);  // RX, TX, on uno its 2, 3
String inString = "";             // string to hold input

float SendDirection = 0;
//buttons
#define button1 22
#define button2 24
#define button3 26
#define button4 28
#define button5 30
float buttons[5];

//servos, up to 12
Servo myservo1;  // create servo object to control a servo
Servo myservo2;
Servo myservo3;
Servo myservo4;
Servo myservo5;
Servo servos[] = { myservo1, myservo2, myservo3, myservo4, myservo5 };

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);      // own
  mySerial.begin(9600);     // with esp
  mySerial.setTimeout(10);  // to make string reading faster
  //servos
  int pins[] = { 8, 7, 6, 5, 3 };
  for (int i = 0; i < 5; i++) {
    servos[i].attach(pins[i]);
  }
  //button initialize
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
}

void loop() {
  //MESSAGES TO ESP
  //MESSAGES FROM ESP, turn into own function later
  if (mySerial.available()) {
    //sensor values from ESP
    inString = mySerial.readStringUntil('\n');

    int SeperatorIndex = inString.indexOf(';');
    String data1 = inString.substring(0, SeperatorIndex);
    String data2 = inString.substring(SeperatorIndex + 1);
    float EspDirection = data1.toFloat();
    float EspSpeed = data2.toFloat();
    //button
    float buttonModifier = readButtons();
    Serial.println("Data received from esp: ");
    Serial.println("Wind direction: " + String(EspDirection) + " ; " + "Wind speed: " + String(EspSpeed));
    //Serial.println(String(buttonModifier));

    //readIncoming();
    // direction stuff
    Serial.println("translates to: ");
    if (EspDirection < 0.33 || EspDirection >= 3.43) {
      float ServoSpeed = calculateSpeed(EspSpeed, false, buttonModifier);
      Serial.println("North; ServoSpeed: " + String(ServoSpeed));
      north(ServoSpeed);
      SendDirection = 1;
    } else if (EspDirection >= 0.33 && EspDirection < 1.36) {
      // Serial.print("Speed: " + String(EspSpeed));
      float ServoSpeed = calculateSpeed(EspSpeed, false, buttonModifier);
      Serial.println("East; ServoSpeed: " + String(ServoSpeed));
      west(ServoSpeed); // west cause home otherwise east
      SendDirection = 2;
    } else if (EspDirection >= 1.36 && EspDirection < 2.4) {
      float ServoSpeed = calculateSpeed(EspSpeed, false, buttonModifier);
      Serial.println("South; ServoSpeed: " + String(ServoSpeed));
      south(ServoSpeed);
      SendDirection = 3;
    } else if (EspDirection >= 2.4 && EspDirection < 3.43) {
      float ServoSpeed = calculateSpeed(EspSpeed, true, buttonModifier);
      Serial.println("West; ServoSpeed: " + String(ServoSpeed));
      east(ServoSpeed); // otherwise west
      SendDirection = 4;
    }
    //Serial.println("ave");
    sendData(EspSpeed, SendDirection, buttonModifier);
  }

  delay(200);  ///2000
}

float readButtons() {

  buttons[0] = digitalRead(button1);
  buttons[1] = digitalRead(button2);
  buttons[2] = digitalRead(button3);
  buttons[3] = digitalRead(button4);
  buttons[4] = digitalRead(button5);

  float buttonMod = 5;
  for (int i = 0; i < 5; i++) {
    buttonMod = buttonMod - buttons[i];
    Serial.println("button: " + String(i) + " = " + String(buttons[i]));
  }
  return (buttonMod / 5);
}

void sendData(float speed, float direction, float button) {
  mySerial.println(String(speed) + ";" + String(direction) + ";" + String(button));
  Serial.println("sending..");
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float calculateSpeed(float insp, bool direction_rightToleft, float buttonValue) {
  // if (insp == 0.00){ // to be able to map ramp up faster
  //   insp = 90;
  // }
  if (direction_rightToleft) {
    insp = mapfloat(insp, 0.0, 3.3, 98, 140);  // doesnt start at 0

  } else {
    insp = mapfloat(insp, 0.0, 3.3, 82, 40);
  }

  return insp;
}

void north(float speed) {
  Serial.println("north is executing ");
  for (int i = 0; i < 5; i++) {
    if (buttons[i] == 0) {
      servos[i].write(speed);
      delay(2000);
      if (i > 0) {
        servos[i - 1].write(90);
      }
    }
    Serial.print(i);
  }
  servos[4].write(90);  // Turn off the last servo
}

void east(float speed) {  // used for it3
  for (int i = 0; i < 5; i++) {

    if (buttons[i] == 0) {
      servos[i].write(speed);
    } else {
      servos[i].write(90);
    }
  }
}

void south(float speed) {
  Servo servos[] = { myservo1, myservo2, myservo3, myservo4, myservo5 };
  for (int i = 5; i >= 0; i--) {
    if (buttons[i] == 0) {
      servos[i].write(speed);
      delay(200);
      if (i < 4) {
        servos[i + 1].write(90);
      }
    }
  }
  servos[0].write(90);  // Turn off the last servo
}

void west(float speed) {
  for (int i = 0; i < 5; i++) {
    if (buttons[i] == 0) {
      servos[i].write(speed);
    } else {
      servos[i].write(90);
    }
  }
}
