#include "buzzer.h"
#include "../main.h"

void BZ_Init() {
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);
}

void BZ_PlayTimerTone(bool isStart, bool isFan) {
    if (isStart) {
        tone(BUZZER_PIN, isFan ? FAN_START_TONE : LIGHT_START_TONE);
        delay(200);
        noTone(BUZZER_PIN);
        delay(100);
        tone(BUZZER_PIN, isFan ? FAN_START_TONE : LIGHT_START_TONE);
        delay(400);
        noTone(BUZZER_PIN);
    } else {
        tone(BUZZER_PIN, isFan ? FAN_START_TONE : LIGHT_START_TONE);
        delay(300);
        noTone(BUZZER_PIN);
        delay(100);
        tone(BUZZER_PIN, isFan ? FAN_START_TONE : LIGHT_START_TONE);
        delay(300);
        noTone(BUZZER_PIN);
        delay(100);
        tone(BUZZER_PIN, isFan ? FAN_END_TONE : LIGHT_END_TONE);
        delay(600);
        noTone(BUZZER_PIN);
    }
}

void BZ_PlayShortBeep(bool isFan) {
    tone(BUZZER_PIN, isFan ? FAN_START_TONE : LIGHT_START_TONE);
    delay(200);
    noTone(BUZZER_PIN);
}