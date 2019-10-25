/******************************************************************************
 * Copyright 2019 Google
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#include "CloudIoTCoreMqtt.h"

#include <cstring>

// Forward global callback declarations
void messageReceived(String &topic, String &payload);


///////////////////////////////
// MQTT common functions
///////////////////////////////
CloudIoTCoreMqtt::CloudIoTCoreMqtt(
    MQTTClient *_mqttClient, Client *_netClient, CloudIoTCoreDevice *_device)
{
  this->mqttClient = _mqttClient;
  this->netClient = _netClient;
  this->device = _device;
}

void CloudIoTCoreMqtt::setLogConnect(boolean enabled)
{
  this->logConnect = enabled;
}

void CloudIoTCoreMqtt::setUseLts(boolean enabled)
{
  this->useLts = enabled;
}

void CloudIoTCoreMqtt::startMQTT()
{
  if (this->useLts) {
    //TODO: Debugging
    //Serial.println("Connect with " + String(CLOUD_IOT_CORE_MQTT_HOST_LTS) + ":" + String(CLOUD_IOT_CORE_MQTT_PORT));
    this->mqttClient->begin(CLOUD_IOT_CORE_MQTT_HOST_LTS, CLOUD_IOT_CORE_MQTT_PORT, *netClient);
  }
  else {
    //TODO: Debugging
    //Serial.println("Connect with " + String(CLOUD_IOT_CORE_MQTT_HOST_LTS) + ":" + String(CLOUD_IOT_CORE_MQTT_PORT));
    this->mqttClient->begin(CLOUD_IOT_CORE_MQTT_HOST, CLOUD_IOT_CORE_MQTT_PORT, *netClient);
  }
  this->mqttClient->onMessage(messageReceived);
}

// bool CloudIoTCoreMqtt::publishTelemetry(String data)
// {
//   char topic[64];
//   device->getEventsTopic(topic);
//   return this->mqttClient->publish(topic, data);
// }

bool CloudIoTCoreMqtt::publishTelemetry(const char* data, int length)
{
  char topic[64];
  device->getEventsTopic(topic);
  return this->mqttClient->publish(topic, data, length);
}

bool CloudIoTCoreMqtt::publishTelemetry(const char* subtopic, const char* data, int length)
{
  char topic[128];
  device->getEventsTopic(topic);
  strcat(topic, subtopic);
  return this->mqttClient->publish(topic, data, length);
}

// bool CloudIoTCoreMqtt::publishTelemetry(String subtopic, String data) {
//   return this->mqttClient->publish(device->getEventsTopic() + subtopic, data);
// }

// bool CloudIoTCoreMqtt::publishTelemetry(String subtopic, const char* data, int length) {
//   return this->mqttClient->publish(String(device->getEventsTopic() + subtopic).c_str(), data, length);
// }

// Helper that just sends default sensor
// bool CloudIoTCoreMqtt::publishState(String data)
// {
//   char topic[64];
//   device->getStateTopic(topic);
//   return this->mqttClient->publish(topic, data);
// }

bool CloudIoTCoreMqtt::publishState(const char* data, int length)
{
  char topic[64];
  device->getStateTopic(topic);
  return this->mqttClient->publish(topic, data, length);
}

void CloudIoTCoreMqtt::onConnect() {
  if (logConnect) {
    char* state = "connected";
    publishState(state, strlen(state));
    //publishTelemetry("/events", device->getDeviceId() + String("-connected"));
  }
}

void CloudIoTCoreMqtt::logError() {
  Serial.println(this->mqttClient->lastError());
  switch(this->mqttClient->lastError()) {
    case (LWMQTT_BUFFER_TOO_SHORT):
      Serial.println("LWMQTT_BUFFER_TOO_SHORT");
      break;
    case (LWMQTT_VARNUM_OVERFLOW):
      Serial.println("LWMQTT_VARNUM_OVERFLOW");
      break;
    case (LWMQTT_NETWORK_FAILED_CONNECT):
      Serial.println("LWMQTT_NETWORK_FAILED_CONNECT");
      break;
    case (LWMQTT_NETWORK_TIMEOUT):
      Serial.println("LWMQTT_NETWORK_TIMEOUT");
      break;
    case (LWMQTT_NETWORK_FAILED_READ):
      Serial.println("LWMQTT_NETWORK_FAILED_READ");
      break;
    case (LWMQTT_NETWORK_FAILED_WRITE):
      Serial.println("LWMQTT_NETWORK_FAILED_WRITE");
      break;
    case (LWMQTT_REMAINING_LENGTH_OVERFLOW):
      Serial.println("LWMQTT_REMAINING_LENGTH_OVERFLOW");
      break;
    case (LWMQTT_REMAINING_LENGTH_MISMATCH):
      Serial.println("LWMQTT_REMAINING_LENGTH_MISMATCH");
      break;
    case (LWMQTT_MISSING_OR_WRONG_PACKET):
      Serial.println("LWMQTT_MISSING_OR_WRONG_PACKET");
      break;
    case (LWMQTT_CONNECTION_DENIED):
      Serial.println("LWMQTT_CONNECTION_DENIED");
      break;
    case (LWMQTT_FAILED_SUBSCRIPTION):
      Serial.println("LWMQTT_FAILED_SUBSCRIPTION");
      break;
    case (LWMQTT_SUBACK_ARRAY_OVERFLOW):
      Serial.println("LWMQTT_SUBACK_ARRAY_OVERFLOW");
      break;
    case (LWMQTT_PONG_TIMEOUT):
      Serial.println("LWMQTT_PONG_TIMEOUT");
      break;
    default:
      Serial.println("This error code should never be reached.");
      break;
  }
}

void CloudIoTCoreMqtt::logReturnCode() {
  Serial.println(this->mqttClient->returnCode());
  switch(this->mqttClient->returnCode()) {
    case (LWMQTT_CONNECTION_ACCEPTED):
      Serial.println("OK");
      break;
    case (LWMQTT_UNACCEPTABLE_PROTOCOL):
      Serial.println("LWMQTT_UNACCEPTABLE_PROTOCOLL");
      break;
    case (LWMQTT_IDENTIFIER_REJECTED):
      Serial.println("LWMQTT_IDENTIFIER_REJECTED");
      break;
    case (LWMQTT_SERVER_UNAVAILABLE):
      Serial.println("LWMQTT_SERVER_UNAVAILABLE");
      break;
    case (LWMQTT_BAD_USERNAME_OR_PASSWORD):
      Serial.println("LWMQTT_BAD_USERNAME_OR_PASSWORD");
      device->invalidateJWT();
      break;
    case (LWMQTT_NOT_AUTHORIZED):
      Serial.println("LWMQTT_NOT_AUTHORIZED");
      device->invalidateJWT();
      break;
    case (LWMQTT_UNKNOWN_RETURN_CODE):
      Serial.println("LWMQTT_UNKNOWN_RETURN_CODE");
      break;
    default:
      Serial.println("This return code should never be reached.");
      break;
  }
}

void CloudIoTCoreMqtt::mqttConnect(bool skip) {
  Serial.print("\nconnecting...");
  bool keepgoing = true;
  char client_id[200];
  char topic[64];
  device->getClientId(client_id);

  while (keepgoing) {
    this->mqttClient->connect(client_id, "unused", device->getJWT(), skip);

    if (this->mqttClient->lastError() != LWMQTT_SUCCESS){
      logError();
      logReturnCode();
      // See https://cloud.google.com/iot/docs/how-tos/exponential-backoff
      if (this->__backoff__ < this->__minbackoff__) {
        this->__backoff__ = this->__minbackoff__;
      }
      this->__backoff__ = (this->__backoff__ * this->__factor__) + random(this->__jitter__);
      if (this->__backoff__ > this->__max_backoff__) {
        this->__backoff__ = this->__max_backoff__;
      }

      // Clean up the client
      this->mqttClient->disconnect();
      skip = false;
      Serial.print("Delaying ");
      Serial.print(this->__backoff__);
      Serial.println("ms");
      delay(this->__backoff__);
      keepgoing = true;
    } else {
      // We're now connected
      Serial.println("\nconnected!");
      keepgoing = false;
      this->__backoff__ = this->__minbackoff__;
    }
  }

  // Set QoS to 1 (ack) for configuration messages
  device->getConfigTopic(topic);
  this->mqttClient->subscribe(topic, 1);
  // QoS 0 (no ack) for commands
  device->getCommandsTopic(topic);
  this->mqttClient->subscribe(topic, 0);

  onConnect();
}
