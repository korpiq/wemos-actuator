#include "Arduino.h"
#include "Configuration.h"
#include "IotHub.h"

extern "C" {
#include "user_interface.h"
}

#define LED 2
#define ACTUATOR_PIN D1

bool serialBegun = false;
bool led_state = false;
bool iotHubStarted = false;
unsigned long actuatorRunningSince = 0;

void blink (uint8_t times, uint16_t hi, uint16_t lo)
{
    while(times--)
    {
      digitalWrite(LED, HIGH);
      delay(hi);
      digitalWrite(LED, LOW);
      delay(lo);
    }
}

void report_memory()
{
    uint32_t free = system_get_free_heap_size();
    Serial.print("Free memory: ");
    Serial.println(free);
    Serial.flush();
}

bool callDeviceMethod(const char * methodName) {
    Serial.printf("Try to invoke method %s.\r\n", methodName);

    if (strncmp(methodName, "start", 6) == 0)
    {
        start();
        return true;
    }
    else if (strncmp(methodName, "stop", 5) == 0)
    {
        stop();
        return true;
    }

    Serial.printf("No method %s found.\r\n", methodName);
    return false;
}

void start() {
    digitalWrite(ACTUATOR_PIN, HIGH);
    actuatorRunningSince = millis();

    Serial.println("Actuator started.");
    Serial.flush();
}

void stop() {
    digitalWrite(ACTUATOR_PIN, LOW);
    actuatorRunningSince = 0;

    Serial.println("Actuator stopped.");
    Serial.flush();
}

void setup ()
{
    pinMode(LED, OUTPUT);
    blink(10, 400, 100);
    digitalWrite(LED, HIGH);

    pinMode(ACTUATOR_PIN, OUTPUT);
    stop();

    Serial.begin(115200);
    Serial.println("Configure?");
    report_memory();

    setupConfiguration(&configuration, "DEVICE_SETUP");

    Serial.println("Configured.");
    reportConfiguration(&configuration);
    report_memory();

    blink(10, 100, 100);

    startIotHub();

    serialBegun = true;
}

void startIotHub() {
    if (configuration.configured && !iotHubStarted) {
        Serial.println("Connecting to IotHub");
        setupIotHub(configuration.mqtt_server_url);
        iotHubStarted = true;
    } else {
        Serial.println("Not connecting to IotHub at this time");
    }
}

void loop ()
{
    if (actuatorRunningSince > 0) {
        if (actuatorRunningSince < millis() + 12000ul) {
            stop();
        }
    }

    if (Serial != serialBegun)
    {
        if (! serialBegun)
        {
            Serial.begin(115200);
            Serial.println("Hello");
            Serial.flush();

            blink(4, 200, 100);
        }
        else
        {
            blink(8, 100, 100);
        }

        serialBegun = !serialBegun;
    }

    if (serialBegun)
    {
        if(Serial.available())
        {
            blink(3, 200, 100);

            char buffer[CONFIG_SIZE + 1];
            size_t buflen = Serial.readBytesUntil('\n', buffer, CONFIG_SIZE);
            if (buflen > 2)
            {
                buffer[buflen] = 0;
                Serial.print("reconfiguring: ");
                Serial.println(buffer);
                Serial.flush();
                reconfigure(&configuration, buffer);
                startIotHub();
            }
            else
            {
                start();
            }

            reportConfiguration(&configuration);
            report_memory();

            blink(5, 100, 100);
        }
    }

    if (iotHubStarted)
    {
        const char * result = talkWithIotHub("[\"Hello there!\"]\r\n");
        if (result)
        {
            Serial.println(result);
        }
    }

    blink(1, 500, 500);
}
