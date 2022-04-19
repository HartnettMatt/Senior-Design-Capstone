#ifndef MQTT_CONSUMER
#define MQTT_CONSUMER

#include <string>

class imqttConsumer{
    public:
        virtual void handleMessage(std::string topic, std::string payload) = 0;
    private:
};

#endif