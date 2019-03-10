#include "Configuration.h"

configuration_t configuration = {
    (char *) "",
    (char *) "",
    (char *) "",
    10000,
    0
};

const char * config_filename = "settings.json";
const char * mqtt_server_url_name = "mqtt_server_url";
const char * wifi_name_name = "wifi_name";
const char * wifi_password_name = "wifi_password";
const char * run_duration_ms_name = "run_duration_ms";

const char * configuration_parameter_names [] = {
    wifi_name_name,
    wifi_password_name,
    mqtt_server_url_name,
    run_duration_ms_name,
    NULL
};

enum configuration_parameter_index {
    WIFI_NAME,
    WIFI_PASSWORD,
    MQTT_SERVER_URL,
    DEFAULT_RUN_MILLISECONDS
};

enum configuration_parameter_type {
    TYPE_TEXT,
    TYPE_ULONG
};

const configuration_parameter_type configuration_parameter_types [] = {
    TYPE_TEXT,
    TYPE_TEXT,
    TYPE_TEXT,
    TYPE_ULONG
};

template <typename Arg>
void log_append(Arg message)
{
    Serial.print(" ");
    Serial.print(message);
}

template <typename Arg, typename... Args>
void log_append(Arg message, Args... message_parts)
{
    Serial.print(" ");
    Serial.print(message);
    log_append(message_parts...);
}

template <typename... Args>
void write_log(Args... message_parts)
{
    Serial.printf("[%04lu]", millis()/1000);
    log_append(message_parts...);
    Serial.println();
    Serial.flush();
}

int getParameterNameIndex(const char * name)
{
    for (int i = 0; configuration_parameter_names[i]; i++)
    {
        if (! strncmp(name, configuration_parameter_names[i], PARAM_NAME_LEN))
        {
            return i;
        }
    }
    return -1;
}

char ** accessTextParameterByIndex(configuration_t * configuration, int index)
{
    switch (index)
    {
        case WIFI_NAME:         return &configuration->wifi_name;
        case WIFI_PASSWORD:     return &configuration->wifi_password;
        case MQTT_SERVER_URL:   return &configuration->mqtt_server_url;
        default:                return NULL;
    }
}

unsigned long * accessULongParameterByIndex(configuration_t * configuration, int index)
{
    switch (index)
    {
        case DEFAULT_RUN_MILLISECONDS: return &configuration->run_duration_ms;
        default: return NULL;
    }
}

char * copy_string_realloc_when_longer(char * target, const char * source, size_t max_length)
{
    size_t source_size = source ? strnlen(source, max_length) : 0;
    size_t target_size = target ? strnlen(target, max_length) : 0;

    if (source_size > target_size)
    {
        target = (char *) (target_size ? realloc(target, source_size + 1) : malloc(source_size + 1));
    }

    if (source && target)
    {
        strncpy(target, source, source_size);
        target[source_size] = 0;
    }

    return target;
}

bool setTextParameterByIndex(configuration_t * configuration, int parameterIndex, const char * value)
{
    char ** textParameter = accessTextParameterByIndex(configuration, parameterIndex);
    if (textParameter)
    {
        *textParameter = copy_string_realloc_when_longer(*textParameter, value, PARAM_LEN);
        configuration->configured = millis();
        return true;
    }

    return false;
}

bool setULongParameterByIndex(configuration_t * configuration, int parameterIndex, const char * value)
{
    unsigned long * parameter = accessULongParameterByIndex(configuration, parameterIndex);
    if (parameter)
    {
        sscanf(value, "%lu", parameter);
        configuration->configured = millis();
        return true;
    }

    return false;
}

bool setConfigurationParameterByName(configuration_t * configuration, const char * name, const char * value)
{
    int parameterIndex = getParameterNameIndex(name);
    if (parameterIndex >= 0)
    {
        write_log("set", name, value);
        switch (configuration_parameter_types[parameterIndex])
        {
            case TYPE_TEXT: return setTextParameterByIndex(configuration, parameterIndex, value);
            case TYPE_ULONG: return setULongParameterByIndex(configuration, parameterIndex, value);
        }
    }

    write_log("Failed to set unknown parameter:", name);

    return false;
}


WiFiManager wifiManager;
bool pleaseSaveConfig = false;

void saveConfigCallback ()
{
    pleaseSaveConfig = true;
}

void serializeConfiguration (configuration_t * configuration, char * buffer, size_t bufsiz)
{
    DynamicJsonBuffer json_buffer;
    JsonObject& json = json_buffer.createObject();
    const char * name;

    for (int i = 0; (name = configuration_parameter_names[i]) != NULL; i++)
    {
        switch (configuration_parameter_types[i])
        {
            case TYPE_TEXT:
                json[name] = *accessTextParameterByIndex(configuration, i);
                break;
            case TYPE_ULONG:
                json[name] = *accessULongParameterByIndex(configuration, i);
                break;
        }
        write_log("Serialized", name, (const char *)json[name]);
    }

    json.printTo(buffer, bufsiz);
}

void deserializeConfiguration(configuration_t * configuration, const char * json)
{
    write_log("Reading new configuration:", json);

    DynamicJsonBuffer json_buffer;
    JsonObject &jsonObject = json_buffer.parseObject(json);

    if (jsonObject.success())
    {
        for (const auto& item : jsonObject)
        {
            setConfigurationParameterByName(configuration, item.key, item.value);
        }
        write_log("Configuration ok:", json);
    }
    else
    {
        write_log("Failed to parse:", json);
    }
}

void saveConfiguration (configuration_t * configuration)
{
    char buffer[CONFIG_SIZE];
    serializeConfiguration(configuration, buffer, CONFIG_SIZE);
    write_log("Saving configuration:", buffer);

    File f = SPIFFS.open(config_filename, "w");
    for (int i=0; i < CONFIG_SIZE && buffer[i]; ++i)
    {
        f.write(buffer[i]);
    }
    f.close();

    write_log("Saved to file:", config_filename);
}

void loadConfiguration (configuration_t * configuration)
{
    char config_string[CONFIG_SIZE];
    write_log("Load configuration from file", config_filename);

    if (SPIFFS.exists(config_filename))
    {
        File f = SPIFFS.open(config_filename, "r");
        write_log("Reading file:", config_filename);

        int config_string_length = f.readBytes(config_string, CONFIG_SIZE);
        f.close();
        config_string[config_string_length] = '\0';
        write_log("Read configuration:", config_string);

        deserializeConfiguration(configuration, config_string);
    }
    else
    {
        write_log("Configuration file does not exist.");
    }
}

void setupWifi (configuration_t * configuration, const char * setup_wlan_name)
{
    WiFiManagerParameter mqtt_server_parameter(mqtt_server_url_name, "MQTT Server", configuration->mqtt_server_url, PARAM_LEN);
    wifiManager.addParameter(&mqtt_server_parameter);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    write_log("Wifi setup starting with fallback WLAN:", setup_wlan_name);

    wifiManager.autoConnect(setup_wlan_name);

    if (pleaseSaveConfig)
    {
        write_log("Saving configuration given over setup WLAN");
        configuration->mqtt_server_url =
            copy_string_realloc_when_longer(configuration->mqtt_server_url, mqtt_server_parameter.getValue(), PARAM_LEN);
        configuration->configured = millis();

        saveConfiguration(configuration);
    }

    write_log("Wifi setup done");
}

void setupConfiguration (configuration_t * configuration, const char * setup_wlan_name)
{
    write_log("Mounting filesystem");

    if (SPIFFS.begin())
    {
        write_log("Filesystem mounted");

        loadConfiguration(configuration);
    }
    else
    {
        write_log("Formatting new filesystem");

        SPIFFS.format();
    }

    setupWifi(configuration, setup_wlan_name);
}

void saveUpdatedConfiguration(configuration_t * configuration)
{
    saveConfiguration(configuration);

    if (configuration->wifi_name && *configuration->wifi_name) {
        Serial.print("Connecting to WiFi network: ");
        Serial.print(configuration->wifi_name);

        WiFi.mode(WIFI_STA);
        WiFi.begin(configuration->wifi_name, configuration->wifi_password);        
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        
        Serial.println("");
        write_log("WiFi connected with IP address:", WiFi.localIP());
    }
}

void reconfigure(configuration_t * configuration, const char * json)
{
    unsigned long last_configured = configuration->configured;
    deserializeConfiguration(configuration, json);
    if (configuration->configured != last_configured)
    {
        saveUpdatedConfiguration(configuration);
    }
    else
    {
        write_log("Configuration failed, not saved.");
    }
}

void reportConfiguration (configuration_t * configuration)
{
    if (configuration->configured)
    {
        char buffer[CONFIG_SIZE];
        serializeConfiguration(configuration, buffer, CONFIG_SIZE);

        Serial.println(buffer);
    }
    else
    {
        Serial.println("\"unconfigured\"");
    }
}
