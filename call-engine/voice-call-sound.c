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


#include "voice-call-sound.h"
#include "vc-core-util.h"
#include "vc-core-engine-types.h"
#include "voice-call-core.h"
#include "voice-call-bt.h"
#include "voice-call-service.h"
#include "voice-call-device.h"
#include <vconf.h>
#include <vconf-keys.h>

#include <mm_message.h>
#include <mm_player.h>
#include <media/sound_manager.h>
#include <mm_sound_private.h>
#include <mm_types.h>
#include <mm_error.h>

#include <mm_session.h>
#include <mm_session_private.h>

#include <devman_haptic.h>

#define VOICE_CALL_SND_INVALID_SND_HANDLE			-1	/**<Invalid Sound lib Handle*/
#define VOICE_CALL_SND_INVALID_PLAYER_HANDLE		0	/**<Invalid player lib Handle*/

#define VOICE_CALL_SND_VIBON_TIME_PERIOD		1500	/**< Vibration On Timer Interval */
#define VOICE_CALL_SND_VIBOFF_TIME_PERIOD		500		/**< Vibration Off Timer Interval  */
#define VOICE_CALL_SND_INCREMENT_TIMER_INTERVAL	2000	/**< Incremental Melody Timer Interval  */
#define VOICE_CALL_SND_VIB_THEN_MELODY_TIMER_INTERVAL	6000	/**< 6 sec  (3 * (VOICE_CALL_SND_VIBON_TIME_PERIOD + VOICE_CALL_SND_VIBOFF_TIME_PERIOD)) approximately*/
#define VOICE_CALL_SND_2ND_CALL_BEEP_INTERVAL	2500

#define VOICE_CALL_SND_DEFAULT_RINGTONE_PATH		MEDIADIR"/01_Minimal_tone.mp3"
#define VOICE_CALL_SND_SECOND_RINGTONE_PATH			MEDIADIR"/Call_WaitingTone.wav"

#define VOICE_CALL_SND_CONNECT_SIGNAL_PATH			MEDIADIR"/03_Call_connect.wav"
#define VOICE_CALL_SND_DISCONNECT_SIGNAL_PATH		MEDIADIR"/04_Call_disconnect.wav"
#define VOICE_CALL_SND_MINUTE_MINDER_SIGNAL_PATH		MEDIADIR"/03_Call_connect.wav"

#define VOICE_CALL_SND_USER_BUSY_SIGNAL_PATH		MEDIADIR"/Call_BusyTone.wav"
#define VOICE_CALL_SND_NW_CONGESTION_SIGNAL_PATH	MEDIADIR"/Call_NwCongestionTone.wav"
#define VOICE_CALL_SND_ERROR_SIGNAL_PATH				MEDIADIR"/Call_ErrorTone.wav"

/**
 * Enumeration for volume level of 10 levels
 */
typedef enum _voicecall_snd_mm_vol_level9_t {
	VOICE_CALL_SND_MM_VOLUME_LEVEL_0_9 = 0,			/**< volume level 1 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_1_9 = 16,		/**< volume level 2 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_2_9 = 28,		/**< volume level 3 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_3_9 = 39,		/**< volume level 4 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_4_9 = 50,		/**< volume level 5 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_5_9 = 62,		/**< volume level 6 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_6_9 = 73,		/**< volume level 7 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_7_9 = 82,		/**< volume level 8 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_8_9 = 91,		/**< volume level 9 of 10 */
	VOICE_CALL_SND_MM_VOLUME_LEVEL_9_9 = 100,		/**< volume level 10 of 10 */
} voicecall_snd_mm_vol_level9_t;

typedef struct __voicecall_snd_path_info_t {
	int phone_path;					/**< normal call state path */
	int alert_phone_path;				/**< alert sound to other party (record alert beep or answering machine guidance) */
	gboolean bmic_on;
} voicecall_snd_path_info_t;

/* Local Fucntion Declerations */
/**
 * This function prepares to start the melody to be played according to the given parameters
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		papp_snd			Handle to the Sound Manager
 * @param[in]		bis_increasing		TRUE - Increasing Melody, FALSE -Normal Melody
 * @param[in]		call_handle		Call Handle of the Incoming Call
 */
static gboolean __voicecall_snd_start_melody(voicecall_snd_mgr_t *papp_snd, gboolean bis_increasing, int call_handle);

 /**
* This function create the mm player
*
* @return	turns TRUE on success or FALSE on failure
* @param[in]		papp_snd		Handle to the Soudn Manager
* @param[in]		pPlayer		Handle to the mm player
* @param[in]		play_type		Play Type of the Sound
*/
static gboolean __voicecall_snd_create_player(voicecall_snd_mgr_t *papp_snd, MMHandleType * pPlayer, voicecall_snd_play_type_t play_type);

 /**
 * This function starts the vibration
 *
 * @return		Returns nothing
 * @param[in]		papp_snd			Handle to the Sound Manager
 */
static void __voicecall_snd_start_vibration(voicecall_snd_mgr_t *papp_snd);

/**
* This function stops the vibration
*
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		papp_snd			Handle to the Sound Manager
*/
static void __voicecall_snd_stop_vibration(voicecall_snd_mgr_t *papp_snd);

 /**
 * This function serves as the callbback for the MM Player
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		message		MM Message from the MM Player
 * @param[in]		data			Data send by the MM Player according to the Message
 * @param[in]		user_data	Local Data by the Caller
 */
static int __voicecall_snd_mmplayer_cb(int message, void *data, void *user_data);

static void __voicecall_snd_mmplayer_signal_cb(gpointer puser_data);

 /**
 * This function serves as the callback for the increasing melody periodic timer
 *
 * @return		Returns TRUE if sound cane be played or FALSE otherwise
 * @param[in]		data		Loca Data set by the caller
 */
static gboolean __voicecall_snd_increasing_melody_cb(void *data);

 /**
* This function plays the ringtone melody according to the settings
*
* @return		void
* @param[in]		papp_snd		Handle to the Sound Manager
* @param[in]		bis_increasing		TRUE - increasing melody, FALSE - normal
*/
static gboolean __voicecall_snd_play_melody(voicecall_snd_mgr_t *papp_snd, gboolean bis_increasing);

/**
* This function sets the mm player volume according to the settings volume level
*
* @return		void
* @param[in]		papp_snd		Handle to the Sound Manager
* @param[in]		bis_increasing		TRUE if incremntal melody type, FALSE otherwise
*/
static void __voicecall_snd_set_mm_volume(voicecall_snd_mgr_t *papp_snd, int bis_increasing);

/**
 * This function retreives the tapi sound path to be used according to the current status
 *
 * @return		void
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[out]	voice_snd_path		Tapi Sound Path
 */
static void __voicecall_snd_get_voice_path(voicecall_snd_mgr_t *papp_snd, int *voice_snd_path);

/*Function Defintions*/
/**
 * This function initializes the sound functionalties required by the Application
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[out]	papp_snd			Handle to the Sound Manager
 */
gboolean voicecall_snd_init(void *pcall_core, voicecall_snd_mgr_t **papp_snd)
{
	voicecall_snd_mgr_t *psnd_mgr = NULL;

	CALL_ENG_DEBUG(ENG_DEBUG, "Sound CM Test");

	psnd_mgr = (voicecall_snd_mgr_t *)calloc(1, sizeof(voicecall_snd_mgr_t));

	if (psnd_mgr == NULL) {
		CALL_ENG_DEBUG(ENG_ERR, "Memory Allocation Failed");
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "psnd_mgr alloctated memory:[%d]", sizeof(voicecall_snd_mgr_t));

	/*Store  voice call Handle */
	psnd_mgr->pcall_core = pcall_core;
	psnd_mgr->current_snd_path = VOICE_CALL_SND_PATH_RECEIVER_EARJACK;
	psnd_mgr->old_snd_path = VOICE_CALL_SND_PATH_NONE;
	psnd_mgr->bsound_cm_state = FALSE;

	psnd_mgr->pmm_player = VOICE_CALL_SND_INVALID_PLAYER_HANDLE;
	psnd_mgr->pmm_signal_player = VOICE_CALL_SND_INVALID_SND_HANDLE;

	psnd_mgr->mmfsoundplay_handle = VOICE_CALL_SND_INVALID_SND_HANDLE;
	psnd_mgr->mmfalternateplay_handle = VOICE_CALL_SND_INVALID_SND_HANDLE;

	psnd_mgr->psignal_play_end_cb = NULL;
	psnd_mgr->psignal_play_end_cb_data = NULL;

	/*Return the created Sound Manager */
	*papp_snd = psnd_mgr;
	return TRUE;

}

void __voicecall_snd_alternate_sound_cb(void *puser_data);
void __voicecall_snd_play_alternate_sound(voicecall_snd_mgr_t *papp_snd);
gboolean __voicecall_snd_stop_alternate_sound(voicecall_snd_mgr_t *papp_snd);

gboolean __voicecall_snd_alternate_sound_idle_cb(void *puser_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)puser_data;
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (TRUE == papp_snd->balternate_play) {
		if ((FALSE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine))
		    && (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine))) {
			/* case : during 2nd incoming call, connected call is cleared. so, just 1 incoming call case... */
			CALL_ENG_DEBUG(ENG_DEBUG, "2nd incoming -> just single incoming call.");
			voicecall_snd_prepare_alert(papp_snd, papp_snd->incoming_call_handle);
			sound_manager_call_session_set_mode(papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_RINGTONE);
			voicecall_snd_play_alert(papp_snd);
		} else {
			__voicecall_snd_play_alternate_sound(papp_snd);
		}
	}

	return FALSE;
}

gboolean __voicecall_snd_alternate_play_timeout_cb(gpointer pdata)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)pdata;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	if (papp_snd->balternate_play == TRUE) {
		__voicecall_snd_play_alternate_sound(papp_snd);
	}

	return FALSE;
}

void __voicecall_snd_play_alternate_sound(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	if (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Incoming call is there!Play Alternate Sound");

		int mmf_error = -1;

		char ring_tone[VOICE_CALL_SND_RINGTONE_PATH_LEN];
		snprintf(ring_tone, sizeof(ring_tone), "file://%s", VOICE_CALL_SND_SECOND_RINGTONE_PATH);

		sound_manager_call_session_set_mode(pcall_core->papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_MEDIA);

		CALL_ENG_DEBUG(ENG_DEBUG, "Call mm_sound_play_sound to play alternate ringtonen");
		mmf_error = mm_sound_play_sound(VOICE_CALL_SND_SECOND_RINGTONE_PATH, VOLUME_TYPE_CALL, __voicecall_snd_alternate_sound_cb, papp_snd, &papp_snd->mmfalternateplay_handle);

		if (MM_ERROR_NONE == mmf_error) {
			papp_snd->balternate_play = TRUE;
			papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_PLAY;
			CALL_ENG_DEBUG(ENG_DEBUG, "Alternate Sound Play Called,papp_snd->balternate_play=%d", papp_snd->balternate_play);
		} else {
			papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOP;
			papp_snd->balternate_play = FALSE;
			CALL_ENG_DEBUG(ENG_DEBUG, "mmf_error = [0x%x]", mmf_error);
		}
		CALL_ENG_DEBUG(ENG_DEBUG, "End of Alternate Sound!");
	}
}

gboolean __voicecall_snd_stop_alternate_sound(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, " papp_snd->balternate_play = %d", papp_snd->balternate_play);
	if (TRUE == papp_snd->balternate_play) {
		/*Only Stop if it is in Play State */
		if (VOICE_CALL_SND_STATUS_PLAY == papp_snd->ringtone_sound_status) {
			int error = 0;
			error = mm_sound_stop_sound(papp_snd->mmfalternateplay_handle);
			papp_snd->mmfalternateplay_handle = -1;
			papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOP;
			CALL_ENG_DEBUG(ENG_ERR, "Alternate Ringtone Stopeed,Error Code: [0x%x]", error);
		}
		papp_snd->balternate_play = FALSE;
		return TRUE;
	}
	return FALSE;
}

void __voicecall_snd_alternate_sound_cb(void *puser_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)puser_data;
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->balternate_play= %d", papp_snd->balternate_play);

	papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOPPED;

	/*If connected call exists then change the audio path */
	if ((TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) || (TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine))) {
		voicecall_snd_change_path(papp_snd);
	}

	g_timeout_add(VOICE_CALL_SND_2ND_CALL_BEEP_INTERVAL, __voicecall_snd_alternate_sound_idle_cb, papp_snd);

}

void voicecall_snd_prepare_alert(voicecall_snd_mgr_t *papp_snd, int call_handle)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;
	int err_code = -1;

	papp_snd->incoming_call_handle = call_handle;

	/* Save the original volume level to reset */
	err_code = mm_sound_volume_get_value(VOLUME_TYPE_RINGTONE, (unsigned int *) &(papp_snd->org_ringtone_value));

	if (MM_ERROR_NONE != err_code) {
		CALL_ENG_DEBUG(ENG_ERR, "Get Volume Error: [0x%x]", err_code);
		return;
	}

	if (TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Alternate Sound Needs to be played");
		papp_snd->ringtone_sound_status = VOICE_CALL_AND_STATUS_ALTERNATE_PLAY;
		return;
	}

	/*
	   Always stop the alert before starting another
	   to make sure the previous alert is proerly closed.
	 */
	voicecall_snd_stop_alert(papp_snd);

	/*Initizlize Variables */
	papp_snd->pmm_player = VOICE_CALL_SND_INVALID_PLAYER_HANDLE;
	papp_snd->settings_sound_status = FALSE;
	papp_snd->settings_vib_status = FALSE;

	err_code = vconf_get_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &papp_snd->settings_sound_status);
	CALL_ENG_DEBUG(ENG_DEBUG, "cur_sound_status = %d, error_code = %d", papp_snd->settings_sound_status, err_code);

	err_code = vconf_get_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &papp_snd->settings_vib_status);
	CALL_ENG_DEBUG(ENG_DEBUG, "cur_vib_status = %d, error_code = %d", papp_snd->settings_vib_status, err_code);

	if (papp_snd->settings_sound_status == FALSE)
		papp_snd->bmute_play = TRUE;

	__voicecall_snd_start_melody(papp_snd, FALSE, call_handle);
}

void voicecall_snd_play_alert(voicecall_snd_mgr_t *papp_snd)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->ringtone_sound_status=%d", papp_snd->ringtone_sound_status);

	if (VOICE_CALL_AND_STATUS_ALTERNATE_PLAY == papp_snd->ringtone_sound_status) {
		if (TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Starting Alternate Sound");
			papp_snd->balternate_play_count = 0;
			__voicecall_snd_play_alternate_sound(papp_snd);
		}
	}

	if (VOICE_CALL_SND_STATUS_READY != papp_snd->ringtone_sound_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, " Invalid ringtone_sound_status: %d", papp_snd->ringtone_sound_status);
		return;
	}

	papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_PROCESSED;

	CALL_ENG_DEBUG(ENG_DEBUG, " [##### papp_snd->settings_sound_status #####] : %d", papp_snd->settings_sound_status);
	CALL_ENG_DEBUG(ENG_DEBUG, " [##### papp_snd->settings_vib_status #####] : %d", papp_snd->settings_vib_status);

	sound_manager_call_session_set_mode(papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_RINGTONE);
	__voicecall_snd_play_melody(papp_snd, FALSE);

	if (papp_snd->settings_vib_status) {
		__voicecall_snd_start_vibration(papp_snd);
	}
}

gboolean voicecall_snd_mute_alert(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	if (papp_snd->pmm_player != VOICE_CALL_SND_INVALID_PLAYER_HANDLE) {
		int ret_value = 0;

		ret_value = mm_player_set_mute(papp_snd->pmm_player, TRUE);
		CALL_ENG_DEBUG(ENG_DEBUG, "MM Set Mute Error code: [0x%x]", ret_value);
	}

	if (TRUE == papp_snd->bvibration) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Vibration is playing, stopping vibration");
		__voicecall_snd_stop_vibration(papp_snd);
	}

	/*Make Vibration than melody flag to FALSE, so melody will not be played when it is muted */
	papp_snd->bvibration_then_melody = FALSE;

	/*Make Increasing Melody flag to FALSE, so melody volume will not be increased when it is muted */
	papp_snd->bincreasingmelody = FALSE;

	return FALSE;
}

/**
 * This function stops the sound alert
 *
 * @return		void
 * @param[in]		papp_snd			Handle to Sound Manager
 */
void voicecall_snd_stop_alert(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	__voicecall_snd_stop_vibration(papp_snd);

	if (TRUE == __voicecall_snd_stop_alternate_sound(papp_snd)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Alternate sound stopped");
		return;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "pmm_player = %d", papp_snd->pmm_player);
	if (papp_snd->pmm_player != VOICE_CALL_SND_INVALID_PLAYER_HANDLE) {
		int ret_value = 0;
		MMPlayerStateType mmplayer_state = MM_PLAYER_STATE_NONE;

		/*Sound Stop requested by the Application */
		papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOP;

		mm_player_get_state(papp_snd->pmm_player, &mmplayer_state);

		CALL_ENG_DEBUG(ENG_DEBUG, "Callling mmplayer_stop, current mm state = %d, Player = %d", mmplayer_state, papp_snd->pmm_player);
		if (MM_PLAYER_STATE_PLAYING == mmplayer_state || MM_PLAYER_STATE_PAUSED == mmplayer_state) {
			ret_value = mm_player_stop(papp_snd->pmm_player);
			if (MM_ERROR_NONE != ret_value) {
				CALL_ENG_DEBUG(ENG_ERR, "mmplayer_stop failed: [0x%x]", ret_value);
			}
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "Callling mmplayer_unrealize, Player = %d", papp_snd->pmm_player);
		ret_value = mm_player_unrealize(papp_snd->pmm_player);
		if (MM_ERROR_NONE != ret_value) {
			CALL_ENG_DEBUG(ENG_ERR, "mmplayer_unrealize failed: [0x%x]", ret_value);
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "Callling mmplayer_destroy, Player = %d", papp_snd->pmm_player);
		ret_value = mm_player_destroy(papp_snd->pmm_player);
		if (MM_ERROR_NONE != ret_value) {
			CALL_ENG_DEBUG(ENG_ERR, "mmplayer_destroy failed: [0x%x]", ret_value);
		}
		papp_snd->pmm_player = VOICE_CALL_SND_INVALID_PLAYER_HANDLE;

		papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOPPED;

		ret_value = mm_sound_volume_set_value(VOLUME_TYPE_RINGTONE, papp_snd->org_ringtone_value);

		if (MM_ERROR_NONE != ret_value) {
			CALL_ENG_DEBUG(ENG_ERR, "Set Volume Error: [0x%x]", ret_value);
		}
	}

	/* Stop All periodic Timers */
	papp_snd->bvibration_then_melody = FALSE;
	papp_snd->bincreasingmelody = FALSE;

	/* Make the Current Sound Playing Call HAndle Invalid */
	papp_snd->current_playing_call_handle = -1;

	papp_snd->bmute_play = FALSE;

}

gboolean voicecall_snd_set_play_end_callback(voicecall_snd_mgr_t *papp_snd, voicecall_snd_callback psignal_play_end_cb, gpointer psignal_play_end_cb_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "signal_type = %d", papp_snd->signal_type);

	if (VOICE_CALL_SND_STATUS_NONE == papp_snd->signal_sound_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Signal Being Played");
		return FALSE;
	}

	papp_snd->psignal_play_end_cb = psignal_play_end_cb;
	papp_snd->psignal_play_end_cb_data = psignal_play_end_cb_data;

	return TRUE;
}

/**
 * This function plays call sound
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		papp_snd			Handle to Sound Manager
 */
gboolean voicecall_snd_is_signal_playing(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "Signal Sound Status : [%d]", papp_snd->signal_sound_status);
	if ((papp_snd->pmm_signal_player != VOICE_CALL_SND_INVALID_SND_HANDLE) && (VOICE_CALL_SND_STATUS_PLAY == papp_snd->signal_sound_status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Signal is playing");
		return TRUE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Signal is not playing");
	return FALSE;
}

gboolean voicecall_snd_play_signal(voicecall_snd_mgr_t *papp_snd, voicecall_snd_callback psignal_play_end_cb, gpointer psignal_play_end_cb_data)
{
	int ret_value = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "signal_type = %d", papp_snd->signal_type);

	if (VOICE_CALL_SIGNAL_NONE == papp_snd->signal_type) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Signal Type Assinged");
		return FALSE;
	}

	/*
	   Always stop the signal before playing another one
	   This is to make sure that previous signal sound is stopeed completely
	 */
	voicecall_snd_stop_signal(papp_snd);

	/*Set status, the signal play is being prepared */
	papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_READY;
	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->signal_sound_status = %d", papp_snd->signal_sound_status);

	papp_snd->psignal_play_end_cb = psignal_play_end_cb;
	papp_snd->psignal_play_end_cb_data = psignal_play_end_cb_data;

	if (TRUE == voicecall_snd_is_effect_playing(papp_snd)) {
		CALL_ENG_DEBUG(ENG_ERR, "Stopping effect tone to play signal");
		voicecall_snd_stop_effect_tone(papp_snd);
	}

	CALL_ENG_DEBUG(ENG_ERR, "Changing path to play signal");
	sound_manager_call_session_set_mode(papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_MEDIA);
	CALL_ENG_DEBUG(ENG_ERR, "Changing path to play signal Over");

	CALL_ENG_DEBUG(ENG_DEBUG, "signal_tone = %s", papp_snd->signal_tone);
	ret_value = mm_sound_play_sound(papp_snd->signal_tone, VOLUME_TYPE_CALL, __voicecall_snd_mmplayer_signal_cb, papp_snd, &papp_snd->pmm_signal_player);
	if (MM_ERROR_NONE != ret_value) {
		papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_NONE;
		CALL_ENG_DEBUG(ENG_ERR, "mm_sound_play_sound failed,Error: [0x%x]", ret_value);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->pmm_signal_player[%d]", papp_snd->pmm_signal_player);
		papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_PLAY;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Signal Play Started, Sound Status is: %d", papp_snd->signal_sound_status);

	return TRUE;
}

/**
 * This function stops the sound alert
 *
 * @return		void
 * @param[in]		papp_snd			Handle to Sound Manager
 */
void voicecall_snd_stop_signal(voicecall_snd_mgr_t *papp_snd)
{
	int ret_value = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "Sound Status: %d, papp_snd->pmm_signal_player(%d)", papp_snd->signal_sound_status, papp_snd->pmm_signal_player);
	if (papp_snd->pmm_signal_player != VOICE_CALL_SND_INVALID_SND_HANDLE) {
		if (VOICE_CALL_SND_STATUS_PLAY == papp_snd->signal_sound_status) {
			papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_STOPPED;

			CALL_ENG_DEBUG(ENG_DEBUG, "Stopping Signal Sound");
			ret_value = mm_sound_stop_sound(papp_snd->pmm_signal_player);
			if (MM_ERROR_NONE != ret_value) {
				CALL_ENG_DEBUG(ENG_ERR, "mm_sound_stop_sound failed: [0x%x]", ret_value);
			}
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "Sound Play Over / Already Stopped");
		}
		papp_snd->pmm_signal_player = VOICE_CALL_SND_INVALID_SND_HANDLE;
	}
	if (papp_snd->psignal_play_end_cb != NULL) {
		papp_snd->psignal_play_end_cb(papp_snd->psignal_play_end_cb_data);
		papp_snd->psignal_play_end_cb = NULL;
		papp_snd->psignal_play_end_cb_data = NULL;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Application Sound Status:%d", papp_snd->signal_sound_status);
}

/**
 * This function sets the end signal of the given end cause type
 *
 * @return		void
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		end_cause_type		Type of the end cause
 */
void voicecall_snd_set_signal_type(voicecall_snd_mgr_t *papp_snd, voicecall_snd_signal_type_t signal_type)
{
	char signal_tone[VOICE_CALL_SND_RINGTONE_PATH_LEN];

	CALL_ENG_DEBUG(ENG_DEBUG, "Signal Type: %d", signal_type);

	memset(signal_tone, 0, sizeof(signal_tone));

	papp_snd->signal_type = signal_type;
	switch (papp_snd->signal_type) {
	case VOICE_CALL_SIGNAL_USER_BUSY_TONE:
		{
			_vc_core_util_strcpy(signal_tone, VOICE_CALL_SND_RINGTONE_PATH_LEN, VOICE_CALL_SND_USER_BUSY_SIGNAL_PATH);
		}
		break;
	case VOICE_CALL_SIGNAL_WRONG_NUMBER_TONE:
		{
			_vc_core_util_strcpy(signal_tone, VOICE_CALL_SND_RINGTONE_PATH_LEN, VOICE_CALL_SND_ERROR_SIGNAL_PATH);
		}
		break;
	case VOICE_CALL_SIGNAL_CALL_FAIL_TONE:
	case VOICE_CALL_SIGNAL_NW_CONGESTION_TONE:
		{
			_vc_core_util_strcpy(signal_tone, VOICE_CALL_SND_RINGTONE_PATH_LEN, VOICE_CALL_SND_NW_CONGESTION_SIGNAL_PATH);
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_ERR, "Invalid Signal Type");
		break;
	}

	_vc_core_util_strcpy(papp_snd->signal_tone, sizeof(papp_snd->signal_tone), signal_tone);

}

static void __voicecall_snd_get_voice_path(voicecall_snd_mgr_t *papp_snd, int *voice_snd_path)
{
	voicecall_audio_path_t tmp_audio_path;

	CALL_ENG_DEBUG(ENG_DEBUG, "current path = %d", voicecall_snd_get_path_status(papp_snd));

	switch (voicecall_snd_get_path_status(papp_snd)) {
	case VOICE_CALL_SND_PATH_BT:
		{
			tmp_audio_path = VC_AUDIO_PATH_BLUETOOTH;
		}
		break;
	case VOICE_CALL_SND_PATH_SPEAKER:
		{
			tmp_audio_path = VC_AUDIO_PATH_SPK_PHONE;
		}
		break;
	case VOICE_CALL_SND_PATH_RECEIVER_EARJACK:
	default:
		{
			if (_voicecall_dvc_get_earjack_connected() == TRUE)
				tmp_audio_path = VC_AUDIO_PATH_HEADSET;
			else
				tmp_audio_path = VC_AUDIO_PATH_HANDSET;
		}
	}
	*voice_snd_path = tmp_audio_path;

	CALL_ENG_DEBUG(ENG_DEBUG, "voice_snd_path = %d", *voice_snd_path);

}

void voicecall_snd_change_mm_path(voicecall_snd_mgr_t *papp_snd)
{
	sound_route_e route = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->current_path_type = %d", papp_snd->current_path_type);

	switch (voicecall_snd_get_path_status(papp_snd)) {
	case VOICE_CALL_SND_PATH_BT:
		CALL_ENG_DEBUG(ENG_DEBUG, "route path : SOUND_ROUTE_INOUT_BLUETOOTH");
		route = SOUND_ROUTE_INOUT_BLUETOOTH;
		break;

	case VOICE_CALL_SND_PATH_SPEAKER:
		CALL_ENG_DEBUG(ENG_DEBUG, "route path : SOUND_ROUTE_IN_MIC_OUT_SPEAKER");
		route = SOUND_ROUTE_IN_MIC_OUT_SPEAKER;
		break;

	case VOICE_CALL_SND_PATH_RECEIVER_EARJACK:
	default:
		{
			if (_voicecall_dvc_get_earjack_connected() == TRUE) {
				if (sound_manager_is_route_available (MM_SOUND_ROUTE_INOUT_HEADSET)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "route path : MM_SOUND_ROUTE_INOUT_HEADSET");
					route = MM_SOUND_ROUTE_INOUT_HEADSET;
				} else if (sound_manager_is_route_available (SOUND_ROUTE_IN_MIC_OUT_HEADPHONE)) {
					route = SOUND_ROUTE_IN_MIC_OUT_HEADPHONE;
					CALL_ENG_DEBUG(ENG_DEBUG, "route path : SOUND_ROUTE_IN_MIC_OUT_HEADPHONE");
				} else {
					route = SOUND_ROUTE_IN_MIC_OUT_RECEIVER;
				}
			} else {
				CALL_ENG_DEBUG(ENG_DEBUG, "route path : SOUND_ROUTE_IN_MIC_OUT_RECEIVER");
				route = SOUND_ROUTE_IN_MIC_OUT_RECEIVER;
			}
		}
		break;
	}
	CALL_ENG_DEBUG(ENG_ERR, "route = 0x04X", route);

	if (papp_snd->bsound_cm_state == TRUE) {
		CALL_ENG_DEBUG(ENG_ERR, "sound_manager_set_active_route [0x%X]", route);
		sound_manager_set_active_route(route);

		CALL_ENG_KPI("mm_sound_set_path done");
		papp_snd->current_path_type = VOICE_CALL_MM_VOICE;
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "** we can't change mm path. check it. ****");
	}
}

/**
 * This function changes the sound path according to the current status
 *
 * @return		void
 * @param[in]		papp_snd			Handle to Sound Manager
 */
void voicecall_snd_change_path(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	CALL_ENG_KPI("voicecall_snd_change_path start");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	if (voicecall_snd_get_path_status(papp_snd) == VOICE_CALL_SND_PATH_BT) {
		int bt_sco_status = 0;

		bt_sco_status = _vc_bt_get_bt_sco_status();
		if (FALSE == bt_sco_status) {
			gboolean bevent_wait = FALSE;

			CALL_ENG_DEBUG(ENG_ERR, "BT Sco is OFF, request BT for path change");
			bevent_wait = voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING);
			if (FALSE == bevent_wait) {
				/*Request BT for change path to headset */
				_vc_bt_request_switch_headset_path(pcall_core, TRUE);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "bevent_wait = %d, waiting for BT Event", bevent_wait);
			}
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "BT SCO is open, Change the path to sync with BT Path");
			voicecall_snd_change_path_real(papp_snd);
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "new PATH is not BT.");
		gboolean bevent_wait = FALSE;

		bevent_wait = voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING);
		if (FALSE == bevent_wait) {
			/*Request BT for change path to headset */
			voicecall_snd_change_path_real(papp_snd);
		} else {
			CALL_ENG_DEBUG(ENG_ERR, "bevent_wait = %d, waiting for BT Event", bevent_wait);
		}
	}
	CALL_ENG_KPI("voicecall_snd_change_path done");
}

void voicecall_snd_change_path_real(voicecall_snd_mgr_t *papp_snd)
{
	sound_call_session_mode_e current_session_mode;
	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	/* Change the mm sound path */
	sound_manager_call_session_get_mode(papp_snd->psnd_session, &current_session_mode);

	CALL_ENG_DEBUG(ENG_ERR, "current session mode = %d..", current_session_mode);

	if (current_session_mode != SOUND_CALL_SESSION_MODE_VOICE) {
		sound_manager_call_session_set_mode(papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_VOICE);
	} else {
		voicecall_snd_change_mm_path(papp_snd);
	}
	voicecall_snd_change_modem_path(papp_snd);
}

void voicecall_snd_change_modem_path(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	int audio_path = VC_AUDIO_PATH_HANDSET;

	/* Change the tapi sound path */
	__voicecall_snd_get_voice_path(papp_snd, &audio_path);

	if (FALSE == voicecall_core_change_audio_path(pcall_core->pcall_engine, audio_path, papp_snd->bextra_volume_status)) {
		CALL_ENG_DEBUG(ENG_ERR, "TAPI Audio Change Path ERROR");
	}
}

/**
 * This function sets the status of the given call audio type
 *
 * @return		void
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		snd_audio_type		Type of the Device to be changed
 * @param[in]		status				Status, TRUE - Enable, FALSE -Disable
 */
void voicecall_snd_set_status(voicecall_snd_mgr_t *papp_snd, voicecall_snd_audio_type_t snd_audio_type, gboolean status)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "snd_audio_type:[%d], status:[%d]", snd_audio_type, status);

	/*if BT headset is connected , switch off loud speaker - it will be reflectd in the next sound path change */
	if (((VOICE_CALL_AUDIO_HEADSET == snd_audio_type) || (VOICE_CALL_AUDIO_RECEIVER_EARJACK == snd_audio_type)) && (TRUE == status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "loud speaker status is to be FALSE");
		papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_SPEAKER] = FALSE;
	}
	papp_snd->bcall_audio_status[snd_audio_type] = status;
}

/**
 * This function returns the current status of the given call audio type
 *
 * @return		Returns TRUE if given call audio type is enables or FALSE otherwise
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		snd_audio_type	Type of the Device to be changed
 */
gboolean voicecall_snd_get_status(voicecall_snd_mgr_t *papp_snd, voicecall_snd_audio_type_t snd_audio_type)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "Status[%d] = %d", snd_audio_type, papp_snd->bcall_audio_status[snd_audio_type]);
	return papp_snd->bcall_audio_status[snd_audio_type];
}

/**
 * This function sets the status of the given call audio type
 *
 * @return		void
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		snd_audio_type		Type of the Device to be changed
 * @param[in]		status				Status, TRUE - Enable, FALSE -Disable
 */
void voicecall_snd_set_path_status(voicecall_snd_mgr_t *papp_snd, voicecall_snd_path_t path)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "current path:[%d], new path:[%d]", papp_snd->current_snd_path, path);

	papp_snd->old_snd_path = papp_snd->current_snd_path;
	papp_snd->current_snd_path = path;
}

voicecall_snd_path_t voicecall_snd_get_path_status(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "old path:[%d], current path:[%d]", papp_snd->old_snd_path, papp_snd->current_snd_path);

	return papp_snd->current_snd_path;
}

/**
* This function sets the volume level for the given volume alert type
*
* @return		void
* @param[in]	papp_snd		Handle to Sound Manager
* @param[in]	vol_alert_type	volume alert type #voicecall_snd_volume_alert_type_t
* @param[in]	volume_level	volume level to be set
*/
void voicecall_snd_set_volume(voicecall_snd_mgr_t *papp_snd, voicecall_snd_volume_alert_type_t vol_alert_type, int volume_level)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "volume_level = %d", volume_level);
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	switch (vol_alert_type) {
	case VOICE_CALL_VOL_TYPE_RINGTONE:
		{
			int ret_value = 0;
			int volume;

			if (VOICE_CALL_SND_INVALID_PLAYER_HANDLE == papp_snd->pmm_player) {
				CALL_ENG_DEBUG(ENG_ERR, "Invalid MM Player Handle");
				return;
			}

			/* Make Increasing Melody flag to FALSE, so melody volume will not be increased when volume it adjusted by user during increasing melody */
			papp_snd->bincreasingmelody = FALSE;

			volume = volume_level;

			ret_value = mm_sound_volume_set_value(VOLUME_TYPE_RINGTONE, volume);
			if (MM_ERROR_NONE != ret_value) {
				CALL_ENG_DEBUG(ENG_ERR, "mmplayer_set_volume failed Error: [0x%x]", ret_value);
			}
		}
		break;
	case VOICE_CALL_VOL_TYPE_VOICE:
		{
			int incall_vol_level = 0;
			int audio_path = 0;

			if (VOICE_CALL_VOL_LEVEL_1 >= volume_level) {
				incall_vol_level = VOICE_CALL_VOL_LEVEL_1;
			} else if (VOICE_CALL_VOL_LEVEL_6 < volume_level) {
				incall_vol_level = VOICE_CALL_VOL_LEVEL_6;
			} else {
				incall_vol_level = volume_level;
			}

			__voicecall_snd_get_voice_path(papp_snd, &audio_path);

			_vc_core_util_set_call_volume(volume_level);

			/* MODEM want to get volume level as 0~5, not a 1~6. So pass -1 value */
			if (FALSE == voicecall_core_set_audio_volume(pcall_core->pcall_engine, audio_path, (incall_vol_level-1))) {
				CALL_ENG_DEBUG(ENG_DEBUG, "voicecall_doc_set_audio_volume failed");
			}
		}
		break;
	case VOICE_CALL_VOL_TYPE_HEADSET:
		{
			int bt_vol_level = 0;
			if (TRUE == voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_VOL_CHNGD_BYBT)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "BT Requested Volume flag is enabled, not sending response");
				voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_VOL_CHNGD_BYBT, FALSE);
			} else {
				bt_vol_level = volume_level;
				CALL_ENG_DEBUG(ENG_DEBUG, "bt_vol_level = %d", bt_vol_level);

				_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SPK_GAIN, bt_vol_level, NULL);
			}

		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "No Actions Defined for the volume alert type: %d", vol_alert_type);

	}
}

/**
 * This function retreives the volume according to the given volume alert type
 *
 * @return		current volume level
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		vol_alert_type		volume alert type #voicecall_snd_volume_alert_type_t
 */
int voicecall_snd_get_volume(voicecall_snd_mgr_t *papp_snd, voicecall_snd_volume_alert_type_t vol_alert_type)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	switch (vol_alert_type) {
	case VOICE_CALL_VOL_TYPE_RINGTONE:
		{
			int ret_val = 0;
			unsigned int volume = 0;

			ret_val = mm_sound_volume_get_value(VOLUME_TYPE_RINGTONE, &volume);
			if (MM_ERROR_NONE != ret_val) {
				CALL_ENG_DEBUG(ENG_DEBUG, "ret_val = [0x%x]", ret_val);
				return VC_INVALID_VOLUME;
			}
			CALL_ENG_DEBUG(ENG_DEBUG, "MM Volume Level : %d", volume);

			return volume;

		}
		break;
	case VOICE_CALL_VOL_TYPE_VOICE:
		{
			return _vc_core_util_get_call_volume();
		}
		break;
	case VOICE_CALL_VOL_TYPE_HEADSET:
		{
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_HEADSET_VOL, 0, NULL);
			return VC_INVALID_VOLUME;	/*Return Invalid Volume level as default */
		}
		break;
	default:
		return VC_INVALID_VOLUME;	/*Return Invalid Volume level as default */
	}

	return VC_INVALID_VOLUME;
}

gboolean voicecall_snd_get_alternate_play(voicecall_snd_mgr_t *papp_snd)
{
	return papp_snd->balternate_play;
}

static gboolean __voicecall_snd_start_melody(voicecall_snd_mgr_t *papp_snd, gboolean bis_increasing, int call_handle)
{
	char *setting_file_path = NULL;
	char ringtone_path[VOICE_CALL_SND_RINGTONE_PATH_LEN] = { 0, };

	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle = %d", call_handle);

	CALL_ENG_DEBUG(ENG_DEBUG, "Contact ring_tone_path = [%s]", papp_snd->ring_tone);

	if (strlen(papp_snd->ring_tone) <= 0) {
		setting_file_path = vconf_get_str(VCONFKEY_SETAPPL_CALL_RINGTONE_PATH_STR);
#ifdef DRM_USED
		if ((setting_file_path == NULL) || (strlen(setting_file_path) <= 0) ||
			(FALSE == g_file_test(setting_file_path, G_FILE_TEST_EXISTS)) ||
			(FALSE == voicecall_util_is_valid_drm_file(setting_file_path, DRM_PERMISSION_PLAY)))
#else
		if ((setting_file_path == NULL) || (strlen(setting_file_path) <= 0) ||
		(FALSE == g_file_test(setting_file_path, G_FILE_TEST_EXISTS)))
#endif
		{
			CALL_ENG_DEBUG(ENG_ERR, "setting ring tone path is invalid : [%s]", setting_file_path);
			/*snprintf(ringtone_path, sizeof(ringtone_path), "file://%s", VOICE_CALL_SND_DEFAULT_RINGTONE_PATH);*/
			return FALSE;
		} else {
			snprintf(ringtone_path, sizeof(ringtone_path), "file://%s", setting_file_path);

		}
		CALL_ENG_DEBUG(ENG_DEBUG, "Ringtone From Settings : %s", ringtone_path);
		_vc_core_util_strcpy(papp_snd->ring_tone, sizeof(papp_snd->ring_tone), ringtone_path);
	}

	/* Create MM Player */
	papp_snd->pmm_player = VOICE_CALL_SND_INVALID_PLAYER_HANDLE;
	__voicecall_snd_create_player(papp_snd, &papp_snd->pmm_player, VOICE_CALL_PLAY_TYPE_RINGTONE);

	/*Set Volume */
	__voicecall_snd_set_mm_volume(papp_snd, bis_increasing);

	papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_READY;

	return TRUE;
}

static gboolean __voicecall_snd_create_player(voicecall_snd_mgr_t *papp_snd, MMHandleType * pPlayer, voicecall_snd_play_type_t play_type)
{
	int ret_value = 0;
	MMMessageCallback callback = NULL;
	char *mmf_error = NULL;

	CALL_ENG_DEBUG(ENG_DEBUG, "Start..");

	if (*pPlayer != VOICE_CALL_SND_INVALID_SND_HANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Player = %d", *pPlayer);
		mm_player_destroy(*pPlayer);
		*pPlayer = VOICE_CALL_SND_INVALID_SND_HANDLE;
	}

	ret_value = mm_player_create(pPlayer);
	if (MM_ERROR_NONE != ret_value || *pPlayer == VOICE_CALL_SND_INVALID_SND_HANDLE) {
		CALL_ENG_DEBUG(ENG_ERR, "mmplayer_create failed , Error:[0x%x]", ret_value);
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "*pPlayer: %d", *pPlayer);

	switch (play_type) {
		/*Set the MM Player Attributes according to the Display Priority Type */
	case VOICE_CALL_PLAY_TYPE_RINGTONE:
		{
			mm_player_set_attribute(*pPlayer, &mmf_error,
						"sound_volume_type", MM_SOUND_VOLUME_TYPE_RINGTONE,
						"profile_uri", papp_snd->ring_tone, VOICE_CALL_SND_RINGTONE_PATH_LEN,
						"profile_play_count", -1, "sound_route", MM_AUDIOROUTE_USE_EXTERNAL_SETTING, "sound_spk_out_only", TRUE, "sound_stop_when_unplugged", FALSE, NULL);
			callback = __voicecall_snd_mmplayer_cb;

		}
		break;
	default:
		break;
	}

	mm_player_set_message_callback(*pPlayer, callback, papp_snd);

	ret_value = mm_player_realize(*pPlayer);
	if (MM_ERROR_NONE != ret_value) {
		CALL_ENG_DEBUG(ENG_ERR, "mmplayer_realize failed , Error:[0x%x]", ret_value);
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Finish..");
	return TRUE;
}

static void __voicecall_snd_set_mm_volume(voicecall_snd_mgr_t *papp_snd, int bis_increasing)
{
#ifdef _VOL_LEVEL_FROM_SETTINGS_
	int ret_value = 0;
	int set_volume_level = VOICE_CALL_VOL_LEVEL_1;
	int volume_val;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	/*Get the settings current ringtone volume level and set to media player */
	if (FALSE == vconf_get_int(VCONFKEY_SETAPPL_CALL_RINGTONE_SOUND_VOLUME_INT, &set_volume_level)) {
		CALL_ENG_DEBUG(ENG_ERR, "settings read failed Error: %d", ret_value);
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Settings Volume Level = %d", set_volume_level);

	if (set_volume_level > 0) {
		if (TRUE == bis_increasing) {
			papp_snd->increment_melody_value = set_volume_level = 1;
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Increasing melody not set to VOICE_CALL_VOL_LEVEL_1");
	}

	volume_val = set_volume_level;
	ret_value = mm_sound_volume_set_value(VOLUME_TYPE_RINGTONE, volume_val);
	if (MM_ERROR_NONE != ret_value) {
		CALL_ENG_DEBUG(ENG_ERR, "Set Volume Error: [0x%x]", ret_value);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Final Volume Set to [%d]", volume_val);
	}
#endif
}

static gboolean __voicecall_snd_play_melody(voicecall_snd_mgr_t *papp_snd, gboolean bis_increasing)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (strlen(papp_snd->ring_tone) > 0) {
		int mm_error = MM_ERROR_NONE;

		mm_error = mm_player_start(papp_snd->pmm_player);
		if (MM_ERROR_NONE != mm_error) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_player_start failed,Error: [0x%x]", mm_error);
		} else {
			if (TRUE == bis_increasing) {
				papp_snd->bincreasingmelody = TRUE;
				g_timeout_add(VOICE_CALL_SND_INCREMENT_TIMER_INTERVAL, __voicecall_snd_increasing_melody_cb, (gpointer) papp_snd);
			}

			CALL_ENG_DEBUG(ENG_DEBUG, "ringtone_sound_status : %d", papp_snd->ringtone_sound_status);

			papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_PLAY;
			return TRUE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "Ringtone is empty(Problem)");
	}

	return FALSE;
}

static int __voicecall_snd_mmplayer_cb(int message, void *data, void *user_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)user_data;
	if (VOICE_CALL_SND_STATUS_PLAY != papp_snd->ringtone_sound_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Sound Play stopped by application, callback not handled");
		return FALSE;
	}

	switch (message) {
	case MM_MESSAGE_END_OF_STREAM:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Ringtone loop count is supported by MMF , need not restart the ringtone");
		}
		break;
	default:
		return FALSE;
		break;
	}
	return TRUE;
}

gboolean __voicecall_snd_mmplayer_signal_end_callback_idle_cb(gpointer pdata)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)pdata;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	/*Callback needs to be called, only when the tone is played completely and it is ended
	   if the signal is explictly stopped, don't call the user callback */
	if (papp_snd->psignal_play_end_cb != NULL) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Calling user callback");
		papp_snd->psignal_play_end_cb(papp_snd->psignal_play_end_cb_data);
		papp_snd->psignal_play_end_cb = NULL;
		papp_snd->psignal_play_end_cb_data = NULL;
	}

	return FALSE;
}

static void __voicecall_snd_mmplayer_signal_cb(gpointer puser_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)puser_data;

	CALL_ENG_DEBUG(ENG_DEBUG, "Sound Play Over");

	if (papp_snd->signal_sound_status != VOICE_CALL_SND_STATUS_PLAY) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Sound Play already stopped by application");
		return;
	}

	papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_STOPPED;

	__voicecall_snd_mmplayer_signal_end_callback_idle_cb(papp_snd);
}

static gboolean __voicecall_snd_increasing_melody_cb(void *data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)data;
	int ret_value = 0;
	int set_volume_level = VOICE_CALL_VOL_LEVEL_1;

	if (FALSE == papp_snd->bincreasingmelody) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Already Deactivated");
		return FALSE;
	}

	if ((set_volume_level != 0) && (papp_snd->increment_melody_value <= set_volume_level)) {
		int volume_val;

		papp_snd->increment_melody_value++;

		volume_val = papp_snd->increment_melody_value;
		ret_value = mm_sound_volume_set_value(VOLUME_TYPE_RINGTONE, volume_val);

		if (MM_ERROR_NONE != ret_value) {
			CALL_ENG_DEBUG(ENG_ERR, "Set Volume Error: [0x%x]", ret_value);
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "Increasing Melody Continuing, Current Increased Melody : %d", papp_snd->increment_melody_value);
		if (papp_snd->increment_melody_value >= set_volume_level) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Increasing Melody Ended");
			return FALSE;
		}

		return TRUE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Increasing Melody Ended");
	return FALSE;
}

static void __voicecall_snd_start_vibration(voicecall_snd_mgr_t *papp_snd)
{
	/*First Stop the previous Vibration and then start it again */

	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	int vib_type = -1;
	int vib_level = -1;
	int haptic_vib_type = -1;

	papp_snd->bvibration = FALSE;

	papp_snd->vibration_handle = device_haptic_open(DEV_IDX_0, 0);

	CALL_ENG_DEBUG(ENG_DEBUG, "srart vibration device_handle=%d", papp_snd->vibration_handle);

	if (papp_snd->vibration_handle < 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_open error");
		return;
	}

	if (vconf_get_int(VCONFKEY_SETAPPL_CALL_VIBRATION_PATTERN_INT, &vib_type)) {
		CALL_ENG_DEBUG(ENG_ERR, "VCONFKEY_SETAPPL_CALL_VIBRATION_PATTERN_INT vconf_get_bool failed.");
	}
	if (vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &vib_level)) {
		CALL_ENG_DEBUG(ENG_ERR, "VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT vconf_get_bool failed.");
	}

	if (vib_level > 5)
		vib_level = 5;

	switch (vib_type) {
	case SETTING_CALL_ALERT_VIB_TYPE1:
		haptic_vib_type = EFFCTVIBE_INCOMING_CALL01;
		break;
	case SETTING_CALL_ALERT_VIB_TYPE2:
		haptic_vib_type = EFFCTVIBE_INCOMING_CALL02;
		break;
	case SETTING_CALL_ALERT_VIB_TYPE3:
		haptic_vib_type = EFFCTVIBE_INCOMING_CALL03;
		break;
	default:
		haptic_vib_type = EFFCTVIBE_INCOMING_CALL01;
		break;
	}
	if (device_haptic_play_pattern(papp_snd->vibration_handle, haptic_vib_type, 255, vib_level)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_play_pattern error");
		return;
	}
	if (device_haptic_play_monotone(papp_snd->vibration_handle, 60000)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_play_monotone error");
		return;
	}

	papp_snd->bvibration = TRUE;

}

static void __voicecall_snd_stop_vibration(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (TRUE == papp_snd->bvibration) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Stopping Vibration , handle=%d", papp_snd->vibration_handle);

		if (device_haptic_stop_play(papp_snd->vibration_handle)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_play_stop error");
			return;
		}

		if (device_haptic_close(papp_snd->vibration_handle)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_close error");
			return;
		}

		papp_snd->vibration_handle = -1;

		papp_snd->bvibration = FALSE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "bvibration is not enabled.");
	}

}

/*
* This function is callback of mm_sound_route_add_callback
* @return	void
* @param[in]	data	call user data
*               policy	system audio route policy which has applied.
*/
void voicecall_snd_route_change_cb(void *data, system_audio_route_t policy)
{
	int error_code = 0;
	CALL_ENG_DEBUG(ENG_ERR, "System audio route policy has changed");
	if (policy != SYSTEM_AUDIO_ROUTE_POLICY_IGNORE_A2DP) {
		error_code = mm_sound_route_set_system_policy(SYSTEM_AUDIO_ROUTE_POLICY_IGNORE_A2DP);
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_sound_route_set_system_policy failed. error_code:[0x%x]", error_code);
		}
	}
}

/**
* This function unregisters the application with the sound conflict manager
*
* @return			void
* @param[in]		papp_snd		Handle to Sound Manager
*/
void voicecall_snd_register_cm(voicecall_snd_mgr_t *papp_snd)
{
	int error_code = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "current bsound_cm_state:[%d].", papp_snd->bsound_cm_state);

	if (FALSE == papp_snd->bsound_cm_state) {
		error_code = sound_manager_call_session_create(SOUND_SESSION_TYPE_CALL, &papp_snd->psnd_session);
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "sound_manager_call_session_create failed. error_code:[0x%x]", error_code);
			return;
		}
		papp_snd->bsound_cm_state = TRUE;
	}
	return;
}

/**
* This function unregisters the application from the sound conflict manager
*
* @return			void
* @param[in]		papp_snd		Handle to Sound Manager
*/
void voicecall_snd_unregister_cm(voicecall_snd_mgr_t *papp_snd)
{
	int error_code = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "current bsound_cm_state:[%d].", papp_snd->bsound_cm_state);

	if (TRUE == papp_snd->bsound_cm_state) {
		/*Reset the Path when the app is closed - safety code */
		CALL_ENG_DEBUG(ENG_DEBUG, "Sound Path reset to Default");

		papp_snd->bsound_cm_state = FALSE;

		error_code = sound_manager_call_session_destroy(papp_snd->psnd_session);
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "sound_manager_call_session_destroy failed. error_code:[0x%x]", error_code);
		}
	}
	/*Set to Defaults */
	voicecall_snd_set_to_defaults(papp_snd);
	return;
}

gboolean __voicecall_snd_effect_idle_cb(gpointer puser_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)puser_data;
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	papp_snd->effect_tone_status = VOICE_CALL_SND_STATUS_NONE;
	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->effect_tone_status = %d", papp_snd->effect_tone_status);

	/*If connected call exists then change the audio path */
	if ((TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) || (TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine))) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Connected call exists, changing path at the end of effect tone");

		CALL_ENG_DEBUG(ENG_ERR, "papp_snd->signal_sound_status = %d", papp_snd->signal_sound_status);
		/*Check the signal play status, if signal is being prepared / played, don't change the path */
		if (VOICE_CALL_SND_STATUS_NONE == papp_snd->signal_sound_status) {
			voicecall_snd_change_path(papp_snd);
		} else {
			CALL_ENG_DEBUG(ENG_ERR, "Signal is playing, skipping path change, it will be done at the end of signal");
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "non connected call");
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Over");
	return FALSE;
}

static void __voicecall_snd_effect_cb(gpointer puser_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)puser_data;

	CALL_ENG_DEBUG(ENG_DEBUG, "effect_tone_status = %d, Calling Idle", papp_snd->effect_tone_status);
	papp_snd->effect_tone_status = VOICE_CALL_SND_STATUS_STOPPED;

	__voicecall_snd_effect_idle_cb(papp_snd);
	CALL_ENG_DEBUG(ENG_DEBUG, "End : papp_snd->effect_tone_status = %d", papp_snd->effect_tone_status);
}

gboolean voicecall_snd_play_effect_tone(voicecall_snd_mgr_t *papp_snd, int effect_type)
{
	int error_code = -1;
	gboolean bzuhause = FALSE;
	gboolean bstatus = FALSE;
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	CALL_ENG_DEBUG(ENG_DEBUG, "effect type:[%d]", effect_type);

	switch (effect_type) {
	case VOICE_CALL_SND_EFFECT_CALL_CONNECT:
		{
			CALL_ENG_KPI("voicecall_core_get_zuhause start");
			voicecall_core_get_zuhause(pcall_core->pcall_engine, &bzuhause);
			CALL_ENG_KPI("voicecall_core_get_zuhause done");

			if (bzuhause == TRUE) {
				CALL_ENG_DEBUG(ENG_DEBUG, "It's zuhause area! don't play connect tone!");
				return FALSE;
			}

			CALL_ENG_KPI("get VCONFKEY_CISSAPPL_CALL_CONNECT_TONE_BOOL start");
			if (vconf_get_bool(VCONFKEY_CISSAPPL_CALL_CONNECT_TONE_BOOL, &bstatus)) {
				CALL_ENG_DEBUG(ENG_ERR, "vconf_get_bool failed.");
			}
			CALL_ENG_KPI("get VCONFKEY_CISSAPPL_CALL_CONNECT_TONE_BOOL done");

			if (!bstatus) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Connect Tone Settings not enabled");
				return FALSE;
			}

			/*First Reset the audio Path to PDA */
			sound_manager_call_session_set_mode(papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_MEDIA);

			error_code = mm_sound_play_sound(VOICE_CALL_SND_CONNECT_SIGNAL_PATH, VOLUME_TYPE_CALL, __voicecall_snd_effect_cb, papp_snd, &papp_snd->mmfsoundplay_handle);
		}
		break;
	case VOICE_CALL_SND_EFFECT_CALL_DISCONNECT:
		{

			if (vconf_get_bool(VCONFKEY_CISSAPPL_CALL_END_TONE_BOOL, &bstatus)) {
				CALL_ENG_DEBUG(ENG_ERR, "vconf_get_bool failed.");
			}

			if (!bstatus) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Disconnect Tone Settings not enabled");
				return FALSE;
			}

			sound_manager_call_session_set_mode(papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_MEDIA);

			error_code = mm_sound_play_sound(VOICE_CALL_SND_DISCONNECT_SIGNAL_PATH, VOLUME_TYPE_CALL, __voicecall_snd_effect_cb, papp_snd, &papp_snd->mmfsoundplay_handle);

		}
		break;

	case VOICE_CALL_SND_EFFECT_CALL_MINUTE_MINDER:
		{
			sound_manager_call_session_set_mode(papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_MEDIA);
			error_code = mm_sound_play_sound(VOICE_CALL_SND_MINUTE_MINDER_SIGNAL_PATH, VOLUME_TYPE_CALL, __voicecall_snd_effect_cb, papp_snd, &papp_snd->mmfsoundplay_handle);
		}
		break;

	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Invalid Effect Type: %d", effect_type);

	}

	if (MM_ERROR_NONE == error_code) {
		papp_snd->effect_tone_status = VOICE_CALL_SND_STATUS_PLAY;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "error code = [0x%x]", error_code);
	return (MM_ERROR_NONE == error_code) ? TRUE : FALSE;
}

gboolean voicecall_snd_is_effect_playing(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_ERR, "Effect tone status: %d", papp_snd->effect_tone_status);
	if (VOICE_CALL_SND_STATUS_PLAY == papp_snd->effect_tone_status) {
		return TRUE;
	}

	return FALSE;
}

void voicecall_snd_stop_effect_tone(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_ERR, "Effect tone status: %d", papp_snd->effect_tone_status);

	if (VOICE_CALL_SND_STATUS_PLAY == papp_snd->effect_tone_status) {
		if (MM_ERROR_NONE != mm_sound_stop_sound(papp_snd->mmfsoundplay_handle)) {
			CALL_ENG_DEBUG(ENG_ERR, "MM Stop Sound Failed");
		}

		papp_snd->effect_tone_status = VOICE_CALL_SND_STATUS_NONE;
		papp_snd->mmfsoundplay_handle = VOICE_CALL_SND_INVALID_SND_HANDLE;
	}
	CALL_ENG_DEBUG(ENG_ERR, "Effect tone status: %d", papp_snd->effect_tone_status);
}

void voicecall_snd_set_to_defaults(voicecall_snd_mgr_t *papp_snd)
{
	call_vc_core_state_t *pcall_core = NULL;

	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd = %p", papp_snd);

	/*Backup core handle */
	pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	/*Reset Sound Magr Data */
	memset(papp_snd, 0, sizeof(voicecall_snd_mgr_t));

	/*Re Assign core handle */
	papp_snd->pcall_core = pcall_core;

	/*Set to Defaults */
	papp_snd->pmm_player = VOICE_CALL_SND_INVALID_PLAYER_HANDLE;
	papp_snd->pmm_signal_player = VOICE_CALL_SND_INVALID_SND_HANDLE;

	papp_snd->mmfsoundplay_handle = VOICE_CALL_SND_INVALID_SND_HANDLE;
	papp_snd->mmfalternateplay_handle = VOICE_CALL_SND_INVALID_SND_HANDLE;

}
