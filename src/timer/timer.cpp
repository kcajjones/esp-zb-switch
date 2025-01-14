#include "timer.h"
#include "../main.h"
#include "../buzzer/buzzer.h"

static TimerHandle_t fanTimer = NULL;
static TimerHandle_t lightTimer = NULL;

static void timerCallback(TimerHandle_t xTimer) {
    uint32_t id = (uint32_t)pvTimerGetTimerID(xTimer);
    if (id == 0) {  // Fan
        digitalWrite(RELAY_FAN_PIN, LOW);
        digitalWrite(LED_FAN_PIN, LOW);
        BZ_PlayTimerTone(false, true);
    } else {  // Light
        digitalWrite(RELAY_LIGHT_PIN, LOW);
        digitalWrite(LED_LIGHT_PIN, LOW);
        BZ_PlayTimerTone(false, false);
    }
}

void TM_StartTimer(uint8_t device) {
    TimerHandle_t *timer = (device == 0) ? &fanTimer : &lightTimer;
    
    if (*timer != NULL) {
        xTimerDelete(*timer, 0);
    }
    
    *timer = xTimerCreate(
        device == 0 ? "FanTimer" : "LightTimer",
        pdMS_TO_TICKS(TIMER_DURATION_MS),
        pdFALSE,
        (void*)(uintptr_t)device,
        timerCallback
    );
    
    if (*timer != NULL) {
        xTimerStart(*timer, 0);
    }
}

void TM_StopTimer(uint8_t device) {
    TimerHandle_t *timer = (device == 0) ? &fanTimer : &lightTimer;
    if (*timer != NULL) {
        xTimerStop(*timer, 0);
    }
}