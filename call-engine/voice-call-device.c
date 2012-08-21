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


#include "voice-call-device.h"
#include "voice-call-dbus.h"
#include "voice-call-core.h"
#include "voice-call-sound.h"
#include "voice-call-bt.h"
#include "voice-call-core.h"
#include "voice-call-engine-msg.h"

#include "vc-core-callagent.h"
#include "vc-core-engine-types.h"
#include "vc-core-util.h"

#include <pmapi.h>
#include <sensor.h>

static gboolean __voicecall_dvc_earjack_status_cb(keynode_t *node, call_vc_core_state_t *pcall_core);
static gboolean __voicecall_dvc_earjackkey_status_cb(keynode_t *node, call_vc_core_state_t *pcall_core);

/**
 * This function handles earjack event
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		node				vconf node
 * @param[in]		pcall_core			Handle to voicecall core
 */
static gboolean __voicecall_dvc_earjack_status_cb(keynode_t *node, call_vc_core_state_t *pcall_core)
{
	int earjack_status;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	earjack_status = vconf_keynode_get_int(node);

	CALL_ENG_DEBUG(ENG_DEBUG, "Earjack Status: %d", earjack_status);

	vc_engine_headset_status_type event_data;

	/*Change path only if outgoing call or connected call exists */
	if ((TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine))
	|| (TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine))) {
		if (earjack_status != FALSE) {
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER_EARJACK);
			voicecall_snd_change_path(pcall_core->papp_snd);
			memset(&event_data, 0, sizeof(event_data));
			event_data.bstatus = earjack_status;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_EARJACK_STATUS_TO_UI, (void *)&event_data);
		} else {
			if (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_RECEIVER_EARJACK) {
				if (_vc_bt_is_bt_connected(pcall_core) == TRUE) {
					voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_BT);
					/*_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_HEADSET, -1, NULL);*/
					_vc_bt_request_switch_headset_path(pcall_core, TRUE);
				} else {
					voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER_EARJACK);
					voicecall_snd_change_path(pcall_core->papp_snd);
				}
				memset(&event_data, 0, sizeof(event_data));
				event_data.bstatus = earjack_status;
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_EARJACK_STATUS_TO_UI, (void *)&event_data);
			} else {
				CALL_ENG_DEBUG(ENG_DEBUG, "No path change");
			}
		}
	}
	return TRUE;
}

/**
 * This function handles earjack key event
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		node				vconf node
 * @param[in]		pcall_core			Handle to voicecall core
 */
static gboolean __voicecall_dvc_earjackkey_status_cb(keynode_t *node, call_vc_core_state_t *pcall_core)
{
	int key_value;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	key_value = vconf_keynode_get_int(node);

	CALL_ENG_DEBUG(ENG_DEBUG, "key_value: %d", key_value);

	if (key_value > 0) {
		if (voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
			voicecall_core_end_all_calls(pcall_core);
		} else if (voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER_EARJACK);
			voicecall_core_answer_call(pcall_core, FALSE);
		} else if (voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine)) {
			voicecall_core_cancel_call(pcall_core);
		}
	}

	return TRUE;
}

/**
 * This function initialize earjack event.
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core			Handle to voicecall core
 */
gboolean _voicecall_dvc_earjack_init(call_vc_core_state_t *pcall_core)
{
	vconf_notify_key_changed(VCONFKEY_SYSMAN_EARJACK, (void *)__voicecall_dvc_earjack_status_cb, pcall_core);
	vconf_notify_key_changed(VCONFKEY_SYSMAN_EARJACKKEY, (void *)__voicecall_dvc_earjackkey_status_cb, pcall_core);
	return TRUE;
}

void _voicecall_dvc_get_earjack_status(call_vc_core_state_t *pcall_core)
{
	int earjack_status = -1;
	if (!vconf_get_int(VCONFKEY_SYSMAN_EARJACK, &earjack_status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "earjack_status:[%d]", earjack_status);
		if (earjack_status == VCONFKEY_SYSMAN_EARJACK_REMOVED) {
			voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_RECEIVER_EARJACK, FALSE);
			CALL_ENG_DEBUG(ENG_DEBUG, "*****************VOICE_CALL_AUDIO_RECEIVER_EARJACK = FALSE **********");
		} else {
			voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_RECEIVER_EARJACK, TRUE);
			CALL_ENG_DEBUG(ENG_DEBUG, "*****************VOICE_CALL_AUDIO_RECEIVER_EARJACK = TRUE **********");
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..");
		voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_RECEIVER_EARJACK, FALSE);
	}
}

gboolean _voicecall_dvc_get_earjack_connected()
{
	int earjack_status = -1;
	if (!vconf_get_int(VCONFKEY_SYSMAN_EARJACK, &earjack_status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "earjack_status:[%d]", earjack_status);
		if (earjack_status == VCONFKEY_SYSMAN_EARJACK_REMOVED) {
			return FALSE;
		} else {
			return TRUE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..");
		return FALSE;
	}
}

void _voicecall_dvc_control_lcd_state(voicecall_lcd_control_t state)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "[%d]", state);
	switch (state) {
	case VC_LCD_OFF:
		pm_change_state(LCD_OFF);
		break;

	case VC_LCD_ON:
		pm_change_state(LCD_NORMAL);
		break;

	case VC_LCD_ON_LOCK:
		pm_lock_state(LCD_NORMAL, GOTO_STATE_NOW, 0);
		break;

	case VC_LCD_ON_UNLOCK:
		pm_unlock_state(LCD_NORMAL, PM_RESET_TIMER);
		break;

	default:
		break;
	}
}

