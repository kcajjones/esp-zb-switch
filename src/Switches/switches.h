#include <Arduino.h>
#include "../main.h"

#define GPIO_FACTORY_RESET_SWITCH GPIO_NUM_9
#define GPIO_FAN_BUTTON GPIO_NUM_19
#define GPIO_LIGHT_BUTTON GPIO_NUM_13
#define PAIR_SIZE(TYPE_STR_PAIR) (sizeof(TYPE_STR_PAIR) / sizeof(TYPE_STR_PAIR[0]))

typedef enum {
    SWITCH_RESET_CONTROL,
    SWITCH_FAN_CONTROL,
    SWITCH_LIGHT_CONTROL
} switch_func_t;

typedef struct {
    uint8_t pin;
    switch_func_t func;
} switch_func_pair_t;

typedef enum {
    SWITCH_IDLE,
    SWITCH_PRESS_ARMED,
    SWITCH_PRESS_DETECTED,
    SWITCH_PRESSED,
    SWITCH_LONG_PRESS_DETECTED,
    SWITCH_RELEASE_DETECTED,
} switch_state_t;

static switch_func_pair_t button_func_pair[] = {
    {GPIO_FACTORY_RESET_SWITCH, SWITCH_RESET_CONTROL},
    {GPIO_FAN_BUTTON, SWITCH_FAN_CONTROL},
    {GPIO_LIGHT_BUTTON, SWITCH_LIGHT_CONTROL}
};

void SW_InitSwitches();
void SW_Loop();
void startTimer(uint8_t device);  // device: 0=fan, 1=light