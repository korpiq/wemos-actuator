#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void webRequest() {
    server.send(200, "application/json", "{\"hello\":\"World!\"}");
}

void startWebServer() {
    server.on("/", webRequest);
}

void webServerStep() {
    server.handleClient();
}

