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

#include <stdio.h>
#include <cstring>

#include "crypto/ecdsa.h"
#include "crypto/nn.h"
#include "crypto/sha256.h"
#include "jwt.h"

// base64_encode copied from https://github.com/ReneNyffenegger/cpp-base64
static const char* base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

void base64_encode(const char *bytes_to_encode, unsigned int in_len,
                   char *out)
{
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++) {
        *out = base64_chars[char_array_4[i]];
        out++;
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++) {
      *out = base64_chars[char_array_4[j]];
      out++;
    }

    while ((i++ < 3)) {
      *out = '=';
      out++;
    }
  }
}

// Get's sha256 of str. digest must have size SHA256_DIGEST_LENGTH
void get_sha(const char* str, unsigned char* digest)
{
  Sha256 sha256Instance;
  sha256Instance.update((const unsigned char*)str, strlen(str));
  sha256Instance.final(digest);
}

// Get base64 signature string from the signature_r and signature_s ecdsa
// signature.
void make_base64_signature(NN_DIGIT *signature_r, NN_DIGIT *signature_s, char *out)
{
  unsigned char signature[64];
  NN_Encode(signature, (NUMWORDS - 1) * NN_DIGIT_LEN, signature_r,
            (NN_UINT)(NUMWORDS - 1));
  NN_Encode(signature + (NUMWORDS - 1) * NN_DIGIT_LEN,
            (NUMWORDS - 1) * NN_DIGIT_LEN, signature_s,
            (NN_UINT)(NUMWORDS - 1));

  base64_encode((char*)signature, 64, out);
}


void create_jwt(char* jwt, const char* project_id, long long int time, NN_DIGIT* priv_key, int jwt_exp_secs)
{
  char* current = jwt;
  memset(jwt, 0, JWT_MAX_LENGTH);

  ecc_init();

  // Header
  const char* header = "{\"alg\":\"ES256\",\"typ\":\"JWT\"}";
  Serial.println(header);
  base64_encode(header, strlen(header), current);
  current += strlen(current);

  *current = '.';
  current++;

  // Payload
  char payload[100];
  memset(payload, 0, 100);
  sprintf(payload, "{\"iat\":%lld,\"exp\":%lld,\"aud\":\"%s\"}",
           (long long int)time,  // iat
           (long long int)(time + jwt_exp_secs),  // exp
           project_id);  // aud
  Serial.println(payload);
  base64_encode(payload, strlen(payload), current);
  current += strlen(current);

  // sha256
  unsigned char sha256[SHA256_DIGEST_LENGTH];
  get_sha(jwt, sha256);

  // Signing sha with ec key. Bellow is the ec private key.
  point_t pub_key;
  ecc_gen_pub_key(priv_key, &pub_key);

  ecdsa_init(&pub_key);

  NN_DIGIT signature_r[NUMWORDS], signature_s[NUMWORDS];
  ecdsa_sign(sha256, signature_r, signature_s, priv_key);

  *current = '.';
  current++;

  make_base64_signature(signature_r, signature_s, current);
}
