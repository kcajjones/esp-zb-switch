// Copyright 2024 Skye Harris
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "Zigbee/zigbee.h"
#include "Switches/switches.h"
#include "timer/timer.h"
#include "buzzer/buzzer.h"
#include "main.h"

static Adafruit_NeoPixel rgbLed(1, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

// States
bool isZigbeeConnected = false;
static bool isZigbeeIdentifying = false;

void flashRgbLed(bool isFan, bool isOn) {
    uint32_t color;
    if (isFan) {
        color = isOn ? rgbLed.Color(0, 255, 0) : rgbLed.Color(255, 0, 0);  // Green/Red
    } else {
        color = isOn ? rgbLed.Color(255, 255, 255) : rgbLed.Color(255, 165, 0);  // White/Orange
    }

    // Double flash pattern
    for (int i = 0; i < 2; i++) {
        rgbLed.setPixelColor(0, color);
        rgbLed.show();
        delay(200);
        rgbLed.clear();
        rgbLed.show();
        delay(200);
    }
}

/********************* Zigbee Callbacks **************************/
void onCreateClusters(esp_zb_cluster_list_t *clusterList) {
    // Add an on/off cluster to our endpoint
    esp_zb_on_off_cluster_cfg_t onOffConfig = {
        .on_off = false
    };

    esp_zb_attribute_list_t *onOffCluster = esp_zb_on_off_cluster_create(&onOffConfig);
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_on_off_cluster(clusterList, onOffCluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
}

esp_err_t onAttributeUpdated(const esp_zb_zcl_set_attr_value_message_t *message) {
    esp_err_t ret = ESP_OK;
    if (!message) {
        return ESP_FAIL;
    }
    
    if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
        bool switchState = *(bool *)(message->attribute.data.value);
        if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT) {
            // Fan direct control
            digitalWrite(RELAY_FAN_PIN, switchState ? HIGH : LOW);
            digitalWrite(LED_FAN_PIN, switchState ? HIGH : LOW);
            if (!switchState) {
                TM_StopTimer(0);  // Stop fan timer when turned off
            }
            flashRgbLed(true, switchState);
        } else if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT + 1) {
            // Light direct control
            digitalWrite(RELAY_LIGHT_PIN, switchState ? HIGH : LOW);
            digitalWrite(LED_LIGHT_PIN, switchState ? HIGH : LOW);
            if (!switchState) {
                TM_StopTimer(1);  // Stop light timer when turned off
            }
            flashRgbLed(false, switchState);
        } else if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT + 2) {
            // Fan Timer
            if (switchState) {
                digitalWrite(RELAY_FAN_PIN, HIGH);
                digitalWrite(LED_FAN_PIN, HIGH);
                TM_StartTimer(0);
                BZ_PlayTimerTone(true, true);
                flashRgbLed(true, true);
            } else {
                TM_StopTimer(0);
            }
        } else if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT + 3) {
            // Light Timer
            if (switchState) {
                digitalWrite(RELAY_LIGHT_PIN, HIGH);
                digitalWrite(LED_LIGHT_PIN, HIGH);
                TM_StartTimer(1);
                BZ_PlayTimerTone(true, false);
                flashRgbLed(false, true);
            } else {
                TM_StopTimer(1);
            }
        }
    }
    return ret;
}

esp_err_t onCustomClusterCommand(const esp_zb_zcl_custom_cluster_command_message_t *message) {
    esp_err_t ret = ESP_OK;

    // handle any logic required when receiving a command
    log_i("Receive Custom Cluster Command: 0x%x", message->info.command);

    return ret;
}

// Identify thread task
static void taskZigbeeIdentify(void *arg) {
    log_i("Identify task started");

    while (isZigbeeIdentifying) {
        rgbLed.setPixelColor(0, rgbLed.Color(255, 165, 0)); // Orange
        rgbLed.show();
        delay(500);
        rgbLed.clear();
        rgbLed.show();
        delay(500);
    }

    log_i("Identify task ended");
    vTaskDelete(NULL);
}

void onZigbeeIdentify(bool isIdentifying) {
    isZigbeeIdentifying = isIdentifying;

    if (isIdentifying) {
        xTaskCreate(taskZigbeeIdentify, "identify_task", 2048, NULL, 10, NULL);
    }
}

void setup() {
    Serial.begin(115200);

    // Init switches
    SW_InitSwitches();

    // Init LED
    rgbLed.begin();
    rgbLed.setBrightness(64);
    rgbLed.clear();
    rgbLed.show();

    // Init hardware
    pinMode(RELAY_FAN_PIN, OUTPUT);
    pinMode(RELAY_LIGHT_PIN, OUTPUT);
    pinMode(LED_FAN_PIN, OUTPUT);
    pinMode(LED_LIGHT_PIN, OUTPUT);
    
    // Init modules
    BZ_Init();

    // Set default states
    digitalWrite(RELAY_FAN_PIN, LOW);
    digitalWrite(RELAY_LIGHT_PIN, LOW);
    digitalWrite(LED_FAN_PIN, LOW);
    digitalWrite(LED_LIGHT_PIN, LOW);

    // Set Zigbee callbacks
    ZB_SetOnCreateClustersCallback(onCreateClusters);
    ZB_SetOnAttributeUpdatedCallback(onAttributeUpdated);
    ZB_SetOnCustomClusterCommandCallback(onCustomClusterCommand);
    ZB_SetOnIdentifyCallback(onZigbeeIdentify);

    // Start Zigbee
    ZB_StartMainTask();
}

void loop() {
    SW_Loop();

    // Handle connection status indication
    static uint32_t lastUpdateTime = 0;
    static int fadeValue = 0;
    static bool fadeDirection = true;

    if (isZigbeeConnected && !isZigbeeIdentifying) {
        if (millis() - lastUpdateTime > 30) {
            lastUpdateTime = millis();

            if (fadeDirection) {
                fadeValue++;
                if (fadeValue >= 255) fadeDirection = false;
            } else {
                fadeValue--;
                if (fadeValue <= 0) fadeDirection = true;
            }

            rgbLed.setPixelColor(0, rgbLed.Color(fadeValue, 0, fadeValue)); // Purple with fade effect
            rgbLed.show();
        }
    }
}
