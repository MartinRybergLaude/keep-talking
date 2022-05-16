#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

enum GameState { INIT, PLAY_1, PLAY_2, PLAY_3, END, WAITING };

enum Colors { RED, GREEN, BLUE };

GameState PREV_STATE = INIT;
GameState GAME_STATE = INIT;
Servo servo;
const int POTENTIO_PIN = A4;
const int BUZZER_PIN = A5;
const int MOTOR_PIN = 9;
const int BTNR_PIN = 2;
const int BTNG_PIN = 3;
const int BTNB_PIN = 4;

const int LED_R = 7;
const int LED_G = 6;
const int LED_B = 5;

int currentData = 0;

// For Running Average filter:
// Number of readings for the running average filter
const int numReadings = 10;
 // An array for the readings from the potentiometer
int readings[numReadings]; 
int readIndex = 0;
int runAvgTotal = 0;

float acc[] = {0,0,0};
float l_way = 3; // Leeway, how far from the correct position you are but still counts as "right"
int r;
float pos[][3] = {  {5.0, 0.0, 10.0},
                    {0.0, -1.0, 10.0}, 
                    {0.0, 10, 0},
                    {0.0, -10.0, 0.0},
                    {10.0, 0.0, 0.0},
                    {-10.0, 0.0, 0.0},
                    {0.0, 1.0, -10.0},
                    {-7.0, 1.3, -6.7},
                    {9.0, 0.0, 4.5},
                    {7.5, 6.0, 4.0},
                    {-6.0, -7.0, 3.0}};
float dist;
// For low pass filter
float total;
float past_total[] = {0,0,0}; // Past value for x, y, and z


void setup() {
  Serial.begin(9600);

  pinMode(POTENTIO_PIN, INPUT);
  pinMode(BTNR_PIN, INPUT_PULLUP);
  pinMode(BTNG_PIN, INPUT_PULLUP);
  pinMode(BTNB_PIN, INPUT_PULLUP);
  pinMode(MOTOR_PIN, OUTPUT);

  establishContact();
  
  //accel.begin();
  servo.attach(9);
}

void readSerial() { 
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    
    GameState state = (GameState)inByte;
    if (state != PREV_STATE) {
      GAME_STATE = state;
      PREV_STATE = state;
    }
    Serial.println(currentData);
  }
}
void loop() {
  readSerial();
  
  switch (GAME_STATE) {
    case INIT:
      servo.write(0);
      
      RGBColor(0, 255, 0);
      currentData = 0;
      playStart();
      currentData = 1;
      GAME_STATE = WAITING;
      break;
    case PLAY_1:
      servo.write(45);
      
      currentData = 0;
      playButtons();
      currentData = 1;
      GAME_STATE = WAITING;
      break;
    case PLAY_2:
      servo.write(90);
      playPotentio();
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
      // Await state change
      RGBColor(255, 255, 255);
      delay(100);
      break;
  }
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("hello");   // send a starting message
    delay(300);
  }
}

bool buttonClicked(int state, int prevState = 1) {
   if (prevState == 0 && state == 1) {
      return true;
   }
   return false;
}

void playAccel() {
  // Accelerometer
  sensors_event_t event; 
  accel.getEvent(&event);
  
  // Saves the accelerometers current values in temporary variables
  acc[0] = event.acceleration.x;
  acc[1] = event.acceleration.y;
  acc[2] = event.acceleration.z;

  // Low pass filter
  for (int i = 0; i < 3; i++) {
    acc[i] = lowPassFilter(i, acc[i], 0.5);
  }
  
  // Calculate distance to correct rotation position
  dist = sqrt((pow(pos[r][0] - acc[0], 2) + pow(pos[r][1] - acc[1], 2) + pow(pos[r][2] - acc[2], 2)));
      
  // Check if the rotation is close enough
  if(dist < l_way){
    GAME_STATE = WAITING;
  }
  else {
    // Play tone depending on how close the rotation is (higher tone = closer)
    tone(BUZZER_PIN, 1000-dist*50, 50);
    delay(250);
  }
}

void playPotentio() {
  runAvgTotal = runAvgTotal - readings[readIndex]; // Subtracts from last reading
  readings[readIndex] = analogRead(POTENTIO_PIN); // Reads from the potentiometer
  runAvgTotal = runAvgTotal + readings[readIndex]; // Adds reading to total
  readIndex++;

  // Loops back to the start of the array if we reach the end
  if(readIndex >= numReadings) {
     readIndex = 0;
  }
  // Calculates average and stores it as the current value
  currentData = runAvgTotal / numReadings;
}

void playStart() {
  bool clicked = false;
  int prevState = 1;
  int state = 1;
  
  while(!clicked) {
    state = digitalRead(BTNR_PIN);
    clicked = buttonClicked(state, prevState);
    prevState = state;
  }
}

void playButtons() {
  int colors_array[3][5] = { { RED, GREEN, BLUE, RED, GREEN }, { RED, GREEN, GREEN, GREEN, RED }, { BLUE, BLUE, BLUE, BLUE, GREEN } };
  int buttonPins[3] = {BTNR_PIN, BTNG_PIN, BTNB_PIN};

  int roundNum = 3;
  while(roundNum > 0) {
    for (int i = 0; i < 5; i++) {
      lightLamp(colors_array[3-roundNum][i]);
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
          if (clicked && (int)colors_array[3-roundNum][i] == j) {
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
  playHappySound();
  roundNum--;
  }
}

float lowPassFilter(int num, float value, float change){
  total = (1-change)*past_total[num] + change*value;
  past_total[num] = total;
  return total;
}

void RGBColor(int led_r_val, int led_g_val, int led_b_val) {
  analogWrite(LED_R, led_r_val);
  analogWrite(LED_G, led_g_val);
  analogWrite(LED_B, led_b_val);
}

void playHappySound() {
   tone(BUZZER_PIN, 349.228, 200);
   delay(200);
   tone(BUZZER_PIN, 440, 200);
   delay(200);
   tone(BUZZER_PIN, 523.25, 200);
   delay(200);
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
