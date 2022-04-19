#ifndef CLIENT_MANAGER
#define CLIENT_MANAGER
#include <map>
#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <mqttConsumer.h>
#include <mqttManager.h>

class clientManager : public imqttConsumer
{
    public:
        clientManager(std::string mqttBrokerAddress, std::string clientID);
        ~clientManager();
        void connectToServer();
        void handleMessage(std::string topic, std::string payload);
        void initDataTransmit(std::string fileID);
        void transmitData(std::string fileID, std::string address);
        void registerFile(std::string filePath, std::string fileID);
    private:
        const std::string m_newNodeTopic = "new_node";
        const std::string m_id;
        bool m_connected = false;
        std::map<std::string, std::string> m_fileIdToPath;
        std::string m_commandTopic;
        mqttManager m_mqttManager;
};

#endif