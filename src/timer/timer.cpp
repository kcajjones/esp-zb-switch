#include "timer.h"
#include "../main.h"
#include "../buzzer/buzzer.h"
#include "../light/light.h"
#include "../rgb/rgb.h"  // Add RGB header

#define TIMER_QUEUE_SIZE 10
#define TIMER_TASK_PRIORITY 2
#define TIMER_STACK_SIZE 2048

typedef struct {
    uint8_t device;
    bool start;
} timer_cmd_t;

TimerHandle_t fanTimer = NULL;
TimerHandle_t lightTimer = NULL;
static QueueHandle_t timerCmdQueue = NULL;
static TaskHandle_t timerTaskHandle = NULL;
static bool isIdentifying = false;

static void timerCallback(TimerHandle_t xTimer) {
    uint32_t id = (uint32_t)pvTimerGetTimerID(xTimer);
    if (id == 0) {  // Fan
        digitalWrite(RELAY_FAN_PIN, LOW);
        digitalWrite(LED_FAN_PIN, LOW);
        BZ_PlayTimerTone(false, true);
        fanTimer = NULL;  // Clear handle after use
        RGB_FlashFanOff();  // Flash red twice
    } else {  // Light
        LIGHT_SetBrightness(0);
        digitalWrite(LED_LIGHT_PIN, LOW);
        BZ_PlayTimerTone(false, false);
        lightTimer = NULL;  // Clear handle after use
        RGB_FlashLightOff();  // Flash purple twice
    }
    // Update RGB status after timer completes
    RGB_ShowTimerStatus(fanTimer != NULL, lightTimer != NULL);
}

static void timerTask(void *pvParameters) {
    timer_cmd_t cmd;
    
    while(1) {
        if(xQueueReceive(timerCmdQueue, &cmd, portMAX_DELAY)) {
            TimerHandle_t *timer = (cmd.device == 0) ? &fanTimer : &lightTimer;
            
            if(cmd.start) {
                // Stop existing timer
                if(*timer != NULL) {
                    xTimerStop(*timer, 0);
                    xTimerDelete(*timer, 0);
                    *timer = NULL;
                }
                
                // Create new timer
                *timer = xTimerCreate(
                    cmd.device == 0 ? "FanTimer" : "LightTimer",
                    pdMS_TO_TICKS(TIMER_DURATION_MS),
                    pdFALSE,
                    (void*)(uintptr_t)cmd.device,
                    timerCallback
                );
                
                if(*timer != NULL) {
                    xTimerStart(*timer, 0);
                    // Play start tone after timer started
                    BZ_PlayTimerTone(true, cmd.device == 0);
                    // Update RGB status when timer starts
                    RGB_ShowTimerStatus(fanTimer != NULL, lightTimer != NULL);
                }
            } else {
                if(*timer != NULL) {
                    xTimerStop(*timer, 0);
                    xTimerDelete(*timer, 0);
                    *timer = NULL;
                }
            }
        }
    }
}

void TM_Init(void) {
    timerCmdQueue = xQueueCreate(TIMER_QUEUE_SIZE, sizeof(timer_cmd_t));
    xTaskCreate(timerTask, "TimerTask", TIMER_STACK_SIZE, NULL, TIMER_TASK_PRIORITY, &timerTaskHandle);
}

void TM_StartTimer(uint8_t device) {
    TimerHandle_t *timer = (device == 0) ? &fanTimer : &lightTimer;
    
    if(*timer != NULL) {
        xTimerDelete(*timer, 0);
        *timer = NULL;
    }
    
    *timer = xTimerCreate(
        device == 0 ? "FanTimer" : "LightTimer",
        pdMS_TO_TICKS(TIMER_DURATION_MS),
        pdFALSE,
        (void*)(uintptr_t)device,
        timerCallback
    );
    
    if(*timer != NULL) {
        xTimerStart(*timer, 0);
        BZ_PlayTimerTone(true, device == 0);
        // Update RGB status when timer starts
        RGB_ShowTimerStatus(fanTimer != NULL, lightTimer != NULL);
    }
}

void TM_StopTimer(uint8_t device) {
    TimerHandle_t *timer = (device == 0) ? &fanTimer : &lightTimer;
    if(*timer != NULL) {
        xTimerDelete(*timer, 0);
        *timer = NULL;
        // Update RGB status when timer stops
        RGB_ShowTimerStatus(fanTimer != NULL, lightTimer != NULL);
    }
}