### Zigbee controlled extractor fan and light strip for my kitchen hob ###

A Zigbee controlled ESP32-C6 with timer functionality, rgb lighting and buzzer alerts for on/off and timer on/off functions, along with local control via two momentary buttons which have an integrated single colour led.

This code is made up of 4 simple zigbee cluster switches with the following function:

Light on/off
Fan on/off
Light Timer on/off
Fan Timer on/off


In detail:

GPIO 4 is the Fan output connected to a relay which switches power to an extractor fan.
GPIO 19 is the physical button for the fan.
GPIO 12 is the LED built into the physical button for the fan.

GPIO 5 is the Light output connected to a relay which switches power to an LED power supply -> LED strip.
GPIO 20 is the physical button for the light.
GPIO 13 is the LED built into the physical button for the light.

GPIO 15 is a motherboard piezo buzzer/speaker.
GPIO 8 is the EPC32-C6-DevKitC-1-N8 integrated RGB LED.


TO DO:
Control LED light strip with dimmer functionality. LED Driver and LED strip is single colour not addressable, but is dimmable.
Need to investigate how to connect ESP32 to LED driver/LED strip. Maybe mosfet?


**NOTE**
*This code is based on Skye-Harris's implemantion based on Espressif's Arduino-ESP32 code and heavily modified largely by AI with my minimal coding knowledge. This code won't be optimal and might not function as intended.*
