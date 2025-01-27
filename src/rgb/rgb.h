#ifndef RGB_H
#define RGB_H

#include <Arduino.h>

void RGB_Init(void);
void RGB_Update(void);
void RGB_StartIdentify(void);
void RGB_StopIdentify(void);
void RGB_ShowTimerStatus(bool isFanTimer, bool isLightTimer);
void RGB_Clear(void);
void RGB_FlashColor(uint8_t r, uint8_t g, uint8_t b, int times);  // Add this line
void RGB_FlashFanOff(void);  // Add this line
void RGB_FlashLightOff(void);  // Add this line

#endif