#include "light.h"
#include "../main.h"
#include <Arduino.h>

static uint8_t currentBrightness = 0;

void LIGHT_Init(void) {
    // Configure the PWM channel
    ledcAttach(PWM_LIGHT_PIN, LIGHT_PWM_FREQ, LIGHT_PWM_RES);  // Adjusted to new API
    ledcWrite(PWM_LIGHT_PIN, 0);
}

void LIGHT_SetBrightness(uint8_t brightness) {
    currentBrightness = brightness;
    ledcWrite(PWM_LIGHT_PIN, brightness);
}

uint8_t LIGHT_GetBrightness(void) {
    return currentBrightness;
}