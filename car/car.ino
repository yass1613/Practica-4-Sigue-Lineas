#include "FastLED.h"
#include <Arduino_FreeRTOS.h>

#define TRIG_PIN 13  
#define ECHO_PIN 12  
#define VEL_SOUND 0.0343
#define PIN_ITR20001_LEFT   A2
#define PIN_ITR20001_MIDDLE A1
#define PIN_ITR20001_RIGHT  A0

#define LINE_TRESHOLD 900
// Enable/Disable motor control.
//  HIGH: motor control enabled
//  LOW: motor control disabled
#define PIN_Motor_STBY 3

// Group A Motors (Right Side)
// PIN_Motor_AIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_AIN_1 7
// PIN_Motor_PWMA: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMA 5

// Group B Motors (Left Side)
// PIN_Motor_BIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_BIN_1 8
// PIN_Motor_PWMB: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMB 6
#define PIN_RBGLED 4
#define NUM_LEDS 1

#define PERIODIC_T1 100
#define PERIODIC_T2 200
#define PERIODIC_T3 4000
#define PERIODIC_T4 150
CRGB leds[NUM_LEDS];

volatile long duration;
volatile float distance;
volatile int left_IR, right_IR, mid_IR;
volatile int endLap = 0;
bool lineLost = false;
volatile int totalCounter = 0, lineCounter = 0;
unsigned long startTime;

void sendMsg(int action, unsigned long arg) {
  Serial.print("{");
  Serial.print(action);

  if (arg) {
    Serial.print(":");
    Serial.print(arg);
  }

  Serial.println("}");
}

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}


static void Task1(void *parameters) {
  TickType_t xLastWakeTime;

  while(!endLap) {
    xLastWakeTime = xTaskGetTickCount();

    left_IR = analogRead(PIN_ITR20001_LEFT);
    mid_IR  = analogRead(PIN_ITR20001_MIDDLE);
    right_IR = analogRead(PIN_ITR20001_RIGHT);
    /*    
    Serial.print("IR L:");
    Serial.print(left_IR);
    Serial.print(" M:");
    Serial.print(mid_IR);
    Serial.print(" R:");
    Serial.println(right_IR);
    */
    if (left_IR < LINE_TRESHOLD && mid_IR < LINE_TRESHOLD && right_IR < LINE_TRESHOLD) {
      if (!lineLost) {
        lineLost = true;
        sendMsg(3, 0);
        sendMsg(5, 0);
      }
    } else {
      lineCounter++;
      if (lineLost){
        lineLost = false;
        sendMsg(7, 0);
        sendMsg(6, 0);
      }
    }
    totalCounter++;
    xTaskDelayUntil(&xLastWakeTime, (PERIODIC_T1 / portTICK_PERIOD_MS));
  }
}

static void Task2(void *parameters) {
  TickType_t xLastWakeTime;

  while(!endLap) {
    xLastWakeTime = xTaskGetTickCount();

    if (left_IR > LINE_TRESHOLD || mid_IR > LINE_TRESHOLD || right_IR > LINE_TRESHOLD)
      FastLED.showColor(Color(0, 255, 0));
    else FastLED.showColor(Color(255, 0, 0));

    xTaskDelayUntil(&xLastWakeTime, (PERIODIC_T2 / portTICK_PERIOD_MS));
  }
}

static void Task3(void *parameters) {
  TickType_t xLastWakeTime;

  while(!endLap) {
    xLastWakeTime = xTaskGetTickCount();
    unsigned long time = millis() - startTime;
    sendMsg(4, time);

    xTaskDelayUntil(&xLastWakeTime, (PERIODIC_T3 / portTICK_PERIOD_MS));
  }
}

static void Task4(void *parameters) {
  TickType_t xLastWakeTime;

  while(!endLap) {
    xLastWakeTime = xTaskGetTickCount();
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration > 0) { 
      distance = duration * VEL_SOUND / 2;
      if (distance < 8.2) {
        endLap = 1;
        sendMsg(2, distance);
        unsigned long time = millis() - startTime;
        sendMsg(1, time);
      }
    }

    xTaskDelayUntil(&xLastWakeTime, (PERIODIC_T4 / portTICK_PERIOD_MS));
  }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
  FastLED.showColor(Color(0, 0, 255));

  pinMode(PIN_ITR20001_LEFT,   INPUT);
  pinMode(PIN_ITR20001_MIDDLE, INPUT);
  pinMode(PIN_ITR20001_RIGHT,  INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);


  String sendBuff;


  while(1) {

    if (Serial.available()) {

      char c = Serial.read();
      sendBuff += c;
      
      if (c == '}')  {
        if (sendBuff == "{OK}") {
          break; 
        }          
        sendBuff = "";
      } 
    }
  }

  xTaskCreate(Task1, "ReadItr", 100, NULL, 4, NULL);
  xTaskCreate(Task2, "LED", 100, NULL, 1, NULL);
  xTaskCreate(Task3, "Ping", 100, NULL, 2, NULL);
  xTaskCreate(Task4, "UltraSound", 100, NULL, 3, NULL);

  sendMsg(0, 0);
  startTime = millis();

}

void loop() {
  // put your main code here, to run repeatedly:

}
