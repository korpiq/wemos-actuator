
// provided by IotHub implementation
extern const char * setupIotHub(const char * mqtt_server_url); // returns error message or null on success
extern const char * talkWithIotHub(const char * message); // returns error message or null on success

// provided by main app
extern bool callDeviceMethod(const char * methodName);

