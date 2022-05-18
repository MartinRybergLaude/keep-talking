#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

enum GameState { INIT, PLAY_1, PLAY_2, PLAY_3, PLAY_4, END, WAITING };

enum Colors { RED, GREEN, BLUE, CLEAR, WHITE };

const int notes[21][2] = {{NOTE_G4, 300},{NOTE_C4, 300}, {NOTE_C5, 900}, {NOTE_AS4, 300}, {NOTE_G4, 1800}, 
  {NOTE_G4, 300}, {NOTE_AS3, 300}, {NOTE_C4, 1200},
  {NOTE_G4, 300}, {NOTE_CS4, 300}, {NOTE_DS4, 600}, {NOTE_F4, 600},
  {NOTE_G4, 300}, {NOTE_C4, 300}, {NOTE_DS5, 900}, {NOTE_D5, 300}, {NOTE_G4, 1800},
  {NOTE_G4, 900}, {NOTE_AS4, 300}, {NOTE_A4, 600}, {NOTE_G4, 1800}};

const int winSound[5][2] = {{NOTE_C4, 300},{NOTE_E4, 300}, {NOTE_G4, 900}, {NOTE_C5, 300}, {NOTE_E5, 1800}};

GameState PREV_STATE = INIT;
GameState GAME_STATE = INIT;
Servo servo;
const int POTENTIO_PIN = A2;
const int BUZZER_PIN = A1;
const int MOTOR_PIN = 9;
const int BTNR_PIN = 2;
const int BTNG_PIN = 3;
const int BTNB_PIN = 4;

const int LED_R = 13;
const int LED_G = 12;
const int LED_B = 8;

int currentData = 0;

// For Running Average filter:
// Number of readings for the running average filter
const int numReadings = 10;
 // An array for the readings from the potentiometer
int readings[numReadings]; 
int readIndex = 0;
float runAvgTotal = 0;

float acc[] = {0,0,0};
float l_way = 3; // Leeway, how far from the correct position you are but still counts as "right"
int r;
float pos[11][3] = {  {5.0, 0.0, 10.0},
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
  
  accel.begin();
  pinMode(POTENTIO_PIN, INPUT);
  pinMode(BTNR_PIN, INPUT_PULLUP);
  pinMode(BTNG_PIN, INPUT_PULLUP);
  pinMode(BTNB_PIN, INPUT_PULLUP);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  playHappySound();

  randomSeed(analogRead(0));
  r = random(0, 12);

  establishContact();
  
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
      lightLamp(GREEN);
      currentData = 0;
      playStart();
      GAME_STATE = WAITING;
      break;
    case PLAY_1:
      servo.write(45);
      currentData = 0;
      readSerial();
      playButtons();
      GAME_STATE = WAITING;
      break;
    case PLAY_2:
      servo.write(90);
      currentData = 0;
      lightLamp(CLEAR);
      playAccel();
      break;
    case PLAY_3:
      servo.write(135);
      lightLamp(CLEAR);
      currentData = getPotentio();
      break;
    case PLAY_4:
      servo.write(180);
      lightLamp(CLEAR);
      currentData = 0;
      readSerial();
      playMusic();
      GAME_STATE = END;
      break;
    case END:
      delay(5000);
      playWinSound();
      GAME_STATE = INIT;
      break;
    case WAITING:
      // Await state change
      currentData = 1;
      lightLamp(WHITE);
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
      sound(1000, 200);
      return true;
   }
   return false;
}

void playMusic() {
  bool success = false;
  int currNote = 0;
  bool clicked = true;
  int prevState = 1;
  int state = 1;
  
  randomSeed(analogRead(0));
  int bpm = random(150, 450);
  
  while (!success) {
    if (clicked) { pace(bpm); }
    if (currNote >= 21) {currNote = 0; }

    // Get btn click
    state = digitalRead(BTNR_PIN);
    clicked = buttonClicked(state, prevState);
    prevState = state;
    
    float reading = (float)analogRead(POTENTIO_PIN);
    float scaled = (reading / 1023.00) * (1.5-0.5) + 0.5;
    
    float note = notes[currNote][0];
    float noteDelay = notes[currNote][1] * scaled;
    
    if (noteDelay > bpm-20 && noteDelay < bpm+20) {success = true;}
    
    sound(note, noteDelay); 
  
    if (success) {
      delay(500);
      playHappySound();
      GAME_STATE = WAITING;
    }
     currNote++;
  }
}

void pace(int pace) {
  for (int i = 0; i < 6; i++) {
    sound(NOTE_G4, pace);
    delay(pace);
  }
  delay(1000);     
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
    playHappySound();
    GAME_STATE = WAITING;
    randomSeed(analogRead(0));
    r = random(0, 12);
  } else {
    // Play tone depending on how close the rotation is (faster tone = closer)
    sound(1000, 50);
    delay(dist*10);
  }
}

int getPotentio() {
  runAvgTotal = runAvgTotal - readings[readIndex]; // Subtracts from last reading
  readings[readIndex] = analogRead(POTENTIO_PIN); // Reads from the potentiometer
  runAvgTotal = runAvgTotal + readings[readIndex]; // Adds reading to total
  readIndex++;

  // Loops back to the start of the array if we reach the end
  if(readIndex >= numReadings) {
     readIndex = 0;
  }
  // Calculates average and stores it as the current value
  float avgValue = runAvgTotal / numReadings;
  int normalised = 24 * (avgValue / 1023);
  return normalised;
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
  currentData = 0;
  readSerial();
  int roundNum = 3;
  while(roundNum > 0) {
    for (int i = 0; i < 5; i++) {
      readSerial();
      delay(400);
      lightLamp(colors_array[3-roundNum][i]);
      delay(400);
      RGBColor(0, 0, 0);
    }
    
    int i = 0;
    while(i<5){
      readSerial();
      int btnStates[] = {1, 1, 1};
      bool hasPressed = false;
      bool correctGuess = false;
      bool clicked = false;
      int state = 1;
      while (!hasPressed) {
        readSerial();
        for (int j = 0; j < 3; j++) {
          readSerial();
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

void playWinSound() {
   for (int i = 0; i < 5; i++) {
    sound(winSound[i][0], winSound[i][1]);
   }
}

float lowPassFilter(int num, float value, float change){
  total = (1-change)*past_total[num] + change*value;
  past_total[num] = total;
  return total;
}

void RGBColor(int led_r_val, int led_g_val, int led_b_val) {
  digitalWrite(LED_R, led_r_val);
  digitalWrite(LED_G, led_g_val);
  digitalWrite(LED_B, led_b_val);
}

void playHappySound() {
   tone(BUZZER_PIN, 349.228, 200);
   delay(200);
   tone(BUZZER_PIN, 440, 200);
   delay(200);
   tone(BUZZER_PIN, 523.25, 200);
   delay(200);
}

void sound(float hz, int delayTime) {
  tone(BUZZER_PIN, hz, delayTime);
  delay(delayTime);
}

void lightLamp(int color) {
  if ((Colors)color == WHITE) {
    RGBColor(HIGH, HIGH, HIGH);
  }
  if ((Colors)color == CLEAR) {
    RGBColor(LOW, LOW, LOW);
  }
  if ((Colors)color == RED) {
    RGBColor(HIGH, LOW, LOW);
  }
  if ((Colors)color == GREEN) {
    RGBColor(LOW, HIGH, LOW);
  }
  if ((Colors)color == BLUE) {
    RGBColor(LOW, LOW, HIGH);
  }
}
