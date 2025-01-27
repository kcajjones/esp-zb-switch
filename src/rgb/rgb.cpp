#include "rgb.h"
#include "../main.h"
#include <Adafruit_NeoPixel.h>

static Adafruit_NeoPixel rgbLed(1, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
static bool isIdentifying = false;
static uint32_t lastUpdateTime = 0;

void RGB_Init(void) {
    rgbLed.begin();
    rgbLed.setBrightness(64);
    rgbLed.clear();
    rgbLed.show();
}

void RGB_Update(void) {
    if (isIdentifying) {
        // Orange blink for identify
        uint32_t now = millis();
        if (now - lastUpdateTime > 500) {
            lastUpdateTime = now;
            static bool state = false;
            rgbLed.setPixelColor(0, state ? rgbLed.Color(255, 165, 0) : rgbLed.Color(0, 0, 0));
            rgbLed.show();
            state = !state;
        }
    }
}

void RGB_ShowTimerStatus(bool isFanTimer, bool isLightTimer) {
    static bool ledState = false;
    uint32_t now = millis();
    
    if (now - lastUpdateTime > 1000) {
        lastUpdateTime = now;
        if (isFanTimer || isLightTimer) {
            if (isFanTimer && isLightTimer) {
                // Alternate between blue and yellow
                rgbLed.setPixelColor(0, ledState ? rgbLed.Color(RGB_COLOR_BLUE) : rgbLed.Color(RGB_COLOR_YELLOW));
                digitalWrite(LED_FAN_PIN, ledState ? HIGH : LOW);
                digitalWrite(LED_LIGHT_PIN, ledState ? HIGH : LOW);
            } else if (isFanTimer) {
                rgbLed.setPixelColor(0, ledState ? rgbLed.Color(RGB_COLOR_BLUE) : rgbLed.Color(RGB_COLOR_OFF));
                digitalWrite(LED_FAN_PIN, ledState ? HIGH : LOW);
                digitalWrite(LED_LIGHT_PIN, LOW);
            } else if (isLightTimer) {
                rgbLed.setPixelColor(0, ledState ? rgbLed.Color(RGB_COLOR_YELLOW) : rgbLed.Color(RGB_COLOR_OFF));
                digitalWrite(LED_FAN_PIN, LOW);
                digitalWrite(LED_LIGHT_PIN, ledState ? HIGH : LOW);
            }
            rgbLed.show();
            ledState = !ledState;
        } else {
            RGB_Clear();
            digitalWrite(LED_FAN_PIN, LOW);
            digitalWrite(LED_LIGHT_PIN, LOW);
        }
    }
}

void RGB_SetColor(uint8_t r, uint8_t g, uint8_t b) {
    rgbLed.setPixelColor(0, rgbLed.Color(r, g, b));
    rgbLed.show();
}

void RGB_Clear(void) {
    rgbLed.clear();
    rgbLed.show();
}

void RGB_StartIdentify(void) {
    isIdentifying = true;
}

void RGB_StopIdentify(void) {
    isIdentifying = false;
    RGB_Clear();
}

void RGB_FlashColor(uint8_t r, uint8_t g, uint8_t b, int times) {
    for (int i = 0; i < times; i++) {
        rgbLed.setPixelColor(0, rgbLed.Color(r, g, b));
        rgbLed.show();
        delay(200);
        RGB_Clear();
        delay(200);
    }
}

void RGB_FlashFanOff() {
    RGB_FlashColor(255, 0, 0, 2);  // Red
}

void RGB_FlashLightOff() {
    RGB_FlashColor(128, 0, 128, 2);  // Purple
}