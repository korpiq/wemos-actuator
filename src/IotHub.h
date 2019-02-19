
// provided by IotHub implementation
extern void setupIotHub(const char * mqtt_server_url);
extern const char * talkWithIotHub(const char * message); // returns error message or null on success

// provided by main app
extern bool callDeviceMethod(const char * methodName);

