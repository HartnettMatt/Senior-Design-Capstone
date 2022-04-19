#include "clientManager.h"

clientManager::clientManager(std::string mqttBrokerAddress, std::string clientID) : m_id(clientID), m_mqttManager(mqttBrokerAddress, clientID)
{
    m_mqttManager.configure();
    m_mqttManager.managerConnect();
    m_mqttManager.registerTopic(m_newNodeTopic, this);
    m_commandTopic = "";
}

clientManager::~clientManager()
{
    m_mqttManager.managerDisconnect();
}

void clientManager::registerFile(std::string filePath, std::string fileID)
{
    m_fileIdToPath[fileID] = filePath;
}

void clientManager::connectToServer()
{
    nlohmann::json jsonPayload;
    jsonPayload["senderID"] = m_id;
    jsonPayload["receiverID"] = "serverNode";
    jsonPayload["data"] = "HELLO";

    while(!m_mqttManager.isConnected());

    m_mqttManager.publishMessage(m_newNodeTopic, jsonPayload.dump());

    while(!m_connected);
}

void clientManager::initDataTransmit(std::string fileID)
{
    if(m_connected)
    {
        nlohmann::json jsonPayload;
        jsonPayload["senderID"] = m_id;
        jsonPayload["receiverID"] = "serverNode";
        jsonPayload["command"] = "data_ready";
        jsonPayload["fileID"] = fileID;

        m_mqttManager.publishMessage(m_commandTopic, jsonPayload.dump());
    }
    else
    {
        // Add file to queue to be sent once connection resumes
    }
}

void clientManager::transmitData(std::string fileID, std::string address)
{
    CURL* env = curl_easy_init();
    struct curl_slist *list = NULL;
    list = curl_slist_append(list, "Expect:");
    list = curl_slist_append(list, "Content-Type: application/json");

    if(env)
    {
        std::ifstream file;
        std::stringstream fileToString;
        file.open(m_fileIdToPath[fileID], std::ifstream::in);
        fileToString << file.rdbuf();
        std::string payload = fileToString.str();
        file.close();
        
        curl_easy_setopt(env, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(env, CURLOPT_URL, address.c_str());
        curl_easy_setopt(env, CURLOPT_POST, 1L);
        curl_easy_setopt(env, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(env, CURLOPT_HTTPHEADER, list);

        CURLcode res = curl_easy_perform(env);
        if(res == CURLE_OK)
        {
            std::cout << "ALERT: CONNECTION WITH SERVER SUCCESSFULLY COMPLETED" << std::endl;
        }        

        curl_easy_cleanup(env);
    }
    
    m_fileIdToPath.erase(fileID);
}

void clientManager::handleMessage(std::string topic, std::string payload)
{
    nlohmann::json jsonPayload = nlohmann::json::parse(payload);
    if(topic == m_newNodeTopic)
    {
        if(jsonPayload["receiverID"] == m_id)
        {
            std::cout << "ALERT: SERVER COMMAND TOPIC: " << jsonPayload["commandTopic"] << std::endl;
            m_commandTopic = jsonPayload["commandTopic"];
            m_mqttManager.registerTopic(m_commandTopic, this);
            m_connected = true;
        }
    }
    else if(topic == m_commandTopic)
    {
        if(jsonPayload["receiverID"] == m_id)
        {
            // Handle commands from Server
            if(jsonPayload["command"] == "send_data")
            {
                std::cout << "ALERT: send_data command received" << std::endl;
                std::cout << "ALERT: TRANSMITTING DATA" << std::endl;
                transmitData(jsonPayload["fileID"], jsonPayload["ip"]);
            }
        }
    }
    else
    {
        std::cerr << "ERROR: Unrecognized topic" << std::endl;
    }
}
