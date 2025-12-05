#include "FastLED.h"
#include <Arduino_FreeRTOS.h>

#define TRIG_PIN 13  
#define ECHO_PIN 12  
#define VEL_SOUND 0.0343
#define PIN_ITR20001_LEFT   A2
#define PIN_ITR20001_MIDDLE A1
#define PIN_ITR20001_RIGHT  A0
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

#define DELAY_T1 
CRGB leds[NUM_LEDS];



long duracion, distancia;
int left_IR, right_IR, mid_IR;

bool read_US() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duracion = pulseIn(ECHO_PIN, HIGH, 30000);
  distancia = duracion * VEL_SOUND / 2;

}

static void Task1(void *parameters) {
  TickType_t xLastWakeTime, aux;

  while(1) {
    xLastWakeTime = xTaskGetTickCount();
    aux = xLastWakeTime;


    while ((aux - xLastWakeTime) * portTICK_PERIOD_MS < TIME_ON_T1) {
      xLastWakeTime = xTaskGetTickCount();
    }

    xTaskDelayUntil(&xLastWakeTime, (DELAY_T1 / portTICK_PERIOD_MS));
  }
}

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);

  xTaskCreate(Task1, "ReadItr", 100, NULL, 1, NULL);


}

void loop() {
  // put your main code here, to run repeatedly:

}
