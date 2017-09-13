#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>

char *ssid = "<REPLACE WITH YOUR SSID>";
char *password = "<REPLACE WITH YOUR PASSWORD>";

#define SERVO_BASE D6
#define SERVO_TOP D8
#define YELLOW_LED D4
#define MAGNET D2

onst char* thingerUser = "<REPLACE WITH THINGER USER>";
const char* deviceCode = "<REPLACE WITH THINGER SECRET CODE>";
const char* deviceName = "<REPLACE WITH THINGER DEVICE NAME>";


Servo servo_base;
Servo servo_top;

int RESET_ANGLE = 0;
int DEFAULT_BASE_MOVEMENT = 5;

int BASE_LEFT_MAX_ANGLE = 110;
int BASE_RIGHT_MAX_ANGLE = RESET_ANGLE;

int HAND_UP_MAX_ANGLE = RESET_ANGLE;
int HAND_DOWN_MAX_ANGLE = 80;

ThingerESP8266 *thing;

unsigned long timer;

void setup() {

  timer = millis();

  pinMode(YELLOW_LED, OUTPUT);
  pinMode(MAGNET, OUTPUT);

  Serial.begin(115200);
  servo_base.attach(SERVO_BASE);
  servo_base.write(RESET_ANGLE);

  servo_top.attach(SERVO_TOP);
  servo_top.write(RESET_ANGLE);

  connectToWiFi();

  thing =  new ThingerESP8266(thingerUser, deviceName, deviceCode);

  (*thing)["crane_base_left"] << [](pson & in) {
    Serial.println("crane_base_left");
    moveBaseServo(DEFAULT_BASE_MOVEMENT);
  };

  (*thing)["crane_base_right"] << [](pson & in) {
    Serial.println("crane_base_right");
    moveBaseServo(DEFAULT_BASE_MOVEMENT * -1);
  };

  (*thing)["crane_hand_left"] << [](pson & in) {
    Serial.println("crane_hand_left");
    moveHandServo(DEFAULT_BASE_MOVEMENT);
  };

  (*thing)["crane_hand_right"] << [](pson & in) {
    Serial.println("crane_hand_right");
    moveHandServo(DEFAULT_BASE_MOVEMENT * -1);
  };

  (*thing)["reset_crane"] << [](pson & in) {
    Serial.println("reset_crane");
    resetCrane();
  };

  // TODO: fix zero case
  (*thing)["crane_base_left_express"] << [](pson & in) {
    Serial.println("crane_base_left_express");
    moveBaseServoAbsolute(BASE_LEFT_MAX_ANGLE);
  };

  (*thing)["crane_base_right_express"] << [](pson & in) {
    Serial.println("crane_base_right_express");
    servo_base.write(RESET_ANGLE);
  };

  // TODO: fix zero case
  (*thing)["crane_hand_up_express"] << [](pson & in) {
    Serial.println("crane_hand_up_express");
    servo_top.write(RESET_ANGLE);
  };

  (*thing)["crane_hand_down_express"] << [](pson & in) {
    Serial.println("crane_hand_down_express");
    moveHandServoAbsolute(HAND_DOWN_MAX_ANGLE);
  };

  (*thing)["magnet_toggle"] << [](pson & in) {
    Serial.println("magnet_toggle");
    digitalWrite(MAGNET, !digitalRead(MAGNET));
  };

  (*thing)["magnet_on"] << [](pson & in) {
    Serial.println("magnet_on");
    digitalWrite(MAGNET, 0);
  };

  (*thing)["magnet_off"] << [](pson & in) {
    Serial.println("magnet_off");
    digitalWrite(MAGNET, 1);
  };
}

void resetCrane() {
  servo_top.write(RESET_ANGLE);
  servo_base.write(RESET_ANGLE);

  digitalWrite(MAGNET, 0);
}

void moveHandServoAbsolute(int absoluteAngle) {
  // Do it slowly or the arm will de-stabilize
  Serial.println("moveHandServoAbsolute start ");
  int currentAngle = servo_top.read();
  for (int i = currentAngle; i <= absoluteAngle; i += absoluteAngle / DEFAULT_BASE_MOVEMENT) {
    Serial.print(i);
    Serial.print(" ");
    servo_top.write(i);
    delay(50);
  }
  Serial.println("moveHandServoAbsolute end ");

}

void moveBaseServoAbsolute(int absoluteAngle) {
  // Do it slowly or the arm will de-stabilize
  int currentAngle = servo_base.read();

  Serial.println("moveBaseServoAbsolute start ");
  for (int i = currentAngle; i <= absoluteAngle; i += absoluteAngle / DEFAULT_BASE_MOVEMENT) {
    Serial.print(i);
    Serial.print(" ");
    servo_base.write(i);
    delay(50);
  }
  Serial.println("moveBaseServoAbsolute end ");
}

void moveHandServo(int angle) {
  angle = servo_top.read() + angle;
  if (angle > HAND_DOWN_MAX_ANGLE) {
    Serial.print("Hand down max angle reached. Ignoring further downs");
    return;
  }
  servo_top.write(angle);
  Serial.print("Hand current angle");
  Serial.println(servo_top.read());
}

void moveBaseServo(int angle) {
  angle = servo_base.read() + angle;
  if (angle > BASE_LEFT_MAX_ANGLE) {
    Serial.print("Base left max angle reached. Ignoring further lefts");
    return;
  }
  servo_base.write(angle);
  Serial.print("Base current angle");
  Serial.println(servo_base.read());
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wifi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    blink(YELLOW_LED, 500, 500);
    delay(500);
  }
  Serial.println("");
  Serial.println("IP Address : ");
  Serial.println(WiFi.localIP());

  resetCrane();
  Serial.println("Moved the servo to 0");

}

void wait(int waitTime) {
  if (millis() - timer > waitTime) {
    Serial.print("Timing : ");
    Serial.println(timer);
    timer = millis();
  }
}

void loop() {
  (*thing).handle();
}

void blink(int pin_number, int on, int off)
{
  digitalWrite(pin_number, HIGH);
  delay(on);
  digitalWrite(pin_number, LOW);
  delay(off);
}
