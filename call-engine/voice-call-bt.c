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


#include "vc-core-util.h"
#include "vc-core-engine-status.h"
#include "vc-core-engine-group.h"
#include "voice-call-core.h"
#include "voice-call-sound.h"
#include "voice-call-bt.h"
#include "vconf.h"
#include "vconf-keys.h"
#include "voice-call-dbus.h"
#include "voice-call-engine-msg.h"

/* For Debug Information, name string constant */
/* CALL --> BT */
static char *gszbt_res_event[BT_AG_RES_CALL_SWAPPED + 1] = {
	"BT_AG_RES_CALL_ORIG",
	"BT_AG_RES_CALL_INCOM",
	"BT_AG_RES_CALL_CONNECT",
	"BT_AG_RES_CALL_END",
	"BT_AG_RES_CALL_HOLD",
	"BT_AG_RES_CALL_RETRIEVE",
	"BT_AG_RES_CALL_JOINED",
	"BT_AG_RES_SPK_GAIN",
	"BT_AG_RES_MIC_GAIN",
	"BT_AG_RES_CALL_REMOTE_RINGING",
	"BT_AG_RES_SWITCH_TO_HEADSET",
	"BT_AG_RES_SWITCH_TO_PHONE",
	"BT_AG_RES_CALL_STATUS",
	"BT_AG_RES_HEADSET_VOL",
	"BT_AG_RES_CALL_SWAPPED"
};

/* CALL <-- BT */
static char *gszbt_req_event[BT_AG_REQ_CALL_STATUS + 1] = {
	"BT_AG_REQ_CONNECT",
	"BT_AG_REQ_CONNECT_ERROR",
	"BT_AG_REQ_DISCONNECT",
	"BT_AG_REQ_SPK_GAIN",
	"BT_AG_REQ_MIC_GAIN",
	"BT_AG_REQ_CALL_ACCEPTED",
	"BT_AG_REQ_CALL_REJECT",
	"BT_AG_REQ_CALL_END",
	"BT_AG_REQ_CALL_HOLD",
	"BT_AG_REQ_CALL_RETRIEVE",
	"BT_AG_REQ_BUTTON_PRESSED",
	"BT_AG_REQ_CALL_REDIAL",
	"BT_AG_REQ_CALL_2_SEND",
	"BT_AG_REQ_CALL_3_SEND",
	"BT_AG_REQ_CALL_0_SEND",
	"BT_AG_REQ_CALL_1_SEND",
	"BT_AG_REQ_HEADSET_VOL",
	"BT_AG_REQ_SWITCH_TO_HEADSET",
	"BT_AG_REQ_SWITCH_TO_PHONE",
	"BT_AG_REQ_DTMF",
	"BT_AG_REQ_CALL_STATUS"
};

#ifdef _NEW_SND_
static gboolean b_user_rqst_path_change = FALSE;
#endif
static int __vc_bt_converted_bt_vol_to_voice_vol(int bt_vol_level);
static void __vc_bt_handle_connectivity_event(call_vc_core_state_t *pcall_core, gboolean bt_headset_connect_status);

static int __vc_bt_converted_bt_vol_to_voice_vol(int bt_vol_level)
{
	int converted_vol_level = -1;

	if (bt_vol_level <= 0) {
		converted_vol_level = 1;
	} else if (bt_vol_level > 15) {
		converted_vol_level = 15;
	} else {
		converted_vol_level = bt_vol_level;
	}
	return converted_vol_level;
}

static void __vc_bt_handle_connectivity_event(call_vc_core_state_t *pcall_core, gboolean bt_headset_connect_status)
{
	VOICECALL_RETURN_IF_FAIL(pcall_core != NULL);

	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "bt_headset_connect_status = %d \n", bt_headset_connect_status);
	if (FALSE == bt_headset_connect_status) {
		voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING, FALSE);
	}

	if (bt_headset_connect_status == pcall_core->bt_connected) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No change in state, Ignoring Event \n");
		CALL_ENG_DEBUG(ENG_DEBUG, "bt_headset_connect_status = %d \n", bt_headset_connect_status);
		return;
	}
	pcall_core->bt_connected = bt_headset_connect_status;

	if (TRUE == pcall_core->bt_connected) {
		/*Check the Call Status and Send Response event to the Bluetooth */
		int call_handle = -1;

		CALL_ENG_DEBUG(ENG_DEBUG, "BT connected, Not changing the sound status \n");
		/*  Headset is connected, Set the Sound Status to Headset
		   and change the path only incase of mocall and connected call */

		_vc_core_engine_status_get_call_handle_bytype(pcall_engine, VC_INCOMING_CALL, &call_handle);
		if (call_handle != -1) {
			call_vc_call_objectinfo_t callobject_info;

			CALL_ENG_DEBUG(ENG_DEBUG, "Incoming Call Exists, call handle = %d \n", call_handle);

			voicecall_core_get_call_object(pcall_engine, call_handle, &callobject_info);

			/*Incoming Call Exists */
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_INCOM, call_handle, callobject_info.tel_number);
			return;
		}

		_vc_core_engine_status_get_call_handle_bytype(pcall_engine, VC_OUTGOING_CALL, &call_handle);
		if (call_handle != -1) {
			int io_state = VC_INOUT_STATE_NONE;

			CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing Call Exists, call handle = %d \n", call_handle);
			_vc_core_engine_status_get_engine_iostate(pcall_engine, &io_state);

			switch (io_state) {
			case VC_INOUT_STATE_OUTGOING_WAIT_ORIG:
			case VC_INOUT_STATE_OUTGOING_WAIT_ALERT:	/*Fall Through */
				{
					call_vc_call_objectinfo_t callobject_info;

					voicecall_core_get_call_object(pcall_engine, call_handle, &callobject_info);

					_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_ORIG, call_handle, callobject_info.tel_number);
				}
				break;
			case VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED:
				{
					_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_REMOTE_RINGING, call_handle, NULL);
#ifdef _NEW_SND_
					voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_BT);
					_vc_bt_request_switch_headset_path(pcall_core, TRUE);
#endif
				}
				break;
			default:
				{
					CALL_ENG_DEBUG(ENG_DEBUG, "Engine not in desired IO State, Current IO State: %d \n", io_state);
					return;
				}
				break;
			}

			return;
		}

		_vc_core_engine_status_get_call_handle_bytype(pcall_engine, VC_CONNECTED_CALL, &call_handle);
		if (call_handle != -1) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Connected Call Exists, call handle = %d \n", call_handle);

			CALL_ENG_DEBUG(ENG_DEBUG, "BT connected, Not changing the path in bt connect event\n");

			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_CONNECT, call_handle, NULL);
		}

	} else {
		vc_engine_headset_status_type event_data;

#ifdef _NEW_SND_
		if (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_BT) {
			if (_voicecall_dvc_get_earjack_connected() == TRUE) {
				voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_EARJACK);
			} else {
				voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER);
			}
			
			/* Headset is disconnected, so voice sound path should be changed to phone. */
			voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_HEADSET, FALSE);
			if (voicecall_core_is_incoming_call_exists(pcall_engine)) {
				if (pcall_core->papp_snd->bmute_play == TRUE) {
					voicecall_snd_change_mm_path(pcall_core->papp_snd, VOICE_CALL_MM_MUTE_PLAY);
				} else {
					voicecall_snd_change_mm_path(pcall_core->papp_snd, VOICE_CALL_MM_RING_TONE);
				}
			} else {
				if (voicecall_core_is_outgoing_call_exists(pcall_engine) || (voicecall_core_is_connected_call_exist(pcall_engine))) {
					voicecall_snd_change_path(pcall_core->papp_snd);
				} else {
					CALL_ENG_DEBUG(ENG_ERR, "No valid calls, not changing the path \n");
				}
			}
		} else {
			CALL_ENG_DEBUG(ENG_ERR, "BT disconnected when path is not on BT. Do NOT change path. only update UI");			
		}
		
		memset(&event_data, 0, sizeof(event_data));
		event_data.bstatus = FALSE;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_HEADSET_STATUS_TO_UI, (void *)&event_data);
#else
		memset(&event_data, 0, sizeof(event_data));
		event_data.bstatus = FALSE;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_HEADSET_STATUS_TO_UI, (void *)&event_data);

		/* Headset is disconnected, so voice sound path should be changed to phone. */
		voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_HEADSET, FALSE);
		if (voicecall_core_is_incoming_call_exists(pcall_engine)) {
			if (pcall_core->papp_snd->bmute_play == TRUE) {
				voicecall_snd_change_mm_path(pcall_core->papp_snd, VOICE_CALL_MM_MUTE_PLAY);
			} else {
				voicecall_snd_change_mm_path(pcall_core->papp_snd, VOICE_CALL_MM_RING_TONE);
			}
		} else {
			if (voicecall_core_is_outgoing_call_exists(pcall_engine) || (voicecall_core_is_connected_call_exist(pcall_engine))) {
				voicecall_snd_change_path(pcall_core->papp_snd);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "No valid calls, not changing the path \n");
			}
		}
#endif
	}

}

gboolean _vc_bt_switch_headset_path(call_vc_core_state_t *pcall_core, gboolean bheadset, gboolean bsend_bt_response)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	connectivity_bt_ag_res_t bt_event = BT_AG_RES_SWITCH_TO_PHONE;
	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "bheadset = %d,bsend_bt_response = %d \n", bheadset, bsend_bt_response);

#ifdef _NEW_SND_
	CALL_ENG_DEBUG(ENG_DEBUG, "b_user_rqst_path_change(%d)", b_user_rqst_path_change);
	if (b_user_rqst_path_change == FALSE) {
		if (TRUE == bheadset) {
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_BT);
		}
		else {
			int earjack_status = -1;
			if (!vconf_get_int(VCONFKEY_SYSMAN_EARJACK, &earjack_status)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "earjack_status:[%d]\n", earjack_status);
				if (earjack_status == VCONFKEY_SYSMAN_EARJACK_REMOVED) {
					voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER);				
				} else {
					voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_EARJACK);				
				}
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int fail");
				voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER);				
			}
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Don't set path_status.. rqst from User. Already setted.");
	}
#else
	/*TODO Check whehter the follwing check and set status to be done inside if((TRUE == bconnected_call_exists) || (TRUE == bmo_call_exists)) */
	/*If Loud Speaker is ON, Change to Normal when Bluetooth Headset is switched ON */
	if ((TRUE == bheadset) && (TRUE == voicecall_snd_get_status(papp_snd, VOICE_CALL_AUDIO_SPEAKER))) {
		voicecall_snd_set_status(papp_snd, VOICE_CALL_AUDIO_SPEAKER, FALSE);
	}
	voicecall_snd_set_status(papp_snd, VOICE_CALL_AUDIO_HEADSET, bheadset);
#endif

	/*Donot change the path for MT Call. Change the Audio Path only for MO and Connected calls */
	if ((TRUE == voicecall_core_is_connected_call_exist(pcall_engine)) || (TRUE == voicecall_core_is_outgoing_call_exists(pcall_engine))) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Changing the Path on BT Request \n");
		voicecall_snd_change_path_real(papp_snd);

		/*Send Response to BT only if the path is changed */
		if (TRUE == bsend_bt_response) {
			bt_event = (TRUE == bheadset) ? BT_AG_RES_SWITCH_TO_HEADSET : BT_AG_RES_SWITCH_TO_PHONE;
			CALL_ENG_DEBUG(ENG_DEBUG, "Sending BT Response bt_event: %d \n", bt_event);
			_vc_bt_send_response_to_bt(pcall_core, bt_event, -1, NULL);
		}
		return TRUE;
	}

	return FALSE;
}

/**
 * This function sends event to BT for switch to Headset/Phone
 *
 * @return		void
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		bheadset			TRUE if sound type is Audio Headset, FALSE otherwise
 */
void _vc_bt_request_switch_headset_path(call_vc_core_state_t *pcall_core, gboolean bheadset)
{
	VOICECALL_RETURN_IF_FAIL(pcall_core != NULL);

	connectivity_bt_ag_res_t bt_event = BT_AG_RES_SWITCH_TO_PHONE;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	/*Donot change the path for MT Call. Change the Audio Path only for MO and Connected calls */
	if ((TRUE == voicecall_core_is_connected_call_exist(pcall_engine)) || (TRUE == voicecall_core_is_outgoing_call_exists(pcall_engine))) {
		bt_event = (TRUE == bheadset) ? BT_AG_RES_SWITCH_TO_HEADSET : BT_AG_RES_SWITCH_TO_PHONE;
		CALL_ENG_DEBUG(ENG_DEBUG, "Sending BT Response bt_event: %d \n", bt_event);
		voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING, TRUE);

		_vc_bt_send_response_to_bt(pcall_core, bt_event, -1, NULL);
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "No Valid calls to switch path \n");
	}
}

/**
 * This function handles the bluetooth notifications sent by blue tooth application
 *
 * @return		TRUE -if event is handled, FALSE otherwise
 * @param[in]		pcall_core			Handle to voicecall core
 * @param[in]		pbt_info				bt notification details
 */
gboolean _vc_bt_handle_bt_events(call_vc_core_state_t *pcall_core, connectivity_bt_ag_param_info_t * pbt_info)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(pbt_info != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Event: %s(%d) \n", gszbt_req_event[pbt_info->param1], pbt_info->param1);

	/*HS Connection not required for sending response to BT_AG_REQ_CALL_STATUS */
	if ((FALSE == pcall_core->bt_connected) && (BT_AG_REQ_CALL_STATUS != pbt_info->param1)) {
		CALL_ENG_DEBUG(ENG_ERR, "BT not connected, Ignoring BT Events, BT Event= %d \n", pbt_info->param1);
		return TRUE;
	}

	switch (pbt_info->param1) {
	case BT_AG_REQ_CALL_STATUS:
		{
			/*Send the Current  Call status to BT */
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_STATUS, -1, NULL);

		}
		break;
	case BT_AG_REQ_SPK_GAIN:
		{
			int bt_vol_level = pbt_info->param2;
			int converted_vol_level;
			vc_engine_vol_set_from_bt_type event_data;

			CALL_ENG_DEBUG(ENG_DEBUG, "Speaker Gain Value : %d\n", bt_vol_level);

#ifdef _NEW_SND_
			if (voicecall_snd_get_path_status(pcall_core->papp_snd) != VOICE_CALL_SND_PATH_BT) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Headset not switched on, Ignoring Speaker Gain Event \n");
				return TRUE;
			}
#else
			if (FALSE == voicecall_snd_get_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_HEADSET)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Headset not switched on, Ignoring Speaker Gain Event \n");
				return TRUE;
			}
#endif
			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_VOL_CHNGD_BYBT, TRUE);

			converted_vol_level = __vc_bt_converted_bt_vol_to_voice_vol(bt_vol_level);

			CALL_ENG_DEBUG(ENG_DEBUG, "converted Speaker Gain Value : %d\n", converted_vol_level);

			memset(&event_data, 0, sizeof(event_data));
			event_data.vol_level = converted_vol_level;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_SET_VOLUME_FROM_BT_TO_UI, (void *)&event_data);
		}
		break;
	case BT_AG_REQ_CALL_ACCEPTED:
		{
			if (voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Incoming call Accept by BT \n");
				voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, TRUE);
				if(_vc_core_util_phonelock_status() == FALSE)
					vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_UNLOCK);
#ifdef _NEW_SND_
				voicecall_snd_set_path_status(pcall_core->papp_snd,VOICE_CALL_SND_PATH_BT);
#endif
				if (TRUE == voicecall_core_answer_call(pcall_core, FALSE)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "voicecall_core_answer_call success \n");
				}
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "No Incoming call, skipping BT request \n");
			}
		}
		break;
	case BT_AG_REQ_CALL_REJECT:
		{
			if (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
				if (FALSE == voicecall_core_reject_mt(pcall_core, TRUE)) {
					CALL_ENG_DEBUG(ENG_ERR, "voicecall_core_reject_mt returned FALSE!\n");
				}
				/*Call rejected, reset the accept by flag */
				voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, FALSE);
			} else if (TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine)) {
				/*To retrieve the held call automatically once the mo call is ended, if held is call is available */
				_vc_core_engine_status_set_end_flag(pcall_core->pcall_engine, VC_RETREIVE_CALL_ON_MOCALL_END);

				voicecall_core_end_mo_call(pcall_core->pcall_engine);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "No Incoming/Outgoing calls, skipping BT request \n");
			}
		}
		break;
	case BT_AG_REQ_BUTTON_PRESSED:
		{
			if (voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
				return voicecall_core_process_hold_call(pcall_core);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "No connected calls, skipping BT request \n");
			}

		}
		break;
	case BT_AG_REQ_CALL_END:
		{
			voicecall_core_end_call(pcall_core);
		}
		break;
	case BT_AG_REQ_CONNECT_ERROR:
		{
			/*To change path , reset flags and to update UI, if this is conisdered as Bt disconnect */
			__vc_bt_handle_connectivity_event(pcall_core, FALSE);
		}
		break;
	case BT_AG_REQ_CALL_0_SEND:
		{
			if (TRUE == voicecall_core_is_any_call_exists(pcall_core->pcall_engine)) {
				voicecall_core_start_incall_ss(pcall_core, CALL_VC_CORE_SS_0);
			}
		}
		break;
	case BT_AG_REQ_CALL_1_SEND:
		{
			if (TRUE == voicecall_core_is_any_call_exists(pcall_core->pcall_engine)) {
				if (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "Incoming call Accept by BT \n");
					voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, TRUE);
				}
				voicecall_core_start_incall_ss(pcall_core, CALL_VC_CORE_SS_1);
			}
		}
		break;
	case BT_AG_REQ_CALL_2_SEND:
		{
			if (TRUE == voicecall_core_is_any_call_exists(pcall_core->pcall_engine)) {
				gboolean bactive_calls = FALSE;
				gboolean bheld_calls = FALSE;

				if (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
					voicecall_core_is_call_exists(pcall_core->pcall_engine, &bactive_calls, &bheld_calls);
					if (!((TRUE == bactive_calls) && (TRUE == bheld_calls))) {
						CALL_ENG_DEBUG(ENG_DEBUG, "Incoming call Accept by BT \n");
						voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, TRUE);
					}
				}
				voicecall_core_start_incall_ss(pcall_core, CALL_VC_CORE_SS_2);
			}
		}
		break;
	case BT_AG_REQ_CALL_3_SEND:
		{
			if (TRUE == voicecall_core_is_any_call_exists(pcall_core->pcall_engine)) {
				voicecall_core_start_incall_ss(pcall_core, CALL_VC_CORE_SS_3);
			}
		}
		break;
	case BT_AG_REQ_HEADSET_VOL:
		{
			int bt_vol_level = pbt_info->param2;
			int converted_vol_level;

			CALL_ENG_DEBUG(ENG_DEBUG, "BT volume Level: %d \n", bt_vol_level);

			if (voicecall_core_is_any_call_exists(pcall_core->pcall_engine)) {
				vc_engine_vol_resp_type event_data;

				converted_vol_level = __vc_bt_converted_bt_vol_to_voice_vol(bt_vol_level);

				memset(&event_data, 0, sizeof(event_data));
				event_data.vol_alert_type = VOICE_CALL_VOL_TYPE_HEADSET;
				event_data.vol_level = converted_vol_level;
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_GET_VOLUME_RESP_TO_UI, (void *)&event_data);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "NO Calls, Skipping BT Response \n");
			}
		}
		break;
	case BT_AG_REQ_SWITCH_TO_HEADSET:
	case BT_AG_REQ_SWITCH_TO_PHONE:
		{
			gboolean bswitch_to_headset = FALSE;
			int bt_error = pbt_info->param2;

#ifdef _NEW_SND_
			if(voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING) == TRUE) {
				b_user_rqst_path_change = TRUE;
			} else {
				b_user_rqst_path_change = FALSE;
			}
#endif

			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING, FALSE);

			/*Switch to response is received from BT, BT decision is fina, so reset the accept by BT */
			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, FALSE);

			CALL_ENG_DEBUG(ENG_DEBUG, "BT Error :%d \n", bt_error);
			if (0 == bt_error) {
				/*Audio Connection/Disconnection Success case */
				bswitch_to_headset = (BT_AG_REQ_SWITCH_TO_HEADSET == pbt_info->param1) ? TRUE : FALSE;
			} else {
				/*Audio Connection/Disconnection Error Case */
				if (BT_AG_RES_AUDIO_CONNECTION_ERROR == bt_error) {
					/*Connection Error, switch to phone */
					bswitch_to_headset = FALSE;
				} else if (BT_AG_RES_AUDIO_DISCONNECTION_ERROR == bt_error) {
					/*Disconnection Error, switch to headset */
					bswitch_to_headset = TRUE;
				} else {
					CALL_ENG_DEBUG(ENG_ERR, "Invalid BT Error: %d \n", bt_error);
				}

			}

			CALL_ENG_DEBUG(ENG_DEBUG, "bswitch_to_headset = %d \n", bswitch_to_headset);

			if ((TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) || (TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine))) {
				gboolean bupdate = FALSE;

				if (0 == bt_error) {
					/*Connection/Diconnection is sucess, don't send response to BT */
					bupdate = _vc_bt_switch_headset_path(pcall_core, bswitch_to_headset, FALSE);
					CALL_ENG_DEBUG(ENG_DEBUG, "bupdate = %d, bswitch_to_headset = %d \n", bupdate, bswitch_to_headset);

					vc_engine_headset_status_type event_data;
					memset(&event_data, 0, sizeof(event_data));
					event_data.bstatus = bswitch_to_headset;
					vcall_engine_send_event_to_client(VC_ENGINE_MSG_HEADSET_STATUS_TO_UI, (void *)&event_data);
				} else {
					/*Connection/Diconnection is NOT sucess, change path and send response to BT */
					bupdate = _vc_bt_switch_headset_path(pcall_core, bswitch_to_headset, TRUE);
				}

			} else {
				CALL_ENG_DEBUG(ENG_ERR, "No connected/outgoing calls, Skipping BT Request \n");
			}
		}
		break;
	case BT_AG_REQ_DTMF:
		{
			gboolean active_calls = FALSE;
			gboolean held_calls = FALSE;

			CALL_ENG_DEBUG(ENG_DEBUG, "Send DTMF(%s)\n", pbt_info->param4);
			if ((voicecall_core_is_call_exists(pcall_core->pcall_engine, &active_calls, &held_calls)) && (TRUE == active_calls)) {
				voicecall_core_send_dtmf(pcall_core, pbt_info->param4);
			}
		}
		break;

	case BT_AG_REQ_CALL_REDIAL:
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Action not defined for bt event: %d\n", pbt_info->param1);
		return FALSE;
		break;

	}

	return TRUE;
}

/**
 * This function sends response back to the bt application
 *
 * @return		void
 * @param[in]		pcall_core			Handle to voicecall core
 * @param[in]		bt_event				bluetooth event type
 * @param[in]		param1				user param1
 * @param[in]		param2				user param2
 */
void _vc_bt_send_response_to_bt(call_vc_core_state_t *pcall_core, int bt_event, int param1, gpointer param2)
{
	VOICECALL_RETURN_IF_FAIL(pcall_core != NULL);

	connectivity_bt_ag_param_info_t bt_response_info;
	CALL_ENG_DEBUG(ENG_DEBUG, "Event: %s(%d), param1=[%d] \n", gszbt_res_event[bt_event], bt_event, param1);

	/*Skip events, if Bluetooth is not connected */
/*	if((FALSE == pcall_core->bt_connected) && (BT_AG_RES_CALL_STATUS != bt_event))
	{
		CALL_ENG_DEBUG(ENG_DEBUG,"Bluetooth not connected , Not sending any responses to BT\n");
		return;
	}
*/
	/*Make BT Response Info */
	memset(&bt_response_info, 0, sizeof(connectivity_bt_ag_param_info_t));

	bt_response_info.param1 = bt_event;

	switch (bt_event) {
	case BT_AG_RES_CALL_STATUS:
	case BT_AG_RES_CALL_SWAPPED:
		{
			connectivity_bt_ag_call_status_info_t call_status_info[10];
			int mt_call_handle = -1;
			int mo_call_handle = -1;
			int connected_call_handle = -1;
			int active_group_member_num = 0;
			int held_group_member_num = 0;
			int index = 0;
			int i = 0;
			char temp_str[10] = { 0, };
			char result_str[256] = { 0, };

			/*Incoming Call */
			_vc_core_engine_status_get_call_handle_bytype(pcall_core->pcall_engine, VC_INCOMING_CALL, &mt_call_handle);
			if (mt_call_handle != -1) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Incoming Call Exists, call handle = %d \n", mt_call_handle);

				call_status_info[index].call_id = mt_call_handle;
				call_status_info[index].call_status = BT_AG_CALL_STATUS_INCOMING;
				index++;
				CALL_ENG_DEBUG(ENG_DEBUG, "total_call_member after mt= %d \n", index);
			}

			/*Outgoing Call */
			_vc_core_engine_status_get_call_handle_bytype(pcall_core->pcall_engine, VC_OUTGOING_CALL, &mo_call_handle);
			if (mo_call_handle != -1) {
				voicecall_call_state_t call_state;
				gboolean bvalid_call = TRUE;

				_vc_core_engine_status_get_call_state_byhandle(pcall_core->pcall_engine, mo_call_handle, &call_state);
				CALL_ENG_DEBUG(ENG_DEBUG, "call state: %d \n", call_state);

				switch (call_state) {
				case VC_CALL_STATE_OUTGOING_ALERT:
					{
						call_status_info[index].call_status = BT_AG_CALL_STATUS_ALERTING;
					}
					break;
				case VC_CALL_STATE_PREPARE_OUTGOING:
				case VC_CALL_STATE_OUTGOING:
				case VC_CALL_STATE_OUTGOING_ORIG:
					{
						call_status_info[index].call_status = BT_AG_CALL_STATUS_DIALLING;
					}
					break;
				default:	/*All Other states , donot consider for mocall */
					CALL_ENG_DEBUG(ENG_DEBUG, "mo call state: %d \n", call_state);
					bvalid_call = FALSE;
				}

				if (TRUE == bvalid_call) {
					call_status_info[index].call_id = mo_call_handle;
					index++;
					CALL_ENG_DEBUG(ENG_DEBUG, "total_call_member after mo= %d \n", index);
				}

			}

			/*Connected call */
			int act_grp_index = -1;
			int held_grp_index = -1;

			_vc_core_engine_group_get_group_indices(pcall_core->pcall_engine, &act_grp_index, &held_grp_index);

			CALL_ENG_DEBUG(ENG_DEBUG, "act_grp_index = %d, held_grp_index = %d\n", act_grp_index, held_grp_index);
			/*Active Connected Call */
			if (act_grp_index != -1) {
				_vc_core_engine_group_get_connected_member_count(pcall_core->pcall_engine, act_grp_index, &active_group_member_num);
				CALL_ENG_DEBUG(ENG_DEBUG, "active_group_connected_member_num = %d \n", active_group_member_num);
				for (i = 0; i < active_group_member_num; i++) {
					_vc_core_engine_group_get_call_handle_byposition(pcall_core->pcall_engine, act_grp_index, i, &connected_call_handle);
					if (connected_call_handle != -1) {
						call_status_info[index].call_id = connected_call_handle;
						call_status_info[index].call_status = BT_AG_CALL_STATUS_CONNECTED;
						index++;
					}
				}
				CALL_ENG_DEBUG(ENG_DEBUG, "total_call_member after active calls= %d \n", index);
			}

			/*Held Connected Call */
			if (held_grp_index != -1) {
				_vc_core_engine_group_get_connected_member_count(pcall_core->pcall_engine, held_grp_index, &held_group_member_num);

				CALL_ENG_DEBUG(ENG_DEBUG, "held_group_member_num = %d \n", held_group_member_num);
				for (i = 0; i < held_group_member_num; i++) {
					_vc_core_engine_group_get_call_handle_byposition(pcall_core->pcall_engine, held_grp_index, i, &connected_call_handle);
					CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle[%d] = %d \n", i, connected_call_handle);
					if (connected_call_handle != -1) {
						call_status_info[index].call_id = connected_call_handle;
						call_status_info[index].call_status = BT_AG_CALL_STATUS_HELD;
						index++;
					}
				}
				CALL_ENG_DEBUG(ENG_DEBUG, "total_call_member after held calls= %d \n", index);
			}

			CALL_ENG_DEBUG(ENG_DEBUG, "Total Calls = %d \n", index);
			bt_response_info.param2 = index;

			memset(result_str, 0x00, sizeof(result_str));
			for (i = 0; i < index; i++) {
				memset(temp_str, 0x00, sizeof(temp_str));
				snprintf(temp_str, sizeof(temp_str), "%d.%d/", call_status_info[i].call_id, call_status_info[i].call_status);
				strncat(result_str, temp_str, sizeof(temp_str) - 1);
			}
			CALL_ENG_DEBUG(ENG_DEBUG, "The resultant string is %s\n", result_str);

			_vc_core_util_strcpy(bt_response_info.param4, sizeof(bt_response_info.param4), result_str);

			CALL_ENG_DEBUG(ENG_DEBUG, "Actual Data Passed is %s \n", bt_response_info.param4);
			CALL_ENG_DEBUG(ENG_DEBUG, "Actual Data Passed to BT \n");
			for (i = 0; i < index; i++) {
				CALL_ENG_DEBUG(ENG_DEBUG, "call id = %d \n", call_status_info[i].call_id);
				CALL_ENG_DEBUG(ENG_DEBUG, "call status= %d \n", call_status_info[i].call_status);
			}

			CALL_ENG_DEBUG(ENG_DEBUG, "Verification of BT data being sent\n");

		}
		break;
	case BT_AG_RES_CALL_INCOM:
		{
			/*Length of Incoming Call Number */
			if (NULL != param2) {
				CALL_ENG_DEBUG(ENG_DEBUG, "phone number=%s\n", (char *)param2);
				bt_response_info.param2 = strlen(param2);
				bt_response_info.param3 = param1;
				_vc_core_util_strcpy(bt_response_info.param4, VC_PHONE_NUMBER_LENGTH_MAX, param2);
			}
		}
		break;
	case BT_AG_RES_CALL_ORIG:
		{
			if (NULL != param2) {
				CALL_ENG_DEBUG(ENG_DEBUG, "phone number=%s\n", (char *)param2);
#ifdef _NEW_SND_
				switch (voicecall_snd_get_path_status(pcall_core->papp_snd)) {
					case VOICE_CALL_SND_PATH_RECEIVER:
					case VOICE_CALL_SND_PATH_SPEAKER:
					case VOICE_CALL_SND_PATH_EARJACK:
						bt_response_info.param2 = FALSE;
						break;
					case VOICE_CALL_SND_PATH_BT:
						bt_response_info.param2 = TRUE;
						break;
				}
#else
				bt_response_info.param2 = strlen(param2);
#endif
				bt_response_info.param3 = param1;
				_vc_core_util_strcpy(bt_response_info.param4, VC_PHONE_NUMBER_LENGTH_MAX, param2);
			}
		}
		break;
	case BT_AG_RES_CALL_REMOTE_RINGING:
		{
			/*Assign Call Handle */
			bt_response_info.param3 = param1;
		}
		break;
	case BT_AG_RES_CALL_CONNECT:
	case BT_AG_RES_CALL_END:
		{
#ifdef _NEW_SND_
			switch (voicecall_snd_get_path_status(pcall_core->papp_snd)) {
				case VOICE_CALL_SND_PATH_RECEIVER:
				case VOICE_CALL_SND_PATH_SPEAKER:
				case VOICE_CALL_SND_PATH_EARJACK:
					bt_response_info.param2 = FALSE;
					break;
				case VOICE_CALL_SND_PATH_BT:
					bt_response_info.param2 = TRUE;
					break;
			}
#endif
			/*Assign Call Handle */
			bt_response_info.param3 = param1;
		}
		break;
	case BT_AG_RES_SPK_GAIN:
		/*BT Volume Level */
		bt_response_info.param2 = param1;
		break;
	case BT_AG_RES_HEADSET_VOL:	/*Request For Current BT Volume Level */
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Requesting for Current BT Volume Level \n");
		}
		break;
	case BT_AG_RES_SWITCH_TO_HEADSET:
	case BT_AG_RES_SWITCH_TO_PHONE:
		break;
	case BT_AG_RES_CALL_HOLD:
	case BT_AG_RES_CALL_RETRIEVE:	/*Fall Through */
		bt_response_info.param3 = param1;
		break;
	case BT_AG_RES_CALL_JOINED:	/*Fall Through */
	default:
		break;
	}

	vc_engine_on_dbus_send_response_to_bt(bt_response_info);

	CALL_ENG_DEBUG(ENG_DEBUG, "bt response ended.\n");

}

/**
 * This function register bt callback.
 *
 * @return		TRUE if bt status is registered, FALSE otherwise
 * @param[in]		void
 */
gboolean _vc_bt_status_init(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	gboolean ret = FALSE;

	int bstatus = 0;
	bstatus = _vc_bt_get_bt_status();
	CALL_ENG_DEBUG(ENG_DEBUG, "bt_connected:[%d]\n", bstatus);
	__vc_bt_handle_connectivity_event(pcall_core, bstatus);

	ret = vconf_notify_key_changed(VCONFKEY_BT_DEVICE, (void *)_vc_bt_handle_phonestatus_bt_events, pcall_core);
	if (0 != ret) {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_notify_key_changed failed..[%d]\n", ret);
		return FALSE;
	}
	return TRUE;
}

/**
 * This function gets the blue tooth active status from the phone status server
 *
 * @return		TRUE - if BT is enabled, FALSE otherwise
 * @param[in]		none
 */
gboolean _vc_bt_get_bt_status(void)
{
	int bt_status = VCONFKEY_BT_DEVICE_NONE;
	gboolean ret = FALSE;

	ret = vconf_get_int(VCONFKEY_BT_DEVICE, &bt_status);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "bt_status = [0x%x] \n", bt_status);
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]\n", ret);
	}

	return (VCONFKEY_BT_DEVICE_HEADSET_CONNECTED == (bt_status & VCONFKEY_BT_DEVICE_HEADSET_CONNECTED)) ? TRUE : FALSE;
}

/**
 * This function gets the BT inband ringtone activate settings
 *
 * @return		TRUE - if BT inband ringtone activate settings is ON, FALSE otherwise
 * @param[in]		none
 */
gboolean _vc_bt_get_inband_ringtone_active(void)
{
	/* InBand_Ringtone_Active for vconf.. */
	return FALSE;
}

/**
 * This function handles the notifications sent by phone status server
 *
 * @return		TRUE -if notification is handled, FALSE otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		bt_status			Blue tooth status
 */
gboolean _vc_bt_handle_phonestatus_bt_events(keynode_t *node, call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(node != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	gboolean bt_conn_status = FALSE;
	int bt_status = vconf_keynode_get_int(node);

	CALL_ENG_DEBUG(ENG_DEBUG, "bt_status = [0x%x] \n", bt_status);

	/*set the sound status */
	bt_conn_status = (VCONFKEY_BT_DEVICE_HEADSET_CONNECTED == (bt_status & VCONFKEY_BT_DEVICE_HEADSET_CONNECTED)) ? TRUE : FALSE;
	CALL_ENG_DEBUG(ENG_DEBUG, "bt_conn_status = %d\n", bt_conn_status);

	__vc_bt_handle_connectivity_event(pcall_core, bt_conn_status);
	return TRUE;
}

/**
 * This function retreives volume level of headset
 *
 * @return		
 * @param[in]		pcall_core		Handle to voicecall core
 */
void _vc_bt_get_headset_volume(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_IF_FAIL(pcall_core != NULL);

	_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_HEADSET_VOL, 0, NULL);
}

/**
 * This function returns the BT connection status
 *
 * @return		TRUE if bt is connected, FALSE otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean _vc_bt_is_bt_connected(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	return pcall_core->bt_connected;
}

/**
 * This function returns the BT SCO status.(Synchronized Connection Oriented link)
 *
 * @return		TRUE if bt is connected, FALSE otherwise
 * @param[in]		void
 */
gboolean _vc_bt_get_bt_sco_status(void)
{
	gboolean sco_status = FALSE;
	gboolean ret = FALSE;

	ret = vconf_get_bool(VCONFKEY_BT_HEADSET_SCO, &sco_status);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "sco_status = [%d] \n", sco_status);
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]\n", ret);
	}

	return sco_status;
}

/**
 * This function processed bt handset event
 *
 * @return		TRUE if bt is connected, FALSE otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		bheadset			headset status 
 */
gboolean _vc_bt_process_bt_handset(call_vc_core_state_t *pcall_core, int bheadset)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "bheadset:[%d] \n", bheadset);
	if (bheadset) {
		if (FALSE == _vc_bt_get_bt_status()) {
			vc_engine_on_dbus_send_connect_to_bt();
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "BT is ON.. switch path to BT");
			_vc_bt_switch_headset_path(pcall_core, bheadset, TRUE);
		}
	} else {
		_vc_bt_switch_headset_path(pcall_core, bheadset, TRUE);
	}
	return TRUE;
}
