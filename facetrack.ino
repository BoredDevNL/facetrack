#include <Servo.h>

// gebruikte pins
const int HOR_SERVO_PIN = 9;
const int VER_SERVO_PIN = 8;
const int SERVO_ON = 3;
const int SERVO_OFF = 4;
const int LED = 6;

unsigned long lastDataTime = 0;
const long TIMEOUT_MS = 500;

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
void clickButtonOn(Servo &servo) {
  servo.write(0);   // druk op knop
  delay(500);
  servo.write(170); // default pos
  delay(500);
}

// Button click
void clickButtonOff(Servo &servo) {
  servo.write(180); // druk op knop
  delay(500);
  servo.write(0);   // default pos
  delay(500);
}

void setup() {
  FaceDetectedPrev = false;
  horServo.attach(HOR_SERVO_PIN);
  verServo.attach(VER_SERVO_PIN);
  PumpOn.attach(SERVO_ON);
  PumpOff.attach(SERVO_OFF);

  Serial.begin(9600);
  Serial.println("Turret Initialized");

  // Initialiseer pan/tilt servos in het midden
  horServo.write(HOR_CENTER_ANGLE);
  verServo.write(VER_CENTER_ANGLE);

  // Initialiseer de knop-servo's
  PumpOn.write(170); // Default pos
  PumpOff.write(0);  // Default pos

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}

void loop() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    serialData += inChar;

    if (inChar == '\n') {
      serialData.trim();
      
      lastDataTime = millis(); 

      int commaIndex = serialData.indexOf(',');

      // check voor gezicht in frame
      if (commaIndex > 0) {
        FaceDetectedNow = true;

        int faceX = serialData.substring(0, commaIndex).toInt();
        int faceY = serialData.substring(commaIndex + 1).toInt();

        // Stuur horizontale servo
        targetHor = map(faceX, 0, CAMERA_WIDTH, 180, 0);
        targetHor = constrain(targetHor, 0, 180);
        horServo.write(targetHor);

        // Stuur verticale servo
        targetVer = map(faceY, 0, CAMERA_HEIGHT, VER_MIN_ANGLE, VER_MAX_ANGLE);
        targetVer = constrain(targetVer, VER_MIN_ANGLE, VER_MAX_ANGLE);
        verServo.write(targetVer);

        digitalWrite(LED, HIGH);

      } else {
        FaceDetectedNow = false; // geen gezicht == motor uit
      }

      if (FaceDetectedNow == true && FaceDetectedPrev == false) {
        clickButtonOn(PumpOn);
        Serial.println("button aan"); 
      }

      if (FaceDetectedNow == false && FaceDetectedPrev == true) {
        clickButtonOff(PumpOff);
        digitalWrite(LED, LOW);
        Serial.println("button uit"); 
      }

      FaceDetectedPrev = FaceDetectedNow;
      serialData = "";
    }
  }
  
  if (FaceDetectedPrev == true && (millis() - lastDataTime > TIMEOUT_MS)) {
    clickButtonOff(PumpOff);
    digitalWrite(LED, LOW);  
    FaceDetectedPrev = false;  
    Serial.println("button uit (TIMEOUT)");
  }
}