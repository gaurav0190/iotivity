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


#ifndef EASYSETUP_ENROLLEE_H__
#define EASYSETUP_ENROLLEE_H__

#include "escommon.h"

/**
 * @file
 *
 * This file contains Enrollee APIs
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Callback function for updating the Enrollee OnBoarding and Provisioning status result
 * to the application
 *
 * @param esResult ESResult provides the current state of the Enrollee Device
 */
typedef void (*ESEnrolleeEventCallback)(ESResult esResult, ESEnrolleeState enrolleeState);



typedef struct
{
    void (*WiFiProvCb) (ESWiFiProvData *);
    void (*DevConfProvCb) (ESDevConfProvData *);
    void (*CloudDataProvCb) (ESCloudProvData *);
} ESProvisioningCallbacks;

/**
 * This function Initializes the EasySetup. This API must be called prior to invoking any other API
 *
 * @param isSecured         True if the Enrollee is operating in secured mode.
 * @param resourceMask      Provisining Resource Type which application wants to make
 *                          ES_WIFI_RESOURCE = 0x01,
 *                          ES_CLOUD_RESOURCE = 0x02,
 *                          ES_DEVCONF_RESOURCE = 0x04
 * @param callbacks         ESProvisioningCallbacks for updating Provisioning Resources' data to the application
 * @return ::ES_OK on success, some other value upon failure.
 */
ESResult ESInitEnrollee(bool isSecured, ESResourceMask resourceMask, ESProvisioningCallbacks callbacks);


/**
 * This function performs termination of Provisioning and Network resources.
 * Also terminates the IoTivity core stack.
 *
 * @return ::ES_OK on success, some other value upon failure.
 */
ESResult ESTerminateEnrollee();

#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* EASYSETUP_ENROLLEE_H__ */

