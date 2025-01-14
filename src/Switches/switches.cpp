#include "../main.h"
#include "switches.h"
#include "Zigbee/zigbee.h"
#include "../buzzer/buzzer.h"
#include "../timer/timer.h"  // Add this include

/********************* GPIO functions **************************/
static QueueHandle_t gpioEventQueue = NULL;
static switch_state_t switch_state = SWITCH_IDLE;  // Add state definition

static void IRAM_ATTR gpioIsrHandler(void *arg) {
    xQueueSendFromISR(gpioEventQueue, (switch_func_pair_t *)arg, NULL);
}

static void setGpioInterruptEnabled(bool enabled) {
    for (int i = 0; i < PAIR_SIZE(button_func_pair); ++i) {
        if (enabled) {
            enableInterrupt((button_func_pair[i]).pin);
        } else {
            disableInterrupt((button_func_pair[i]).pin);
        }
    }
}

#define LONG_PRESS_TIME 1500  // 1.5 seconds
static unsigned long pressStartTime = 0;

static void onButtonPress(switch_func_pair_t *button_func_pair) {
    switch (button_func_pair->func) {
        case SWITCH_RESET_CONTROL:
            if (switch_state != SWITCH_LONG_PRESS_DETECTED) {  // Ignore long press for reset
                ZB_FactoryReset();
            }
            break;
        case SWITCH_FAN_CONTROL:
            if (switch_state == SWITCH_LONG_PRESS_DETECTED) {
                digitalWrite(RELAY_FAN_PIN, HIGH);
                digitalWrite(LED_FAN_PIN, HIGH);
                TM_StartTimer(0);
                BZ_PlayTimerTone(true, true);
            } else {
                bool newState = !digitalRead(RELAY_FAN_PIN);
                digitalWrite(RELAY_FAN_PIN, newState);
                digitalWrite(LED_FAN_PIN, newState);
                if (!newState) {
                    TM_StopTimer(0);  // Stop timer when turned off
                }
                BZ_PlayShortBeep(true);
            }
            break;
        case SWITCH_LIGHT_CONTROL:
            if (switch_state == SWITCH_LONG_PRESS_DETECTED) {
                digitalWrite(RELAY_LIGHT_PIN, HIGH);
                digitalWrite(LED_LIGHT_PIN, HIGH);
                TM_StartTimer(1);
                BZ_PlayTimerTone(true, false);
            } else {
                bool newState = !digitalRead(RELAY_LIGHT_PIN);
                digitalWrite(RELAY_LIGHT_PIN, newState);
                digitalWrite(LED_LIGHT_PIN, newState);
                if (!newState) {
                    TM_StopTimer(1);  // Stop timer when turned off
                }
                BZ_PlayShortBeep(false);
            }
            break;
    }
}

void SW_InitSwitches() {
    // Init button switch
    for (int i = 0; i < PAIR_SIZE(button_func_pair); i++) {
        pinMode(button_func_pair[i].pin, INPUT_PULLUP);
        /* create a queue to handle gpio event from isr */
        gpioEventQueue = xQueueCreate(10, sizeof(switch_func_pair_t));
        if (gpioEventQueue == 0) {
            log_e("Queue was not created and must not be used");
            while (1);
        }
        attachInterruptArg(button_func_pair[i].pin, gpioIsrHandler, (void *)(button_func_pair + i), FALLING);
    }
}

void SW_Loop() {
    // Handle button switch in loop()
    uint8_t pin = 0;
    switch_func_pair_t button_func_pair;

    /* check if there is any queue received, if yes read out the button_func_pair */
    if (xQueueReceive(gpioEventQueue, &button_func_pair, portMAX_DELAY)) {
        pin = button_func_pair.pin;
        setGpioInterruptEnabled(false);

        while (true) {
            bool value = digitalRead(pin);

            switch (switch_state) {
                case SWITCH_IDLE:
                    if (value == LOW) {
                        pressStartTime = millis();
                        switch_state = SWITCH_PRESS_DETECTED;
                    }
                    break;
                case SWITCH_PRESS_DETECTED:
                    if (value == LOW) {
                        if (millis() - pressStartTime > LONG_PRESS_TIME) {
                            switch_state = SWITCH_LONG_PRESS_DETECTED;
                        }
                    } else {
                        switch_state = SWITCH_RELEASE_DETECTED;
                    }
                    break;
                case SWITCH_RELEASE_DETECTED:
                    switch_state = SWITCH_IDLE;
                    /* callback to button_handler */
                    (*onButtonPress)(&button_func_pair);
                    break;

                default:
                    break;
            }

            if (switch_state == SWITCH_IDLE) {
                setGpioInterruptEnabled(true);

                break;
            }

            delay(10);
        }
    }
}