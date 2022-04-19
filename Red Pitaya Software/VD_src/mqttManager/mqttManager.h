#ifndef MQTT_MANAGER
#define MQTT_MANAGER

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <map>
#include <mutex>
#include <mqtt/async_client.h>

#include <mqttConsumer.h>

const int           QOS = 1;
const int	        N_RETRY_ATTEMPTS = 5;

class mqttManager : public virtual mqtt::callback, 
                    public virtual mqtt::iaction_listener
{
    public:
        mqttManager(std::string serverAddress, std::string clientID);
        void managerConnect();
        void managerDisconnect();
        void registerTopic(std::string topic, imqttConsumer* handler);
        void publishMessage(std::string topic, std::string payload);
        bool isConnected();
        void configure();
    private:
        void on_failure(const mqtt::token& tok) override;
        void on_success(const mqtt::token& tok) override;
        void connected(const std::string& cause) override;
        void connection_lost(const std::string& cause) override;
        void message_arrived(mqtt::const_message_ptr msg) override;
        void updateTopics();
        bool m_configured = false;
        bool m_connected = false;
        mqtt::async_client m_client;
        mqtt::connect_options m_connectionOptions;
        std::map<std::string, imqttConsumer*> m_topicToConsumer;
        std::vector<std::string> m_topicsToAdd;
        std::mutex m_topicsToAddMutex;
};

#endif