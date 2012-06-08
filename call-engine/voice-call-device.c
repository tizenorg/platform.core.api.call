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

static int g_proximity_sensor_handle = -1;
static int g_proximity_sensor_state = -1;

static gboolean __voicecall_dvc_proximity_sensor_is_request(unsigned int type, sensor_event_data_t *event, void *data);
static void __voicecall_dvc_proximity_sensor_callback_func(unsigned int type, sensor_event_data_t *event, void *data);
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
	gboolean bRecieverPath = FALSE;
	int earjack_status;

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	earjack_status = vconf_keynode_get_int(node);

	CALL_ENG_DEBUG(ENG_DEBUG, "Earjack Status: %d \n", earjack_status);

	vc_engine_headset_status_type event_data;

	/*Change path only if outgoing call or connected call exists */
#ifdef _NEW_SND_
	if ((TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine)) 
	|| (TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine))) {
		if (earjack_status == TRUE) {
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_EARJACK);
			voicecall_snd_change_path(pcall_core->papp_snd);
			memset(&event_data, 0, sizeof(event_data));
			event_data.bstatus = earjack_status;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_EARJACK_STATUS_TO_UI, (void *)&event_data); 			
		} else {
			if (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_EARJACK) {
				if (_vc_bt_is_bt_connected(pcall_core) == TRUE) {
					voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_BT);				
					/*_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_HEADSET, -1, NULL);*/
					_vc_bt_request_switch_headset_path(pcall_core, TRUE);
				} else {
					voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER);				
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
#else
	if ((TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine)) || (TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine))) {
		memset(&event_data, 0, sizeof(event_data));
		event_data.bstatus = earjack_status;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_EARJACK_STATUS_TO_UI, (void *)&event_data);
	}
#endif
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

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	key_value = vconf_keynode_get_int(node);

	CALL_ENG_DEBUG(ENG_DEBUG, "key_value: %d \n", key_value);

	if (key_value > 0) {
		if (voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
			voicecall_core_end_all_calls(pcall_core);
		} else if (voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
#ifdef _NEW_SND_
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_EARJACK);
#endif
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
#ifdef _NEW_SND_
#else
	int earjack_status = -1;
	if (!vconf_get_int(VCONFKEY_SYSMAN_EARJACK, &earjack_status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "earjack_status:[%d]\n", earjack_status);
		if (earjack_status == VCONFKEY_SYSMAN_EARJACK_REMOVED) {
			voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_EARJACK, FALSE);
			CALL_ENG_DEBUG(ENG_DEBUG, "*****************VOICE_CALL_AUDIO_EARJACK = FALSE **********\n");
		} else {
			voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_EARJACK, TRUE);
			CALL_ENG_DEBUG(ENG_DEBUG, "*****************VOICE_CALL_AUDIO_EARJACK = TRUE **********\n");
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..\n");
		voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_EARJACK, FALSE);
	}
#endif

	vconf_notify_key_changed(VCONFKEY_SYSMAN_EARJACK, (void *)__voicecall_dvc_earjack_status_cb, pcall_core);
	vconf_notify_key_changed(VCONFKEY_SYSMAN_EARJACKKEY, (void *)__voicecall_dvc_earjackkey_status_cb, pcall_core);
	return TRUE;
}

void _voicecall_dvc_get_earjack_status(call_vc_core_state_t *pcall_core)
{
	int earjack_status = -1;
	if (!vconf_get_int(VCONFKEY_SYSMAN_EARJACK, &earjack_status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "earjack_status:[%d]\n", earjack_status);
		if (earjack_status == VCONFKEY_SYSMAN_EARJACK_REMOVED) {
			voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_EARJACK, FALSE);
			CALL_ENG_DEBUG(ENG_DEBUG, "*****************VOICE_CALL_AUDIO_EARJACK = FALSE **********\n");
		} else {
			voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_EARJACK, TRUE);
			CALL_ENG_DEBUG(ENG_DEBUG, "*****************VOICE_CALL_AUDIO_EARJACK = TRUE **********\n");
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..\n");
		voicecall_snd_set_status(pcall_core->papp_snd, VOICE_CALL_AUDIO_EARJACK, FALSE);
	}
}

gboolean _voicecall_dvc_get_earjack_connected()
{
	int earjack_status = -1;
	if (!vconf_get_int(VCONFKEY_SYSMAN_EARJACK, &earjack_status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "earjack_status:[%d]\n", earjack_status);
		if (earjack_status == VCONFKEY_SYSMAN_EARJACK_REMOVED) {
			return FALSE;
		} else {
			return TRUE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..\n");
		return FALSE;
	}
}

void _voicecall_dvc_control_lcd_state(voicecall_lcd_control_t state)
{
	CALL_ENG_DEBUG(ENG_DEBUG,"[%d]", state);
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
 
gboolean _voicecall_dvc_proximity_sensor_init(void *data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	int ret = -1;
	int handle = -1;
#ifdef _POLLING_PROXIMITY_SENSOR_
	event_condition_t my_cond;
#endif
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)data;
	sensor_data_t cur_sensor_data;

	if (g_proximity_sensor_handle >= 0) {
		CALL_ENG_DEBUG(ENG_WARN, "already initialized");
		return FALSE;
	}

	handle = sf_connect(PROXIMITY_SENSOR);
	if (handle < 0) {
		CALL_ENG_DEBUG(ENG_ERR, "sf_connect failed");
		return FALSE;
	}

#ifdef _POLLING_PROXIMITY_SENSOR_
	my_cond.cond_op = CONDITION_EQUAL;
	my_cond.cond_value1 = 200;
	ret = sf_register_event(handle, PROXIMITY_EVENT_CHANGE_STATE, &my_cond, __voicecall_dvc_proximity_sensor_callback_func, pcall_core);
#else
	ret = sf_register_event(handle, PROXIMITY_EVENT_CHANGE_STATE, NULL, __voicecall_dvc_proximity_sensor_callback_func, pcall_core);
#endif
	if (ret < 0) {
		CALL_ENG_DEBUG(ENG_ERR, "sf_register_event failed");
		return FALSE;
	}

	ret = sf_start(handle, 0);
	if (ret < 0) {
		CALL_ENG_DEBUG(ENG_ERR, "sensor_start fail");
		return FALSE;
	}

	ret = sf_get_data(handle, PROXIMITY_BASE_DATA_SET, &cur_sensor_data);
	if (ret < 0) {
		CALL_ENG_DEBUG(ENG_ERR, "sf_get_data fail");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "proximity_state:[%d]\n", cur_sensor_data.values[0]);
	if (cur_sensor_data.values[0] == PROXIMITY_STATE_NEAR) {
		if (__voicecall_dvc_proximity_sensor_is_required(pcall_core)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "PROXIMITY_STATE_NEAR");
			g_proximity_sensor_state = VCALL_SENSOR_NEAR;
			_voicecall_dvc_control_lcd_state(VC_LCD_OFF);
		}
	}

	g_proximity_sensor_handle = handle;
	CALL_ENG_DEBUG(ENG_DEBUG, "_voicecall_dvc_proximity_sensor_init done");
}

gboolean __voicecall_dvc_proximity_sensor_is_required(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = NULL;
	CALL_ENG_DEBUG(ENG_DEBUG, "");

	if (pcall_core == NULL) {
		CALL_ENG_DEBUG(ENG_ERR, "Wrong pointer for pcall_core");
		return FALSE;
	}

	pcall_engine = pcall_core->pcall_engine;
	if (pcall_engine == NULL) {
		CALL_ENG_DEBUG(ENG_ERR, "Wrong pointer for pcall_engine");
		return FALSE;
	}

	if (_vc_core_cm_isexists_incoming_call(&(pcall_engine->call_manager)) == TRUE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "we'll not excute sensor in case of incoming call");
		return FALSE;
	}

	if ((_vc_core_cm_isexists_connected_call(&(pcall_engine->call_manager)) == FALSE) &&
		(_vc_core_cm_isexits_outgoing_call(&(pcall_engine->call_manager)) == FALSE)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "we'll not excute sensor in case of NO call");
		return FALSE;
	}

	if (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_SPEAKER) {
		CALL_ENG_DEBUG(ENG_DEBUG, "we'll not excute sensor in case of speaker mode");
		return FALSE;
	}

	return TRUE;
}

gboolean _voicecall_dvc_proximity_sensor_deinit(void)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	int ret = -1;

	if (g_proximity_sensor_handle < 0) {
		CALL_ENG_DEBUG(ENG_WARN, "not initialized.");
		return FALSE;
	}

	ret = sf_unregister_event(g_proximity_sensor_handle, PROXIMITY_EVENT_CHANGE_STATE);
	if (ret < 0) {
		CALL_ENG_DEBUG(ENG_ERR, "sf_unregister_event failed");
	}
	ret = sf_stop(g_proximity_sensor_handle);
	if (ret < 0) {
		CALL_ENG_DEBUG(ENG_ERR, "sf_stop failed");
	}
	ret = sf_disconnect(g_proximity_sensor_handle);
	if (ret < 0) {
		CALL_ENG_DEBUG(ENG_ERR, "sf_disconnect failed");
	}

	g_proximity_sensor_handle = -1;
	return TRUE;
}

static void __voicecall_dvc_proximity_sensor_callback_func(unsigned int type, sensor_event_data_t *event, void *data)
{
	int *proxi_state;
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)data;
	voicecall_engine_t *pcall_engine = NULL;
	CALL_ENG_DEBUG(ENG_DEBUG, "");

	if (__voicecall_dvc_proximity_sensor_is_required(pcall_core) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Proximity sensor update is not required");
		return;
	}

	if (type != PROXIMITY_EVENT_CHANGE_STATE) {
		return;
	}

	proxi_state = (int *)(event->event_data);
	switch (*proxi_state) {
	case PROXIMITY_STATE_FAR:
		CALL_ENG_DEBUG(ENG_DEBUG, "PROXIMITY_STATE_FAR");
		g_proximity_sensor_state = VCALL_SENSOR_FAR;
		_voicecall_dvc_control_lcd_state(VC_LCD_ON);
		break;
	case PROXIMITY_STATE_NEAR:
		CALL_ENG_DEBUG(ENG_DEBUG, "PROXIMITY_STATE_NEAR");
		g_proximity_sensor_state = VCALL_SENSOR_NEAR;
		_voicecall_dvc_control_lcd_state(VC_LCD_OFF);
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "wrong data");
		break;
	}
}

int _voicecall_dvc_get_proximity_sensor_state(void)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "g_proximity_sensor_state(%d)", g_proximity_sensor_state);

	return g_proximity_sensor_state;
}
