#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

void BZ_Init();
void BZ_PlayTimerTone(bool isStart, bool isFan);
void BZ_PlayShortBeep(bool isFan);  // New function

#define FAN_START_TONE 2000
#define FAN_END_TONE 1900
#define LIGHT_START_TONE 1800
#define LIGHT_END_TONE 1750

#endif