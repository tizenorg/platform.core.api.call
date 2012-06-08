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


#ifndef _VOICE_CALL_ENGINE_H_
#define _VOICE_CALL_ENGINE_H_

#include <glib.h>
#include "vc-core-engine-types.h"

#define VCALL_SENSOR_NEAR	1
#define VCALL_SENSOR_FAR	2

/**
 * Voicecall Engine API result values
 */
typedef enum _vcall_engine_api_result_t {
	VCALL_ENGINE_API_FAILED = -1,
	VCALL_ENGINE_API_SUCCESS,

	VCALL_ENGINE_API_ACTIVE_CALL_EXIST,
	VCALL_ENGINE_API_ACTIVE_AND_HELD_CALL_EXIST,

	VCALL_ENGINE_API_CALLNOTALLOWED,
	VCALL_ENGINE_API_VOICE_CALL_IS_NOT_ALLOWED_DURING_VIDEO_CALL,
	VCALL_ENGINE_API_UNAVAILABLE,
	VCALL_ENGINE_API_CHANGEOFFLINEMODETOCALL,
	VCALL_ENGINE_API_CALLING_EMERG_ONLY,
	VCALL_ENGINE_API_FDNCALLONLY,
	VCALL_ENGINE_API_CALLFAILED,
	VCALL_ENGINE_API_CALLENDED,
	VCALL_ENGINE_API_SOS_CALL_ONLY_IN_NO_SIM_MODE,
	VCALL_ENGINE_API_PHONE_NOT_INITIALISED,
	VCALL_ENGINE_API_CAUSE_WRONG_NUMBER,

	VCALL_ENGINE_API_DTMFSENDING_FAIL,
	VCALL_ENGINE_API_HOLD_FAILED,
	VCALL_ENGINE_API_UNABLE_TO_RETRIEVE,
	VCALL_ENGINE_API_SWAP_FAILED,
	VCALL_ENGINE_API_SPLIT_FAILED,
	VCALL_ENGINE_API_JOIN_FAILED,
	VCALL_ENGINE_API_TRANSFER_FAILED,
	VCALL_ENGINE_API_SWAP_NOT_SUPPORTED,
	VCALL_ENGINE_API_HOLD_NOT_SUPPORTED,
	VCALL_ENGINE_API_UNHOLD_NOT_SUPPORTED,
	VCALL_ENGINE_API_JOIN_NOT_SUPPORTED,
	VCALL_ENGINE_API_SPLIT_NOT_SUPPORTED,
	VCALL_ENGINE_API_TRANSFER_NOT_SUPPORTED,
	VCALL_ENGINE_API_INCOMPLETE,

	VCALL_ENGINE_API_NOT_ENOUGH_MEMORY_FOR_RECORDING,
	VCALL_ENGINE_API_NO_CONNECTED_CALL_FOR_RECORDING,

	VCALL_ERROR_MAX
} vcall_engine_api_result_t;

/** 
 * This enum defines call answer types
 */
typedef enum _vcall_engine_answer_type_t {
	VCALL_ENGINE_ANSWER_HOLD_ACTIVE_AND_ACCEPT = 1,			/**< Puts the active call on hold and accepts the call (Only CONNECTED will be sent to client) */
	VCALL_ENGINE_ANSWER_RELEASE_ACTIVE_AND_ACCEPT,	/**< Releases the active call and accept the call (END and CONNECTED will be sent to Client) */
	VCALL_ENGINE_ANSWER_RELEASE_HOLD_AND_ACCEPT,	/**< Releases the active call and accept the call (END and  CONNECTED will be sent to client) */
	VCALL_ENGINE_ANSWER_RELEASE_ALL_AND_ACCEPT		/**< Releases the all calls and accept the call (END and  CONNECTED will be sent to client) */
} vcall_engine_answer_type_t;

/** 
 * This enum defines call release types
 */
typedef enum _vcall_engine_release_type_t {
	VCALL_ENGINE_RELEASE_ALL_ACTIVE_CALLS,		/**< To end all available active calls*/
	VCALL_ENGINE_RELEASE_ALL_HELD_CALLS,		/**< To end all available held calls*/
	VCALL_ENGINE_RELEASE_ALL_CALLS,					/**< To end all available calls(active,held,incoming/outgoing*/
} vcall_engine_release_type_t;

/** 
 * This enum defines call answer types
 */
typedef enum _vcall_engine_vol_type_t {
	VCALL_ENGINE_VOL_TYPE_RINGTONE,		/**< ringtone volume*/
	VCALL_ENGINE_VOL_TYPE_VOICE,		/**< voice volume*/
	VCALL_ENGINE_VOL_TYPE_HEADSET		/**< headset volume*/
} vcall_engine_vol_type_t;

/**
* This enumeration defines names of the on call audio status
*/
typedef enum _vcall_engine_audio_type_t {
	VCALL_ENGINE_AUDIO_NONE,				/**< none*/
	VCALL_ENGINE_AUDIO_SPEAKER,			/**< System LoudSpeaker Audio */
	VCALL_ENGINE_AUDIO_RECEIVER,		/**< System receiver Audio */
	VCALL_ENGINE_AUDIO_HEADSET,			/**< System Headset Audio */
	VCALL_ENGINE_AUDIO_EARJACK,			/**< System Earjack Audio */
	VCALL_ENGINE_AUDIO_MAX,
} vcall_engine_audio_type_t;

/**
* This enumeration defines names of lcd control
*/
typedef enum _vcall_engine_lcd_control_t {
	VCALL_ENGINE_LCD_OFF,		/**< LCD Off*/
	VCALL_ENGINE_LCD_ON,		/**< LCD On */
	VCALL_ENGINE_LCD_ON_LOCK,		/**< LCD ON lock */
	VCALL_ENGINE_LCD_ON_UNLOCK,		/**< LCD ON unlock */
	VCALL_ENGINE_LCD_MAX,
} vcall_engine_lcd_control_t;

typedef enum {
	VC_LCD_OFF = 1,
	VC_LCD_ON,
	VC_LCD_ON_LOCK,
	VC_LCD_ON_UNLOCK,
} voicecall_lcd_control_t;

/** 
 * This struct provides a structure for call setup info data.
 */
typedef struct _vcall_engine_setup_info_t {
	int contact_index;
	int storage_type;
	int phone_type;
	int bemergency;
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];
	char call_name[VC_DISPLAY_NAME_LENGTH_MAX];
	char call_file_path[VC_IMAGE_PATH_LENGTH_MAX];
	char call_full_file_path[VC_IMAGE_PATH_LENGTH_MAX];
} vcall_engine_setup_info_t;

/** 
 * This struct provides a structure for sat setup call info data.
 */
typedef struct _vcall_engine_sat_setup_call_info_t {
	int command_id;								/**<Proactive Command Number sent by USIM*/
	int command_qualifier;	/**<call type*/
	char disp_text[500 + 1];	/**<character data*/
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];;							/**<call number*/
	unsigned int duration;						/**<maximum repeat duration*/
} vcall_engine_sat_setup_call_info_t;

/** 
 * This struct provides a structure for call incoming info data.
 */
typedef struct _vcall_engine_incoming_info_t {
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
} vcall_engine_incoming_info_t;

/**< This is the prototype of the client callback function  */
typedef gboolean(*vcall_engine_app_cb) (int event, void *pdata, void *puser_data);

gboolean vcall_engine_send_event_to_client(int event, void *pdata);

/**
 * This function initialize voice call engine.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_init(vcall_engine_app_cb pcb_func, void *pusr_data);

/**
 * This function processes mo nomal call.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_process_normal_call(char *number, int ct_index, gboolean b_download_call);

/**
 * This function processes mo emergency call.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_process_emergency_call(char *number);

int vcall_engine_process_sat_setup_call(vcall_engine_sat_setup_call_info_t *sat_setup_call_info);

/**
 * This function processes incoming call.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_process_incoming_call(vcall_engine_incoming_info_t *incoming_call_info);

/**
 * This function answers an incoming call
 *
 * @return	int	API Result Code.
 * @param[in]		none
*/
int vcall_engine_answer_call(void);

/**
 * This function answers an incoming call  according to the given type
 *
 * @return	int	API Result Code.
 * @param[in]		answer_type		answer type
 */
int vcall_engine_answer_call_by_type(vcall_engine_answer_type_t answer_type);

/**
 * This function cancel outgoing call
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_cancel_call(void);

/**
 * This function reject incoming call
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_reject_call(void);

/**
 * This function release a active or held call
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_release_call(void);

/**
 * This function release a call by handle
 *
 * @return	int	API Result Code.
 * @param[in]		int	call_handle
 */
int vcall_engine_release_call_by_handle(int call_handle);

/**
 * This function release calls by type
 *
 * @return	int	API Result Code.
 * @param[in]		release_type release_type
 */
int vcall_engine_release_call_by_type(vcall_engine_release_type_t release_type);

/**
 * This function processes hold/retrive/swap calls.
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_process_hold_call(void);

 /**
 * This function processes in call SS code..
 *
 * @return	NONE
 * @param[in]		none
 */
void vcall_engine_process_incall_ss(char *number);

/**
 * This function sets up a conference calls
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_join_call(void);

/**
 * This function make a private call in conference calls.
 *
 * @return	int	API Result Code.
 * @param[in]		int	call_handle
 */
int vcall_engine_split_call(int call_handle);

/**
 * This function transfers the call from active call to the held call
 *
 * @return	int	API Result Code.
 * @param[in]		int	call_handle
 */
int vcall_engine_transfer_call(void);

/**
 * This function processed loud speaker.
 *
 * @return	int	API Result Code.
 * @param[in]		int	bstatus
 */
int vcall_engine_process_loudspeaker(int bstatus);

/**
 * This function processed voice mute status.
 *
 * @return	int	API Result Code.
 * @param[in]		int	bstatus
 */
int vcall_engine_process_voice_mute(int bstatus);

/**
 * This function gets the volume level
 *
 * @return	int	API Result Code.
 * @param[in]		vcall_engine_vol_type_t vol_type
 */
int vcall_engine_get_volume_level(vcall_engine_vol_type_t vol_type);

/**
 * This function sets the volume level
 *
 * @return	int	API Result Code.
 * @param[in]		vcall_engine_vol_type_t vol_type
 * @param[in]		int vol_level
 */
int vcall_engine_set_volume_level(vcall_engine_vol_type_t vol_type, int vol_level);

/**
 * This function stop alert
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_stop_alert(void);

/**
 * This function mute alert.
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_mute_alert(void);

/**
 * This function process auto redial.
 *
 * @return	int	API Result Code.
 * @param[in]		int bstatus
 */
int vcall_engine_process_auto_redial(int bstatus);

/**
 * This function process voice record.
 *
 * @return	int	API Result Code.
 * @param[in]		int bstatus
 */
int vcall_engine_process_voice_record(int bstatus);

/**
 * This function send the DTMF number
 *
 * @return	int	API Result Code.
 * @param[in]		char* dtmf_number
 */
int vcall_engine_send_dtmf_number(char *dtmf_number);

/**
 * This function processed sound path
 *
 * @return	int	API Result Code.
 * @param[in]		int	sound_path
 */
int vcall_engine_change_sound_path(vcall_engine_audio_type_t sound_path);

/**
 * This function get sound path
 *
 * @return	int	API Result Code.
 * @param[out]		int	sound_path
 */
int vcall_engine_get_sound_path(int *sound_path);

int vcall_engine_set_to_default();

/**
 * This function retrieves the core state instance
 *
 * @return	instance of the core state
 * @param[in]		void
 */
gpointer vcall_engine_get_core_state(void);

/**
 * This function is interface to call-utility to perform string copy
 *
 * @return	instance of the core state
 * @param[out]	pbuffer		Target Buffer
 * @param[in]		buf_count	Size of Target Buffer
 * @param[in]		pstring		Source String
 */
gboolean vcall_engine_util_strcpy(char *pbuffer, int buf_count, const char *pstring);

/**
 * This function returns the number of groups
 *
 * @param[in]		pcount		count of the groups
 */
gboolean vcall_engine_get_group_count(int *pcount);

/**
 * This function is interface to call-utility to perform string copy
 *
 * @return	instance of the core state
 * @param[out]	pbuffer		Target Buffer
 * @param[in]	time		time
 */
char *vcall_engine_util_get_date_time(time_t time);

/**
 * This function is force reset all engine status.
 *
 * @return	void
 * @param[in] void
 */
void vcall_engine_force_reset(void);
#endif				/* _VOICE_CALL_ENGINE_H_ */
