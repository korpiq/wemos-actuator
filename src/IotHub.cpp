#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#include "IotHub.h"

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = NULL;

int deviceMethodCallback(
    const char *methodName,
    const unsigned char *payload,
    size_t size,
    unsigned char **response,
    size_t *response_size,
    void *userContextCallback)
{
    return callDeviceMethod(methodName) ? 200 : 404;
}

IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    callDeviceMethod("start");
    return IOTHUBMESSAGE_ACCEPTED;
}

static void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
        // ok
    }
    else
    {
        // voe voe
    }
}

static const char * sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, const char * buffer)
{
    const char * result = NULL;
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char *)buffer, strlen(buffer));
    if (messageHandle == NULL)
    {
        result = "IoTHubMessage_CreateFromByteArray failed.";
    }
    else
    {
        MAP_HANDLE properties = IoTHubMessage_Properties(messageHandle);
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            result = "IoTHubClient_LL_SendEventAsync failed.";
        }

        IoTHubMessage_Destroy(messageHandle);
    }

    return result;
}

void twinCallback(
    DEVICE_TWIN_UPDATE_STATE updateState,
    const unsigned char *payLoad,
    size_t size,
    void *userContextCallback)
{
    callDeviceMethod("start");
}

void setupIotHub(const char * mqtt_server_url) {
    iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(mqtt_server_url, MQTT_Protocol);
    IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "wemos-actuator");
    IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
    IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
    IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);
}

const char * talkWithIotHub(const char * message) {
    if (message) {
        return sendMessage(iotHubClientHandle, message);
    }

    IoTHubClient_LL_DoWork(iotHubClientHandle);
    return NULL;
}

