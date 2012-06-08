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


#ifndef _VOICE_CALL_SOUND_H_
#define _VOICE_CALL_SOUND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <mm_message.h>
#include <mm_player.h>
#include <mm_sound.h>
#include <mm_sound_private.h>
#include <mm_types.h>
#include <mm_error.h>
#include <glib.h>

#define VOICE_CALL_SND_RINGTONE_PATH_LEN			256	/**<Max File length for Ringtone */

/**
 * This enum defines the sound actions with respect to the sound conflict notifications
 */
typedef enum _voicecall_snd_status_t {
	VOICE_CALL_SND_STATUS_NONE,		/**<Initial State */
	VOICE_CALL_AND_STATUS_ALTERNATE_PLAY,	/**<Alternate Play can be done */
	VOICE_CALL_SND_STATUS_READY,		/**<Sound Player Prepared, ready to play */
	VOICE_CALL_SND_STATUS_PROCESSED,	/**< Intial Sound Play Started */
	VOICE_CALL_SND_STATUS_STOP,		/**< The Sound is stopped by the application */
	VOICE_CALL_SND_STATUS_PLAY,		/**< Specifies that the sound is currently being played */
	VOICE_CALL_SND_STATUS_PAUSE,		/**< It specifies that the sound is currently paused */
	VOICE_CALL_SND_STATUS_STOPPED		/**< The sound is stopeed by other application */
} voicecall_snd_status_t;

/**
* This enumeration defines names of the on call audio status
*/
typedef enum _voicecall_snd_audio_type_t {
	VOICE_CALL_AUDIO_NONE,					/**< none */
	VOICE_CALL_AUDIO_SPEAKER,			/**< System LoudSpeaker Audio */
	VOICE_CALL_AUDIO_RECEIVER,			/**< System receiver Audio */
	VOICE_CALL_AUDIO_HEADSET,			/**< System Headset Audio */
	VOICE_CALL_AUDIO_EARJACK,			/**< System Earjack Audio */
	VOICE_CALL_AUDIO_MAX,
} voicecall_snd_audio_type_t;

#ifdef _NEW_SND_
/**
* This enumeration defines names of the on call sound path status
*/
typedef enum _voicecall_snd_path_t {
	VOICE_CALL_SND_PATH_NONE,					/**< none */
	VOICE_CALL_SND_PATH_SPEAKER,			/**< System LoudSpeaker path */
	VOICE_CALL_SND_PATH_RECEIVER,			/**< System Receiver path */
	VOICE_CALL_SND_PATH_BT,				/**< System BT Headset path */
	VOICE_CALL_SND_PATH_EARJACK,			/**< System Earjack path */
	VOICE_CALL_SND_PATH_MAX,
} voicecall_snd_path_t;
#endif

/**
* This enumeration defines voice recorder status
*/
typedef enum __voicecall_snd_record_status_t {
	VOICE_CALL_REC_NONE,
	VOICE_CALL_REC_ON,
	VOICE_CALL_REC_PAUSED
} voicecall_snd_record_status_t;

/**
* This enumeration defines volume alert type
*/
typedef enum __voicecall_snd_volume_alert_type_t {
	VOICE_CALL_VOL_TYPE_RINGTONE,
	VOICE_CALL_VOL_TYPE_VOICE,
	VOICE_CALL_VOL_TYPE_HEADSET
} voicecall_snd_volume_alert_type_t;

/**
* This enumeration defines volume level to be used
*/
typedef enum __voicecall_snd_volume_level_t {
	VOICE_CALL_VOL_LEVEL_1 = 1,
	VOICE_CALL_VOL_LEVEL_2,
	VOICE_CALL_VOL_LEVEL_3,
	VOICE_CALL_VOL_LEVEL_4,
	VOICE_CALL_VOL_LEVEL_5,
	VOICE_CALL_VOL_LEVEL_6
} voicecall_snd_volume_level_t;

typedef enum __voicecall_snd_play_type_t {
	VOICE_CALL_PLAY_TYPE_RINGTONE,
	VOICE_CALL_PLAY_TYPE_SIGNAL,
	VOICE_CALL_PLAY_TYPE_MAX
} voicecall_snd_play_type_t;

typedef enum __voicecall_snd_signal_type_t {
	VOICE_CALL_SIGNAL_NONE,
	VOICE_CALL_SIGNAL_USER_BUSY_TONE,
	VOICE_CALL_SIGNAL_WRONG_NUMBER_TONE,
	VOICE_CALL_SIGNAL_CALL_FAIL_TONE,
	VOICE_CALL_SIGNAL_NW_CONGESTION_TONE,
	VOICE_CALL_SIGNAL_MAX,
} voicecall_snd_signal_type_t;

typedef enum _voicecall_snd_effect_type_t {
	VOICE_CALL_SND_EFFECT_CALL_CONNECT,
	VOICE_CALL_SND_EFFECT_CALL_DISCONNECT,
	VOICE_CALL_SND_EFFECT_CALL_MINUTE_MINDER,
	VOICE_CALL_SND_EFFECT_MAX
} voicecall_snd_effect_type_t;

typedef enum __voicecall_snd_mm_path_type_t {
	VOICE_CALL_MM_NONE,
	VOICE_CALL_MM_ALERT_TONE,	/*When playing the in call alert tones like call end tone, call connected tone */
	VOICE_CALL_MM_RING_TONE,	/*When playing the incoming call ringtone */
	VOICE_CALL_MM_SECOND_CALL_TONE,	/*When playing the second incoming call tone */
	VOICE_CALL_MM_SIGNAL_TONE,	/*When playing the outgoing call fail signal tone */
	VOICE_CALL_MM_RECORD,	/*When playing the voice recording beep during call */
	VOICE_CALL_MM_VOICE,	/*During outgoing call and connected call */
	VOICE_CALL_MM_RESET,	/*This should be used when app is closed after call */
	VOICE_CALL_MM_MUTE_PLAY,	/*This should be used when ringtone needs to be played when mute profile is enabled */
	VOICE_CALL_MM_MAX
} voicecall_snd_mm_path_type_t;

typedef enum __voicecall_snd_cm_status {
	VOICE_CALL_SND_NONE,
	VOICE_CALL_SND_VIDEOCALL,
	VOICE_CALL_SND_OTHER,
	VOICE_CALL_SND_MAX
} voicecall_snd_cm_status_type_t;

typedef void (*voicecall_snd_callback) (gpointer pdata);

/**
 * This enum defines sound managers components
 */
typedef struct __voicecall_snd_mgr_t {
	MMHandleType pmm_player;		/**< Handle to MM Player */
	system_audio_route_t backup_route_policy;
	int vibration_handle;					 /**< Handle to System Vibration Module*/

	gboolean bsound_cm_state;

	voicecall_snd_status_t ringtone_sound_status;		/**< Holds a current sound play status for Ringtone player*/
#ifdef _NEW_SND_
	voicecall_snd_path_t old_snd_path;				/**< Holds a old sound path status*/ 
	voicecall_snd_path_t current_snd_path;				/**< Holds a current sound path status*/ 
#endif
	void *pcall_core;					/**< Holds a pointer to the voicecall core Handle */
	gboolean bcall_audio_status[VOICE_CALL_AUDIO_MAX];		  /**< Holds of status of the #voicecall_snd_audio_type_t*/
	gboolean bmute_status;			/**< voice mute*/

	gboolean bincreasingmelody;				/**<TRUE - Increasing Melody Activated, FALSE - Otherwise*/
	int increment_melody_value;				/**<specifies the current level of increasing melody*/
	gboolean bvibration;					/**<TRUE - Vibration activated, FALSE -otherwise*/
	gboolean bvibration_then_melody;			/**<TRUE - Vibration then meldody is activated, FALSE -otherwise*/
	gboolean balternate_play;
	gboolean bmute_play;					/**<TRUE - Player created for playing the tone during mute play*/
	int balternate_play_count;

	char ring_tone[VOICE_CALL_SND_RINGTONE_PATH_LEN];			/**< Holds the Path of the Ringtone to be played */
	int current_playing_call_handle;			/**< Holds the Call Handle of Currently Playing Call */
	int incoming_call_handle;			/**< Holds the Call Handle of Incoming Call */
	int vibalert_onoff;					/**< On/Off Counter for periodic Vibration */

	int pmm_signal_player;					/**< Handle to MM Signal Player */
	voicecall_snd_signal_type_t signal_type;		/**< Signal Type */
	char signal_tone[VOICE_CALL_SND_RINGTONE_PATH_LEN];			/**< Holds the Path of the Signal tone to be played */
	voicecall_snd_status_t signal_sound_status;		/**< Holds a current sound play status for Signal player*/

	voicecall_snd_mm_path_type_t current_path_type;

	int mmfsoundplay_handle;
	int mmfalternateplay_handle;

	int settings_sound_status;
	int settings_vib_status;

	voicecall_snd_callback psignal_play_end_cb;		/**< Callback called after Sound Play ends */
	gpointer psignal_play_end_cb_data;			/**< Sound Play end callback data */

	voicecall_snd_status_t effect_tone_status;

#ifdef VOICE_CALL_RINGTONE_ELEVATOR
	guint ring_elvator_timerid;
#endif
} voicecall_snd_mgr_t;

 /**
 * This function initializes the sound functionalties required by the Application
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[out]	papp_snd			Handle to the Sound Manager
 */
gboolean voicecall_snd_init(void *pcall_core, voicecall_snd_mgr_t **papp_snd);

/**
 * This function stops the sound alert
 *
 * @return		void
 * @param[in]		papp_snd			Handle to Sound Manager
 */
void voicecall_snd_stop_alert(voicecall_snd_mgr_t *papp_snd);

/**
 * This function mutes the sound play
 *
 * @return		TRUE - Success, FALSE - Otherwise
 * @param[in]	papp_snd			Handle to Sound Manager
 */
gboolean voicecall_snd_mute_alert(voicecall_snd_mgr_t *papp_snd);

/**
 * This function stops the alternate sound play
 *
 * @return		void
 * @param[in]	papp_snd			Handle to Sound Manager
 */
void voicecall_snd_stop_altenate_sound(voicecall_snd_mgr_t *papp_snd);

/**
 * This function plays call sound
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		papp_snd			Handle to Sound Manager
 */
gboolean voicecall_snd_play_signal(voicecall_snd_mgr_t *papp_snd, voicecall_snd_callback pplay_end_callback, void *pcallback_data);
gboolean voicecall_snd_set_play_end_callback(voicecall_snd_mgr_t *papp_snd, voicecall_snd_callback pplay_end_callback, void *pcallback_data);
gboolean voicecall_snd_is_signal_playing(voicecall_snd_mgr_t *papp_snd);

/**
 * This function stops the sound alert
 *
 * @return		void
 * @param[in]		papp_snd			Handle to Sound Manager
 */
void voicecall_snd_stop_signal(voicecall_snd_mgr_t *papp_snd);

/**
 * This function sets the end signal of the given end cause type
 *
 * @return		void
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		end_cause_type		Type of the end cause
 */
void voicecall_snd_set_signal_type(voicecall_snd_mgr_t *papp_snd, voicecall_snd_signal_type_t signal_type);

/**
 * This function plays the effect tone accordign to the given effect type
 *
 * @return		void
 * @param[in]	papp_snd	Handle to Sound Manager
 * @param[in]	effect_type	Type of effect tone to be played
 */
gboolean voicecall_snd_play_effect_tone(voicecall_snd_mgr_t *papp_snd, int effect_type);

/**
 * This function stops the effect tone play
 *
 * @return		void
 * @param[in]	papp_snd	Handle to Sound Manager
 */
void voicecall_snd_stop_effect_tone(voicecall_snd_mgr_t *papp_snd);

/**
 * This function changes the mm sound path according to the current status
 *
 * @return		void
 * @param[in]		papp_snd			Handle to Sound Manager
 */
void voicecall_snd_change_mm_path(voicecall_snd_mgr_t *papp_snd, voicecall_snd_mm_path_type_t mm_path_type);

/**
 * This function changes the sound path according to the current status
 *
 * @return		void
 * @param[in]		papp_snd			Handle to Sound Manager
 */
void voicecall_snd_change_path(voicecall_snd_mgr_t *papp_snd);
void voicecall_snd_change_path_real(voicecall_snd_mgr_t *papp_snd);

void voicecall_snd_change_modem_path(voicecall_snd_mgr_t *papp_snd);

#ifdef _NEW_SND_
/**
 * This function sets the status of the given call audio type
 *
 * @return		void
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		snd_audio_type		Type of the Device to be changed
 * @param[in]		status				Status, TRUE - Enable, FALSE -Disable
 */
void voicecall_snd_set_path_status(voicecall_snd_mgr_t *papp_snd, voicecall_snd_path_t path);

voicecall_snd_path_t voicecall_snd_get_path_status(voicecall_snd_mgr_t *papp_snd);
#else
/**
 * This function sets the status of the given call audio type
 *
 * @return		void
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		snd_audio_type		Type of the Device to be changed
 * @param[in]		status				Status, TRUE - Enable, FALSE -Disable
 */
void voicecall_snd_set_status(voicecall_snd_mgr_t *papp_snd, voicecall_snd_audio_type_t snd_audio_type, gboolean status);

/**
 * This function returns the current status of the given call audio type
 *
 * @return		Returns TRUE if given call audio type is enables or FALSE otherwise
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		snd_audio_type		Type of the Device to be changed
 */
gboolean voicecall_snd_get_status(voicecall_snd_mgr_t *papp_snd, voicecall_snd_audio_type_t snd_audio_type);
#endif

/**
* This function sets the volume level for the given volume alert type
*
* @return		void
* @param[in]	papp_snd		Handle to Sound Manager
* @param[in]	vol_alert_type	volume alert type #voicecall_snd_volume_alert_type_t
* @param[in]	volume_level	volume level to be set
*/
void voicecall_snd_set_volume(voicecall_snd_mgr_t *papp_snd, voicecall_snd_volume_alert_type_t vol_alert_type, int volume_level);

/**
 * This function retreives the volume according to the given volume alert type
 *
 * @return		current volume level
 * @param[in]		papp_snd		Handle to Sound Manager
 * @param[in]		vol_alert_type		volume alert type #voicecall_snd_volume_alert_type_t
 */
int voicecall_snd_get_volume(voicecall_snd_mgr_t *papp_snd, voicecall_snd_volume_alert_type_t vol_alert_type);

/**
 * This function retreives the volume according to the given volume alert type
 *
 * @return		return TRUE if alternate playing
 * @param[in]		papp_snd		Handle to Sound Manager
 */
gboolean voicecall_snd_get_alternate_play(voicecall_snd_mgr_t *papp_snd);

/**
* This function registers the application with the sound conflict manager
*
* @return			void
* @param[in]		papp_snd		Handle to Sound Manager
*/
void voicecall_snd_register_cm(voicecall_snd_mgr_t *papp_snd);

/**
* This function unregisters the application from the sound conflict manager
*
* @return			void
* @param[in]		papp_snd		Handle to Sound Manager
*/
void voicecall_snd_unregister_cm(voicecall_snd_mgr_t *papp_snd);

void voicecall_snd_prepare_alert(voicecall_snd_mgr_t *papp_snd, int call_handle);
void voicecall_snd_play_alert(voicecall_snd_mgr_t *papp_snd);

void voicecall_snd_set_to_defaults(voicecall_snd_mgr_t *papp_snd);

gboolean voicecall_snd_is_effect_playing(voicecall_snd_mgr_t *papp_snd);
#ifdef __cplusplus
}
#endif

#endif
