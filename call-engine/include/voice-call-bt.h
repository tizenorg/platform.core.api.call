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


#ifndef _VOICE_CALL_BT_H_
#define _VOICE_CALL_BT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "voice-call-core.h"
#include "vconf.h"
#include "vconf-keys.h"

#define BT_IPC_STRING_SIZE 256

/* Enum describes the AG request commands */
typedef enum {
	BT_AG_REQ_CONNECT,     /**< Headset Connected */
	BT_AG_REQ_CONNECT_ERROR,    /**< Headset Connection Failed */
	BT_AG_REQ_DISCONNECT,     /**< Headset Disconnected */
	BT_AG_REQ_SPK_GAIN,     /**< Speaker volume changed */
	BT_AG_REQ_MIC_GAIN,     /**< Microphone gain changed */
	BT_AG_REQ_CALL_ACCEPTED,    /**< Call accepted */
	BT_AG_REQ_CALL_REJECT,     /**< Incoming call rejected */
	BT_AG_REQ_CALL_END,     /**< Call Ended*/
	BT_AG_REQ_CALL_HOLD,     /**< Put Call on Hold */
	BT_AG_REQ_CALL_RETRIEVE,    /**< Retrieve held call */
	BT_AG_REQ_BUTTON_PRESSED,   /**< Headset Button was pressed */
	BT_AG_REQ_CALL_REDIAL,     /**< Handsfree requested call redial */
	BT_AG_REQ_CALL_2_SEND,     /**< Places all active calls on hold and accepts the other call */
	BT_AG_REQ_CALL_3_SEND,     /**< Adds a held call to the conversation */
	BT_AG_REQ_CALL_0_SEND,     /**< Releases all held calls or sets User Determined User Busy for a waiting call */
	BT_AG_REQ_CALL_1_SEND,     /**< Releases all active calls and accepts the other call  */
	BT_AG_REQ_HEADSET_VOL,     /**< Send current Headset Volume level to Call */
	BT_AG_REQ_SWITCH_TO_HEADSET,/**< Switch to HS */
	BT_AG_REQ_SWITCH_TO_PHONE,  /**< Switch to Phone */
	BT_AG_REQ_DTMF,      /**< Send DTMF tone */
	BT_AG_REQ_CALL_STATUS     /**< Ask current call status */
} connectivity_bt_ag_req_t;

/* Enum describes the AG response commands. */
typedef enum {
	BT_AG_RES_CALL_ORIG,     /**< Phone originated a call.  */
	BT_AG_RES_CALL_INCOM,     /**< Incoming call notification to Headset */
	BT_AG_RES_CALL_CONNECT,     /**< Call connected */
	BT_AG_RES_CALL_END,     /**< MO or MT call ended */
	BT_AG_RES_CALL_HOLD,     /**< Call on Hold */
	BT_AG_RES_CALL_RETRIEVE,    /**< Held call retrieved */
	BT_AG_RES_CALL_JOINED,     /**< Held call joined */
	BT_AG_RES_SPK_GAIN,     /**< Speaker volume changed */
	BT_AG_RES_MIC_GAIN,     /**< Microphone gain changed */
	BT_AG_RES_CALL_REMOTE_RINGING,
	/**< Remote party call alerted */
	BT_AG_RES_SWITCH_TO_HEADSET,/**< Switch to HS */
	BT_AG_RES_SWITCH_TO_PHONE,  /**< Switch to Phone */
	BT_AG_RES_CALL_STATUS,     /**< For call status information to headset */
	BT_AG_RES_HEADSET_VOL,      /**< Request Current Headset Volume level from Call */
	BT_AG_RES_CALL_SWAPPED
} connectivity_bt_ag_res_t;

/**
*   This enum is for ag call status
*/
typedef enum {
	BT_AG_CALL_STATUS_NONE,     /**< AG Call status is None */
	BT_AG_CALL_STATUS_DIALLING, /**< AG Call status is Dialing */
	BT_AG_CALL_STATUS_INCOMING, /**< Incoming Call */
	BT_AG_CALL_STATUS_INCOMING_HELD,
	/**< Incoming call is held */
	BT_AG_CALL_STATUS_CONNECTED,/**< Call is connected */
	BT_AG_CALL_STATUS_HELD,     /**< Call is held */
	BT_AG_CALL_STATUS_RETRIEVED,/**< Call is retrieved */
	BT_AG_CALL_STATUS_RETRIVING,/**< Call is retreving */
	BT_AG_CALL_STATUS_WAITING,  /**< Incoming call Waiting */
	BT_AG_CALL_STATUS_ALERTING, /**< Remote Party being alerted in outgoing call */
	BT_AG_CALL_STATUS_CANDIDATE /**< The call hed state is moved to call_candidate */
} connectivity_bt_ag_call_value_t;

typedef enum {
	BT_AG_RES_AUDIO_CONNECTION_ERROR = 0x00F0,
	BT_AG_RES_AUDIO_DISCONNECTION_ERROR,
} connectivity_bt_ag_resp_status_t;

typedef struct {
	int param1;	/* Req or Res Type */
	int param2;
	int param3;
	char param4[BT_IPC_STRING_SIZE];
} __attribute__ ((packed)) connectivity_bt_ag_param_info_t;

typedef struct {
	unsigned int call_id;			/**< Call identification */
	connectivity_bt_ag_call_value_t call_status;	/**< Status of the call */
} __attribute__ ((packed)) connectivity_bt_ag_call_status_info_t;

gboolean _vc_bt_switch_headset_path(call_vc_core_state_t *pcall_core, gboolean bheadset, gboolean bsend_bt_response);

/**
 * This function sends event to BT for switch to Headset/Phone
 *
 * @return		void
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		bheadset			TRUE if sound type is Audio Headset, FALSE otherwise
 */
void _vc_bt_request_switch_headset_path(call_vc_core_state_t *pcall_core, gboolean bheadset);

/**
 * This function handles the bluetooth notifications sent by blue tooth application
 *
 * @return		TRUE -if event is handled, FALSE otherwise
 * @param[in]		pcall_core			Handle to voicecall core
 * @param[in]		pbt_info				bt notification details
 */
gboolean _vc_bt_handle_bt_events(call_vc_core_state_t *pcall_core, connectivity_bt_ag_param_info_t * pbt_info);

/**
 * This function sends response back to the bt application
 *
 * @return		void
 * @param[in]		pcall_core			Handle to voicecall core
 * @param[in]		bt_event				bluetooth event type
 * @param[in]		param1				user param1
 * @param[in]		param2				user param2
 */
void _vc_bt_send_response_to_bt(call_vc_core_state_t *pcall_core, int bt_event, int param1, gpointer param2);

/**
 * This function register bt callback.
 *
 * @return		TRUE if bt status is registered, FALSE otherwise
 * @param[in]		void
 */
gboolean _vc_bt_status_init(call_vc_core_state_t *pcall_core);

/**
 * This function gets the blue tooth active status from the phone status server
 *
 * @return		TRUE - if BT is enabled, FALSE otherwise
 * @param[in]		none
 */
gboolean _vc_bt_get_bt_status(void);

/**
 * This function gets the BT inband ringtone activate settings
 *
 * @return		TRUE - if BT inband ringtone activate settings is ON, FALSE otherwise
 * @param[in]		none
 */
gboolean _vc_bt_get_inband_ringtone_active(void);

/**
 * This function handles the notifications sent by phone status server
 *
 * @return		TRUE -if notification is handled, FALSE otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		bt_status			Blue tooth status
 */
gboolean _vc_bt_handle_phonestatus_bt_events(keynode_t *node, call_vc_core_state_t *pcall_core);

/**
 * This function retreives volume level of headset
 *
 * @param[in]		pcall_core		Handle to voicecall core
 */
void _vc_bt_get_headset_volume(call_vc_core_state_t *pcall_core);

/**
 * This function returns the BT connection status
 *
 * @return		TRUE if bt is connected, FALSE otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean _vc_bt_is_bt_connected(call_vc_core_state_t *pcall_core);

/**
 * This function returns the BT SCO status.(Synchronized Connection Oriented link)
 *
 * @return		TRUE if bt is connected, FALSE otherwise
 * @param[in]		void
 */
gboolean _vc_bt_get_bt_sco_status(void);

#ifdef __cplusplus
}
#endif
#endif
