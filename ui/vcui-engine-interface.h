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


#ifndef _VOICE_CALL_UI_IPC_
#define _VOICE_CALL_UI_IPC_

#include "voice-call-engine.h"

typedef enum _vcui_snd_path_type_t{
	VCUI_SND_PATH_NONE,
	VCUI_SND_PATH_HEADSET,
	VCUI_SND_PATH_EARJACK,
	VCUI_SND_PATH_SPEAKER,
	VCUI_SND_PATH_MAX,
}vcui_snd_path_type_t;

typedef enum _vcui_call_type_t {
	VCUI_CALL_TYPE_MO,
	VCUI_CALL_TYPE_MT,
	VCUI_CALL_TYPE_ECC,
	VCUI_CALL_TYPE_ECC_TEST,
	VCUI_CALL_TYPE_DOWNLOAD_CALL,
	VCUI_CALL_TYPE_SAT,
	VCUI_CALL_TYPE_MAX
} vcui_call_type_t;

typedef struct _vcui_call_mo_data_t {
	char call_number[VC_PHONE_NUMBER_LENGTH_MAX];
	int ct_index;
} vcui_call_mo_data_t;

typedef struct _vcui_call_mt_data_t {
	int call_handle;
	int call_type;
	int cli_presentation_indicator;
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];
	int calling_name_mode;
	char calling_name[VC_PHONE_NAME_LENGTH_MAX];
	char redirected_number[VC_PHONE_NUMBER_LENGTH_MAX];
	char redirected_sub_address[VC_PHONE_SUBADDRESS_LENGTH_MAX];
	int cli_cause;
	int bfwded;
	int active_line;
} vcui_call_mt_data_t;

typedef struct _vcui_call_ecc_data_t {
	char call_number[VC_PHONE_NUMBER_LENGTH_MAX];
} vcui_call_ecc_data_t;

typedef struct _vcui_call_sat_data_t {
	int command_id;								/**<Proactive Command Number sent by USIM*/
	int command_qualifier;						/**<call type*/
	char disp_text[500 + 1];					/**<character data*/
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];;	/**<call number*/
	unsigned int duration;						/**<maximum repeat duration*/
} vcui_call_sat_data_t;

typedef enum _vcui_vol_type_t {
	VCUI_VOL_RING,
	VCUI_VOL_VOICE,
	VCUI_VOL_HEADSET,
	VCUI_VOL_MAX
} vcui_vol_type_t;

typedef enum _vcui_rec_result_t {
	VCUI_REC_RESULT_SUCCESS,
	VCUI_REC_RESULT_NOT_ENOUGH_MEMORY,
	VCUI_REC_RESULT_NO_CONNECTED_CALL,
	VCUI_REC_RESULT_MAX
} vcui_rec_result_t;

/**
* This enumeration defines names of the on call audio status
*/
typedef enum _vcui_audio_type_t {
	VCUI_AUDIO_NONE,				/**< none*/
	VCUI_AUDIO_SPEAKER,			/**< System LoudSpeaker Audio */
	VCUI_AUDIO_RECEIVER, 		/**< System Receiver Audio */
	VCUI_AUDIO_HEADSET,			/**< System Headset Audio */
	VCUI_AUDIO_EARJACK,			/**< System Earjack Audio */
	VCUI_AUDIO_MAX,
} vcui_audio_type_t;

void _vcui_engine_init(vcui_app_call_data_t *ad);
void _vcui_engine_answer_call(void);
void _vcui_engine_answer_call_by_type(int type);
void _vcui_engine_cancel_call(void);
void _vcui_engine_reject_call(void);
void _vcui_engine_stop_alert(void);
void _vcui_engine_end_call(void);
void _vcui_engine_end_call_by_handle(int handle);
void _vcui_engine_end_all_call(void);
void _vcui_engine_end_active_calls(void);
void _vcui_engine_end_held_calls(void);
void _vcui_engine_hold_unhold_swap_call(void);
void _vcui_engine_join_call(void);
void _vcui_engine_split_call(int call_handle);
void _vcui_engine_transfer_call(void);
void _vcui_engine_speaker_on_off(int bLoundSpeaker);
void _vcui_engine_mute_on_off(int bMute);
void _vcui_engine_set_volume_level(vcui_vol_type_t vol_type, int level);
int _vcui_engine_get_volume_level(vcui_vol_type_t vol_type);
void _vcui_engine_change_sound_path(vcui_audio_type_t sound_path);
vcui_audio_type_t _vcui_engine_get_sound_path(void);

void _vcui_engine_interface_process_auto_redial(int bRedial);
void _vcui_engine_interface_process_voice_record(int bRecord);
void _vcui_engine_interface_process_mute_alert(void);
void _vcui_engine_interface_send_dtmf_number(char data);
void _vcui_engine_interface_process_mo_call(vcui_call_type_t call_type, vcui_call_mo_data_t *data);
void _vcui_engine_interface_process_mt_call(vcui_call_type_t call_type, vcui_call_mt_data_t *data);
void _vcui_engine_interface_process_ecc_call(vcui_call_type_t call_type, vcui_call_ecc_data_t *data);
void _vcui_engine_interface_process_sat_call(vcui_call_type_t call_type, vcui_call_sat_data_t *data);

// Add callback function to handle voicecall-engine library events
void _vcui_engine_callback(int event, void *pdata, void *puser_data);

#endif
