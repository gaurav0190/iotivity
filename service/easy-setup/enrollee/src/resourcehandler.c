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

#include "resourcehandler.h"

#include <stdio.h>

#include "ocpayload.h"
#include "oic_string.h"

/**
 * @var ES_RH_TAG
 * @brief Logging tag for module name.
 */
#define ES_RH_TAG "ES_RH"
//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------

/**
 * @var gProvResource
 * @brief Structure for holding the Provisioning status and target information required to
 * connect to the target network
 */
static ProvResource gProvResource;
static WiFiResource gWiFiResource;
static CloudResource gCloudResource;
static DevConfResource gDevConfResource;

/**
 * @var gWiFiData
 * @brief Structure for holding the target information required to
 * connect to the target network
 */
 static ESWiFiProvData gWiFiData;

/**
 * @var gDevConfData
 * @brief Structure for holding the device information
 */
 static ESDevConfProvData gDevConfData;

/**
 * @var gCloudData
 * @brief Structure for holding the cloud information required to
 * connect to CI Server
 */
 static ESCloudProvData gCloudData;



//-----------------------------------------------------------------------------
// Private internal function prototypes
//-----------------------------------------------------------------------------
OCEntityHandlerResult OCEntityHandlerCb(OCEntityHandlerFlag flag, OCEntityHandlerRequest *ehRequest,
        void *callback);
const char *getResult(OCStackResult result);
OCEntityHandlerResult ProcessGetRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload** payload);
OCEntityHandlerResult ProcessPutRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload** payload);
OCEntityHandlerResult ProcessPostRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload** payload);
void updateProvResource(OCEntityHandlerRequest* ehRequest, OCRepPayload* input);
void updateWiFiResource(OCRepPayload* input);
void updateCloudResource(OCRepPayload* input);
void updateDevConfResource(OCRepPayload* input);

ESWiFiCB gWifiRsrcEvtCb = NULL;
ESCloudCB gCloudRsrcEvtCb = NULL;
ESDevConfCB gDevConfRsrcEvtCb = NULL;

void RegisterWifiRsrcEventCallBack(ESWiFiCB cb)
{
    gWifiRsrcEvtCb = cb;
}

void RegisterCloudRsrcEventCallBack(ESCloudCB cb)
{
    gCloudRsrcEvtCb = cb;
}

void RegisterDevConfRsrcEventCallBack(ESDevConfCB cb)
{
    gDevConfRsrcEvtCb = cb;
}

void UnRegisterResourceEventCallBack()
{
    if (gWifiRsrcEvtCb)
    {
        gWifiRsrcEvtCb = NULL;
    }
    if (gCloudRsrcEvtCb)
    {
        gCloudRsrcEvtCb = NULL;
    }
    if (gDevConfRsrcEvtCb)
    {
        gDevConfRsrcEvtCb = NULL;
    }
}

void GetTargetNetworkInfoFromProvResource(char *name, char *pass)
{
    if (name != NULL && pass != NULL)
    {
        OICStrcpy(name, MAXSSIDLEN, gWiFiResource.ssid);
        OICStrcpy(pass, MAXNETCREDLEN, gWiFiResource.cred);
    }
}

OCStackResult initProvResource(bool isSecured)
{
    gProvResource.status = NO_PROVISION;
    gProvResource.trigger = false;

    OCStackResult res = OC_STACK_ERROR;
    if (isSecured)
    {
        res = OCCreateResource(&gProvResource.handle, OC_RSRVD_ES_RES_TYPE_PROV,
        OC_RSRVD_INTERFACE_DEFAULT,
        OC_RSRVD_ES_URI_PROV, OCEntityHandlerCb,
        NULL, OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);
    }else
    {
        res = OCCreateResource(&gProvResource.handle, OC_RSRVD_ES_RES_TYPE_PROV,
        OC_RSRVD_INTERFACE_DEFAULT,
        OC_RSRVD_ES_URI_PROV, OCEntityHandlerCb,
        NULL, OC_DISCOVERABLE | OC_OBSERVABLE);
    }
    if(res)
    {
        OIC_LOG_V(INFO, ES_RH_TAG, "Created Prov resource with result: %s", getResult(res));
        return res;
    }

    res = OCBindResourceInterfaceToResource(gProvResource.handle, OC_RSRVD_INTERFACE_LL);
    if(res)
    {
        OIC_LOG_V(INFO, ES_RH_TAG, "Created Prov resource with result: %s", getResult(res));
        return res;
    }
    res = OCBindResourceInterfaceToResource(gProvResource.handle, OC_RSRVD_INTERFACE_BATCH);
    if(res)
    {
        OIC_LOG_V(INFO, ES_RH_TAG, "Created Prov resource with result: %s", getResult(res));
        return res;
    }

    OIC_LOG_V(INFO, ES_RH_TAG, "Created Prov resource with result: %s", getResult(res));
    return res;
}

OCStackResult initWiFiResource(bool isSecured)
{
    OCStackResult res = OC_STACK_ERROR;

    gWiFiResource.supportedFreq = WiFi_BOTH;
    gWiFiResource.supportedMode[0] = WiFi_11A;
    gWiFiResource.supportedMode[1] = WiFi_11B;
    gWiFiResource.supportedMode[2] = WiFi_11G;
    gWiFiResource.supportedMode[3] = WiFi_11N;
    gWiFiResource.numMode = 4;
    gWiFiResource.authType = NONE_AUTH;
    gWiFiResource.encType = NONE_ENC;
    OICStrcpy(gWiFiResource.ssid, sizeof(gWiFiResource.ssid), "");
    OICStrcpy(gWiFiResource.cred, sizeof(gWiFiResource.cred), "");

    if (isSecured)
    {
        res = OCCreateResource(&gWiFiResource.handle, OC_RSRVD_ES_RES_TYPE_WIFI,
        OC_RSRVD_INTERFACE_DEFAULT,
        OC_RSRVD_ES_URI_WIFI, OCEntityHandlerCb,
        NULL, OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);
    }else
    {
        res = OCCreateResource(&gWiFiResource.handle, OC_RSRVD_ES_RES_TYPE_WIFI,
        OC_RSRVD_INTERFACE_DEFAULT,
        OC_RSRVD_ES_URI_WIFI, OCEntityHandlerCb,
        NULL, OC_DISCOVERABLE | OC_OBSERVABLE);
    }

    OIC_LOG_V(INFO, ES_RH_TAG, "Created WiFi resource with result: %s", getResult(res));
    return res;

}

OCStackResult initCloudServerResource(bool isSecured)
{
    OCStackResult res = OC_STACK_ERROR;

    OICStrcpy(gCloudResource.authCode, sizeof(gCloudResource.authCode), "");
    OICStrcpy(gCloudResource.authProvider, sizeof(gCloudResource.authProvider), "");
    OICStrcpy(gCloudResource.ciServer, sizeof(gCloudResource.ciServer), "");

    if (isSecured)
    {
        res = OCCreateResource(&gCloudResource.handle, OC_RSRVD_ES_RES_TYPE_CLOUDSERVER,
        OC_RSRVD_INTERFACE_DEFAULT,
        OC_RSRVD_ES_URI_CLOUDSERVER, OCEntityHandlerCb,
        NULL, OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);
    }else
    {
        res = OCCreateResource(&gCloudResource.handle, OC_RSRVD_ES_RES_TYPE_CLOUDSERVER,
        OC_RSRVD_INTERFACE_DEFAULT,
        OC_RSRVD_ES_URI_CLOUDSERVER, OCEntityHandlerCb,
        NULL, OC_DISCOVERABLE | OC_OBSERVABLE);
    }

    OIC_LOG_V(INFO, ES_RH_TAG, "Created CloudServer resource with result: %s", getResult(res));
    return res;

}

OCStackResult initDevConfResource(bool isSecured)
{
    OCStackResult res = OC_STACK_ERROR;

    OICStrcpy(gDevConfResource.devName, sizeof(gDevConfResource.devName), "");
    OICStrcpy(gDevConfResource.country, sizeof(gDevConfResource.country), "");
    OICStrcpy(gDevConfResource.language, sizeof(gDevConfResource.language), "");

    if (isSecured)
    {
        res = OCCreateResource(&gDevConfResource.handle, OC_RSRVD_ES_RES_TYPE_DEVCONF,
        OC_RSRVD_INTERFACE_DEFAULT,
        OC_RSRVD_ES_URI_DEVCONF, OCEntityHandlerCb,
        NULL, OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);
    }else
    {
        res = OCCreateResource(&gDevConfResource.handle, OC_RSRVD_ES_RES_TYPE_DEVCONF,
        OC_RSRVD_INTERFACE_DEFAULT,
        OC_RSRVD_ES_URI_DEVCONF, OCEntityHandlerCb,
        NULL, OC_DISCOVERABLE | OC_OBSERVABLE);
    }

    OIC_LOG_V(INFO, ES_RH_TAG, "Created DevConf resource with result: %s", getResult(res));
    return res;

}

void updateProvResource(OCEntityHandlerRequest* ehRequest, OCRepPayload* input)
{
    OIC_LOG_V(INFO, ES_RH_TAG, "gProvResource.status %lld", gProvResource.status);

    if(ehRequest->query)
    {
        if(strstr(ehRequest->query, OC_RSRVD_INTERFACE_BATCH))
        {
        // When Provisioning resource has a POST with BatchInterface
            updateCloudResource(input);
            updateWiFiResource(input);
            updateDevConfResource(input);
        }
    }
}

void updateWiFiResource(OCRepPayload* input)
{
    char* ssid = NULL;
    if (OCRepPayloadGetPropString(input, OC_RSRVD_ES_SSID, &ssid))
    {
        OICStrcpy(gWiFiResource.ssid, sizeof(gWiFiResource.ssid), ssid);
        OICStrcpy(gWiFiData.ssid, sizeof(gWiFiData.ssid), ssid);
        OIC_LOG_V(INFO, ES_RH_TAG, "gWiFiResource.ssid : %s", gWiFiResource.ssid);
    }

    char* cred = NULL;
    if (OCRepPayloadGetPropString(input, OC_RSRVD_ES_CRED, &cred))
    {
        OICStrcpy(gWiFiResource.cred, sizeof(gWiFiResource.cred), cred);
        OICStrcpy(gWiFiData.pwd, sizeof(gWiFiData.pwd), cred);
        OIC_LOG_V(INFO, ES_RH_TAG, "gWiFiResource.cred %s", gWiFiResource.cred);
    }

    int64_t authType = -1;
    if (OCRepPayloadGetPropInt(input, OC_RSRVD_ES_AUTHTYPE, &authType))
    {
        gWiFiResource.authType = authType;
        gWiFiData.authtype = gWiFiResource.authType;
        OIC_LOG_V(INFO, ES_RH_TAG, "gWiFiResource.authType %u", gWiFiResource.authType);
    }

    int64_t encType = -1;
    if (OCRepPayloadGetPropInt(input, OC_RSRVD_ES_ENCTYPE, &encType))
    {
        gWiFiResource.encType = encType;
        gWiFiData.enctype = gWiFiResource.encType;
        OIC_LOG_V(INFO, ES_RH_TAG, "gWiFiResource.encType %u", gWiFiResource.encType);
    }

    if(ssid || cred || authType!= -1 || encType != -1)
    {
        OIC_LOG(INFO, ES_RH_TAG, "Send WiFiRsrc Callback To ES");

        // TODO : Need to check appropriateness of gWiFiData
        if(gWifiRsrcEvtCb != NULL)
        {
            gWifiRsrcEvtCb(ES_OK, &gWiFiData);
        }
        else
        {
            OIC_LOG(ERROR, ES_RH_TAG, "gWifiRsrcEvtCb is NULL");
        }
    }

}

void updateCloudResource(OCRepPayload* input)
{
    char *authCode = NULL;
    if (OCRepPayloadGetPropString(input, OC_RSRVD_ES_AUTHCODE, &authCode))
    {
        OICStrcpy(gCloudResource.authCode, sizeof(gCloudResource.authCode), authCode);
        OICStrcpy(gCloudData.authCode, sizeof(gCloudData.authCode), authCode);
        OIC_LOG_V(INFO, ES_RH_TAG, "gCloudResource.authCode %s", gCloudResource.authCode);
    }

    char *authProvider = NULL;
    if (OCRepPayloadGetPropString(input, OC_RSRVD_ES_AUTHPROVIDER, &authProvider))
    {
        OICStrcpy(gCloudResource.authProvider, sizeof(gCloudResource.authProvider), authProvider);
        OICStrcpy(gCloudData.authProvider, sizeof(gCloudData.authProvider), authProvider);
        OIC_LOG_V(INFO, ES_RH_TAG, "gCloudResource.authServerUrl %s", gCloudResource.authProvider);
    }

    char *ciServer = NULL;
    if (OCRepPayloadGetPropString(input, OC_RSRVD_ES_CISERVER, &ciServer))
    {
        OICStrcpy(gCloudResource.ciServer, sizeof(gCloudResource.ciServer), ciServer);
        OICStrcpy(gCloudData.ciServer, sizeof(gCloudData.ciServer), ciServer);
        OIC_LOG_V(INFO, ES_RH_TAG, "gCloudResource.ciServer %s", gCloudResource.ciServer);
    }

    if(authCode || authProvider || ciServer)
    {
        OIC_LOG(INFO, ES_RH_TAG, "Send CloudRsrc Callback To ES");

        // TODO : Need to check appropriateness of gCloudData
        if(gCloudRsrcEvtCb != NULL)
        {
            gCloudRsrcEvtCb(ES_OK, &gCloudData);
        }
        else
        {
            OIC_LOG(ERROR, ES_RH_TAG, "gCloudRsrcEvtCb is NULL");
        }
    }
}

void updateDevConfResource(OCRepPayload* input)
{
    char *country = NULL;
    if (OCRepPayloadGetPropString(input, OC_RSRVD_ES_AUTHCODE, &country))
    {
        OICStrcpy(gDevConfResource.country, sizeof(gDevConfResource.country), country);
        OICStrcpy(gDevConfData.country, sizeof(gDevConfData.country), country);
        OIC_LOG_V(INFO, ES_RH_TAG, "gDevConfResource.country %s", gDevConfResource.country);
    }

    char *language = NULL;
    if (OCRepPayloadGetPropString(input, OC_RSRVD_ES_AUTHPROVIDER, &language))
    {
        OICStrcpy(gDevConfResource.language, sizeof(gDevConfResource.language), language);
        OICStrcpy(gDevConfData.language, sizeof(gDevConfData.language), language);
        OIC_LOG_V(INFO, ES_RH_TAG, "gDevConfResource.language %s", gDevConfResource.language);
    }

    if(country || language)
    {
        OIC_LOG(INFO, ES_RH_TAG, "Send DevConfRsrc Callback To ES");

        // TODO : Need to check appropriateness of gDevConfData
        if(gDevConfRsrcEvtCb != NULL)
        {
            gDevConfRsrcEvtCb(ES_OK, &gDevConfData);
        }
        else
        {
            OIC_LOG(ERROR, ES_RH_TAG, "gDevConfRsrcEvtCb is NULL");
        }
    }
}

OCRepPayload* constructResponseOfWiFi()
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if (!payload)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "Failed to allocate Payload");
        return NULL;
    }

    OIC_LOG(INFO, ES_RH_TAG, "constructResponse wifi res");
    OCRepPayloadSetUri(payload, OC_RSRVD_ES_URI_WIFI);

    size_t dimensions[MAX_REP_ARRAY_DEPTH] = {gWiFiResource.numMode, 0, 0};
    OCRepPayloadSetIntArray(payload, OC_RSRVD_ES_SUPPORTEDWIFIMODE, (int64_t *)gWiFiResource.supportedMode, dimensions);

    OCRepPayloadSetPropInt(payload, OC_RSRVD_ES_SUPPORTEDWIFIFREQ, gWiFiResource.supportedFreq);
    OCRepPayloadSetPropString(payload, OC_RSRVD_ES_SSID, gWiFiResource.ssid);
    OCRepPayloadSetPropString(payload, OC_RSRVD_ES_CRED, gWiFiResource.cred);
    OCRepPayloadSetPropInt(payload, OC_RSRVD_ES_AUTHTYPE, (int) gWiFiResource.authType);
    OCRepPayloadSetPropInt(payload, OC_RSRVD_ES_ENCTYPE, (int) gWiFiResource.encType);

    printf("%s\n", gWiFiResource.ssid);

    return payload;
}

OCRepPayload* constructResponseOfCloud()
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if (!payload)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "Failed to allocate Payload");
        return NULL;
    }

    OIC_LOG(INFO, ES_RH_TAG, "constructResponse prov res");
    OCRepPayloadSetUri(payload, OC_RSRVD_ES_URI_CLOUDSERVER);
    OCRepPayloadSetPropString(payload, OC_RSRVD_ES_AUTHCODE, gCloudResource.authCode);
    OCRepPayloadSetPropString(payload, OC_RSRVD_ES_AUTHPROVIDER, gCloudResource.authProvider);
    OCRepPayloadSetPropString(payload, OC_RSRVD_ES_CISERVER, gCloudResource.ciServer);

    return payload;
}

OCRepPayload* constructResponseOfDevConf()
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if (!payload)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "Failed to allocate Payload");
        return NULL;
    }

    OIC_LOG(INFO, ES_RH_TAG, "constructResponse prov res");
    OCRepPayloadSetUri(payload, OC_RSRVD_ES_URI_DEVCONF);
    OCRepPayloadSetPropString(payload, OC_RSRVD_ES_DEVNAME, gDevConfResource.devName);
    OCRepPayloadSetPropString(payload, OC_RSRVD_ES_LANGUAGE, gDevConfResource.language);
    OCRepPayloadSetPropString(payload, OC_RSRVD_ES_COUNTRY, gDevConfResource.country);

    return payload;
}

OCRepPayload* constructResponseOfProv(OCEntityHandlerRequest *ehRequest)
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if (!payload)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "Failed to allocate Payload");
        return NULL;
    }

    OIC_LOG(INFO, ES_RH_TAG, "constructResponse prov res");
    OCRepPayloadSetUri(payload, OC_RSRVD_ES_URI_PROV);
    OCRepPayloadSetPropInt(payload, OC_RSRVD_ES_PROVSTATUS, gProvResource.status);
    OCRepPayloadSetPropBool(payload, OC_RSRVD_ES_TRIGGER, gProvResource.trigger);

    if(ehRequest->query)
    {
        if(strstr(ehRequest->query, OC_RSRVD_INTERFACE_BATCH))
        {// When Provisioning resource has a GET with BatchInterface
            payload->next = constructResponseOfWiFi();

            if(payload->next)
                payload->next->next = constructResponseOfCloud();
            else
                return payload;

            if(payload->next->next)
                payload->next->next->next = constructResponseOfDevConf();
            else
                return payload;
        }
    }

    return payload;
}


OCStackResult CreateEasySetupResources(bool isSecured, ESResourceMask resourceMask)
{
    OCStackResult res = OC_STACK_ERROR;
    bool maskFlag = false;

    res = initProvResource(isSecured);
    if(res != OC_STACK_OK)
    {
        // TODO: destroy logic will be added
        OIC_LOG_V(ERROR, ES_RH_TAG, "initProvResource result: %s", getResult(res));

        return res;
    }

    if((resourceMask & ES_WIFI_RESOURCE) == ES_WIFI_RESOURCE)
    {
        maskFlag = true;
        res = initWiFiResource(isSecured);
        if(res != OC_STACK_OK)
        {
            // TODO: destroy logic will be added
            OIC_LOG_V(ERROR, ES_RH_TAG, "initWiFiResource result: %s", getResult(res));

            return res;
        }

        res = OCBindResource(gProvResource.handle, gWiFiResource.handle);
        if(res != OC_STACK_OK)
        {
            // TODO : Error Handling
            OIC_LOG_V(ERROR, ES_RH_TAG, "Bind WiFiResource result: %s", getResult(res));

            return res;
        }

    }

    if((resourceMask & ES_CLOUD_RESOURCE) == ES_CLOUD_RESOURCE)
    {
        maskFlag = true;
        res = initCloudServerResource(isSecured);
        if(res != OC_STACK_OK)
        {
            // TODO: destroy logic will be added
            OIC_LOG_V(ERROR, ES_RH_TAG, "initCloudResource result: %s", getResult(res));

            return res;
        }

        res = OCBindResource(gProvResource.handle, gCloudResource.handle);
        if(res != OC_STACK_OK)
        {
            // TODO : Error Handling
            OIC_LOG_V(ERROR, ES_RH_TAG, "Bind CloudResource result: %s", getResult(res));

            return res;
        }
    }

    if((resourceMask & ES_DEVCONF_RESOURCE) == ES_DEVCONF_RESOURCE)
    {
        maskFlag = true;
        res = initDevConfResource(isSecured);
        if(res != OC_STACK_OK)
        {
            // TODO: destroy logic will be added
            OIC_LOG_V(ERROR, ES_RH_TAG, "initDevConf result: %s", getResult(res));

            return res;
        }

        res = OCBindResource(gProvResource.handle, gDevConfResource.handle);
        if(res != OC_STACK_OK)
        {
            // TODO : Error Handling
            OIC_LOG_V(ERROR, ES_RH_TAG, "Bind DevConfResource result: %s", getResult(res));

            return res;
        }
    }


    if(maskFlag == false)
    {
        // TODO: destroy logic will be added
        OIC_LOG_V(ERROR, ES_RH_TAG, "Invalid ResourceMask");
        return OC_STACK_ERROR;

    }
    OIC_LOG_V(INFO, ES_RH_TAG, "Created all resources with result: %s", getResult(res));
    return res;
}

OCStackResult DeleteProvisioningResource()
{
    OCStackResult res = OCDeleteResource(gProvResource.handle);
    if (res != OC_STACK_OK)
    {
        OIC_LOG_V(INFO, ES_RH_TAG, "Deleting Prov resource error with result: %s", getResult(res));
    }

    return res;
}

OCStackResult DeleteEasySetupResources()
{
    OCStackResult res = OCDeleteResource(gProvResource.handle);
    if (res != OC_STACK_OK)
    {
        OIC_LOG_V(INFO, ES_RH_TAG, "Deleting Prov resource error with result: %s", getResult(res));
    }
    res = OCDeleteResource(gWiFiResource.handle);
    if (res != OC_STACK_OK)
    {
        OIC_LOG_V(INFO, ES_RH_TAG, "Deleting WiFi resource error with result: %s", getResult(res));
    }
    res = OCDeleteResource(gCloudResource.handle);
    if (res != OC_STACK_OK)
    {
        OIC_LOG_V(INFO, ES_RH_TAG, "Deleting CloudServer resource error with result: %s", getResult(res));
    }
    res = OCDeleteResource(gDevConfResource.handle);
    if (res != OC_STACK_OK)
    {
        OIC_LOG_V(INFO, ES_RH_TAG, "Deleting DevConf resource error with result: %s", getResult(res));
    }

    return res;
}

OCEntityHandlerResult ProcessGetRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload **payload)
{
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    if (!ehRequest)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "Request is Null");
        return ehResult;
    }
    if (ehRequest->payload && ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "Incoming payload not a representation");
        return ehResult;
    }

    OCRepPayload *getResp = NULL;

    if(ehRequest->resource == gProvResource.handle)
        getResp = constructResponseOfProv(ehRequest);
    else if(ehRequest->resource == gWiFiResource.handle)
        getResp = constructResponseOfWiFi();
    else if(ehRequest->resource == gCloudResource.handle)
        getResp = constructResponseOfCloud();
    else if(ehRequest->resource == gDevConfResource.handle)
        getResp = constructResponseOfDevConf();

    if (!getResp)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "constructResponse failed");
        return OC_EH_ERROR;
    }

    *payload = getResp;
    ehResult = OC_EH_OK;

    return ehResult;
}

OCEntityHandlerResult ProcessPostRequest(OCEntityHandlerRequest *ehRequest, OCRepPayload** payload)
{
    OIC_LOG(INFO, ES_RH_TAG, "ProcessPostRequest enter");
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    if (ehRequest->payload && ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "Incoming payload not a representation");
        return ehResult;
    }

    OCRepPayload* input = (OCRepPayload*) (ehRequest->payload);
    if (!input)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "Failed to parse");
        return ehResult;
    }

    if(ehRequest->resource == gProvResource.handle)
        updateProvResource(ehRequest, input);
    else if(ehRequest->resource == gWiFiResource.handle)
        updateWiFiResource(input);
    else if(ehRequest->resource == gCloudResource.handle)
        updateCloudResource(input);
    else if(ehRequest->resource == gDevConfResource.handle)
        updateDevConfResource(input);

    // TBD : Discuss about triggering flag (to be existed or not)
    // ES_PS_PROVISIONING_COMPLETED state indicates that already provisioning is completed.
    // A new request for provisioning means overriding existing network provisioning information.
    // if (gProvResource.trigger)
    // {
    //     OIC_LOG(DEBUG, ES_RH_TAG, "Provisioning already completed."
    //             "Tiggering the network connection");

    //     if (gNetworkInfoProvEventCb)
    //     {
    //         gNetworkInfoProvEventCb(ES_RECVTRIGGEROFPROVRES);
    //         ehResult = OC_EH_OK;
    //     }
    //     else
    //     {
    //         OIC_LOG(ERROR, ES_RH_TAG, "gNetworkInfoProvEventCb is NULL."
    //                 "Network handler not registered. Failed to connect to the network");
    //         ehResult = OC_EH_ERROR;
    //         return ehResult;
    //     }
    // }
    // else
    // {
    //     OIC_LOG(DEBUG, ES_RH_TAG, "Provisioning the network information to the Enrollee.");
    // }

    OCRepPayload *getResp = NULL;
    if(ehRequest->resource == gProvResource.handle)
        getResp = constructResponseOfProv(ehRequest);
    else if(ehRequest->resource == gWiFiResource.handle)
        getResp = constructResponseOfWiFi();
    else if(ehRequest->resource == gCloudResource.handle)
        getResp = constructResponseOfCloud();
    else if(ehRequest->resource == gDevConfResource.handle)
        getResp = constructResponseOfDevConf();

    if (gProvResource.trigger)// Trigger false should be restored after executed
        gProvResource.trigger = false;

    if (!getResp)
    {
        OIC_LOG(ERROR, ES_RH_TAG, "constructResponse failed");
        return OC_EH_ERROR;
    }

    *payload = getResp;
    ehResult = OC_EH_OK;

    return ehResult;
}

OCEntityHandlerResult ProcessPutRequest(OCEntityHandlerRequest * ehRequest,
        OCRepPayload** payload)
{
    (void) ehRequest;
    (void) payload;
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    return ehResult;
}
/**
 * This is the entity handler for the registered resource.
 * This is invoked by OCStack whenever it recevies a request for this resource.
 */
OCEntityHandlerResult OCEntityHandlerCb(OCEntityHandlerFlag flag,
        OCEntityHandlerRequest* entityHandlerRequest, void *callback)
{
    (void) callback;
    OCEntityHandlerResult ehRet = OC_EH_OK;
    OCEntityHandlerResponse response =
    { 0, 0, OC_EH_ERROR, 0, 0,
    { },
    { 0 }, false };
    OCRepPayload* payload = NULL;

    if (entityHandlerRequest && (flag & OC_REQUEST_FLAG))
    {
        if (OC_REST_GET == entityHandlerRequest->method)
        {
            OIC_LOG(INFO, ES_RH_TAG, "Received GET request");
            ehRet = ProcessGetRequest(entityHandlerRequest, &payload);
        }
        else if (OC_REST_PUT == entityHandlerRequest->method)
        {
            OIC_LOG(INFO, ES_RH_TAG, "Received PUT request");

            //PUT request will be handled in the internal implementation
            if (gProvResource.handle != NULL)
            {
                ehRet = ProcessPutRequest(entityHandlerRequest, &payload);
            }
            else
            {
                OIC_LOG(ERROR, ES_RH_TAG, "Cannot process put");
                ehRet = OC_EH_ERROR;
            }
        }
        else if (OC_REST_POST == entityHandlerRequest->method)
        {
            OIC_LOG(INFO, ES_RH_TAG, "Received OC_REST_POST from client");
            if (gProvResource.handle != NULL)
            {
                ehRet = ProcessPostRequest(entityHandlerRequest, &payload);
            }
            else
            {
                OIC_LOG(ERROR, ES_RH_TAG, "Cannot process put");
                ehRet = OC_EH_ERROR;
            }
        }

        if (ehRet == OC_EH_OK)
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = entityHandlerRequest->requestHandle;
            response.resourceHandle = entityHandlerRequest->resource;
            response.ehResult = ehRet;
            //response uses OCPaylod while all get,put methodes use OCRepPayload
            response.payload = (OCPayload*) (payload);
            response.numSendVendorSpecificHeaderOptions = 0;
            memset(response.sendVendorSpecificHeaderOptions, 0,
                    sizeof(response.sendVendorSpecificHeaderOptions));
            memset(response.resourceUri, 0, sizeof(response.resourceUri));
            // Indicate that response is NOT in a persistent buffer
            response.persistentBufferFlag = 0;

            // Send the response
            if (OCDoResponse(&response) != OC_STACK_OK)
            {
                OIC_LOG(ERROR, ES_RH_TAG, "Error sending response");
                ehRet = OC_EH_ERROR;
            }
        }
    }

    return ehRet;
}

const char *getResult(OCStackResult result)
{
    switch (result)
    {
        case OC_STACK_OK:
            return "OC_STACK_OK";
        case OC_STACK_INVALID_URI:
            return "OC_STACK_INVALID_URI";
        case OC_STACK_INVALID_QUERY:
            return "OC_STACK_INVALID_QUERY";
        case OC_STACK_INVALID_IP:
            return "OC_STACK_INVALID_IP";
        case OC_STACK_INVALID_PORT:
            return "OC_STACK_INVALID_PORT";
        case OC_STACK_INVALID_CALLBACK:
            return "OC_STACK_INVALID_CALLBACK";
        case OC_STACK_INVALID_METHOD:
            return "OC_STACK_INVALID_METHOD";
        case OC_STACK_NO_MEMORY:
            return "OC_STACK_NO_MEMORY";
        case OC_STACK_COMM_ERROR:
            return "OC_STACK_COMM_ERROR";
        case OC_STACK_INVALID_PARAM:
            return "OC_STACK_INVALID_PARAM";
        case OC_STACK_NOTIMPL:
            return "OC_STACK_NOTIMPL";
        case OC_STACK_NO_RESOURCE:
            return "OC_STACK_NO_RESOURCE";
        case OC_STACK_RESOURCE_ERROR:
            return "OC_STACK_RESOURCE_ERROR";
        case OC_STACK_SLOW_RESOURCE:
            return "OC_STACK_SLOW_RESOURCE";
        case OC_STACK_NO_OBSERVERS:
            return "OC_STACK_NO_OBSERVERS";
        case OC_STACK_ERROR:
            return "OC_STACK_ERROR";
        default:
            return "UNKNOWN";
    }
}

