//
// Created by mark9 on 16/02/2025.
//

#ifndef MQTT_H
#define MQTT_H

#include "pico/stdlib.h"
//#include "pico/cyw43_arch.h"
//#include <MQTTClient.h>
//#include "IPStack.h"
//#include "Countdown.h"
#include <functional>

/*
class MQTTTEST {
public:
    MQTTTEST(const char* broker, int port,
               const char* cmd_topic, const char* status_topic,IPStack &ipstack);

    void connect();
    void publish(const char* message);
    void checkMessages();
    bool isConnected();

private:
    static MQTTTEST *instance;
    const char* ssid;
    const char* password;
    const char* broker;
    int port;
    const char* cmd_topic;
    const char* status_topic;

    IPStack &ipstack;
    MQTT::Client<IPStack, Countdown> client;

    void setupWiFi();
    static void messageArrived(MQTT::MessageData& md);
};

*/
#endif //MQTT_H
