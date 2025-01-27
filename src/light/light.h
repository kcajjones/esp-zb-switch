#ifndef LIGHT_H
#define LIGHT_H

#include <Arduino.h>

#define LIGHT_PWM_FREQ     5000U  // Use unsigned literal to avoid overflow
#define LIGHT_PWM_RES      8
#define LIGHT_PWM_CHANNEL  0
#define LIGHT_FADE_TIME    750

void LIGHT_Init(void);
void LIGHT_SetBrightness(uint8_t brightness);
uint8_t LIGHT_GetBrightness(void);

#endif