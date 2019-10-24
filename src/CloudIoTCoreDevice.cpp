/******************************************************************************
 * Copyright 2018 Google
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

#include "CloudIoTCoreDevice.h"
#include <stdio.h>
#include <cstring>
#include "jwt.h"


CloudIoTCoreDevice::CloudIoTCoreDevice(const char *_project_id,
                                       const char *_location,
                                       const char *_registry_id,
                                       const char *_device_id,
                                       const char *_private_key)
{
  strcpy(this->project_id, _project_id);
  strcpy(this->location, _location);
  strcpy(this->registry_id, _registry_id);
  strcpy(this->device_id, _device_id);
  setPrivateKey(_private_key);
}

void CloudIoTCoreDevice::createJWT(long long int current_time)
{
  // Disable software watchdog as these operations can take a while.
  ESP.wdtDisable();
  Serial.println("Refreshing JWT");

  create_jwt(jwt, project_id, current_time, priv_key, jwt_exp_secs);
  iss = current_time;
  exp = current_time + jwt_exp_secs;

  Serial.println(jwt);

  ESP.wdtEnable(0);
}

const char* CloudIoTCoreDevice::getJWT()
{
  long long int current_time = time(nullptr);
  if (0 == iss || exp + 1*60 < current_time) {
    createJWT(current_time);
  }
  return jwt;
}


void CloudIoTCoreDevice::invalidateJWT()
{
  iss = 0;
  exp = 0;
}

void CloudIoTCoreDevice::getFullPath(const char* path, char* out)
{
  sprintf(out, "/v1/projects/%s/locations/%s/registries/%s/devices/%s%s",
      project_id, location, registry_id, device_id, path);
}

void CloudIoTCoreDevice::getClientId(char* out)
{
  sprintf(out, "projects/%s/locations/%s/registries/%s/devices/%s",
    project_id, location, registry_id, device_id);
}

void CloudIoTCoreDevice::getConfigTopic(char* out)
{
  sprintf(out, "/devices/%s/config", device_id);
}

void CloudIoTCoreDevice::getCommandsTopic(char* out)
{
  sprintf(out, "/devices/%s/commands/#", device_id);
}

void CloudIoTCoreDevice::getDeviceId(char* out)
{
  sprintf(out, "%s", device_id);
}

void CloudIoTCoreDevice::getEventsTopic(char* out)
{
  sprintf(out, "/devices/%s/events", device_id);
}

void CloudIoTCoreDevice::getStateTopic(char* out)
{
  sprintf(out, "/devices/%s/state", device_id);
}

void CloudIoTCoreDevice::getConfigPath(int version, char* out)
{
  char path[40];
  sprintf(path, "/config?local_version=%d", version);
  getFullPath(path, out);
}

// String CloudIoTCoreDevice::getLastConfigPath() {
//   return this->getConfigPath(0);
// }

void CloudIoTCoreDevice::getSendTelemetryPath(char* out)
{
  getFullPath(":publishEvent", out);
}

void CloudIoTCoreDevice::getSetStatePath(char* out)
{
  getFullPath(":setState", out);
}

void CloudIoTCoreDevice::setJwtExpSecs(int exp_in_secs) {
  this->jwt_exp_secs = exp_in_secs;
}

CloudIoTCoreDevice &CloudIoTCoreDevice::setPrivateKey(const char *private_key)
{
  size_t length = strlen(private_key);
  if (length != 95) {
    Serial.print("Warning: expected private key to be 95, was: ");
    Serial.println(length);
    return *this;
  }

  // fillPrivateKey();
  priv_key[8] = 0;
  for (int i = 7; i >= 0; i--) {
    priv_key[i] = 0;
    for (int byte_num = 0; byte_num < 4; byte_num++) {
      priv_key[i] = (priv_key[i] << 8) + strtoul(private_key, NULL, 16);
      private_key += 3;
    }
  }

  return *this;
}
