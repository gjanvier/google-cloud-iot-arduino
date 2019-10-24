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

#ifndef CloudIoTCoreDevice_h
#define CloudIoTCoreDevice_h

#include <Arduino.h>
#include "jwt.h"

class CloudIoTCoreDevice {
 private:
  char project_id[32];
  char location[32];
  char registry_id[32];
  char device_id[32];

  NN_DIGIT priv_key[9];

  char jwt[JWT_MAX_LENGTH];
  unsigned long iss = 0;
  unsigned long exp = 0;
  int jwt_exp_secs = 3600;

  CloudIoTCoreDevice &setPrivateKey(const char *private_key);
  void createJWT(long long int current_time);
  void getFullPath(const char* path, char* out);

 public:
  CloudIoTCoreDevice(const char *project_id, const char *location,
                     const char *registry_id, const char *device_id,
                     const char *private_key);

  void setJwtExpSecs(int exp_in_secs);

  /* Get a valid JWT */
  const char* getJWT();

  void invalidateJWT();

  /* HTTP methods path */
  void getConfigPath(int version, char* out);
  //String getLastConfigPath();
  void getSendTelemetryPath(char* out);
  void getSetStatePath(char* out);

  /* MQTT methods */
  void getClientId(char* out);
  void getCommandsTopic(char* out);
  void getConfigTopic(char* out);
  void getDeviceId(char* out);
  void getEventsTopic(char* out);
  void getStateTopic(char* out);
};
#endif  // CloudIoTCoreDevice_h
