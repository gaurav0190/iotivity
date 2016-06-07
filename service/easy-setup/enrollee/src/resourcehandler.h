//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Do not remove the include below

#include <string.h>
#include "logger.h"
#include "ocstack.h"
#include "escommon.h"
//#include "networkhandler.h"
#include "octypes.h"

#ifndef ES_RESOURCE_HANDLER_H_
#define ES_RESOURCE_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ESEnrolleeResourceEventCallback)(ESResult);

/* Structure to represent a Light resource */
typedef struct PROVRESOURCE
{
    OCResourceHandle handle;
    int64_t status; // provisiong status, 1 : need to provisioning, 2 : Connected to Enroller.
    bool trigger; // Trigger network connection, 0 : Init value, 1 : Connected to the target network.
    int64_t errorCode;
} ProvResource;

typedef struct
{
    OCResourceHandle handle;
    int64_t supportedMode[NUM_WIFIMODE];
    uint8_t numMode;
    int64_t supportedFreq;
    char ssid[MAXSSIDLEN]; // target network name, i.e. SSID for WLAN, MAC address for BT.
    char cred[MAXNETCREDLEN]; // credential information.
    int64_t authType;
    int64_t encType;
} WiFiResource;

typedef struct
{
    OCResourceHandle handle;
    char authCode[OIC_STRING_MAX_VALUE];
    char authProvider[OIC_STRING_MAX_VALUE];
    char ciServer[OIC_STRING_MAX_VALUE];
} CloudResource;

typedef struct
{
    OCResourceHandle handle;
    char devName[OIC_STRING_MAX_VALUE];
    char language[OIC_STRING_MAX_VALUE];
    char country[OIC_STRING_MAX_VALUE];
} DevConfResource;


OCStackResult CreateProvisioningResource(bool isSecured);
OCStackResult CreateEasySetupResources(bool isSecured);
OCStackResult DeleteProvisioningResource();
OCStackResult DeleteEasySetupResources();


void GetTargetNetworkInfoFromProvResource(char *, char *);
void RegisterResourceEventCallBack(ESEnrolleeResourceEventCallback);
void UnRegisterResourceEventCallBack(void);

#ifdef __cplusplus
}
#endif

#endif //ES_RESOURCE_HANDLER_H_
