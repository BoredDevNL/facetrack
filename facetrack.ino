#include <Servo.h>

// gebruikte pins
const int HOR_SERVO_PIN = 9;
const int VER_SERVO_PIN = 8;
const int SERVO_ON = 5;
const int SERVO_OFF = 4;
const int LED = 6;

Servo horServo;
Servo verServo;
Servo PumpOn;
Servo PumpOff;
// Midden van servo
const int HOR_CENTER_ANGLE = 90;
const int VER_CENTER_ANGLE = 90;
// Camera RES
const int CAMERA_WIDTH = 640;
const int CAMERA_HEIGHT = 480;
// Minimum en Maximum angles van servo
const int VER_MIN_ANGLE = 20;
const int VER_MAX_ANGLE = 160;

int targetHor = HOR_CENTER_ANGLE;
int targetVer = VER_CENTER_ANGLE;

// Variabelen om te zien of een gezicht nu zichtbaar is
// maar niet in een eerdere frame.
bool FaceDetectedNow = false;
bool FaceDetectedPrev = false;

String serialData = "";
// Button click
void clickButton(Servo &servo) {
  servo.write(180); // druk op knop
  delay(50);
  servo.write(0); // default pos
  delay(50);
}

void setup() {
  horServo.attach(HOR_SERVO_PIN);
  verServo.attach(VER_SERVO_PIN);
  PumpOn.attach(SERVO_ON);
  PumpOff.attach(SERVO_OFF);

  Serial.begin(9600);
  Serial.println("Turret Initialized");

  horServo.write(HOR_CENTER_ANGLE);
  verServo.write(VER_CENTER_ANGLE);

  PumpOn.write(0);
  PumpOff.write(0);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}

void loop() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    serialData += inChar;

    if (inChar == '\n') {
      serialData.trim();

      int commaIndex = serialData.indexOf(',');

      // check voor gezicht in frame
      if (commaIndex > 0) {
        FaceDetectedNow = true;

        int faceX = serialData.substring(0, commaIndex).toInt();
        int faceY = serialData.substring(commaIndex + 1).toInt();

        targetHor = map(faceX, 0, CAMERA_WIDTH, 180, 0);
        targetHor = constrain(targetHor, 0, 180);
        horServo.write(targetHor);

        targetVer = map(faceY, 0, CAMERA_HEIGHT, VER_MIN_ANGLE, VER_MAX_ANGLE);
        targetVer = constrain(targetVer, VER_MIN_ANGLE, VER_MAX_ANGLE);
        verServo.write(targetVer);

        digitalWrite(LED, HIGH);

      } else {
        FaceDetectedNow = false; // geen gezicht == motor uit
        digitalWrite(LED, LOW); // geen gezicht == lamp uit
      }

      if (FaceDetectedNow == true && FaceDetectedPrev == false) {
        clickButton(PumpOn);   // Gezicht gevonden en niet in een eerdere frame gezien
      }

      if (FaceDetectedNow == false && FaceDetectedPrev == true) {
        clickButton(PumpOff);  // Geen gezicht gevonden maar eerder wel
      }

      FaceDetectedPrev = FaceDetectedNow;
      serialData = "";
    }
  }
}
