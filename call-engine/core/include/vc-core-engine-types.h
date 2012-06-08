/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __VC_CORE_ENGINE_TYPES_H_
#define __VC_CORE_ENGINE_TYPES_H_

#include <stdbool.h>
#include <glib.h>

/**< This is the prototype of the client callback function  */
typedef gboolean(*voicecall_cb) (int event, int param1, int param2, void *param3, void *puser_data);

/**
 * This opaque structure is the interface for the client application to communicate to the engine
 */
typedef struct _voicecall_engine_t voicecall_engine_t;

/**< General Macro Definitions  */
#define TAG_CALL						"CALL"
#define	TAG_CALL_LAUNCH					"VCKPI"
#define VC_DISPLAY_NAME_LENGTH_MAX		(255+1)			/**< Voiecall Display Name Max Length  */
#define VC_IMAGE_PATH_LENGTH_MAX		(255+1)			/**< Max File length for image */
#define VC_RINGTONE_PATH_LENGTH_MAX		(255+1)			/**< Max File length for Ringtone */
#define	VC_DATA_LENGTH_MAX				(255+1)			/**< Max data length for misc */
#define VC_PHONE_NUMBER_LENGTH_MAX		(82+1)			/**< Maximum length of a phone number  */
#define VC_PHONE_NAME_LENGTH_MAX		(80+1)	/**< Maximum length of a calling name  */
#define VC_PHONE_SUBADDRESS_LENGTH_MAX	(40+1)	/**< Maximum length of a SUB address  */
#define VC_MAX_CALL_GROUP_MEMBER		5			/**< Maximum number of members in a group */
#define	VC_MAX_CALL_MEMBER				8			/**< This is the maximum number of members possible */
#define VC_INVALID_CALL_ID				0			/**< This is the invalid entry for call ID */
#define VC_MAX_CALL_ID					7			/**< Maximum Allowed Call IDS (1 -7) (5 Conf Calls, 1 Other Connected Call and 1 Outgoing/Incoming Call)*/

#define VC_AOC_CURRENCY_CODE_LEN_MAX	3			/**< Max Length of the AOC Currency Code */

#define	VC_INVALID_CALL_INDEX			-1
#define VC_INVALID_CALL_COST			-1
#define	VC_INVALID_PPM					-1

#define VC_TAPI_INVALID_CALLHANDLE		-1
#define VC_RQSTID_DEFAULT				-1		/**< To collect request ID of Async TAPI request*/

#define VC_ED_HANDLE					0
#define	VC_SAT_ICON_SIZE				(64*64)

#define VC_EMERGENCY_NUM_MAX			8

#define VC_RECORD_FULL_FILE_PATH_LENGTH (4095+1)
#ifndef VC_NO_ERROR
#define VC_NO_ERROR 0
#endif

#ifndef VC_ERROR
#define VC_ERROR -1
#endif

#ifndef VC_INVALID_VOLUME
#define VC_INVALID_VOLUME -1
#endif
/**< General Macro Definitions  */

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

/**
 * This enumeration defines the call states
 */
typedef enum {
	VC_CALL_ACCEPT_1,
	VC_CALL_ACCEPT_2
} voicecall_accept_choice_t;

/**
 * This enumeration defines the call states
 */
typedef enum {
	VC_CALL_STATE_NONE,					/**< No call state */
	VC_CALL_STATE_INCOME,				/**< Incoming state  */
	VC_CALL_STATE_REJECTED,				/**< Rejected state */
	VC_CALL_STATE_PREPARE_OUTGOING,	/**< Prepare for outgoing */
	VC_CALL_STATE_OUTGOING,				/**< Outgoing state */
	VC_CALL_STATE_OUTGOING_ORIG,		/**< Outgoing originatged state */
	VC_CALL_STATE_OUTGOING_ALERT,		/**< Outgoing alert state */
	VC_CALL_STATE_CANCELLED,			/**< Cancelled state */
	VC_CALL_STATE_CONNECTED,			/**< Connected state */
	VC_CALL_STATE_RELEASE_WAIT,		/**< Release wait state */
	VC_CALL_STATE_ENDED,				/**< Call ended state */
	VC_CALL_STATE_REDIAL,				/**< Redial state */
	VC_CALL_STATE_ENDED_FINISH,		/**< Call ended finish state */
	VC_CALL_STATE_MAX_NUM				/**< Max value */
} voicecall_call_state_t;

/**
 * This enumeration defines the call setup parties
 */
typedef enum {
	VC_CALL_SETUP_BY_NORMAL,		/**< Normal Call setup */
	VC_CALL_SETUP_BY_SAT,			/**< Call setup by SAT */
	VC_CALL_SETUP_BY_MAILBOX		/**< Call setup by Mail box */
} voicecall_call_type_bysetup_t;

/**
 * This enumeration defines the call end cause types
 */
typedef enum {
	VC_ENDCAUSE_CALL_ENDED,						/**< Call ended */

	VC_ENDCAUSE_CALL_DISCONNECTED,				/**< Call disconnected */
	VC_ENDCAUSE_CALL_SERVICE_NOT_ALLOWED,		/**< Service not allowed */
	VC_ENDCAUSE_CALL_BARRED,						/**< Call barred */
	VC_ENDCAUSE_NO_SERVICE,						/**< No Service */
	VC_ENDCAUSE_NW_BUSY,							/**< Network busy */

	VC_ENDCAUSE_NW_FAILED,						/**< Network failed */
	VC_ENDCAUSE_NO_ANSWER,						/**< No anwer from other party */
	VC_ENDCAUSE_NO_CREDIT,						/**< No credit available */
	VC_ENDCAUSE_REJECTED,							/**< Call rejected */
	VC_ENDCAUSE_USER_BUSY,						/**< user busy */

	VC_ENDCAUSE_WRONG_GROUP,					/**< Wrong group */
	VC_ENDCAUSE_CALL_NOT_ALLOWED,				/**< Call not allowed */
	VC_ENDCAUSE_TAPI_ERROR,						/**< Tapi error */
	VC_ENDCAUSE_CALL_FAILED,						/**< Call Failed */
	VC_ENDCAUSE_NO_USER_RESPONDING,				/**< User not responding */

	VC_ENDCAUSE_USER_ALERTING_NO_ANSWER,		/**< User Alerting No Answer */
	VC_ENDCAUSE_SERVICE_TEMP_UNAVAILABLE,		/**< Circuit Channel Unavailable,Network is out of Order,Switching equipment congestion,Temporary Failure */
	VC_ENDCAUSE_USER_UNAVAILABLE,				/**< Called Party Rejects the Call */
	VC_ENDCAUSE_INVALID_NUMBER_FORMAT,			/**< Entered number is invalid or incomplete */
	VC_ENDCAUSE_NUMBER_CHANGED,				/**< Entered number has been changed */

	VC_ENDCAUSE_UNASSIGNED_NUMBER,				/**< Unassigned/Unallocated number*/
	VC_ENDCAUSE_USER_DOESNOT_RESPOND,			/**< Called Party does not respond*/
	VC_ENDCAUSE_IMEI_REJECTED,			/**< Called Party does not respond*/
	/*VC_ENDCAUSE_SWITCHING_EQUIPMENT_CONGESTION,     /**<  Switching Equipment Congestion       : 20090627 match as NW_BUSY*/
} voice_call_end_cause_type_t;

/**
* This enumeration defines the call types used to end the calls in end call API
*/
typedef enum {
	VC_END_OUTGOING_CALL,			/**< To end only mobile originated outgoing call*/
	VC_END_INCOMING_CALL,				/**< To end only mobile terminated incoming call*/
	VC_END_ACTIVE_OR_HELD_CALLS,		/**< To end first active call if available or to end end held call*/
	VC_END_ALL_ACTIVE_CALLS,			/**< To end all available active calls*/
	VC_END_ALL_HELD_CALLS,			/**< To end all available held calls*/
	VC_END_ALL_CALLS					/**< To end all available calls(active,held,incoming/outgoing*/
} _vc_core_engine_end_call_type_t;

/**
* This enumeration defines flags to be used with _vc_core_engine_status_set_end_flag
*/
typedef enum {
	VC_RETREIVE_CALL_ON_MOCALL_END	/**< To be used before ending the mo call to retrieve the held call on mo call end if held call is available*/
} voicecall_end_flag_t;

/**
 * This enum defines the different call types by source of origination of call
 */
typedef enum {
	VC_CALL_ORIG_TYPE_NORMAL,						/**< Normal Call */
	VC_CALL_ORIG_TYPE_EMERGENCY,						/**< Emergency Call*/
	VC_CALL_ORIG_TYPE_VOICEMAIL,						/**< Voiece Mail Call*/
	VC_CALL_ORIG_TYPE_SAT,							/**< SAT Requested Call*/
	VC_CALL_ORIG_TYPE_NOSIM,							/**< Emergency Call with out SIM*/
	VC_CALL_ORIG_TYPE_PINLOCK						/**< Emergency Call with Pin Lock*/
} voicecall_call_orig_type_t;

/**
 * This enum defines the different call types to be used with the assiociated APIs
 */
typedef enum {
	VC_INCOMING_CALL,			/**< Mobile Terminated Call*/
	VC_OUTGOING_CALL,			/**< Mobile Originated Call*/
	VC_CONNECTED_CALL			/**< Connected Call (Both Parties Connected)*/
} voicecall_call_type_t;

/**
* This enum defines the type of calls being termed once the calls are connected
*/
typedef enum {
	VC_ACTIVE_CALL,		/**< Connected calls in active state*/
	VC_HELD_CALL			/**< Connected calls in held state*/
} voicecall_connected_call_type_t;

/**
 * This enum defines alternate line service types
 */
typedef enum {
	VC_CALL_CPHS_ALS_NONE,						/**< No ALS */
	VC_CALL_CPHS_ALS_LINE1,						/**< Line 1*/
	VC_CALL_CPHS_ALS_LINE2						/**< Line 2 */
} voice_call_cphs_alsline_t;

/**
 * This enum defines the call group states
 */
typedef enum {
	CALL_VC_GROUP_STATE_NONE,					/**< Group State Not Exist */
	CALL_VC_GROUP_STATE_ACTIVE,					/**< Active group state */
	CALL_VC_GROUP_STATE_HOLD					/**< Hold group state */
} call_vc_groupstate_t;

/**
 * This enum defines the mobile originated call states
 */
typedef enum {
	VC_MOCALL_STATE_NONE,						/**< MO State Not Exist */
	VC_MOCALL_STATE_END,							/**< MO State End */
	VC_MOCALL_STATE_CANCEL,						/**< MO State Cancel */
	VC_MOCALL_STATE_DESTROY						/**< MO State Destroy */
} voice_call_mocall_state_t;

#ifdef _CALL_LONG_DTMF
/**
 * This enum defines dtmf buffer states
 */
typedef enum {
	CALL_VC_DTMF_BUF_NONE,					/**< Buffer state Not Exist */
	CALL_VC_DTMF_BUF_START,					/**< Buffer start */
	CALL_VC_DTMF_BUF_RECEIVED_START_ACK,	/**< Reveived start acknowledgement */
	CALL_VC_DTMF_BUF_WAIT_STOP_ACK			/**< Wait stop acknowledgement */
} call_vc_dtmf_bufferstate_t;
#endif

/**
* This enum defines vc engine sat mo call ctrl response type
*/
typedef enum __voicecall_engine_sat_mo_call_ctrl_res {
	CALL_ALLOWED_NO_MOD,						/**< Call Allowed without any modification in the caller details */
	CALL_NOT_ALLOWED,							/**< Call Not Allowed - (User should be notified) */
	CALL_ALLOWED_WITH_MOD,					/**< Call Allowed with modification in the caller details- (UI should be updated) */
	CALL_CHANGED_TO_SS						/**< Call number modified to SS String */
} voicecall_engine_sat_mo_call_ctrl_res;

/**
 * This enum defines sat request response types
 */
typedef enum __voicecall_engine_sat_rqst_resp_type {
	SAT_RQST_SETUP_CALL,						/**< Sat setup call request */
	SAT_RQST_SEND_DTMF,						/**< Sat send dtmf request */
	SAT_RESP_SETUP_CALL						/**< Sat setup call response */
} voicecall_engine_sat_rqst_resp_type;

/**
 * This enum defines voicecall engine callback event types
 */
typedef enum _voicecall_engine_event_t {
	/* Tapi Call Related Event Types */
	VC_CALL_INCOM,								/**< Incoming call */
	VC_CALL_OUTGOING,							/**< Outgoing call. This event will be sent, when outgoing deferred and made after some time */
	VC_CALL_OUTGOING_ORIG,					/**< Outgoing call originated */
	VC_CALL_OUTGOING_ALERT,					/**< Outgoing alert call */
	VC_CALL_CONNECTED,						/**< Call connected */
	VC_CALL_NORMAL_END,						/**< Normal end */
	VC_CALL_INCOM_END,						/**< Incoming call end */
	VC_CALL_INCOM_DROPPED,					/**< Incoming call ended before it is being precessed by the engine */
	VC_CALL_REJECTED_END,						/**< Rejected call end */
	VC_CALL_OUTGOING_END,					/**< Outgoing call end */
	VC_CALL_OUTGOING_ABORTED,				/**< Outgoing call ended before making the call to the NW */
	VC_CALL_DTMF_ACK,							/**< DTMF Acknowledgement */
	VC_CALL_AUTO_REDIAL_CHECK,				/**< Auto Redial Check */
	VC_CALL_ANSWER_CNF,						/**< Answer confirm */

	/*Tapi Call Dependent SS related events */
	VC_CALL_SS_HELD,							/**< SS Held */
	VC_CALL_SS_RETREIVED,						/**< SS Retriebved */
	VC_CALL_SS_SWAP,							/**< SS call swap */
	VC_CALL_SS_SETUP_CONF,					/**< SS setup conference */
	VC_CALL_SS_SPLIT_CONF,					/**< SS Split conference */
	VC_CALL_SS_TRANSFERRED,					/**< SS call transferred */
	VC_CALL_SS_CONNECT_LINE_IND,				/**< SS connectect line indication */

	/*Tapi Call SS Indication Related Events */
	VC_CALL_IND_FORWARD,						/**< Call forward */
	VC_CALL_IND_ACTIVATE,						/**< Activate call */
	VC_CALL_IND_HOLD,							/**< Hold call */
	VC_CALL_IND_TRANSFER,						/**< Call transfer */
	VC_CALL_IND_SETUPCONFERENCE,				/**< Setup conference */
	VC_CALL_IND_BARRING,						/**< Call barring */
	VC_CALL_IND_WAITING,						/**< Call waiting */
	VC_CALL_IND_CUGINFO,						/**< Closed User Group information */
	VC_CALL_IND_SSNOTIFY,						/**< SS nofify */
	VC_CALL_IND_CALLINGNAMEINFO,				/**< Caller name information */
	VC_CALL_IND_REDIRECT_CNF,					/**< Redirect confirmation */
	VC_CALL_IND_ACTIVATECCBS_CNF,			/**< Activate CCBS confirmation */
	VC_CALL_IND_ACTIVATECCBS_USERINFO,		/**< Activate CCBS user information */
	VC_CALL_IND_AOC,							/**< AOC indication */

	/*Tapi Response Error Related Events */
	VC_ERROR_OCCURED,							/**< Error */

	/* Voicecall Engine triggered Event Type */
	VC_ACTION_INCOM_FORCE,	/**< If any outgoing data needs to be destroyed or cleaned up in the client during an incoming call, this event will be sent */
	VC_ACTION_SAT_REQUEST,	/**< SAT Request. This event will be sent to client , when SAT Engine requests any service from voicecall. Voicecall Engine holds the SAT engine related information, clients can get the information from the engine when it is required */
	VC_ACTION_SAT_RESPONSE,	/**< SAT Response */
	VC_ACTION_CALL_END_HELD_RETREIVED,	/**< If a held call is being retreived on end of an active call, this event will be emitted */
	VC_ACTION_NO_ACTIVE_TASK,			/**< This event will be published when engine becomes idle after executing/aborting a request from other apps - eg) if SAT request is not processed*/

	VC_CALL_GET_VOLUME_RESP,			/**< Response data from tapi for get tapi sound volume*/
	VC_ENGINE_EVENT_MAX
} voicecall_engine_event_t;

/**
 * This enum defines call forwarding indication types
 */
typedef enum _voicecall_forward_ind_type_t {
	VC_FRWD_IND_INCOM_IS_FRWD,					/**< Is incoming call a forwarded call? */
	VC_FRWD_IND_INCOM_FRWD,						/**< Incoming call Forwarded */
	VC_FRWD_IND_OUTGOING_FRWD					/**< Outgoing call Forwarded */
} voicecall_forward_ind_type_t;

/**
 * This enum defines call barring indication types
 */
typedef enum _voicecall_barr_ind_type_t {
	VC_BARR_IND_NONE,								/**< No call barring indication */
	VC_BARR_IND_ALL,								/**< Barring all outgoing and incoming calls */
	VC_BARR_IND_BAOC,								/**< Bar All Outgoing Calls indication */
	VC_BARR_IND_BOIC,								/**< Bar Outgoing International Calls indication */
	VC_BARR_IND_BOIC_EXHC,						/**< Bar Outgoing International Calls EXcept Home Calls indication */
	VC_BARR_IND_BAIC,								/**< Bar All Incoming Calls indication */
	VC_BARR_IND_BICROAM							/**< Bar Incoming Calls when ROAMing indication */
} voicecall_barr_ind_type_t;

/**
 * This enum defines supplementary services notificatoin indication types
 */
typedef enum _voicecall_ssnotify_ind_type_t {
	VC_SSNOTIFY_IND_CFU,							/**<  SS CFU indication */
	VC_SSNOTIFY_IND_CFB,							/**<  SS CFB indication */
	VC_SSNOTIFY_IND_CFNRY,						/**<  SS CFNRY indication */
	VC_SSNOTIFY_IND_CFNRC,						/**<  SS CFNRC indication */
	VC_SSNOTIFY_IND_CLIR,							/**<  SS CLIR indication */
	VC_SSNOTIFY_IND_ALL_COND_FORWARDING,		/**<  SS all condtional call forwarding indication */
	VC_SSNOTIFY_IND_BARRING_OF_OUTGOING		/**<  SS Outging call baring indication */
} voicecall_ssnotify_ind_type_t;

/**
 * This enum defines engine/ Input Output sate of call agent
 */
typedef enum {
	VC_INOUT_STATE_NONE,									/**<  I/O state none */
	VC_INOUT_STATE_OUTGOING_START,						/**< Outgoing enum value start */
	VC_INOUT_STATE_OUTGOING_WAIT_HOLD,					/**< Outgoing wait for hold */
	VC_INOUT_STATE_OUTGOING_WAIT_ORIG,					/**< Outgoing wait for Origination event */
	VC_INOUT_STATE_OUTGOING_WAIT_ALERT,					/**< Outgoing wait alert */
	VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED,				/**< Outgoing wait connected */
	VC_INOUT_STATE_OUTGOING_WAIT_RELEASE,				/**< Outgoing wait release */
	VC_INOUT_STATE_OUTGOING_ABORTED,						/**< Outgoing call aborted */
	VC_INOUT_STATE_OUTGOING_SHOW_REDIALCAUSE,			/**< Outgoing show redial cause */
	VC_INOUT_STATE_OUTGOING_WAIT_REDIAL,					/**< Outgoing wait redial */
	VC_INOUT_STATE_OUTGOING_SHOW_RETRY_CALLBOX,			/**< Outgoing showing retry call box : not used*/
	VC_INOUT_STATE_OUTGOING_END,							/**< Outgoing enum value end */

	VC_INOUT_STATE_INCOME_SELFEVET_WAIT,	/**< Incoming waiting for self event : not used*/
	VC_INOUT_STATE_INCOME_START,							/**< Incoming enumvalue start */
	VC_INOUT_STATE_INCOME_BOX,								/**< Incoming box */
	VC_INOUT_STATE_INCOME_WAIT_REDIRECTCNF,	/**< Incoming wait for redirect confirmation : not used*/
	VC_INOUT_STATE_INCOME_WAIT_REDIRECT_END,	/**< Incoming wait for redirect end : not used*/
	VC_INOUT_STATE_INCOME_WAIT_CONNECTED,				/**< Incoming wait connected */
	VC_INOUT_STATE_INCOME_WAIT_HOLD_CONNECTED,			/**< Incoming wait for hold and then connected event */
	VC_INOUT_STATE_INCOME_WAIT_RELEASE_ACTIVE_CONNECTED, /**< Incoming wait for release and then connected event */
	VC_INOUT_STATE_INCOME_WAIT_HOLD,	/**< Incoming wait hold : not used*/
	VC_INOUT_STATE_INCOME_WAIT_RELEASE_ACTIVECALL,		/**< Incoming wait release active call */
	VC_INOUT_STATE_INCOME_WAIT_RELEASE_HOLDCALL,		/**< Incoming wait release hold call */
#ifdef RELEASE_ALL_AND_ACCEPT_SUPPORT
	VC_INOUT_STATE_INCOME_WAIT_RELEASE_HOLDCALL_FOR_ALL_RELEASE,	/**< Incoming wait release hold call in case of release all and accept */
#endif
	VC_INOUT_STATE_INCOME_WAIT_RELEASE,					/**< Incoming wait release call */
	VC_INOUT_STATE_INCOME_END,							/**< Incoming enum value end */
	VC_INOUT_STATE_MAX_NUM								/**< Max value */
} voicecall_inout_state_t;

/**
 * This enum defines call answer types
 */
typedef enum _voicecall_answer_type_t {
	VC_ANSWER_NORMAL,						/**< Accepts the call in normal scenario(Only CONNECTED will be sent to the client) */
	VC_ANSWER_HOLD_ACTIVE_AND_ACCEPT,		/**< Puts the active call on hold and accepts the call (Only CONNECTED will be sent to client) */
	VC_ANSWER_RELEASE_ACTIVE_AND_ACCEPT,	/**< Releases the active call and accept the call (END and CONNECTED will be sent to Client) */
	VC_ANSWER_RELEASE_HOLD_AND_ACCEPT,	/**< Releases the active call and accept the call (END and  CONNECTED will be sent to client) */
#ifdef RELEASE_ALL_AND_ACCEPT_SUPPORT
	VC_ANSWER_RELEASE_ALL_AND_ACCEPT		/**< Releases the all calls and accept the call (END and  CONNECTED will be sent to client) */
#endif
} voicecall_answer_type_t;

/**
 * This enum defines the cphs csp status names
 */
typedef enum _voicecall_cphs_csp_service {
	/**< CPHS Teleservice*/
	VC_CPHS_CSP_ALS,		/**<Alternate Line Service*/

	/**< Call completion service*/
	VC_CPHS_CSP_HOLD,		/**<Call Hold*/
	VC_CPHS_CSP_CW,		/**<Call Waiting*/
	VC_CPHS_CSP_CBS,		/**<Completion of Call to Busy Subscribe*/
	VC_CPHS_CSP_UUS,		/**<User User Signalling*/

	/**< Call Offering Service*/
	VC_CPHS_CSP_CFU,		/**<Call Forwarding Unconditional*/
	VC_CPHS_CSP_CFB,		/**<Call Forwarding On User Busy*/
	VC_CPHS_CSP_CFNRY,	/**<Call Forwarding on No Reply*/
	VC_CPHS_CSP_CFNRC,	/**<Call Forwarding On User Not Reachable*/
	VC_CPHS_CSP_CT,		/**<Call Transfer*/

	/**< Other Supplementary Service*/
	VC_CPHS_CSP_MPTY,		/**<Multi-Party Service*/
	VC_CPHS_CSP_CUG,		/**<Closed User Group*/
	VC_CPHS_CSP_AOC,		/**<Advice Of Charge*/
	VC_CPHS_CSP_PREFCUG,	/**<Preferential CUG*/
	VC_CPHS_CSP_CUGOA	/**<CUG Outgoing Access*/
} voicecall_cphs_csp_service;

/**
 * This enum defines audio path values to be used when setting the voicecall audio path
 */
typedef enum __voicecall_audio_path_t {
	VC_AUDIO_PATH_HANDSET,			/**<Handset normal receiver*/
	VC_AUDIO_PATH_HEADSET,			/**<Headset Attached*/
	VC_AUDIO_PATH_HANDSFREE,		/**<Handsfree Attached*/
	VC_AUDIO_PATH_BLUETOOTH,		/**<Bluetooth headset Attached*/
	VC_AUDIO_PATH_STEREO_BLUETOOTH,	/**<Bluetooth Stero headset Attached*/
	VC_AUDIO_PATH_SPK_PHONE,		/**<Handset Speaker Phone receiver*/
	VC_AUDIO_PATH_HEADSET_3_5PI		/**<Headset */
} voicecall_audio_path_t;

/**
 * This enum defines volume level to be used when setting the voicecall volume
 */
typedef enum __voicecall_audio_volume_t {
	VC_AUDIO_VOLUME_LEVEL_0,
	VC_AUDIO_VOLUME_LEVEL_1,
	VC_AUDIO_VOLUME_LEVEL_2,
	VC_AUDIO_VOLUME_LEVEL_3,
	VC_AUDIO_VOLUME_LEVEL_4,
	VC_AUDIO_VOLUME_LEVEL_5,
	VC_AUDIO_VOLUME_LEVEL_6,
	VC_AUDIO_VOLUME_LEVEL_7,
	VC_AUDIO_VOLUME_LEVEL_8,
	VC_AUDIO_VOLUME_LEVEL_9
} voicecall_audio_volume_t;

/**
* Closed User Group Details
*/
typedef struct __voicecall_cug_info_t {
	gboolean bcug_used;				/**<TRUE -if CUG information used ,FALSE otherwise */
	int cug_index;					/**< CUG Index Value */
	gboolean bpref_cug;				/**<TRUE-if preferential CUG capablity available, FALSE otherwise */
	gboolean boa_cug;				/**<TRUE -if Outgoing Access capablity available, FALSE otherwise */
} voicecall_cug_info_t;

/**
 * This struct provides a structure for call setup info data.
 */
typedef struct _voicecall_setup_info_t {
	voicecall_call_type_bysetup_t call_setup_by;							/**<defines source of the call setup*/
	voicecall_call_orig_type_t call_type;											/**<  Call type */
	char source_tel_number[VC_PHONE_NUMBER_LENGTH_MAX];							/**<  Telephone number */
	char tel_number[VC_PHONE_NUMBER_LENGTH_MAX];							/**<  Telephone number */
	int mo_call_index;															/**< Outgoing call index */
	voicecall_cug_info_t cug_info;												/**< CUG Details */
	int identity_mode;														/**< Identity mode, 0 - default, 1-show, 2-hide */
	int ecc_category;														/**< ecc category*/
} voicecall_setup_info_t;

/**
 * This enumeration defines name mode to be verified dueing an incoming call
 */
typedef enum {
	CALL_VC_NAME_MODE_UNAVAILABLE = 0x00,	/**<  Caller Name Unavailable*/
	CALL_VC_NAME_MODE_REJECT = 0x01,		/**<  Caller Name Rejected by the caller*/
	CALL_VC_NAME_MODE_INTERACTION = 0x02,	/**<  Caller Name Unavailable*/
	CALL_VC_NAME_MODE_PAYPHONE = 0x03,		/**<  Caller using Payphone*/
	CALL_VC_NAME_MODE_MAX
} call_vc_name_mode_t;

/**
 * This enumeration defines calling name mode to be verified dueing an incoming call
 */
typedef enum {
	CALL_VC_CALLING_NAME_MODE_AVAILABLE,	/**<  Calling Name Unavailable*/
	CALL_VC_CALLING_NAME_MODE_RESTRICTED,		/**<  Calling Name restricted by the caller*/
	CALL_VC_CALLING_NAME_MODE_UNAVAILABLE,	/**<  Calling Name Unavailable*/
	CALL_VC_CALLING_NAME_MODE_AVAILABLE_RESTRICTED,		/**<  Calling name is available but restricted*/
	CALL_VC_CALLING_NAME_MODE_MAX
} call_vc_calling_name_mode_t;

/**
 * This structure defines the members of call object
 */
typedef struct _call_vc_call_objectinfo_t {
	/*Fields from TAPI */
	int call_handle;																/**< Call Handle */
	float aoc_ccm;																	/**< AOC Current Call Meter */
	char aoc_currency[VC_AOC_CURRENCY_CODE_LEN_MAX + 1];						/**< AOC Currency Code */
#ifdef _CPHS_DEFINED_
	voice_call_cphs_alsline_t als_type;											/**< Alternate Line Service Type */
#endif

	/*Engine Fields */
	int call_id;																	/**< Call ID */

	/* Call Object Status Flags */
	gboolean bemergency_number;													/**< emergency call? */
	gboolean bused_sim;																/**< used sim? */
	gboolean mo;																/**< Mobile originated call? */
	gboolean bccbs_call;															/**< CCBS call? */
	gboolean brestricted_namemode;												/**< Name mode Restricted? */
	call_vc_name_mode_t name_mode;											/**< Name mode when BDC number doesn't exist */
	call_vc_calling_name_mode_t bcalling_namemode;							/**< Name mode of calling name information */

	/*Caller Details */
/*PDIAL_SEND_DTMF*/
	char source_tel_number[VC_PHONE_NUMBER_LENGTH_MAX];						/**< Source Telephone number */
/*PDIAL_SEND_DTMF*/
	char tel_number[VC_PHONE_NUMBER_LENGTH_MAX];						/**< Telephone number */
	char calling_name[VC_PHONE_NAME_LENGTH_MAX];						/**< Calling part name */
	char dtmf_number[VC_PHONE_NUMBER_LENGTH_MAX];						/**< DTMF number */
	char connected_telnumber[VC_PHONE_NUMBER_LENGTH_MAX];					/**< Changed number to be connected by call forwarding */
	char connected_name[VC_PHONE_NUMBER_LENGTH_MAX];					/**< Changed name to be connected by call forwarding  */

	/*Call State Flags */
	voice_call_end_cause_type_t end_cause_type;							/**< End cause type */
	voicecall_call_state_t state;													/**< Call State */
	voicecall_call_orig_type_t call_type;												/**< Call Type */

	/*CUG Flags */
	voicecall_cug_info_t cug_info;												/**< Closed User Group Details */

	/*Idendity Mode */
	int identity_mode;														/**< Show My Number Mode, 0-Default,1-Show, 2- Hide */
	gboolean bincoming_call_is_fwded;

	int ecc_category;												/**< emergency category(see the TelSimEccEmergencyServiceInfo_t or TelCallEmergencyCategory_t)*/
} call_vc_call_objectinfo_t;

/**
 * Structure for SAT call info
 */
typedef struct {
	char disp_text[VC_PHONE_NUMBER_LENGTH_MAX];								/**< Display text */
	char call_number[VC_PHONE_NUMBER_LENGTH_MAX];								/**< Call NUmber */
	unsigned int duration;														/**< Duration of call */
	int sat_mo_call_ctrl_res;
	gboolean bicon_present;
	gboolean bsat_hidden;
} voicecall_sat_callinfo_t;

/**
 * Structure to hold the SAT Icon Details
 */
typedef struct {
	int width;					/**< Width of the SAT icon being held by the engine */
	int height;					/**< Height of the SAT icon being held by the engine */
	guchar *psat_icon_data;		/**< SAT icon Raw Data */
} voicecall_sat_call_icon_data_t;

/**
 * This enumeration defines SAT reponse types
 */
typedef enum {
	CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND,						/**< Unable to process command */
	CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND,					/**< Network unable to process command */
	CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND_WITHOUT_CAUSE,  /**< Network unable to process command without cause */
	CALL_VC_ME_CONTROL_PERMANENT_PROBLEM,							/**< Control permanent problem */
	CALL_VC_ME_CLEAR_DOWN_BEFORE_CONN,							/**< Clear down before connection */
	CALL_VC_ME_RET_SUCCESS											/**< Return success */
} call_vc_sat_reponse_type_t;

#endif				/*__VC_CORE_ENGINE_TYPES_H_*/
