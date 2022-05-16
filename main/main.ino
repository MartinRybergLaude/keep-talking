#include <Servo.h>
#include <StringSplitter.h>

enum GameState { INIT, PLAY_1, PLAY_2, PLAY_3, END, WAITING };

enum Colors { RED, GREEN, BLUE };

GameState GAME_STATE = INIT;
Servo servo;
const int LIGHT_PIN = A1;
const int BUZZER_PIN = A5;
const int MOTOR_PIN = 9;
const int BTNR_PIN = 2;
const int BTNG_PIN = 3;
const int BTNB_PIN = 4;

const int LED_R = 7;
const int LED_G = 6;
const int LED_B = 5;

int currentData = 0;

void setup() {
  Serial.begin(9600);

  pinMode(LIGHT_PIN, INPUT);
  pinMode(BTNR_PIN, INPUT_PULLUP);
  pinMode(BTNG_PIN, INPUT_PULLUP);
  pinMode(BTNB_PIN, INPUT_PULLUP);
  pinMode(MOTOR_PIN, OUTPUT);

  servo.attach(9);
}

String readSerial() { return Serial.available() ? Serial.readString() : ""; }

void loop() {
  String read = readSerial();
  switch (GAME_STATE) {
    case INIT:
      currentData = 0;
      servo.write(0);
      playButtons();
      currentData = 1;
      GAME_STATE = WAITING;
      break;
    case PLAY_1:
      servo.write(45);
      break;
    case PLAY_2:
      servo.write(90);
      break;
    case PLAY_3:
      servo.write(135);
      break;
    case END:
      servo.write(180);
      delay(5000);
      GAME_STATE = INIT;
      break;
    case WAITING:
      // Send currentData to processing
      RGBColor(255, 255, 255);
      Serial.println(currentData);
      delay(100);
      break;
  }
}

bool buttonClicked(int state, int prevState = 1) {
   if (prevState == 0 && state == 1) {
    return true;
   }
   return false;
}

void playButtons() {
  int colors[5] = {RED, GREEN, BLUE, RED, GREEN};
  int buttonPins[3] = {BTNR_PIN, BTNG_PIN, BTNB_PIN};

  for (int i = 0; i < 5; i++) {
    lightLamp(colors[i]);
    delay(200);
    RGBColor(0, 0, 0);
    delay(200);
  }
  
  int i = 0;
  while(i<5){
    int btnStates[] = {1, 1, 1};
    bool hasPressed = false;
    bool correctGuess = false;
    bool clicked = false;
    int state = 1;
    while (!hasPressed) {
      for (int j = 0; j < 3; j++) {
        state = digitalRead(buttonPins[j]);
        clicked = buttonClicked(state, btnStates[j]);
        delay(20);
        btnStates[j] = state;
        if (clicked && (int)colors[i] == j) {
           correctGuess = true;
           hasPressed = true;
        } else if (clicked) {
          tone(BUZZER_PIN, 500, 500);
          hasPressed = true;
          i = 0;
        }
      }
    }
    if (correctGuess) {
      i++;
    }
  }
}

void RGBColor(int led_r_val, int led_g_val, int led_b_val) {
  analogWrite(LED_R, led_r_val);
  analogWrite(LED_G, led_g_val);
  analogWrite(LED_B, led_b_val);
}

void lightLamp(int color) {
  if ((Colors)color == RED) {
    RGBColor(255, 0, 0);
  }
  if ((Colors)color == GREEN) {
    RGBColor(0, 255, 0);
  }
  if ((Colors)color == BLUE) {
    RGBColor(0, 0, 255);
  }
}
