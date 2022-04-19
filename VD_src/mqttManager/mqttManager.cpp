#include "mqttManager.h"

mqttManager::mqttManager(std::string serverAddress, std::string clientID) : m_client(serverAddress, clientID)
{}

void mqttManager::configure()
{
    m_connectionOptions.set_clean_session(false);
    m_client.set_callback(*this);
    m_configured = true;
}

void mqttManager::managerConnect()
{
    std::cout << "ALERT: MQTT Manager is now attempting to connect." << std::endl;
    try
    {
        m_connected = m_client.connect(m_connectionOptions, nullptr, *this)->wait_for(1000);
    }
    catch(const mqtt::exception& e)
    {
        throw e;
    }
}

void mqttManager::managerDisconnect()
{
    try 
    {
        m_client.disconnect()->wait();
    }
    catch (const mqtt::exception& e) 
    {
        throw e;
    }
}

void mqttManager::message_arrived(mqtt::const_message_ptr msg) 
{
    std::string topic = msg->get_topic();
    std::string payload = msg->to_string();

    m_topicToConsumer[topic]->handleMessage(topic, payload);
}

void mqttManager::connection_lost(const std::string& cause) 
{
    std::cout << "ERROR: Connection lost" << std::endl; 
    std::cout << "ALERT: MQTT Manager will now attempt to reconnect" << std::endl;
    m_connected = false;
    managerConnect();
}

void mqttManager::updateTopics()
{
    m_topicsToAddMutex.lock();
    if(!m_topicsToAdd.empty())
    {
        while (!m_topicsToAdd.empty())
        {
            std::cout << "ALERT: Client now subscribing topic " << m_topicsToAdd.back() << std::endl;
            std::cout << std::flush;
            m_client.subscribe(m_topicsToAdd.back(), QOS);
            m_topicsToAdd.pop_back();
        }
    }
    m_topicsToAddMutex.unlock();
}

void mqttManager::connected(const std::string& cause) 
{
    // Connected to broker, now subsribe to new_node topic to start processing new nodes
    std::cout << "ALERT: Successfully connected to MQTT Broker!" << std::endl;
    m_connected = true;
    updateTopics();
}

void mqttManager::on_failure(const mqtt::token& tok) 
{
    std::cout << "ERROR: Failed to connect to MQTT Broker" << std::endl;
    m_connected = false;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    managerConnect();
}

void mqttManager::on_success(const mqtt::token& tok)
{
    updateTopics();
}

bool mqttManager::isConnected()
{
    return m_connected;
}

void mqttManager::registerTopic(std::string topic, imqttConsumer* handler)
{
    if(m_connected)
    {
        try
        {
            m_client.subscribe(topic, QOS);
        }
        catch(const mqtt::exception& e)
        {
            throw e;
        }

        m_topicToConsumer[topic] = handler;
    }
    else
    {
        std::cerr << "WARNING: Attempting to subscribe to topic " << topic << " without being connected to broker!" << std::endl;
        m_topicToConsumer[topic] = handler;
        m_topicsToAddMutex.lock();
        m_topicsToAdd.push_back(topic);
        m_topicsToAddMutex.unlock();
    }
}

void mqttManager::publishMessage(std::string topic, std::string payload)
{
    updateTopics();
    mqtt::message_ptr finalMessage = mqtt::make_message(topic, payload);
    finalMessage->set_qos(QOS);
    m_client.publish(finalMessage)->wait_for(2000);
}