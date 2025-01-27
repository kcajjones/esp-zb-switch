#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

#define TIMER_DURATION_MS (8 * 60 * 1000)  // 8 minutes

extern TimerHandle_t fanTimer;
extern TimerHandle_t lightTimer;

void TM_Init(void);
void TM_StartTimer(uint8_t device);
void TM_StopTimer(uint8_t device);

#endif