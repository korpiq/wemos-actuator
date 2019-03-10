#include <ESP8266WebServer.h>
#include "Configuration.h"
#include "WebServer.h"

ESP8266WebServer server(80);

const size_t html_buffer_maxlen = 8000;

const char * html_page_template =
"<html>"
    "<head>"
        "<title>Power Actuator</title>"
        "<style>"
            "body { font-family: sans-serif }"
            "span { min-width: 10rem; text-align: right; margin-right: 0.1rem; float: left }"
            "button { margin-left: 10.1rem; }"
        "</style>"
    "</head>"
    "<body>"
        "<div>"
            "<h1>Power Actuator</h1>"

            "<h2>Run</h2>"

            "<form action=\"/run\" method=\"post\">"
                "<span>Duration: </span><input name=\"duration\" value=\"%lu\" size=\"6\" /> seconds"
                "<br/>"
                "<button type=\"submit\">Run</button>"
            "</form>"

            "<hr>"

            "<h2>Settings</h2>"

            "<form action=\"/setup\" method=\"post\">"
                "<span>Wifi network: </span><input name=\"wifi_name\" value=\"%s\" size=\"40\" /> (SSID)"
                "<br/>"
                "<span>Wifi password: </span><input name=\"wifi_password\" type=\"password\" size=\"40\" />"
                "<br/>"
                "<span>Send data to: </span><input name=\"mqtt_server_url\" value=\"%s\" size=\"100\" />"
                "<br/>"
                "<span>Default run duration: </span><input name=\"run_duration_ms\" value=\"%lu\" size=\"6\" /> milliseconds"
                "<br/>"
                "<button type=\"submit\">Save settings</button>"
            "</form>"
        "</div>"
    "</body>"
"</html>";

const int html_buffer_length = min(
    html_buffer_maxlen,
    strnlen(html_page_template, html_buffer_maxlen) +
    4 * PARAM_LEN // duration twice, wifi name, uplink
);

void servePage()
{
    char html_buffer[html_buffer_length + 1];

    snprintf(
        html_buffer,
        html_buffer_length,
        html_page_template,

        configuration.run_duration_ms / 1000,
        configuration.wifi_name,
        configuration.mqtt_server_url,
        configuration.run_duration_ms
    );

    server.send(200, "text/html", html_buffer);
}

void redirectToIndex()
{
    server.sendHeader("Location", "/");
    Serial.println("Redirecting to index");
    server.send(302);
    Serial.println("Redirected to index");
}

double durationFromArgs()
{
    for (int i = server.args(); i--;)
    {
        if (server.argName(i).equals("duration"))
        {
            return atof(server.arg(i).c_str());
        }
    }

    return 0.0;
}

void run()
{
    double duration_seconds = durationFromArgs();

    if (duration_seconds > 0.0)
    {
        unsigned long duration_milliseconds = static_cast<unsigned long>(duration_seconds * 1000.0);
        start(duration_milliseconds);
    }

    redirectToIndex();
}

void jsonFromRequestData(char * buffer, size_t bufsiz)
{
    DynamicJsonBuffer json_buffer;
    JsonObject& json = json_buffer.createObject();

    for (int i = min(NUM_PARAMS, server.args()); i--;)
    {
        json[server.argName(i).c_str()] = server.arg(i);
    }

    json.printTo(buffer, bufsiz);
}

void saveSetup()
{
    char name[PARAM_NAME_LEN];
    char value[PARAM_LEN];

    for (int i = min(NUM_PARAMS, server.args()); i--;)
    {
        strncpy(name, server.argName(i).c_str(), PARAM_NAME_LEN);
        strncpy(value, server.arg(i).c_str(), PARAM_LEN);

        setConfigurationParameterByName(&configuration, name, value);
    }

    redirectToIndex();
}

void startWebServer()
{
    server.on("/", HTTP_GET, servePage);
    server.on("/run", HTTP_POST, run);
    server.on("/setup", HTTP_POST, saveSetup);
    server.begin();
}

void webServerStep()
{
    server.handleClient();
}
