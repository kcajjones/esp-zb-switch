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

// RGB LED pin
#define RGB_LED_PIN GPIO_NUM_8
#define RELAY_PIN GPIO_NUM_15
static Adafruit_NeoPixel rgbLed(1, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

// States
bool isZigbeeConnected = false;
static bool isZigbeeIdentifying = false;

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

    // handle any logic required when attributes are updated
    if (!message) {
        log_e("Empty message");
    } else if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        log_e("Received message: error status(%d)", message->info.status);
    } else {
        log_i("Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.size);

        // Handle our remote switch toggle
        if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT && message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
            switch (message->attribute.id) {
                case ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID: {
                    bool switchState = *(bool *)(message->attribute.data.value);
                    log_i("Switch state changed to: %s", switchState ? "on" : "off");

                    // Flash green twice when turning the relay on
                    if (switchState) {
                        for (int i = 0; i < 2; i++) {
                            rgbLed.setPixelColor(0, rgbLed.Color(0, 255, 0)); // Green
                            rgbLed.show();
                            delay(200);
                            rgbLed.clear();
                            rgbLed.show();
                            delay(200);
                        }
                    } else { // Flash red twice when turning the relay off
                        for (int i = 0; i < 2; i++) {
                            rgbLed.setPixelColor(0, rgbLed.Color(255, 0, 0)); // Red
                            rgbLed.show();
                            delay(200);
                            rgbLed.clear();
                            rgbLed.show();
                            delay(200);
                        }
                    }

                    // Set the relay pin based on switch state
                    digitalWrite(RELAY_PIN, switchState ? HIGH : LOW);
                    break;
                }

                default:
                    log_i("Unhandled attribute update");
                    break;
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

/********************* Arduino functions **************************/
void setup() {
    Serial.begin(115200);

    // Init switches
    SW_InitSwitches();

    // Init LED for identify function
    rgbLed.begin();
    rgbLed.setBrightness(64);
    rgbLed.clear();
    rgbLed.show();

    // Initialize the relay pin
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Ensure relay is off by default

    // Set our callbacks for Zigbee events
    ZB_SetOnCreateClustersCallback(onCreateClusters);
    ZB_SetOnAttributeUpdatedCallback(onAttributeUpdated);
    ZB_SetOnCustomClusterCommandCallback(onCustomClusterCommand);
    ZB_SetOnIdentifyCallback(onZigbeeIdentify);

    // Start Zigbee task
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
