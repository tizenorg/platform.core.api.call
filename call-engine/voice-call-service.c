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


#include "voice-call-service.h"
#include "voice-call-dbus.h"
#include "vc-core-util.h"
#include "voice-call-core.h"
#include "voice-call-sound.h"
#include "voice-call-bt.h"

/**
 * This function on the loud speaker state
 *
 * @return		gboolean
 * @param[in]		pcall_core		Handle to voicecall core		
 */
gboolean voicecall_service_loudspeaker_on(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);
	
	gboolean bloud_speaker = FALSE;
	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	gboolean ret = FALSE;
	int total_call_member = -1;

	CALL_ENG_DEBUG(ENG_DEBUG, "\n");

	voicecall_core_get_total_call_member(pcall_core->pcall_engine, &total_call_member);
	if (total_call_member == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "There are not active calls hence it should not work \n");
		return FALSE;
	}

	/* Toggle the LoudSpeaker Status */
#ifdef _NEW_SND_
	if (voicecall_snd_get_path_status(pcall_core->papp_snd) != VOICE_CALL_SND_PATH_SPEAKER) {
		if (TRUE == _vc_bt_is_bt_connected(pcall_core) && 
		(voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_BT)) {
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_SPEAKER);
/*			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_PHONE, -1, NULL);*/
			_vc_bt_request_switch_headset_path(pcall_core, FALSE);
		} else {
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_SPEAKER);
			voicecall_snd_change_path(papp_snd);
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "loudspeacker is already on.\n");
		ret = FALSE;
	}
#else
	bloud_speaker = voicecall_snd_get_status(papp_snd, VOICE_CALL_AUDIO_SPEAKER);
	if (bloud_speaker == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Change to Speaker On. \n");

		/*If current Path is Set to BT Headset, Change Path from Headset to Phone */
		if (TRUE == _vc_bt_is_bt_connected(pcall_core)) {
			if (TRUE == voicecall_snd_get_status(papp_snd, VOICE_CALL_AUDIO_HEADSET)) {
				voicecall_snd_set_status(papp_snd, VOICE_CALL_AUDIO_HEADSET, FALSE);
/*				_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_PHONE, -1, NULL);*/
				_vc_bt_request_switch_headset_path(pcall_core, FALSE);
				voicecall_snd_change_path(papp_snd);
			}
		}

		voicecall_snd_set_status(papp_snd, VOICE_CALL_AUDIO_SPEAKER, TRUE);

		/* Change Audio Path according to the current status */
		voicecall_snd_change_path(papp_snd);
		ret = TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "loudspeacker is already on.\n");
		ret = FALSE;
	}
#endif
	return ret;
}

/**
 * This function off the loud speaker state
 *
 * @return		gboolean
 * @param[in]		pcall_core		Handle to voicecall core		
 */
gboolean voicecall_service_loudspeaker_off(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	gboolean bloud_speaker = FALSE;
	gboolean ret = FALSE;
	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	int total_call_member = -1;
	voicecall_core_get_total_call_member(pcall_core->pcall_engine, &total_call_member);
	if (total_call_member == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "There are not active calls hence it should not work \n");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "\n");

#ifdef _NEW_SND_
		if (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_SPEAKER) {
			if (TRUE == _vc_bt_is_bt_connected(pcall_core)) {
				voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_BT);
				/*_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_HEADSET, -1, NULL);*/
				_vc_bt_request_switch_headset_path(pcall_core, TRUE);
			} else {
				voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER);
				voicecall_snd_change_path(papp_snd);
			}
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "loudspeacker is already on.\n");
			ret = FALSE;
		}
#else

	/* Toggle the LoudSpeaker Status */
	bloud_speaker = voicecall_snd_get_status(papp_snd, VOICE_CALL_AUDIO_SPEAKER);
	if (bloud_speaker == TRUE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Change to Speaker Off. \n");

		voicecall_snd_set_status(papp_snd, VOICE_CALL_AUDIO_SPEAKER, FALSE);

		/* Change Audio Path according to the current status */
		voicecall_snd_change_path(papp_snd);

		ret = TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "loudspeacker is already off.\n");
		ret = FALSE;
	}
#endif

	return ret;
}

/**
 * This function is mute on
 *
 * @return		void
 * @param[in]		pcall_core		Handle to voicecall core		
 */

gboolean voicecall_service_mute_status_on(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	gboolean bactive_calls = FALSE;
	gboolean bheld_calls = FALSE;

	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	voicecall_core_is_call_exists(pcall_engine, &bactive_calls, &bheld_calls);

	if (FALSE == bactive_calls && TRUE == bheld_calls) {
		CALL_ENG_DEBUG(ENG_DEBUG, "nothing to do.\n");
		/*Mute button should not be handled if only held calls exists */
		return TRUE;
	}

	if (FALSE == papp_snd->bmute_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Setting Voice Audio Mute Status On. \n");
		voicecall_core_set_audio_mute_status(pcall_engine, TRUE);
		papp_snd->bmute_status = TRUE;
		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "mute status is already on.\n");
		return FALSE;
	}

}

/**
 * This function is mute off
 *
 * @return		void
 * @param[in]		pcall_core		Handle to voicecall core		
 */
gboolean voicecall_service_mute_status_off(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	gboolean bactive_calls = FALSE;
	gboolean bheld_calls = FALSE;

	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	voicecall_core_is_call_exists(pcall_engine, &bactive_calls, &bheld_calls);

	if (FALSE == bactive_calls && TRUE == bheld_calls) {
		CALL_ENG_DEBUG(ENG_DEBUG, "nothing to do.\n");
		/*Mute button should not be handled if only held calls exists */
		return TRUE;
	}

	if (TRUE == papp_snd->bmute_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Setting Voice Audio Mute Status Off. \n");
		voicecall_core_set_audio_mute_status(pcall_engine, FALSE);
		papp_snd->bmute_status = FALSE;
		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "mute status is already off.\n");
		return FALSE;
	}

}

/**
 * This function set volume level.
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core			Handle to voicecall core		
 * @param[in]		vol_alert_type			volume alert type
 * @param[in]		volume_level			volume level to be set
 */
gboolean voicecall_service_set_volume(call_vc_core_state_t *pcall_core, voicecall_snd_volume_alert_type_t vol_alert_type, int volume_level)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;

	voicecall_snd_set_volume(papp_snd, vol_alert_type, volume_level);

	return TRUE;
}
