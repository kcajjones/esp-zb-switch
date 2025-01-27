#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Zigbee/zigbee.h"
#include "Switches/switches.h"
#include "timer/timer.h"
#include "buzzer/buzzer.h"
#include "light/light.h"
#include "main.h"
#include "rgb/rgb.h"

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
                RGB_FlashFanOff();  // Flash red twice
            } else {
                RGB_FlashColor(0, 0, 255, 1);  // Flash blue once
            }
        } else if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT + 1) {
            // Light direct control - modified for PWM
            LIGHT_SetBrightness(switchState ? 255 : 0);
            digitalWrite(LED_LIGHT_PIN, switchState ? HIGH : LOW);
            if (!switchState) {
                TM_StopTimer(1);  // Stop light timer when turned off
                RGB_FlashLightOff();  // Flash purple twice
            } else {
                RGB_FlashColor(255, 255, 0, 1);  // Flash yellow once
            }
        } else if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT + 2) {
            // Fan Timer
            if (switchState) {
                digitalWrite(RELAY_FAN_PIN, HIGH);
                digitalWrite(LED_FAN_PIN, HIGH);
                TM_StartTimer(0);
            } else {
                TM_StopTimer(0);
            }
        } else if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT + 3) {
            // Light Timer
            if (switchState) {
                TM_StartTimer(1);                        // Start timer first
                LIGHT_SetBrightness(255);                // Start fade last
                digitalWrite(LED_LIGHT_PIN, HIGH);        // Update indicator
            } else {
                TM_StopTimer(1);
            }
        }
        // Update RGB status after attribute update
        RGB_ShowTimerStatus(fanTimer != NULL, lightTimer != NULL);
    } else if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {
        if (message->info.dst_endpoint == HA_ESP_SENSOR_ENDPOINT + 1) {
            uint8_t level = *(uint8_t *)(message->attribute.data.value);
            LIGHT_SetBrightness(level);
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
    RGB_StartIdentify();
    while (isZigbeeIdentifying) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    RGB_StopIdentify();
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

    // Init hardware
    pinMode(RELAY_FAN_PIN, OUTPUT);
    pinMode(LED_FAN_PIN, OUTPUT);
    pinMode(PWM_LIGHT_PIN, OUTPUT);
    pinMode(LED_LIGHT_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // Init modules
    BZ_Init();

    // Set default states
    digitalWrite(RELAY_FAN_PIN, LOW);
    digitalWrite(LED_FAN_PIN, LOW);
    digitalWrite(LED_LIGHT_PIN, LOW);

    // Setup light control
    LIGHT_Init();
    LIGHT_SetBrightness(0);

    TM_Init();  // Will handle RGB LED init
    RGB_Init();

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
        }
    }

    if (isZigbeeConnected) {
        RGB_Update();
    } else {
        // LED off when not connected
    }

    // Update RGB status based on timer status
    RGB_ShowTimerStatus(fanTimer != NULL, lightTimer != NULL);
}