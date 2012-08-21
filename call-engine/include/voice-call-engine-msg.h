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


#ifndef _VOICE_CALL_ENGINE_MSG_H_
#define _VOICE_CALL_ENGINE_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vc-core-engine-types.h"

typedef enum _vc_engine_msgid_t  { 
	/* Tapi Call Related Event Types */ 
	VC_ENGINE_MSG_INCOM_TO_UI, /**< Incoming call */ 
	VC_ENGINE_MSG_OUTGOING_TO_UI, /**< Outgoing call. This event will be sent, when outgoing deferred and made after some time */ 
	VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI, /**< Outgoing call originated */ 
	VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI_TEST,  /**< Outgoing local test */ 
	VC_ENGINE_MSG_OUTGOING_ALERT_TO_UI, /**< Outgoing alert call */ 
	VC_ENGINE_MSG_CONNECTED_TO_UI, /**< Call connected */ 
	VC_ENGINE_MSG_NORMAL_END_TO_UI, /**< Normal end */ 
	VC_ENGINE_MSG_INCOM_END_TO_UI, /**< Incoming call end */ 
	VC_ENGINE_MSG_REJECTED_END_TO_UI, /**< Rejected call end */ 
	VC_ENGINE_MSG_OUTGOING_END_TO_UI, /**< Outgoing call end */ 
	VC_ENGINE_MSG_OUTGOING_END_SIGNAL_PLAY_TO_UI, /**< Outgoing call end signal play */ 
	VC_ENGINE_MSG_OUTGOING_ABORTED_TO_UI, /**< Outgoing call ended before making the call to the NW */ 
	VC_ENGINE_MSG_DTMF_ACK_TO_UI, /**< DTMF Acknowledgement */ 
	
	/*Tapi Call Dependent SS related events */ 
	VC_ENGINE_MSG_SS_HELD_TO_UI, /**< SS Held */ 
	VC_ENGINE_MSG_SS_RETREIVED_TO_UI, /**< SS Retriebved */ 
	VC_ENGINE_MSG_SS_SWAP_TO_UI, /**< SS call swap */ 
	VC_ENGINE_MSG_SS_SETUP_CONF_TO_UI, /**< SS setup conference */ 
	VC_ENGINE_MSG_SS_SPLIT_CONF_TO_UI, /**< SS Split conference */ 
	VC_ENGINE_MSG_SS_TRANSFERRED_TO_UI, /**< SS call transferred */ 
	VC_ENGINE_MSG_SS_CONNECT_LINE_IND_TO_UI,	/**< SS connectect line indication */
	
	/*Tapi Call SS Indication Related Events */ 
	VC_ENGINE_MSG_IND_FORWARD_TO_UI, /**< Call forward */ 
	VC_ENGINE_MSG_IND_ACTIVATE_TO_UI, /**< Activate call */ 
	VC_ENGINE_MSG_IND_HOLD_TO_UI, /**< Hold call */ 
	VC_ENGINE_MSG_IND_TRANSFER_TO_UI, /**< Call transfer */ 
	VC_ENGINE_MSG_IND_SETUPCONFERENCE_TO_UI, /**< Setup conference */ 
	VC_ENGINE_MSG_IND_BARRING_TO_UI, /**< Call barring */ 
	VC_ENGINE_MSG_IND_WAITING_TO_UI, /**< Call waiting */ 
	VC_ENGINE_MSG_IND_CUGINFO_TO_UI, /**< Closed User Group information */ 
	VC_ENGINE_MSG_IND_SSNOTIFY_TO_UI, /**< SS nofify */ 
	VC_ENGINE_MSG_IND_CALLINGNAMEINFO_TO_UI, /**< Caller name information */ 
	VC_ENGINE_MSG_IND_REDIRECT_CNF_TO_UI, /**< Redirect confirmation */ 
	VC_ENGINE_MSG_IND_ACTIVATECCBS_CNF_TO_UI, /**< Activate CCBS confirmation */ 
	VC_ENGINE_MSG_IND_ACTIVATECCBS_USERINFO_TO_UI, /**< Activate CCBS user information */ 
	VC_ENGINE_MSG_IND_AOC_TO_UI, /**< AOC indication */ 

	/*Tapi Response Error Related Events */ 
	VC_ENGINE_MSG_ERROR_OCCURED_TO_UI, /**< Error */ 
	
	/* Voicecall Engine triggered Event Type */ 
	VC_ENGINE_MSG_ACTION_INCOM_FORCE_TO_UI, /**< If any outgoing data needs to be destroyed or cleaned up in the client during an incoming call, this event will be sent */ 
	VC_ENGINE_MSG_ACTION_SAT_REQUEST_TO_UI, /**< SAT Request. This event will be sent to client , when SAT Engine requests any service from voicecall. Voicecall Engine holds the SAT engine related information, clients can get the information from the engine when it is required */ 
	VC_ENGINE_MSG_ACTION_SAT_RESPONSE_TO_UI, /**< SAT Response */ 
	VC_ENGINE_MSG_ACTION_CALL_END_HELD_RETREIVED_TO_UI, /**< If a held call is being retreived on end of an active call, this event will be emitted */ 
	VC_ENGINE_MSG_ACTION_NO_ACTIVE_TASK_TO_UI, /**< This event will be published when engine becomes idle after executing/aborting a request from other apps - eg) if SAT request is not processed*/ 
	VC_ENGINE_MSG_GET_VOLUME_RESP_TO_UI, /**< Response data from tapi for get tapi sound volume (with ringtone vol)*/ 
	VC_ENGINE_MSG_SET_VOLUME_FROM_BT_TO_UI, /**< volume change form bt headset */ 
	VC_ENGINE_MSG_HEADSET_STATUS_TO_UI, /**< Headset status to UI */ 
	VC_ENGINE_MSG_EARJACK_STATUS_TO_UI, /**< Headset status to UI */ 
	VC_ENGINE_MSG_ACCEPT_CHOICE_BOX_TO_UI, /**< Accept choice box to UI */ 
	VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, /**< Create Message box */ 
	VC_ENGINE_MSG_REDIAL_TO_UI, /* To send the redial message to the UI */ 
	VC_ENGINE_MSG_NOTI_WBAMR_TO_UI, /* WBAMR notification */
	VC_ENGINE_MSG_MAX_TO_UI 
} vc_engine_msgid_t;

typedef enum _vc_engine_msgbox_string_id_t {
	IDS_CALL_POP_CALL_IS_DIVERTED,
	IDS_CALL_POP_CALLFAILED,
	IDS_CALL_POP_CALLING_EMERG_ONLY,
	IDS_CALL_POP_CALLNOTCALLOWED,
	IDS_CALL_POP_CAUSE_WRONG_NUMBER,
	IDS_CALL_POP_CHANGEOFFLINEMODETOCALL,
	IDS_CALL_POP_DTMFSENDING_FAIL,
	IDS_CALL_POP_FDNCALLONLY,
	IDS_CALL_POP_HOLD_FAILED,
	IDS_CALL_POP_HOLD_NOT_SUPPORTED,
	IDS_CALL_POP_INCOMPLETE,
	IDS_CALL_POP_JOIN_FAILED,
	IDS_CALL_POP_JOIN_NOT_SUPPORTED,
	IDS_CALL_POP_OPERATION_REFUSED,
	IDS_CALL_POP_PHONE_NOT_INITIALISED,
	IDS_CALL_POP_REJECTED,
	IDS_CALL_POP_SENDING,
	IDS_CALL_POP_SOS_CALL_ONLY_IN_NO_SIM_MODE,
	IDS_CALL_POP_SPLIT_FAILED,
	IDS_CALL_POP_SPLIT_NOT_SUPPORTED,
	IDS_CALL_POP_SWAP_FAILED,
	IDS_CALL_POP_SWAP_NOT_SUPPORTED,
	IDS_CALL_POP_TRANSFER_FAILED,
	IDS_CALL_POP_TRANSFER_NOT_SUPPORTED,
	IDS_CALL_POP_UNABLE_TO_RETRIEVE,
	IDS_CALL_POP_UNAVAILABLE,
	IDS_CALL_POP_UNHOLD_NOT_SUPPORTED,
	IDS_CALL_POP_VOICE_CALL_IS_NOT_ALLOWED_DURING_VIDEO_CALL,
	IDS_CALL_POP_WAITING_ACTIVE,
	IDS_CALL_BODY_CALLENDED, 
	IDS_CALL_POP_INVALID_DTMF,
	IDS_CALL_POP_DTMF_SENT, 
	IDS_CALL_MAX 
} vc_engine_msgbox_string_id_t;


typedef enum _vc_engine_end_cause_type_t {
	VC_ENGINE_ENDCAUSE_CALL_ENDED, /**< Call ended */ 
	VC_ENGINE_ENDCAUSE_CALL_DISCONNECTED, /**< Call disconnected */ 
	VC_ENGINE_ENDCAUSE_CALL_SERVICE_NOT_ALLOWED, /**< Service not allowed */ 
	VC_ENGINE_ENDCAUSE_CALL_BARRED, /**< Call barred */ 
	VC_ENGINE_ENDCAUSE_NO_SERVICE, /**< No Service */ 
	VC_ENGINE_ENDCAUSE_NW_BUSY, /**< Network busy */ 
	VC_ENGINE_ENDCAUSE_NW_FAILED, /**< Network failed */ 
	VC_ENGINE_ENDCAUSE_NO_ANSWER, /**< No anwer from other party */ 
	VC_ENGINE_ENDCAUSE_NO_CREDIT, /**< No credit available */ 
	VC_ENGINE_ENDCAUSE_REJECTED, /**< Call rejected */ 
	VC_ENGINE_ENDCAUSE_USER_BUSY, /**< user busy */ 
	VC_ENGINE_ENDCAUSE_WRONG_GROUP, /**< Wrong group */ 
	VC_ENGINE_ENDCAUSE_CALL_NOT_ALLOWED, /**< Call not allowed */ 
	VC_ENGINE_ENDCAUSE_TAPI_ERROR, /**< Tapi error */ 
	VC_ENGINE_ENDCAUSE_CALL_FAILED, /**< Call Failed */ 
	VC_ENGINE_ENDCAUSE_NO_USER_RESPONDING, /**< User not responding */ 
	VC_ENGINE_ENDCAUSE_USER_ALERTING_NO_ANSWER, /**< User Alerting No Answer */ 
	VC_ENGINE_ENDCAUSE_SERVICE_TEMP_UNAVAILABLE, /**< Circuit Channel Unavailable,Network is out of Order,Switching equipment congestion,Temporary Failure */ 
	VC_ENGINE_ENDCAUSE_USER_UNAVAILABLE, /**< Called Party Rejects the Call */ 
	VC_ENGINE_ENDCAUSE_INVALID_NUMBER_FORMAT, /**< Entered number is invalid or incomplete */ 
	VC_ENGINE_ENDCAUSE_NUMBER_CHANGED, /**< Entered number has been changed */ 
	VC_ENGINE_ENDCAUSE_UNASSIGNED_NUMBER, /**< Unassigned/Unallocated number*/ 
	VC_ENGINE_ENDCAUSE_USER_DOESNOT_RESPOND, /**< Called Party does not respond*/ 
	VC_ENGINE_ENDCAUSE_IMEI_REJECTED, /**< Called Party does not respond*/ 
} vc_engine_end_cause_type_t;

typedef struct {
	int len;
	int msg_id;
} vc_engine_msg_hdr_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int call_handle;
	int contact_index;
	int phone_type;
	int brejected;
	int brestricted;
	int bpayphone;
	int bday_remaining_days;
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];
	char call_name[VC_DISPLAY_NAME_LENGTH_MAX];
	char call_file_path[VC_IMAGE_PATH_LENGTH_MAX];
	char call_full_file_path[VC_IMAGE_PATH_LENGTH_MAX];
} vc_engine_incoming_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int contact_index;
	int phone_type;
	int bday_remaining_days;
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];
	char call_name[VC_DISPLAY_NAME_LENGTH_MAX];
	char call_file_path[VC_IMAGE_PATH_LENGTH_MAX];
	char call_full_file_path[VC_IMAGE_PATH_LENGTH_MAX];
} vc_engine_outgoing_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int call_handle;
	int end_cause_type;
} vc_engine_normal_end_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int call_handle;
	int bt_status;
} vc_engine_connected_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int call_handle;
	int bemergency;
} vc_engine_outgoing_orig_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int call_handle;
	int end_cause_type;
	int bauto_redial;
} vc_engine_outgoing_end_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int call_handle;
	int end_cause_type;
} vc_engine_outgoing_end_signal_play_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int call_handle;
	int end_cause_type;
} vc_engine_redial_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int call_handle;
} vc_engine_common_with_handle_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
} vc_engine_common_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int fwd_type;
} vc_engine_ind_forward_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int ss_type;
} vc_engine_ind_ssnotify_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int error_code;
} vc_engine_error_occured_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int vol_alert_type;
	int vol_level;
} vc_engine_vol_resp_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int vol_level;
} vc_engine_vol_set_from_bt_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int bstatus;
} vc_engine_headset_status_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int bstatus;
} vc_engine_earjack_status_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int choice;
} vc_engine_accept_choice_box_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int string_id;
	char diverted_num[VC_PHONE_NUMBER_LENGTH_MAX];
} vc_engine_msg_box_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	gboolean bstatus;
	int string_id;
	char display_string[VC_DISPLAY_NAME_LENGTH_MAX];
} vc_engine_dtmf_ack_type;

typedef struct {
	vc_engine_msg_hdr_type hdr;
	int bstatus;
} vc_engine_wbamr_status_type;

typedef union {
	vc_engine_msg_hdr_type hdr;
	vc_engine_incoming_type incoming;
	vc_engine_outgoing_type outgoing;
	vc_engine_outgoing_orig_type outgoing_orig;
	vc_engine_common_with_handle_type outgoing_alert;
	vc_engine_connected_type connected;
	vc_engine_normal_end_type normal_end;
	vc_engine_common_with_handle_type incom_end;
	vc_engine_common_with_handle_type incom_droped;
	vc_engine_common_with_handle_type rejected_end;
	vc_engine_outgoing_end_type outgoing_end;
	vc_engine_outgoing_end_signal_play_type outgoing_end_signal_play;
	vc_engine_common_with_handle_type outgoing_aborted;
	vc_engine_common_with_handle_type dtmf_ack;
	vc_engine_common_with_handle_type auto_redial_check;
	vc_engine_common_type ss_held;
	vc_engine_common_type ss_retreived;
	vc_engine_common_type ss_swap;
	vc_engine_common_type ss_setup_conf;
	vc_engine_common_with_handle_type ss_split_conf;
	vc_engine_common_type ss_transferred;
	vc_engine_common_type ss_connnect_line_ind;
	vc_engine_ind_forward_type ss_ind_forward;
	vc_engine_common_type ss_ind_activate;
	vc_engine_common_type ss_ind_hold;
	vc_engine_common_type ss_ind_transfer;
	vc_engine_common_type ss_ind_setupconference;
	vc_engine_common_type ss_ind_barring;
	vc_engine_common_type ss_ind_wating;
	vc_engine_common_type ss_ind_cuginfo;
	vc_engine_ind_ssnotify_type ss_ind_ssnotify;
	vc_engine_common_type ss_ind_callingnameinfo;
	vc_engine_common_type ss_ind_redirect_cnf;
	vc_engine_common_type ss_ind_activateccbs_cnf;
	vc_engine_common_type ss_ind_activatedccbs_userinfo;
	vc_engine_common_type ss_ind_aoc;
	vc_engine_error_occured_type error_occured;
	vc_engine_vol_resp_type vol_resp;
	vc_engine_vol_set_from_bt_type vol_set_from_bt;
	vc_engine_headset_status_type headset_status;
	vc_engine_earjack_status_type earjack_status;
	vc_engine_accept_choice_box_type accept_choice_box;
	vc_engine_msg_box_type msg_box;
	vc_engine_redial_type redial;
	vc_engine_dtmf_ack_type dtmf_progress;
	vc_engine_wbamr_status_type wbamr_status;
} vc_engine_msg_type;

typedef enum {
	VC_CALL_UI_ACCEPT_1,
	VC_CALL_UI_ACCEPT_2 
} voicecal_ui_accept_choice_t;

typedef struct {
	int len;
	int msg_id;
} vcui_msg_hdr_type;

typedef struct {
	vcui_msg_hdr_type hdr;
} vcui_common_type;

typedef struct {
	vcui_msg_hdr_type hdr;
	int call_handle;
} vcui_common_with_handle_type;

typedef struct {
	vcui_msg_hdr_type hdr;
	int ans_type;
} vcui_accept_with_type;

typedef struct {
	vcui_msg_hdr_type hdr;
	int vol_alert_type;
} vcui_vol_get_type;

typedef struct {
	vcui_msg_hdr_type hdr;
	int vol_level;
} vcui_tapi_vol_set_type;

typedef struct {
	vcui_msg_hdr_type hdr;
	int vol_level;
} vcui_ringtone_vol_set_type;

typedef struct {
	vcui_msg_hdr_type hdr;
	int manual_redial;
} vcui_common_with_redial_type;

typedef struct {
	vcui_msg_hdr_type hdr;
	char dtmf_number[2];
} vcui_dtmf_type;

typedef union {
	vcui_msg_hdr_type hdr;
	vcui_common_type accept;
	vcui_accept_with_type accept_with_type;
	vcui_common_type reject;
	vcui_common_type reject_with_msg;
	vcui_common_type end;
	vcui_common_with_handle_type end_with_handle;
	vcui_common_type end_all_calls;
	vcui_common_type end_active_calls;
	vcui_common_type end_held_calls;
	vcui_common_type hold;
	vcui_common_type unhold;
	vcui_common_type swap;
	vcui_common_type join;
	vcui_common_with_handle_type split;
	vcui_common_type transfer;
	vcui_common_type spkon;
	vcui_common_type spkoff;
	vcui_common_type muteon;
	vcui_common_type muteoff;
	vcui_vol_get_type vol_get;
	vcui_tapi_vol_set_type tapi_vol_set;
	vcui_ringtone_vol_set_type ringtone_vol_set;
	vcui_common_type headset_on;
	vcui_common_type headset_off;
	vcui_common_with_redial_type redial_type;
	vcui_common_type redial_stop;
	vcui_dtmf_type dtmf;
} vcui_msg_type;
#ifdef __cplusplus
}
#endif

#endif

