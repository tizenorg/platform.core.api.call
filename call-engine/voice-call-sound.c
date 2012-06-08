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
#include <vconf.h>
#include <vconf-keys.h>

#include <mm_message.h>
#include <mm_player.h>
#include <mm_sound.h>
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

#ifdef VOICE_CALL_RINGTONE_ELEVATOR
#define VOICE_CALL_SND_INITIAL_VOL_LEVEL			1
#define VOICE_CALL_SND_RING_ELEVATOR_TIME_VAL		2000	/*3 Seconds Approx. */
#endif

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
* This function serves as the callback for vibration then melody timer
*
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		data			Local data set by the caller
*/
static gboolean __voicecall_snd_vib_then_melody_cb(void *data);

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
 * This function retreives the tapi sound path to be used according to the current status
 *
 * @return		void
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[out]	voice_snd_path		Tapi Sound Path
 */
static void __voicecall_snd_get_voice_path(voicecall_snd_mgr_t *papp_snd, int *voice_snd_path);

#ifdef VOICE_CALL_RINGTONE_ELEVATOR

static gboolean __voicecall_and_start_ring_elevator_cb(gpointer pdata);
static void __voicecall_snd_start_ring_elevator(voicecall_snd_mgr_t *papp_snd);
static void __voicecall_snd_cancel_ring_elevator(voicecall_snd_mgr_t *papp_snd);
#endif

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

	CALL_ENG_DEBUG(ENG_DEBUG, "Sound CM Test\n");

	psnd_mgr = (voicecall_snd_mgr_t *)calloc(1, sizeof(voicecall_snd_mgr_t));

	if (psnd_mgr == NULL) {
		CALL_ENG_DEBUG(ENG_ERR, "Memory Allocation Failed\n");
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "psnd_mgr alloctated memory:[%d]\n", sizeof(voicecall_snd_mgr_t));

	/*Store  voice call Handle */
	psnd_mgr->pcall_core = pcall_core;
#ifdef _NEW_SND_
	psnd_mgr->current_snd_path = VOICE_CALL_SND_PATH_RECEIVER;
	psnd_mgr->old_snd_path = VOICE_CALL_SND_PATH_NONE;
#endif

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

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	if (TRUE == papp_snd->balternate_play) {
		if ((FALSE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine))
		    && (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine))) {
			/* case : during 2nd incoming call, connected call is cleared. so, just 1 incoming call case... */
			CALL_ENG_DEBUG(ENG_DEBUG, "2nd incoming -> just single incoming call.\n");
			voicecall_snd_prepare_alert(papp_snd, papp_snd->incoming_call_handle);
			voicecall_snd_change_mm_path(papp_snd, VOICE_CALL_MM_RING_TONE);
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

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");
	if (papp_snd->balternate_play == TRUE) {
		__voicecall_snd_play_alternate_sound(papp_snd);
	}

	return FALSE;
}

void __voicecall_snd_play_alternate_sound(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	if (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Incoming call is there!Play Alternate Sound \n");

		int mmf_error = -1;

		char ring_tone[VOICE_CALL_SND_RINGTONE_PATH_LEN];
		snprintf(ring_tone, sizeof(ring_tone), "file://%s", VOICE_CALL_SND_SECOND_RINGTONE_PATH);

		voicecall_snd_change_mm_path(pcall_core->papp_snd, VOICE_CALL_MM_SECOND_CALL_TONE);

		CALL_ENG_DEBUG(ENG_DEBUG, "Call mm_sound_play_sound to play alternate ringtonen\n");
		mmf_error = mm_sound_play_sound(VOICE_CALL_SND_SECOND_RINGTONE_PATH, VOLUME_TYPE_CALL, __voicecall_snd_alternate_sound_cb, papp_snd, &papp_snd->mmfalternateplay_handle);

		if (MM_ERROR_NONE == mmf_error) {
			papp_snd->balternate_play = TRUE;
			papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_PLAY;
			CALL_ENG_DEBUG(ENG_DEBUG, "Alternate Sound Play Called,papp_snd->balternate_play=%d \n", papp_snd->balternate_play);
		} else {
			papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOP;
			papp_snd->balternate_play = FALSE;
			CALL_ENG_DEBUG(ENG_DEBUG, "mmf_error = [0x%x] \n", mmf_error);
		}
		CALL_ENG_DEBUG(ENG_DEBUG, "End of Alternate Sound!\n");
	}
}

gboolean __voicecall_snd_stop_alternate_sound(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, " papp_snd->balternate_play = %d\n", papp_snd->balternate_play);
	if (TRUE == papp_snd->balternate_play) {
		/*Only Stop if it is in Play State */
		if (VOICE_CALL_SND_STATUS_PLAY == papp_snd->ringtone_sound_status) {
			int error = 0;
			error = mm_sound_stop_sound(papp_snd->mmfalternateplay_handle);
			papp_snd->mmfalternateplay_handle = -1;
			papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOP;
			CALL_ENG_DEBUG(ENG_ERR, "Alternate Ringtone Stopeed,Error Code: [0x%x] \n", error);
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

	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->balternate_play= %d\n", papp_snd->balternate_play);

	papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOPPED;

	/*If connected call exists then change the audio path */
	if ((TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) || (TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine))) {
		voicecall_snd_change_path(papp_snd);
	}

	g_timeout_add(VOICE_CALL_SND_2ND_CALL_BEEP_INTERVAL, __voicecall_snd_alternate_sound_idle_cb, papp_snd);

}

void voicecall_snd_prepare_alert(voicecall_snd_mgr_t *papp_snd, int call_handle)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;
	int err_code = -1;

	papp_snd->incoming_call_handle = call_handle;

	if (TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Alternate Sound Needs to be played \n");
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
	CALL_ENG_DEBUG(ENG_DEBUG, "cur_sound_status = %d, error_code = %d\n", papp_snd->settings_sound_status, err_code);

	err_code = vconf_get_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &papp_snd->settings_vib_status);
	CALL_ENG_DEBUG(ENG_DEBUG, "cur_vib_status = %d, error_code = %d\n", papp_snd->settings_vib_status, err_code);

	if (papp_snd->settings_sound_status == FALSE)
		papp_snd->bmute_play = TRUE;

	__voicecall_snd_start_melody(papp_snd, FALSE, call_handle);

}

void voicecall_snd_play_alert(voicecall_snd_mgr_t *papp_snd)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->ringtone_sound_status=%d \n ", papp_snd->ringtone_sound_status);

	if (VOICE_CALL_AND_STATUS_ALTERNATE_PLAY == papp_snd->ringtone_sound_status) {
		if (TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Starting Alternate Sound \n");
			papp_snd->balternate_play_count = 0;
			__voicecall_snd_play_alternate_sound(papp_snd);
		}
	}

	if (VOICE_CALL_SND_STATUS_READY != papp_snd->ringtone_sound_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, " Invalid ringtone_sound_status: %d \n ", papp_snd->ringtone_sound_status);
		return;
	}

	papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_PROCESSED;

	CALL_ENG_DEBUG(ENG_DEBUG, " [##### papp_snd->settings_sound_status #####] : %d \n ", papp_snd->settings_sound_status);
	CALL_ENG_DEBUG(ENG_DEBUG, " [##### papp_snd->settings_vib_status #####] : %d \n ", papp_snd->settings_vib_status);

	if (papp_snd->settings_sound_status) {
		__voicecall_snd_play_melody(papp_snd, FALSE);
	} else if (papp_snd->bmute_play) {
		/*Change the path to the earjack headset and play the ringtone */
		voicecall_snd_change_mm_path(papp_snd, VOICE_CALL_MM_MUTE_PLAY);
		__voicecall_snd_play_melody(papp_snd, FALSE);
	}
	if (papp_snd->settings_vib_status) {
		__voicecall_snd_start_vibration(papp_snd);
	}
}

#ifdef VOICE_CALL_RINGTONE_ELEVATOR

gboolean __voicecall_and_start_ring_elevator_cb(gpointer pdata)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)pdata;
	int volume_level = VOICE_CALL_VOL_LEVEL_1;
	int ret_value = -1;

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	/*Get the settings current ringtone volume level and set to media player */
	ret_value = vconf_get_int(VCONFKEY_SETAPPL_PROFILE_CURRENT_CALL_VOLUME_INT, &volume_level);
	if (ret_value != 0) {
		CALL_ENG_DEBUG(ENG_ERR, "settings read failed Error: %d\n", ret_value);
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Settings Volume Level = %d\n", volume_level);

	if (papp_snd->pmm_player != VOICE_CALL_SND_INVALID_PLAYER_HANDLE) {
		ret_value = mm_sound_volume_set_value(VOLUME_TYPE_RINGTONE, volume_level);
		if (MM_ERROR_NONE != ret_value) {
			CALL_ENG_DEBUG(ENG_ERR, "Set Volume Error: [0x%x]\n", ret_value);
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "Invalid MM Plauer %d\n", papp_snd->pmm_player);
	}

	return FALSE;
}

void __voicecall_snd_start_ring_elevator(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	papp_snd->ring_elvator_timerid = g_timeout_add(VOICE_CALL_SND_RING_ELEVATOR_TIME_VAL, __voicecall_and_start_ring_elevator_cb, papp_snd);
}

void __voicecall_snd_cancel_ring_elevator(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");
	if (papp_snd->ring_elvator_timerid > 0) {
		g_source_remove(papp_snd->ring_elvator_timerid);
		papp_snd->ring_elvator_timerid = 0;
	}
}
#endif

gboolean voicecall_snd_mute_alert(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");
	if (papp_snd->pmm_player != VOICE_CALL_SND_INVALID_PLAYER_HANDLE) {
		int ret_value = 0;

		ret_value = mm_player_set_mute(papp_snd->pmm_player, TRUE);
		CALL_ENG_DEBUG(ENG_DEBUG, "MM Set Mute Error code: [0x%x] \n", ret_value);
	}

	if (TRUE == papp_snd->bvibration) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Vibration is playing, stopping vibration \n");
		__voicecall_snd_stop_vibration(papp_snd);
	}
#ifdef VOICE_CALL_RINGTONE_ELEVATOR
	__voicecall_snd_cancel_ring_elevator(papp_snd);
#endif

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

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	__voicecall_snd_stop_vibration(papp_snd);

	if (TRUE == __voicecall_snd_stop_alternate_sound(papp_snd)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Alternate sound stopped \n");
		return;
	}
#ifdef VOICE_CALL_RINGTONE_ELEVATOR
	/*Stop Ring Elevator */
	__voicecall_snd_cancel_ring_elevator(papp_snd);
#endif

	CALL_ENG_DEBUG(ENG_DEBUG, "pmm_player = %d \n", papp_snd->pmm_player);
	if (papp_snd->pmm_player != VOICE_CALL_SND_INVALID_PLAYER_HANDLE) {
		int ret_value = 0;
		MMPlayerStateType mmplayer_state = MM_PLAYER_STATE_NONE;

		/*Sound Stop requested by the Application */
		papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOP;

		mm_player_get_state(papp_snd->pmm_player, &mmplayer_state);

		CALL_ENG_DEBUG(ENG_DEBUG, "Callling mmplayer_stop, current mm state = %d, Player = %d\n", mmplayer_state, papp_snd->pmm_player);
		if (MM_PLAYER_STATE_PLAYING == mmplayer_state || MM_PLAYER_STATE_PAUSED == mmplayer_state) {
			ret_value = mm_player_stop(papp_snd->pmm_player);
			if (MM_ERROR_NONE != ret_value) {
				CALL_ENG_DEBUG(ENG_ERR, "mmplayer_stop failed: [0x%x]\n", ret_value);
			}
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "Callling mmplayer_unrealize, Player = %d\n", papp_snd->pmm_player);
		ret_value = mm_player_unrealize(papp_snd->pmm_player);
		if (MM_ERROR_NONE != ret_value) {
			CALL_ENG_DEBUG(ENG_ERR, "mmplayer_unrealize failed: [0x%x]\n", ret_value);
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "Callling mmplayer_destroy, Player = %d\n", papp_snd->pmm_player);
		ret_value = mm_player_destroy(papp_snd->pmm_player);
		if (MM_ERROR_NONE != ret_value) {
			CALL_ENG_DEBUG(ENG_ERR, "mmplayer_destroy failed: [0x%x]\n", ret_value);
		}
		papp_snd->pmm_player = VOICE_CALL_SND_INVALID_PLAYER_HANDLE;

		papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_STOPPED;
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
	CALL_ENG_DEBUG(ENG_DEBUG, "signal_type = %d\n", papp_snd->signal_type);

	if (VOICE_CALL_SND_STATUS_NONE == papp_snd->signal_sound_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Signal Being Played\n");
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
	CALL_ENG_DEBUG(ENG_DEBUG, "Signal Sound Status : [%d] \n", papp_snd->signal_sound_status);
	if ((papp_snd->pmm_signal_player != VOICE_CALL_SND_INVALID_SND_HANDLE) && (VOICE_CALL_SND_STATUS_PLAY == papp_snd->signal_sound_status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Signal is playing \n");
		return TRUE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Signal is not playing \n");
	return FALSE;
}

gboolean voicecall_snd_play_signal(voicecall_snd_mgr_t *papp_snd, voicecall_snd_callback psignal_play_end_cb, gpointer psignal_play_end_cb_data)
{
	int ret_value = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "signal_type = %d\n", papp_snd->signal_type);

	if (VOICE_CALL_SIGNAL_NONE == papp_snd->signal_type) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Signal Type Assinged\n");
		return FALSE;
	}

	/*
	   Always stop the signal before playing another one
	   This is to make sure that previous signal sound is stopeed completely
	 */
	voicecall_snd_stop_signal(papp_snd);

	/*Set status, the signal play is being prepared */
	papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_READY;
	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->signal_sound_status = %d\n", papp_snd->signal_sound_status);

	papp_snd->psignal_play_end_cb = psignal_play_end_cb;
	papp_snd->psignal_play_end_cb_data = psignal_play_end_cb_data;

	if (TRUE == voicecall_snd_is_effect_playing(papp_snd)) {
		CALL_ENG_DEBUG(ENG_ERR, "Stopping effect tone to play signal \n ");
		voicecall_snd_stop_effect_tone(papp_snd);
	}

	CALL_ENG_DEBUG(ENG_ERR, "Changing path to play signal\n ");
	voicecall_snd_change_mm_path(papp_snd, VOICE_CALL_MM_SIGNAL_TONE);
	CALL_ENG_DEBUG(ENG_ERR, "Changing path to play signal Over\n ");

	CALL_ENG_DEBUG(ENG_DEBUG, "signal_tone = %s\n", papp_snd->signal_tone);
	ret_value = mm_sound_play_sound(papp_snd->signal_tone, VOLUME_TYPE_CALL, __voicecall_snd_mmplayer_signal_cb, papp_snd, &papp_snd->pmm_signal_player);
	if (MM_ERROR_NONE != ret_value) {
		papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_NONE;
		CALL_ENG_DEBUG(ENG_ERR, "mm_sound_play_sound failed,Error: [0x%x]\n", ret_value);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->pmm_signal_player[%d]\n", papp_snd->pmm_signal_player);
		papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_PLAY;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Signal Play Started, Sound Status is: %d\n", papp_snd->signal_sound_status);

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

	CALL_ENG_DEBUG(ENG_DEBUG, "Sound Status: %d, papp_snd->pmm_signal_player(%d)\n", papp_snd->signal_sound_status, papp_snd->pmm_signal_player);
	if (papp_snd->pmm_signal_player != VOICE_CALL_SND_INVALID_SND_HANDLE) {
		if (VOICE_CALL_SND_STATUS_PLAY == papp_snd->signal_sound_status) {
			papp_snd->signal_sound_status = VOICE_CALL_SND_STATUS_STOPPED;

			CALL_ENG_DEBUG(ENG_DEBUG, "Stopping Signal Sound \n");
			ret_value = mm_sound_stop_sound(papp_snd->pmm_signal_player);
			if (MM_ERROR_NONE != ret_value) {
				CALL_ENG_DEBUG(ENG_ERR, "mm_sound_stop_sound failed: [0x%x]\n", ret_value);
			}
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "Sound Play Over / Already Stopped \n");
		}
		papp_snd->pmm_signal_player = VOICE_CALL_SND_INVALID_SND_HANDLE;
	}
	if (papp_snd->psignal_play_end_cb != NULL) {
		papp_snd->psignal_play_end_cb(papp_snd->psignal_play_end_cb_data);
		papp_snd->psignal_play_end_cb = NULL;
		papp_snd->psignal_play_end_cb_data = NULL;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Application Sound Status:%d \n", papp_snd->signal_sound_status);
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

	CALL_ENG_DEBUG(ENG_DEBUG, "Signal Type: %d\n", signal_type);

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
		CALL_ENG_DEBUG(ENG_ERR, "Invalid Signal Type \n");
		break;
	}

	_vc_core_util_strcpy(papp_snd->signal_tone, sizeof(papp_snd->signal_tone), signal_tone);

}

static void __voicecall_snd_get_voice_path(voicecall_snd_mgr_t *papp_snd, int *voice_snd_path)
{
	voicecall_audio_path_t tmp_audio_path;

#ifdef _NEW_SND_
	CALL_ENG_DEBUG(ENG_DEBUG, "current path = %d\n", voicecall_snd_get_path_status(papp_snd));
	
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
	case VOICE_CALL_SND_PATH_EARJACK:
		{
			tmp_audio_path = VC_AUDIO_PATH_HEADSET;
		}
		break;
	case VOICE_CALL_SND_PATH_RECEIVER:
	default:
		{
			tmp_audio_path = VC_AUDIO_PATH_HANDSET;
		}
	}
#else
	CALL_ENG_DEBUG(ENG_DEBUG, "Headset Status  = %d\n", papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_HEADSET]);
	CALL_ENG_DEBUG(ENG_DEBUG, "Loud Speaker Status  = %d\n", papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_SPEAKER]);

	/*Priority is given to Headset, incase both loudspeaker and headset is enabled */
	if (TRUE == papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_HEADSET]) {
		tmp_audio_path = VC_AUDIO_PATH_BLUETOOTH;
	} else if (TRUE == papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_SPEAKER]) {
		tmp_audio_path = VC_AUDIO_PATH_SPK_PHONE;
	} else if (TRUE == papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_EARJACK]) {
		tmp_audio_path = VC_AUDIO_PATH_HEADSET;
	} else {
		tmp_audio_path = VC_AUDIO_PATH_HANDSET;
	}
#endif
	*voice_snd_path = tmp_audio_path;

	CALL_ENG_DEBUG(ENG_DEBUG, "voice_snd_path = %d\n", *voice_snd_path);

}

void voicecall_snd_change_mm_path(voicecall_snd_mgr_t *papp_snd, voicecall_snd_mm_path_type_t mm_path_type)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;
	int in_path = 0;
	int out_path = 0;
	int gain_type = 0;
	int mm_error = 0;
	int option_field = MM_SOUND_PATH_OPTION_AUTO_HEADSET_CONTROL;

	CALL_ENG_DEBUG(ENG_DEBUG, "mm_path_type = %d\n", mm_path_type);
	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->current_path_type = %d\n", papp_snd->current_path_type);

#ifdef _NEW_SND_
	switch (voicecall_snd_get_path_status(papp_snd)) {
	case VOICE_CALL_SND_PATH_BT:
		{
			out_path = MM_SOUND_PATH_BTHEADSET;
			in_path = MM_SOUND_PATH_BTMIC;
		}
		break;
	case VOICE_CALL_SND_PATH_SPEAKER:
		{
			out_path = MM_SOUND_PATH_SPK;
			in_path = MM_SOUND_PATH_MIC;
			option_field = MM_SOUND_PATH_OPTION_NONE;
		}
		break;
	case VOICE_CALL_SND_PATH_EARJACK:
	case VOICE_CALL_SND_PATH_RECEIVER:
	default:
		{
			out_path = MM_SOUND_PATH_RECV;
			in_path = MM_SOUND_PATH_MIC;
		}
		break;
	}
	
#else
	/*Define OutPath */
	if (TRUE == papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_HEADSET]) {
		out_path = MM_SOUND_PATH_BTHEADSET;
	} else if (TRUE == papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_SPEAKER]) {
		out_path = MM_SOUND_PATH_SPK;
		option_field = MM_SOUND_PATH_OPTION_NONE;
	} else {		/*Normal Phone Reciever */

		out_path = MM_SOUND_PATH_RECV;
	}

	if (TRUE == papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_HEADSET]) {
		in_path = MM_SOUND_PATH_BTMIC;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Don't off the MIC. Mute only modem\n");
		in_path = MM_SOUND_PATH_MIC;
	}
#endif

	CALL_ENG_DEBUG(ENG_DEBUG, "Out Path = %d, In Path = %d \n", out_path, in_path);

	/* This patch is required : if voice path is not reset,
	 * the volume played for ringtone and signal tone will not be audible enough to hear
	 */
	if ((VOICE_CALL_MM_VOICE == papp_snd->current_path_type) && ((VOICE_CALL_MM_RING_TONE == mm_path_type) || (VOICE_CALL_MM_SIGNAL_TONE == mm_path_type))) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Resetting Voice Path before changing to some other path \n");
	}

	/*Define Gain Type */
	switch (mm_path_type) {
	case VOICE_CALL_MM_ALERT_TONE:
		{
			gain_type = MM_SOUND_GAIN_CALLTONE;
			in_path = MM_SOUND_PATH_NONE;
		}
		break;
	case VOICE_CALL_MM_SECOND_CALL_TONE:
		{
			/* MMFW sound request. always set it. */
			gain_type = MM_SOUND_GAIN_KEYTONE;	/* In case of earjack inserting, during second call */
			in_path = MM_SOUND_PATH_NONE;
		}
		break;
	case VOICE_CALL_MM_RING_TONE:
		{
			int io_state = VC_INOUT_STATE_NONE;

			gain_type = MM_SOUND_GAIN_RINGTONE;
			in_path = MM_SOUND_PATH_NONE;
			option_field = MM_SOUND_PATH_OPTION_SPEAKER_WITH_HEADSET;

			voicecall_core_get_engine_state(pcall_engine, &io_state);
			if ((voicecall_core_is_incoming_call_exists(pcall_engine)) && (io_state != VC_INOUT_STATE_INCOME_END)) {
				out_path = MM_SOUND_PATH_SPK;
			}
		}
		break;
	case VOICE_CALL_MM_MUTE_PLAY:
		{
			gain_type = MM_SOUND_GAIN_RINGTONE;
			in_path = MM_SOUND_PATH_NONE;
			out_path = MM_SOUND_PATH_HEADSET;
		}
		break;
	case VOICE_CALL_MM_RESET:
		{
			gain_type = MM_SOUND_GAIN_VOICECALL;
			in_path = MM_SOUND_PATH_NONE;
			out_path = MM_SOUND_PATH_NONE;
		}
		break;
	case VOICE_CALL_MM_SIGNAL_TONE:
		{
			gain_type = MM_SOUND_GAIN_CALLTONE;
			in_path = MM_SOUND_PATH_NONE;
		}
		break;
	case VOICE_CALL_MM_RECORD:
		{
			gain_type = MM_SOUND_GAIN_VOICECALL;
		}
		break;
	case VOICE_CALL_MM_VOICE:
		{
			gain_type = MM_SOUND_GAIN_VOICECALL;
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Invalid MM Path Type: %d\n", mm_path_type);
	}

	if (papp_snd->bsound_cm_state == TRUE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "mm_sound_set_path- Gain:[%d],OutPath:[%d],InPath:[%d],option:[%d] \n", gain_type, out_path, in_path, option_field);
		CALL_ENG_KPI("mm_sound_set_path start");
		mm_error = mm_sound_set_path(gain_type, out_path, in_path, option_field);
		CALL_ENG_KPI("mm_sound_set_path done");
		if (mm_error != MM_ERROR_NONE) {
			CALL_ENG_DEBUG(ENG_ERR, "MM Path Change Failed,mm_error =  [0x%x]\n", mm_error);
		}
		papp_snd->current_path_type = mm_path_type;
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "** we can't change mm path. check it. **** \n");
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
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");
	CALL_ENG_KPI("voicecall_snd_change_path start");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

#ifdef _NEW_SND_
	if (voicecall_snd_get_path_status(papp_snd) == VOICE_CALL_SND_PATH_BT) {
		int bt_sco_status = 0;

		bt_sco_status = _vc_bt_get_bt_sco_status();
		if (FALSE == bt_sco_status) {
			gboolean bevent_wait = FALSE;

			CALL_ENG_DEBUG(ENG_ERR, "BT Sco is OFF, request BT for path change\n");
			bevent_wait = voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING);
			if (FALSE == bevent_wait) {
				/*Request BT for change path to headset */
				_vc_bt_request_switch_headset_path(pcall_core, TRUE);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "bevent_wait = %d, waiting for BT Event \n", bevent_wait);
			}
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "BT SCO is open, Change the path to sync with BT Path\n");
			voicecall_snd_change_path_real(papp_snd);
		}
	}else {
		CALL_ENG_DEBUG(ENG_DEBUG, "new PATH is not BT.");
		gboolean bevent_wait = FALSE;

		bevent_wait = voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING);
		if (FALSE == bevent_wait) {
			/*Request BT for change path to headset */
			voicecall_snd_change_path_real(papp_snd);
		} else {
			CALL_ENG_DEBUG(ENG_ERR, "bevent_wait = %d, waiting for BT Event \n", bevent_wait);
		}
	}
#else
	if ((TRUE == _vc_bt_is_bt_connected(pcall_core))
	    && (TRUE == papp_snd->bcall_audio_status[VOICE_CALL_AUDIO_HEADSET])) {

		int bt_sco_status = 0;

		bt_sco_status = _vc_bt_get_bt_sco_status();
		if (FALSE == bt_sco_status) {
			gboolean bevent_wait = FALSE;

			CALL_ENG_DEBUG(ENG_ERR, "BT Sco is OFF, request BT for path change\n");
			bevent_wait = voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_BT_EVENT_WAITING);
			if (FALSE == bevent_wait) {
				/*Request BT for change path to headset */
				_vc_bt_request_switch_headset_path(pcall_core, TRUE);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "bevent_wait = %d, waiting for BT Event \n", bevent_wait);
			}
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "BT SCO is open, Change the path to sync with BT Path\n");
			voicecall_snd_change_path_real(papp_snd);
		}
	} else if ((TRUE == _vc_bt_is_bt_connected(pcall_core)) && (TRUE == voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT))) {
		/*Request BT for change path to headset, actual voice path will be changed in the response from BT */
		CALL_ENG_DEBUG(ENG_ERR, "Headset Connected Call is accepted by BT, requesting BT to change path\n");
		_vc_bt_request_switch_headset_path(pcall_core, TRUE);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Headset connected/Headset connected call accepted by Phone, Normal Path Change..\n");
		voicecall_snd_change_path_real(papp_snd);
	}
#endif
	CALL_ENG_KPI("voicecall_snd_change_path done");

}

void voicecall_snd_change_path_real(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	/* Change the mm sound path */
	voicecall_snd_change_mm_path(papp_snd, VOICE_CALL_MM_VOICE);

	voicecall_snd_change_modem_path(papp_snd);
}

void voicecall_snd_change_modem_path(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	int audio_path = VC_AUDIO_PATH_HANDSET;

	/* Change the tapi sound path */
	__voicecall_snd_get_voice_path(papp_snd, &audio_path);

	if (FALSE == voicecall_core_change_audio_path(pcall_core->pcall_engine, audio_path)) {
		CALL_ENG_DEBUG(ENG_ERR, "TAPI Audio Change Path ERROR \n");
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
	CALL_ENG_DEBUG(ENG_DEBUG, "snd_audio_type:[%d], status:[%d]\n", snd_audio_type, status);

	/*if BT headset is connected , switch off loud speaker - it will be reflectd in the next sound path change */
	if (((VOICE_CALL_AUDIO_HEADSET == snd_audio_type) || (VOICE_CALL_AUDIO_EARJACK == snd_audio_type)) && (TRUE == status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "loud speaker status is to be FALSE\n");
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
	CALL_ENG_DEBUG(ENG_DEBUG, "Status[%d] = %d \n", snd_audio_type, papp_snd->bcall_audio_status[snd_audio_type]);
	return papp_snd->bcall_audio_status[snd_audio_type];
}

#ifdef _NEW_SND_
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
	CALL_ENG_DEBUG(ENG_DEBUG, "current path:[%d], new path:[%d]\n", papp_snd->current_snd_path, path);

	papp_snd->old_snd_path = papp_snd->current_snd_path;
	papp_snd->current_snd_path = path;
}

voicecall_snd_path_t voicecall_snd_get_path_status(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "old path:[%d], current path:[%d]\n", papp_snd->old_snd_path, papp_snd->current_snd_path);

	return papp_snd->current_snd_path;
}
#endif

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
	CALL_ENG_DEBUG(ENG_DEBUG, "volume_level = %d \n", volume_level);
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	switch (vol_alert_type) {
	case VOICE_CALL_VOL_TYPE_RINGTONE:
		{
			int ret_value = 0;
			int volume;

			if (VOICE_CALL_SND_INVALID_PLAYER_HANDLE == papp_snd->pmm_player) {
				CALL_ENG_DEBUG(ENG_ERR, "Invalid MM Player Handle \n");
				return;
			}

			/* Make Increasing Melody flag to FALSE, so melody volume will not be increased when volume it adjusted by user during increasing melody */
			papp_snd->bincreasingmelody = FALSE;

			volume = volume_level;

			ret_value = mm_sound_volume_set_value(VOLUME_TYPE_RINGTONE, volume);
			if (MM_ERROR_NONE != ret_value) {
				CALL_ENG_DEBUG(ENG_ERR, "mmplayer_set_volume failed Error: [0x%x]\n", ret_value);
			}
#ifdef VOICE_CALL_RINGTONE_ELEVATOR
			__voicecall_snd_cancel_ring_elevator(papp_snd);
#endif
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
				CALL_ENG_DEBUG(ENG_DEBUG, "voicecall_doc_set_audio_volume failed\n");
			}
		}
		break;
	case VOICE_CALL_VOL_TYPE_HEADSET:
		{
			int bt_vol_level = 0;
			if (TRUE == voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_VOL_CHNGD_BYBT)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "BT Requested Volume flag is enabled, not sending response \n");
				voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_VOL_CHNGD_BYBT, FALSE);
			} else {
				bt_vol_level = volume_level;
				CALL_ENG_DEBUG(ENG_DEBUG, "bt_vol_level = %d\n", bt_vol_level);

				_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SPK_GAIN, bt_vol_level, NULL);
			}

		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "No Actions Defined for the volume alert type: %d \n", vol_alert_type);

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
	CALL_ENG_DEBUG(ENG_DEBUG, "\n");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	switch (vol_alert_type) {
	case VOICE_CALL_VOL_TYPE_RINGTONE:
		{
			int ret_val = 0;
			unsigned int volume = 0;

			ret_val = mm_sound_volume_get_value(VOLUME_TYPE_RINGTONE, &volume);
			if (MM_ERROR_NONE != ret_val) {
				CALL_ENG_DEBUG(ENG_DEBUG, "ret_val = [0x%x]\n", ret_val);
				return VC_INVALID_VOLUME;
			}
			CALL_ENG_DEBUG(ENG_DEBUG, "MM Volume Level : %d\n", volume);

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
	call_vc_call_objectinfo_t callobject_info;
	voicecall_contact_info_t ct_info;

	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle = %d\n", call_handle);

	CALL_ENG_DEBUG(ENG_DEBUG, "Contact ring_tone_path = [%s]\n", papp_snd->ring_tone);

	if (strlen(papp_snd->ring_tone) <= 0) {
		setting_file_path = vconf_get_str(VCONFKEY_SETAPPL_CALL_RINGTONE_PATH_STR);
		if ((setting_file_path == NULL) || (strlen(setting_file_path) <= 0) ||
		(FALSE == g_file_test(setting_file_path, G_FILE_TEST_EXISTS)))
		{
			CALL_ENG_DEBUG(ENG_ERR, "setting ring tone path is invalid : [%s]\n", setting_file_path);
			/*snprintf(ringtone_path, sizeof(ringtone_path), "file://%s", VOICE_CALL_SND_DEFAULT_RINGTONE_PATH);*/
			return;
		} else {
			snprintf(ringtone_path, sizeof(ringtone_path), "file://%s", setting_file_path);

		}
		CALL_ENG_DEBUG(ENG_DEBUG, "Ringtone From Settings : %s\n", ringtone_path);
		_vc_core_util_strcpy(papp_snd->ring_tone, sizeof(papp_snd->ring_tone), ringtone_path);
	}

	/* Create MM Player */
	papp_snd->pmm_player = VOICE_CALL_SND_INVALID_PLAYER_HANDLE;
	__voicecall_snd_create_player(papp_snd, &papp_snd->pmm_player, VOICE_CALL_PLAY_TYPE_RINGTONE);

	papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_READY;

	return TRUE;
}

static gboolean __voicecall_snd_create_player(voicecall_snd_mgr_t *papp_snd, MMHandleType * pPlayer, voicecall_snd_play_type_t play_type)
{
	int ret_value = 0;
	MMMessageCallback callback = NULL;
	char *mmf_error = NULL;

	CALL_ENG_DEBUG(ENG_DEBUG, "Start..\n");

	if (*pPlayer != VOICE_CALL_SND_INVALID_SND_HANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Player = %d \n", *pPlayer);
		mm_player_destroy(*pPlayer);
		*pPlayer = VOICE_CALL_SND_INVALID_SND_HANDLE;
	}

	ret_value = mm_player_create(pPlayer);
	if (MM_ERROR_NONE != ret_value || *pPlayer == VOICE_CALL_SND_INVALID_SND_HANDLE) {
		CALL_ENG_DEBUG(ENG_ERR, "mmplayer_create failed , Error:[0x%x]\n", ret_value);
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "*pPlayer: %d\n", *pPlayer);

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
		CALL_ENG_DEBUG(ENG_ERR, "mmplayer_realize failed , Error:[0x%x]\n", ret_value);
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Finish..\n");
	return TRUE;
}

static gboolean __voicecall_snd_play_melody(voicecall_snd_mgr_t *papp_snd, gboolean bis_increasing)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	if (strlen(papp_snd->ring_tone) > 0) {
		int mm_error = MM_ERROR_NONE;

		mm_error = mm_player_start(papp_snd->pmm_player);
		if (MM_ERROR_NONE != mm_error) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_player_start failed,Error: [0x%x]\n", mm_error);
		} else {
			if (TRUE == bis_increasing) {
				papp_snd->bincreasingmelody = TRUE;
				g_timeout_add(VOICE_CALL_SND_INCREMENT_TIMER_INTERVAL, __voicecall_snd_increasing_melody_cb, (gpointer) papp_snd);
			} else {
#ifdef VOICE_CALL_RINGTONE_ELEVATOR
				CALL_ENG_DEBUG(ENG_DEBUG, "Starting Ringtone Elevator\n");
				/*Start only when the volume is not incremental volume */
				__voicecall_snd_start_ring_elevator(papp_snd);
#endif
			}

			papp_snd->ringtone_sound_status = VOICE_CALL_SND_STATUS_PLAY;
			return TRUE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "Ringtone is empty(Problem) \n");
	}

	return FALSE;
}

static int __voicecall_snd_mmplayer_cb(int message, void *data, void *user_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)user_data;
	if (VOICE_CALL_SND_STATUS_PLAY != papp_snd->ringtone_sound_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Sound Play stopped by application, callback not handled \n");
		return FALSE;
	}

	switch (message) {
	case MM_MESSAGE_END_OF_STREAM:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Ringtone loop count is supported by MMF , need not restart the ringtone \n");
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

	CALL_ENG_DEBUG(ENG_DEBUG, " \n");

	/*Callback needs to be called, only when the tone is played completely and it is ended
	   if the signal is explictly stopped, don't call the user callback */
	if (papp_snd->psignal_play_end_cb != NULL) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Calling user callback \n");
		papp_snd->psignal_play_end_cb(papp_snd->psignal_play_end_cb_data);
		papp_snd->psignal_play_end_cb = NULL;
		papp_snd->psignal_play_end_cb_data = NULL;
	}

	return FALSE;
}

static void __voicecall_snd_mmplayer_signal_cb(gpointer puser_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)puser_data;

	CALL_ENG_DEBUG(ENG_DEBUG, "Sound Play Over \n");

	if (papp_snd->signal_sound_status != VOICE_CALL_SND_STATUS_PLAY) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Sound Play already stopped by application \n");
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
		CALL_ENG_DEBUG(ENG_DEBUG, "Already Deactivated\n");
		return FALSE;
	}

	if (FALSE == vconf_get_int(VCONFKEY_SETAPPL_PROFILE_CURRENT_CALL_VOLUME_INT, &set_volume_level)) {
		CALL_ENG_DEBUG(ENG_ERR, "settings read failed Error: %d\n", ret_value);
	}

	if ((set_volume_level != 0) && (papp_snd->increment_melody_value <= set_volume_level)) {
		int volume_val;

		papp_snd->increment_melody_value++;

		volume_val = papp_snd->increment_melody_value;
		ret_value = mm_sound_volume_set_value(VOLUME_TYPE_RINGTONE, volume_val);

		if (MM_ERROR_NONE != ret_value) {
			CALL_ENG_DEBUG(ENG_ERR, "Set Volume Error: [0x%x]\n", ret_value);
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "Increasing Melody Continuing, Current Increased Melody : %d\n", papp_snd->increment_melody_value);
		if (papp_snd->increment_melody_value >= set_volume_level) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Increasing Melody Ended\n");
			return FALSE;
		}

		return TRUE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Increasing Melody Ended\n");
	return FALSE;
}

static void __voicecall_snd_start_vibration(voicecall_snd_mgr_t *papp_snd)
{
	/*First Stop the previous Vibration and then start it again */

	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");
	int vib_type = -1;
	int vib_level = -1;
	int haptic_vib_type = -1;

	papp_snd->bvibration = FALSE;

	papp_snd->vibration_handle = device_haptic_open(DEV_IDX_0, 0);

	CALL_ENG_DEBUG(ENG_DEBUG, "srart vibration device_handle=%d \n", papp_snd->vibration_handle);

	if (papp_snd->vibration_handle < 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_open error \n");
		return;
	}

	if (vconf_get_int(VCONFKEY_SETAPPL_CALL_VIBRATION_PATTERN_INT, &vib_type)) {
		CALL_ENG_DEBUG(ENG_ERR, "VCONFKEY_SETAPPL_CALL_VIBRATION_PATTERN_INT vconf_get_bool failed.\n");
	}
	if (vconf_get_int(VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT, &vib_level)) {
		CALL_ENG_DEBUG(ENG_ERR, "VCONFKEY_SETAPPL_NOTI_VIBRATION_LEVEL_INT vconf_get_bool failed.\n");
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
		CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_play_pattern error \n");
		return;
	}
	if (device_haptic_play_monotone(papp_snd->vibration_handle, 60000)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_play_monotone error \n");
		return;
	}

	papp_snd->bvibration = TRUE;

}

static gboolean __voicecall_snd_vib_then_melody_cb(void *data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)data;
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	__voicecall_snd_stop_vibration(papp_snd);

	if (FALSE == papp_snd->bvibration_then_melody) {
		return FALSE;
	}

	if (FALSE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {

		int ret_value = -1;
		int set_volume_level = 0;
		int volume_val;

		vconf_get_int(VCONFKEY_SETAPPL_PROFILE_CURRENT_CALL_VOLUME_INT, &set_volume_level);

		volume_val = set_volume_level;

		ret_value = mm_sound_volume_set_value(VOLUME_TYPE_RINGTONE, volume_val);

		CALL_ENG_DEBUG(ENG_ERR, "__voicecall_snd_vib_then_melody_cb() : volume = %d\n", volume_val);

		__voicecall_snd_play_melody(papp_snd, FALSE);
	}

	return FALSE;
}

static void __voicecall_snd_stop_vibration(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	if (TRUE == papp_snd->bvibration) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Stopping Vibration , handle=%d \n", papp_snd->vibration_handle);

		if (device_haptic_stop_play(papp_snd->vibration_handle)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_play_stop error \n");
			return;
		}

		if (device_haptic_close(papp_snd->vibration_handle)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "device_haptic_close error \n");
			return;
		}

		papp_snd->vibration_handle = -1;

		papp_snd->bvibration = FALSE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "bvibration is not enabled.\n");
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
			CALL_ENG_DEBUG(ENG_ERR, "mm_sound_route_set_system_policy failed. error_code:[0x%x]\n", error_code);
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

	CALL_ENG_DEBUG(ENG_DEBUG, "current bsound_cm_state:[%d]. \n", papp_snd->bsound_cm_state);

	if (FALSE == papp_snd->bsound_cm_state) {
		CALL_ENG_KPI("mm_session_init start");
		error_code = mm_session_init(MM_SESSION_TYPE_CALL);
		CALL_ENG_KPI("mm_session_init done");
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_session_init failed. error_code:[0x%x]\n", error_code);
			return;
		}
		error_code = mm_sound_route_get_system_policy(&papp_snd->backup_route_policy);
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_sound_route_get_system_policy failed. error_code:[0x%x]\n", error_code);
			return;
		}
		error_code = mm_sound_route_set_system_policy(SYSTEM_AUDIO_ROUTE_POLICY_IGNORE_A2DP);
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_sound_route_set_system_policy failed. error_code:[0x%x]\n", error_code);
			return;
		}
		error_code = mm_sound_route_add_change_callback(voicecall_snd_route_change_cb, NULL);
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_sound_route_add_change_callback failed. error_code:[0x%x]\n", error_code);
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

	CALL_ENG_DEBUG(ENG_DEBUG, "current bsound_cm_state:[%d]. \n", papp_snd->bsound_cm_state);

	if (TRUE == papp_snd->bsound_cm_state) {
		/*Reset the Path when the app is closed - safety code */
		voicecall_snd_change_mm_path(papp_snd, VOICE_CALL_MM_RESET);
		CALL_ENG_DEBUG(ENG_DEBUG, "Sound Path reset to Default\n");

		papp_snd->bsound_cm_state = FALSE;

		CALL_ENG_DEBUG(ENG_DEBUG, "Unregistering Sound CM\n");
		error_code = mm_session_finish();
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_session_finish failed. error_code:[0x%x]\n", error_code);
		}
		error_code = mm_sound_route_remove_change_callback();
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_sound_route_remove_change_callback failed. error_code:[0x%x]\n", error_code);
		}
		error_code = mm_sound_route_set_system_policy(papp_snd->backup_route_policy);
		if (error_code) {
			CALL_ENG_DEBUG(ENG_ERR, "mm_sound_route_set_system_policy failed. error_code:[0x%x]\n", error_code);
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
	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd->effect_tone_status = %d\n", papp_snd->effect_tone_status);

	/*If connected call exists then change the audio path */
	if ((TRUE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) || (TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine))) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Connected call exists, changing path at the end of effect tone \n");

		CALL_ENG_DEBUG(ENG_ERR, "papp_snd->signal_sound_status = %d \n", papp_snd->signal_sound_status);
		/*Check the signal play status, if signal is being prepared / played, don't change the path */
		if (VOICE_CALL_SND_STATUS_NONE == papp_snd->signal_sound_status) {
			voicecall_snd_change_path(papp_snd);
		} else {
			CALL_ENG_DEBUG(ENG_ERR, "Signal is playing, skipping path change, it will be done at the end of signal \n");
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "non connected call\n");
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Over\n");
	return FALSE;
}

static void __voicecall_snd_effect_cb(gpointer puser_data)
{
	voicecall_snd_mgr_t *papp_snd = (voicecall_snd_mgr_t *)puser_data;

	CALL_ENG_DEBUG(ENG_DEBUG, "effect_tone_status = %d, Calling Idle\n", papp_snd->effect_tone_status);
	papp_snd->effect_tone_status = VOICE_CALL_SND_STATUS_STOPPED;

	__voicecall_snd_effect_idle_cb(papp_snd);
	CALL_ENG_DEBUG(ENG_DEBUG, "End : papp_snd->effect_tone_status = %d\n", papp_snd->effect_tone_status);
}

gboolean voicecall_snd_play_effect_tone(voicecall_snd_mgr_t *papp_snd, int effect_type)
{
	int error_code = -1;
	gboolean bzuhause = FALSE;
	gboolean bstatus = FALSE;
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)papp_snd->pcall_core;

	CALL_ENG_DEBUG(ENG_DEBUG, "effect type:[%d] \n", effect_type);

	switch (effect_type) {
	case VOICE_CALL_SND_EFFECT_CALL_CONNECT:
		{
			CALL_ENG_KPI("voicecall_core_get_zuhause start");
			voicecall_core_get_zuhause(pcall_core->pcall_engine, &bzuhause);
			CALL_ENG_KPI("voicecall_core_get_zuhause done");

			if (bzuhause == TRUE) {
				CALL_ENG_DEBUG(ENG_DEBUG, "It's zuhause area! don't play connect tone!\n");
				return FALSE;
			}

			CALL_ENG_KPI("get VCONFKEY_CISSAPPL_CALL_CONNECT_TONE_BOOL start");
			if (vconf_get_bool(VCONFKEY_CISSAPPL_CALL_CONNECT_TONE_BOOL, &bstatus)) {
				CALL_ENG_DEBUG(ENG_ERR, "vconf_get_bool failed.\n");
			}
			CALL_ENG_KPI("get VCONFKEY_CISSAPPL_CALL_CONNECT_TONE_BOOL done");

			if (!bstatus) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Connect Tone Settings not enabled \n");
				return FALSE;
			}

			/*First Reset the audio Path to PDA */
			voicecall_snd_change_mm_path(papp_snd, VOICE_CALL_MM_ALERT_TONE);

			error_code = mm_sound_play_sound(VOICE_CALL_SND_CONNECT_SIGNAL_PATH, VOLUME_TYPE_CALL, __voicecall_snd_effect_cb, papp_snd, &papp_snd->mmfsoundplay_handle);
		}
		break;
	case VOICE_CALL_SND_EFFECT_CALL_DISCONNECT:
		{

			if (vconf_get_bool(VCONFKEY_CISSAPPL_CALL_END_TONE_BOOL, &bstatus)) {
				CALL_ENG_DEBUG(ENG_ERR, "vconf_get_bool failed.\n");
			}

			if (!bstatus) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Disconnect Tone Settings not enabled \n");
				return FALSE;
			}

			voicecall_snd_change_mm_path(papp_snd, VOICE_CALL_MM_ALERT_TONE);

			error_code = mm_sound_play_sound(VOICE_CALL_SND_DISCONNECT_SIGNAL_PATH, VOLUME_TYPE_CALL, __voicecall_snd_effect_cb, papp_snd, &papp_snd->mmfsoundplay_handle);

		}
		break;

	case VOICE_CALL_SND_EFFECT_CALL_MINUTE_MINDER:
		{
			voicecall_snd_change_mm_path(papp_snd, VOICE_CALL_MM_ALERT_TONE);
			error_code = mm_sound_play_sound(VOICE_CALL_SND_MINUTE_MINDER_SIGNAL_PATH, VOLUME_TYPE_CALL, __voicecall_snd_effect_cb, papp_snd, &papp_snd->mmfsoundplay_handle);
		}
		break;

	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Invalid Effect Type: %d \n", effect_type);

	}

	if (MM_ERROR_NONE == error_code) {
		papp_snd->effect_tone_status = VOICE_CALL_SND_STATUS_PLAY;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "error code = [0x%x] \n", error_code);
	return (MM_ERROR_NONE == error_code) ? TRUE : FALSE;
}

gboolean voicecall_snd_is_effect_playing(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_ERR, "Effect tone status: %d\n", papp_snd->effect_tone_status);
	if (VOICE_CALL_SND_STATUS_PLAY == papp_snd->effect_tone_status) {
		return TRUE;
	}

	return FALSE;
}

void voicecall_snd_stop_effect_tone(voicecall_snd_mgr_t *papp_snd)
{
	CALL_ENG_DEBUG(ENG_ERR, "Effect tone status: %d\n", papp_snd->effect_tone_status);

	if (VOICE_CALL_SND_STATUS_PLAY == papp_snd->effect_tone_status) {
		if (MM_ERROR_NONE != mm_sound_stop_sound(papp_snd->mmfsoundplay_handle)) {
			CALL_ENG_DEBUG(ENG_ERR, "MM Stop Sound Failed \n");
		}

		papp_snd->effect_tone_status = VOICE_CALL_SND_STATUS_NONE;
		papp_snd->mmfsoundplay_handle = VOICE_CALL_SND_INVALID_SND_HANDLE;
	}
	CALL_ENG_DEBUG(ENG_ERR, "Effect tone status: %d\n", papp_snd->effect_tone_status);
}

void voicecall_snd_set_to_defaults(voicecall_snd_mgr_t *papp_snd)
{
	call_vc_core_state_t *pcall_core = NULL;

	CALL_ENG_DEBUG(ENG_DEBUG, "papp_snd = %p \n", papp_snd);

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
