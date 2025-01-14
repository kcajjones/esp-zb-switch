#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

// Hardware definitions
#define RGB_LED_PIN GPIO_NUM_8
#define RELAY_FAN_PIN GPIO_NUM_4
#define RELAY_LIGHT_PIN GPIO_NUM_5
#define LED_FAN_PIN GPIO_NUM_12
#define LED_LIGHT_PIN GPIO_NUM_13
#define BUZZER_PIN GPIO_NUM_15

// Function declarations
void startTimer(uint8_t device);
void playTimerTone(bool isStart, bool isFan);

#endif