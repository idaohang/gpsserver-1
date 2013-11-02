/* soapStub.h
   Generated by gSOAP 2.7.10 from imRoot.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef soapStub_H
#define soapStub_H
#include "stdsoap2.h"
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
 *                                                                            *
 * Enumerations                                                               *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Classes and Structs                                                        *
 *                                                                            *
\******************************************************************************/


#if 0 /* volatile type: do not redeclare here */

#endif

#ifndef SOAP_TYPE__ns1__OperationSms
#define SOAP_TYPE__ns1__OperationSms (7)
/* ns1:OperationSms */
struct _ns1__OperationSms
{
	char *in0;	/* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE__ns1__OperationSmsResponse
#define SOAP_TYPE__ns1__OperationSmsResponse (8)
/* ns1:OperationSmsResponse */
struct _ns1__OperationSmsResponse
{
	char *out;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE__ns1__OperationCenter
#define SOAP_TYPE__ns1__OperationCenter (9)
/* ns1:OperationCenter */
struct _ns1__OperationCenter
{
	char *in0;	/* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE__ns1__OperationCenterResponse
#define SOAP_TYPE__ns1__OperationCenterResponse (10)
/* ns1:OperationCenterResponse */
struct _ns1__OperationCenterResponse
{
	char *out;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE__ns1__sayHello
#define SOAP_TYPE__ns1__sayHello (11)
/* ns1:sayHello */
struct _ns1__sayHello
{
	char *in0;	/* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE__ns1__sayHelloResponse
#define SOAP_TYPE__ns1__sayHelloResponse (12)
/* ns1:sayHelloResponse */
struct _ns1__sayHelloResponse
{
	char *out;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__TakePhotoReq
#define SOAP_TYPE_ns2__TakePhotoReq (13)
/* ns2:TakePhotoReq */
struct ns2__TakePhotoReq
{
	int action;	/* required element of type xsd:int */
	int camID;	/* required element of type xsd:int */
	int photoSize;	/* required element of type xsd:int */
	char *simPhone;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__TakePhotoResp
#define SOAP_TYPE_ns2__TakePhotoResp (14)
/* ns2:TakePhotoResp */
struct ns2__TakePhotoResp
{
	char *photoName;	/* optional element of type xsd:string */
	int result;	/* required element of type xsd:int */
};
#endif

#ifndef SOAP_TYPE_ns2__QueryVerReq
#define SOAP_TYPE_ns2__QueryVerReq (15)
/* ns2:QueryVerReq */
struct ns2__QueryVerReq
{
	char *simPhone;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__QueryVerResp
#define SOAP_TYPE_ns2__QueryVerResp (16)
/* ns2:QueryVerResp */
struct ns2__QueryVerResp
{
	int result;	/* required element of type xsd:int */
	char *version;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__SendMsgReq
#define SOAP_TYPE_ns2__SendMsgReq (17)
/* ns2:SendMsgReq */
struct ns2__SendMsgReq
{
	char *message;	/* optional element of type xsd:string */
	char *simPhone;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__SendMsgResp
#define SOAP_TYPE_ns2__SendMsgResp (18)
/* ns2:SendMsgResp */
struct ns2__SendMsgResp
{
	int result;	/* required element of type xsd:int */
};
#endif

#ifndef SOAP_TYPE_ns2__SetGPSFrequencyReq
#define SOAP_TYPE_ns2__SetGPSFrequencyReq (19)
/* ns2:SetGPSFrequencyReq */
struct ns2__SetGPSFrequencyReq
{
	int interval;	/* required element of type xsd:int */
	char *simPhone;	/* optional element of type xsd:string */
	int times;	/* required element of type xsd:int */
};
#endif

#ifndef SOAP_TYPE_ns2__SetGPSFrequencyResp
#define SOAP_TYPE_ns2__SetGPSFrequencyResp (20)
/* ns2:SetGPSFrequencyResp */
struct ns2__SetGPSFrequencyResp
{
	int result;	/* required element of type xsd:int */
};
#endif

#ifndef SOAP_TYPE_ns2__SetMileageReq
#define SOAP_TYPE_ns2__SetMileageReq (21)
/* ns2:SetMileageReq */
struct ns2__SetMileageReq
{
	int mileageFloat;	/* required element of type xsd:int */
	int mileageInt;	/* required element of type xsd:int */
	char *simPhone;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__SetMileageResp
#define SOAP_TYPE_ns2__SetMileageResp (22)
/* ns2:SetMileageResp */
struct ns2__SetMileageResp
{
	int result;	/* required element of type xsd:int */
};
#endif

#ifndef SOAP_TYPE_ns2__UpdateSysConfReq
#define SOAP_TYPE_ns2__UpdateSysConfReq (23)
/* ns2:UpdateSysConfReq */
struct ns2__UpdateSysConfReq
{
	char *command;	/* optional element of type xsd:string */
	char *simPhone;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__UpdateSysConfResp
#define SOAP_TYPE_ns2__UpdateSysConfResp (24)
/* ns2:UpdateSysConfResp */
struct ns2__UpdateSysConfResp
{
	int result;	/* required element of type xsd:int */
};
#endif

#ifndef SOAP_TYPE_ns2__UpdateServerInfoReq
#define SOAP_TYPE_ns2__UpdateServerInfoReq (25)
/* ns2:UpdateServerInfoReq */
struct ns2__UpdateServerInfoReq
{
	char *logServerIp;	/* optional element of type xsd:string */
	int logServerPort;	/* required element of type xsd:int */
	char *password;	/* optional element of type xsd:string */
	char *simPhone;	/* optional element of type xsd:string */
	char *updateServerIp;	/* optional element of type xsd:string */
	int updateServerPort;	/* required element of type xsd:int */
	char *userName;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__UpdateServerInfoResp
#define SOAP_TYPE_ns2__UpdateServerInfoResp (26)
/* ns2:UpdateServerInfoResp */
struct ns2__UpdateServerInfoResp
{
	int result;	/* required element of type xsd:int */
};
#endif

#ifndef SOAP_TYPE___ns1__OperationSms
#define SOAP_TYPE___ns1__OperationSms (30)
/* Operation wrapper: */
struct __ns1__OperationSms
{
	struct _ns1__OperationSms *ns1__OperationSms;	/* optional element of type ns1:OperationSms */
};
#endif

#ifndef SOAP_TYPE___ns1__OperationCenter
#define SOAP_TYPE___ns1__OperationCenter (34)
/* Operation wrapper: */
struct __ns1__OperationCenter
{
	struct _ns1__OperationCenter *ns1__OperationCenter;	/* optional element of type ns1:OperationCenter */
};
#endif

#ifndef SOAP_TYPE___ns1__sayHello
#define SOAP_TYPE___ns1__sayHello (38)
/* Operation wrapper: */
struct __ns1__sayHello
{
	struct _ns1__sayHello *ns1__sayHello;	/* optional element of type ns1:sayHello */
};
#endif

#ifndef SOAP_TYPE_ns3__takePhotoResponse
#define SOAP_TYPE_ns3__takePhotoResponse (39)
/* ns3:takePhotoResponse */
struct ns3__takePhotoResponse
{
	struct ns2__TakePhotoResp *_takePhotoReturn;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type ns2:TakePhotoResp */
};
#endif

#ifndef SOAP_TYPE_ns3__takePhoto
#define SOAP_TYPE_ns3__takePhoto (44)
/* ns3:takePhoto */
struct ns3__takePhoto
{
	struct ns2__TakePhotoReq *_in0;	/* optional element of type ns2:TakePhotoReq */
};
#endif

#ifndef SOAP_TYPE_ns3__queryVersionResponse
#define SOAP_TYPE_ns3__queryVersionResponse (45)
/* ns3:queryVersionResponse */
struct ns3__queryVersionResponse
{
	struct ns2__QueryVerResp *_queryVersionReturn;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type ns2:QueryVerResp */
};
#endif

#ifndef SOAP_TYPE_ns3__queryVersion
#define SOAP_TYPE_ns3__queryVersion (50)
/* ns3:queryVersion */
struct ns3__queryVersion
{
	struct ns2__QueryVerReq *_in0;	/* optional element of type ns2:QueryVerReq */
};
#endif

#ifndef SOAP_TYPE_ns3__sendMessageResponse
#define SOAP_TYPE_ns3__sendMessageResponse (51)
/* ns3:sendMessageResponse */
struct ns3__sendMessageResponse
{
	struct ns2__SendMsgResp *_sendMessageReturn;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type ns2:SendMsgResp */
};
#endif

#ifndef SOAP_TYPE_ns3__sendMessage
#define SOAP_TYPE_ns3__sendMessage (56)
/* ns3:sendMessage */
struct ns3__sendMessage
{
	struct ns2__SendMsgReq *_in0;	/* optional element of type ns2:SendMsgReq */
};
#endif

#ifndef SOAP_TYPE_ns3__setGPSFrequencyResponse
#define SOAP_TYPE_ns3__setGPSFrequencyResponse (57)
/* ns3:setGPSFrequencyResponse */
struct ns3__setGPSFrequencyResponse
{
	struct ns2__SetGPSFrequencyResp *_setGPSFrequencyReturn;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type ns2:SetGPSFrequencyResp */
};
#endif

#ifndef SOAP_TYPE_ns3__setGPSFrequency
#define SOAP_TYPE_ns3__setGPSFrequency (62)
/* ns3:setGPSFrequency */
struct ns3__setGPSFrequency
{
	struct ns2__SetGPSFrequencyReq *_in0;	/* optional element of type ns2:SetGPSFrequencyReq */
};
#endif

#ifndef SOAP_TYPE_ns3__setMileageResponse
#define SOAP_TYPE_ns3__setMileageResponse (63)
/* ns3:setMileageResponse */
struct ns3__setMileageResponse
{
	struct ns2__SetMileageResp *_setMileageReturn;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type ns2:SetMileageResp */
};
#endif

#ifndef SOAP_TYPE_ns3__setMileage
#define SOAP_TYPE_ns3__setMileage (68)
/* ns3:setMileage */
struct ns3__setMileage
{
	struct ns2__SetMileageReq *_in0;	/* optional element of type ns2:SetMileageReq */
};
#endif

#ifndef SOAP_TYPE_ns3__updateSysConfResponse
#define SOAP_TYPE_ns3__updateSysConfResponse (69)
/* ns3:updateSysConfResponse */
struct ns3__updateSysConfResponse
{
	struct ns2__UpdateSysConfResp *_updateSysConfReturn;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type ns2:UpdateSysConfResp */
};
#endif

#ifndef SOAP_TYPE_ns3__updateSysConf
#define SOAP_TYPE_ns3__updateSysConf (74)
/* ns3:updateSysConf */
struct ns3__updateSysConf
{
	struct ns2__UpdateSysConfReq *_in0;	/* optional element of type ns2:UpdateSysConfReq */
};
#endif

#ifndef SOAP_TYPE_ns3__updateServerInfoResponse
#define SOAP_TYPE_ns3__updateServerInfoResponse (75)
/* ns3:updateServerInfoResponse */
struct ns3__updateServerInfoResponse
{
	struct ns2__UpdateServerInfoResp *_updateServerInfoReturn;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type ns2:UpdateServerInfoResp */
};
#endif

#ifndef SOAP_TYPE_ns3__updateServerInfo
#define SOAP_TYPE_ns3__updateServerInfo (80)
/* ns3:updateServerInfo */
struct ns3__updateServerInfo
{
	struct ns2__UpdateServerInfoReq *_in0;	/* optional element of type ns2:UpdateServerInfoReq */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Header
#define SOAP_TYPE_SOAP_ENV__Header (81)
/* SOAP Header: */
struct SOAP_ENV__Header
{
#ifdef WITH_NOEMPTYSTRUCT
	char dummy;	/* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Code
#define SOAP_TYPE_SOAP_ENV__Code (82)
/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
	char *SOAP_ENV__Value;	/* optional element of type xsd:QName */
	struct SOAP_ENV__Code *SOAP_ENV__Subcode;	/* optional element of type SOAP-ENV:Code */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Detail
#define SOAP_TYPE_SOAP_ENV__Detail (84)
/* SOAP-ENV:Detail */
struct SOAP_ENV__Detail
{
	int __type;	/* any type of element <fault> (defined below) */
	void *fault;	/* transient */
	char *__any;
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Reason
#define SOAP_TYPE_SOAP_ENV__Reason (87)
/* SOAP-ENV:Reason */
struct SOAP_ENV__Reason
{
	char *SOAP_ENV__Text;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Fault
#define SOAP_TYPE_SOAP_ENV__Fault (88)
/* SOAP Fault: */
struct SOAP_ENV__Fault
{
	char *faultcode;	/* optional element of type xsd:QName */
	char *faultstring;	/* optional element of type xsd:string */
	char *faultactor;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *detail;	/* optional element of type SOAP-ENV:Detail */
	struct SOAP_ENV__Code *SOAP_ENV__Code;	/* optional element of type SOAP-ENV:Code */
	struct SOAP_ENV__Reason *SOAP_ENV__Reason;	/* optional element of type SOAP-ENV:Reason */
	char *SOAP_ENV__Node;	/* optional element of type xsd:string */
	char *SOAP_ENV__Role;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *SOAP_ENV__Detail;	/* optional element of type SOAP-ENV:Detail */
};
#endif

/******************************************************************************\
 *                                                                            *
 * Types with Custom Serializers                                              *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Typedefs                                                                   *
 *                                                                            *
\******************************************************************************/

#ifndef SOAP_TYPE__QName
#define SOAP_TYPE__QName (5)
typedef char *_QName;
#endif

#ifndef SOAP_TYPE__XML
#define SOAP_TYPE__XML (6)
typedef char *_XML;
#endif


/******************************************************************************\
 *                                                                            *
 * Typedef Synonyms                                                           *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Externals                                                                  *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Service Operations                                                         *
 *                                                                            *
\******************************************************************************/


SOAP_FMAC5 int SOAP_FMAC6 __ns1__OperationSms(struct soap*, struct _ns1__OperationSms *ns1__OperationSms, struct _ns1__OperationSmsResponse *ns1__OperationSmsResponse);

SOAP_FMAC5 int SOAP_FMAC6 __ns1__OperationCenter(struct soap*, struct _ns1__OperationCenter *ns1__OperationCenter, struct _ns1__OperationCenterResponse *ns1__OperationCenterResponse);

SOAP_FMAC5 int SOAP_FMAC6 __ns1__sayHello(struct soap*, struct _ns1__sayHello *ns1__sayHello, struct _ns1__sayHelloResponse *ns1__sayHelloResponse);

SOAP_FMAC5 int SOAP_FMAC6 ns3__takePhoto(struct soap*, struct ns2__TakePhotoReq *_in0, struct ns3__takePhotoResponse *_param_1);

SOAP_FMAC5 int SOAP_FMAC6 ns3__queryVersion(struct soap*, struct ns2__QueryVerReq *_in0, struct ns3__queryVersionResponse *_param_2);

SOAP_FMAC5 int SOAP_FMAC6 ns3__sendMessage(struct soap*, struct ns2__SendMsgReq *_in0, struct ns3__sendMessageResponse *_param_3);

SOAP_FMAC5 int SOAP_FMAC6 ns3__setGPSFrequency(struct soap*, struct ns2__SetGPSFrequencyReq *_in0, struct ns3__setGPSFrequencyResponse *_param_4);

SOAP_FMAC5 int SOAP_FMAC6 ns3__setMileage(struct soap*, struct ns2__SetMileageReq *_in0, struct ns3__setMileageResponse *_param_5);

SOAP_FMAC5 int SOAP_FMAC6 ns3__updateSysConf(struct soap*, struct ns2__UpdateSysConfReq *_in0, struct ns3__updateSysConfResponse *_param_6);

SOAP_FMAC5 int SOAP_FMAC6 ns3__updateServerInfo(struct soap*, struct ns2__UpdateServerInfoReq *_in0, struct ns3__updateServerInfoResponse *_param_7);

/******************************************************************************\
 *                                                                            *
 * Stubs                                                                      *
 *                                                                            *
\******************************************************************************/


SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__OperationSms(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct _ns1__OperationSms *ns1__OperationSms, struct _ns1__OperationSmsResponse *ns1__OperationSmsResponse);

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__OperationCenter(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct _ns1__OperationCenter *ns1__OperationCenter, struct _ns1__OperationCenterResponse *ns1__OperationCenterResponse);

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__sayHello(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct _ns1__sayHello *ns1__sayHello, struct _ns1__sayHelloResponse *ns1__sayHelloResponse);

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns3__takePhoto(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ns2__TakePhotoReq *_in0, struct ns3__takePhotoResponse *_param_1);

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns3__queryVersion(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ns2__QueryVerReq *_in0, struct ns3__queryVersionResponse *_param_2);

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns3__sendMessage(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ns2__SendMsgReq *_in0, struct ns3__sendMessageResponse *_param_3);

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns3__setGPSFrequency(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ns2__SetGPSFrequencyReq *_in0, struct ns3__setGPSFrequencyResponse *_param_4);

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns3__setMileage(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ns2__SetMileageReq *_in0, struct ns3__setMileageResponse *_param_5);

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns3__updateSysConf(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ns2__UpdateSysConfReq *_in0, struct ns3__updateSysConfResponse *_param_6);

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns3__updateServerInfo(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ns2__UpdateServerInfoReq *_in0, struct ns3__updateServerInfoResponse *_param_7);

/******************************************************************************\
 *                                                                            *
 * Skeletons                                                                  *
 *                                                                            *
\******************************************************************************/

SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___ns1__OperationSms(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___ns1__OperationCenter(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___ns1__sayHello(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns3__takePhoto(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns3__queryVersion(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns3__sendMessage(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns3__setGPSFrequency(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns3__setMileage(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns3__updateSysConf(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns3__updateServerInfo(struct soap*);

#ifdef __cplusplus
}
#endif

#endif

/* End of soapStub.h */
