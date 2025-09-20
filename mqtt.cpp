#include "mqtt.h"
#include <cstring>
#include <iostream>

//MQTTTEST* MQTTTEST::instance = nullptr;

/*
MQTTTEST::MQTTTEST(
    const char* broker, int port,
    const char* cmd_topic, const char* status_topic,IPStack &ipstack) :
    broker(broker), port(port),
    cmd_topic(cmd_topic), status_topic(status_topic),ipstack(ipstack), client(ipstack) {
    //IPStack ipstack(ssid, password);
    instance = this;
    connect();
}
void messageArrived(MQTT::MessageData &md) {
    MQTT::Message &message = md.message;

    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n",
           message.qos, message.retained, message.dup, message.id);
    printf("Payload %s\n", (char *) message.payload);
}

void MQTTTEST::connect()
{
    Connect to WiFi
    if (cyw43_arch_init()) return false;
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password,
        CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        return false;
    }

    // Connect to MQTT broker
    int rc = ipstack.connect(broker, port);
    if (rc != 1) {
        std::cout << "rc from TCP connect is "<< rc << std::endl;
    }
    std::cout << "MQTT connecting" <<std::endl;

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)"garage";
    rc = client.connect(data);
    if (rc != 0) {
        std::cout << "Connection Failed.rc from MQTT connect is "<< rc << std::endl;
    }else{
        std::cout << "MQTT connected." <<std::endl;
    }

    // Subscribe to command topic
    client.subscribe(cmd_topic, MQTT::QOS0, messageArrived);
 //   return true;//TODO change to comment instead return

}

void MQTTTEST::publish(const char* message) {
    if(!client.isConnected()){}
    MQTT::Message msg;
    msg.qos = MQTT::QOS0;
    msg.retained = false;
    msg.dup = false;
    msg.payload = (void*)message;
    msg.payloadlen = strlen(message) + 1;
    client.publish(status_topic, msg);
}

void MQTTTEST::checkMessages() {
    client.yield(1);
}


void MQTTTEST::messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char* payload = static_cast<char*>(message.payload);
    payload[message.payloadlen] = '\0'; // Null-terminate

    // Forward to application callback
  //  instance-> //TODO whatever function to handle message
 //   if (md.topicName == std::string(cmd_topic)) {}
        // You'll need to implement a way to pass this to GarageDoor
}
*/