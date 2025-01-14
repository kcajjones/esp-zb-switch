#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

void TM_Init();
void TM_StartTimer(uint8_t device);
void TM_StopTimer(uint8_t device);

#define TIMER_DURATION_MS (8 * 60 * 1000)

#endif