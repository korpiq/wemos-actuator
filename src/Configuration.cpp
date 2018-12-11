#include "Configuration.h"

configuration_t configuration = {
    ""
};

char * const config_filename = "/config.json";
char * const mqtt_server_url_name = "mqtt_server_url";

#define PARAM_LEN 200
#define PARAM_NAME_LEN 20
#define NUM_PARAMS 1
#define CONFIG_SIZE (PARAM_LEN + PARAM_NAME_LEN + 6) * NUM_PARAMS

StaticJsonBuffer<CONFIG_SIZE> json_buffer;

WiFiManager wifiManager;
bool pleaseSaveConfig = false;

void saveConfigCallback ()
{
    pleaseSaveConfig = true;
}

void serialize (configuration_t * configuration, char * buffer, size_t bufsiz)
{
    JsonObject& json = json_buffer.createObject();

    json[mqtt_server_url_name] = configuration->mqtt_server_url;

    json.printTo(buffer, bufsiz);
}

void saveConfiguration (configuration_t * configuration)
{
    char buffer[CONFIG_SIZE];
    serialize(configuration, buffer, CONFIG_SIZE);

    File f = SPIFFS.open(config_filename, "w");
    for (int i=0; i < CONFIG_SIZE && buffer[i]; ++i)
    {
        f.write(buffer[i]);
    }
    f.close();
}

void loadConfiguration (configuration_t * configuration)
{
    char config_string[CONFIG_SIZE];
    File f = SPIFFS.open(config_filename, "r");
    int config_string_length = f.readBytes(config_string, CONFIG_SIZE);
    f.close();
    config_string[config_string_length] = '\0';

    JsonObject & json = json_buffer.parseObject(config_string);
    if (json.success())
    {
        configuration->mqtt_server_url = json[mqtt_server_url_name];
    }
}

void setupWifi (configuration_t * configuration, char * const setup_wlan_name)
{
    WiFiManagerParameter mqtt_server_parameter(mqtt_server_url_name, "MQTT Server", configuration->mqtt_server_url, PARAM_LEN);
    wifiManager.addParameter(&mqtt_server_parameter);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    wifiManager.autoConnect(setup_wlan_name);

    if (pleaseSaveConfig)
    {
        saveConfiguration(configuration);
    }
}

void setupConfiguration (configuration_t * configuration, char * const setup_wlan_name)
{
    if (SPIFFS.begin())
    {
        loadConfiguration(configuration);
    }
    else
    {
        SPIFFS.format();
    }

    setupWifi(configuration, setup_wlan_name);
}

void reportConfiguration (configuration_t * configuration)
{
    char buffer[CONFIG_SIZE];
    serialize(configuration, buffer, CONFIG_SIZE);

    Serial.println(buffer);
}