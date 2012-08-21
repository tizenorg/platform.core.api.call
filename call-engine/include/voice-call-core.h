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


#ifndef _VOICE_CALL_CORE_H_
#define _VOICE_CALL_CORE_H_

#ifdef __cplusplus
extern "C" {

#endif

#include "vc-core-util.h"
#include "vc-core-engine-types.h"
#include "voice-call-sound.h"
#include "voice-call-engine.h"

typedef enum _call_vc_core_flags_t {
	CALL_VC_CORE_FLAG_NONE = 0x00000000, /**< NONE state */
	CALL_VC_CORE_FLAG_SOS_CALL_ONLY = 0x00000001, /**< SET - Emergency Calls Only, UNSET - All Calls Allowed */
	CALL_VC_CORE_FLAG_MSG_SVC_INIT = 0x00000002, /**< SET - Messenger service intialization done, UNSET - otherwise */
	CALL_VC_CORE_FLAG_FDN_SVC_ENABLED = 0x00000004, /**< SET - FDN enabled, UNSET - otherwise */
	CALL_VC_CORE_FLAG_VOL_CHNGD_BYBT = 0x00000008, /**< SET - volume change request from BT, UNSET - otherwise */
	CALL_VC_CORE_FLAG_BT_EVENT_WAITING = 0x00000010, /**< SET - Waiting for Switch Headset/Phone Event fom Bluetooth, UNSET - otherwise */
	CALL_VC_CORE_FLAG_IT_WAS_LOCKED = 0x00000020, /**< SET - It was locked when voice call app luanched, UNSET - otherwise */
	CALL_VC_CORE_FLAG_UNLOC_BY_SLIDER = 0x00000040, /**< SET - Voice call unlock by slider, UNSET - otherwise */
	CALL_VC_CORE_FLAG_EXPT_APPLET_QUIT = 0x00000080, /**< SET - Thread safe applet quit required, UNSET - otherwise */
	CALL_VC_CORE_FLAG_LANG_CHGD_UPDATE = 0x00000100, /**< SET - UI needs to be updated for language change, UNSET - otherwise */
	CALL_VC_CORE_FLAG_WAIT_SWITCH_FROM_BT = 0x00000200, /**< SET - Don't change path in path_change API, UNSET - change path according to current status */
	CALL_VC_CORE_FLAG_SETUPCALL_FAIL = 0x00000400, /**< SET - Setup call fail.(ex. Emergency call only). UNSET - Normal mo call.*/
	CALL_VC_CORE_FLAG_ACCEPT_BY_BT = 0x00000800, /**< SET - MT call accpeted by BT Headset, UNSET - MT call accept by Phone/ Call ended.*/
	CALL_VC_CORE_FLAG_QC_SIM_INSERTED = 0x00001000, /**< SET - Inserted SIM is QC Test SIM, UNSET - Inserted SIM is Normal SIM.*/
	CALL_VC_CORE_FLAG_MOCALL_END_BY_USER = 0x00002000, /**< SET - MO Call Ended by user, UNSET - MO Call not ended by user.*/
} call_vc_core_flags_t;

/**
 * This enum defines the In Call Supplementary Services State
 */
typedef enum _call_vc_core_incall_ss_state_t {
	CALL_VC_CORE_SS_NONE, /**< Idle State*/
	CALL_VC_CORE_SS_0, /**< Releases all held calls or Set UDUB for a waiting call*/
	CALL_VC_CORE_SS_1, /**< Releases all active calls and accepts the other(held or waiting) calls*/
	CALL_VC_CORE_SS_1X, /**< Releases a specific active call X*/
	CALL_VC_CORE_SS_2, /**< Places all active calls (if  any exist) on hold and accepts the other(held or waiting)call*/
	CALL_VC_CORE_SS_2X, /**< Places all active calls on hold except call X with which communication shall be supported*/
	CALL_VC_CORE_SS_3, /**< Adds a held call to the conversation*/
	CALL_VC_CORE_SS_4, /**< ECT */
	CALL_VC_CORE_SS_USSD /**< USSD */
} call_vc_core_incall_ss_state_t;

/**
* This structure defines the details of the DTMF related data handled in document
*/
typedef struct _voicecall_dtmf_info_t {
	gboolean bdtmf_queue;		/**< TRUE -dtmf queue enabled, FALSE - dtmf quue disabled */
	int dtmf_index;				/**< index of the current dtmf digit sent */
	int dtmf_max_length;		/**< maximum length of the dtmf number being sent */
	int dtmf_pause_timer;		/**< Timer handle of the DTMF Pause Timer */
	gboolean bsat_dtmf;			/**< SAT DTMF Type Engine Flag*/
	gboolean bsat_hidden;		/**< SAT DTMF Hidden Engine Flag*/
	gboolean bdtmf_wait;		/**< Wait enabled while sending dtmf string*/
	char dtmf_number[VC_PHONE_NUMBER_LENGTH_MAX];
} voicecall_dtmf_info_t;

/**
 * This structure defines voicecall core info
 */
typedef struct _call_vc_core_state_t {
	voicecall_setup_info_t call_setup_info;		/**< Info of MO Call */
	voicecall_engine_t *pcall_engine;		/**<  Handle to Voicecall Engine */
	voicecall_snd_mgr_t *papp_snd;			/**< Handle to Sound Manager */
	voicecall_dtmf_info_t dtmf_info;				/**<  DTMF Info*/

	unsigned int core_status;					/**< call core status */
	gboolean bt_connected;					/**< TRUE - Bluetooth connected, FALSE - otherwise */
	gboolean bdtmf_queue;						/**< TRUE -dtmf queue enabled, FALSE - dtmf quue disabled */
	int auto_redial_count;			      /** Auto Redial count **/
	unsigned int auto_answer_timer;				/**< Timer Handle for Auto Answering */
	unsigned int callstatus_timer;				/**< Timer Handle for callstatus set. */
	int selected_call_id_in_ss;						/**<Call Id used for SS operations while on Call */
	int mo_end_cause_type;						/**<MO call end cause type */
	int mo_end_call_handle;						/**<MO call end call handle */
	int mtcall_silent_reject_handle;
	guint mo_redial_timer;
	guint minute_minder_timer;
} call_vc_core_state_t;
/**
 * This struct provides a structure for call incoming info data.
 */
typedef struct _call_vc_core_incoming_info_t {
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
} call_vc_core_incoming_info_t;

/**
 * This function puts the currently active call on hold
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_hold_call(voicecall_engine_t *pcall_engine);

/**
 * This function retreives the currently held call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_retrieve_call(voicecall_engine_t *pcall_engine);

/**
 * This function swaps the currently available active and held calls
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_swap_calls(voicecall_engine_t *pcall_engine);

/**
 * This function clears the MO Call Details
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_clear_mo_call(voicecall_engine_t *pcall_engine);

/**
 * This function clears the Connected Call Details
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		call_handle		Handle of the Connected Call to be cleared
 */
inline gboolean voicecall_core_clear_connected_call(voicecall_engine_t *pcall_engine, int call_handle);

/**
 * This function changes the voicecall engine's state
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		eng_state		Engine State to be changed
 */
inline gboolean voicecall_core_change_engine_state(voicecall_engine_t *pcall_engine, int eng_state);

/**
 * This function ends an Outgoing Call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_end_mo_call(voicecall_engine_t *pcall_engine);

/**
 * This function retreives the Voicecall Engine's State
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[out]	io_state			Voicecall Engine InOut State
 */
inline gboolean voicecall_core_get_engine_state(voicecall_engine_t *pcall_engine, int *eng_state);

/**
 * This function checks whether any call exists
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[out]	active_calls		TRUE - If active call exists or FALSE If active call doesn't exists
 * @param[out]	held_calls		TRUE - If held call exists or FALSE If held call doesn't exists
 */
inline gboolean voicecall_core_is_call_exists(voicecall_engine_t *pcall_engine, gboolean *active_calls, gboolean *held_calls);

/**
 * This function checks whether incoming call exists or not
 *
 * @return		Returns TRUE if incoming call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_incoming_call_exists(voicecall_engine_t *pcall_engine);

/**
 * This function checks whether outgoing call exists or not
 *
 * @return		Returns TRUE if outgoing call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_outgoing_call_exists(voicecall_engine_t *pcall_engine);

/**
 * This function checks whether any connexcted call exists or not
 *
 * @return		Returns TRUE if connected call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_connected_call_exist(voicecall_engine_t *pcall_engine);

/**
 * This function checks whether any connexcted call exists or not in the given group
 *
 * @return		Returns TRUE if connected call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		group_index		Group Index to be searhced
 */
inline gboolean voicecall_core_is_connected_call_exist_in_group(voicecall_engine_t *pcall_engine, int group_index);

/**
 * This function checks whether any call exists
 *
 * @return		Returns TRUE if connected call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_any_call_exists(voicecall_engine_t *pcall_engine);

/**
 * This function retreives the totally number of availavle calls including connected, MO and MT Calls
 *
 * @return		Returns TRUE if success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[out]	ptotal_call_member	number of avialble calls
 */
inline gboolean voicecall_core_get_total_call_member(voicecall_engine_t *pcall_engine, int *ptotal_call_member);

/**
 * This function checks whether voicecall engine's call agent is idle or not
 *
 * @return		Returns TRUE if call agent is idle or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_callagent_idle(voicecall_engine_t *pcall_engine);

/**
* This function checks the current call status and engine status
*
* @return		TRUE, if connected calls available and engine is in idle, FALSE otherwise
* @param[in]		pcall_engine		Handle to voicecall engine
*/
inline gboolean voicecall_core_is_incall_request_possible(voicecall_engine_t *pcall_engine);

/**
 * This function changes the modem call audio path
 *
 * @return		TRUE sucess, FALSE otherwise
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		audio_path		audio path to be changed
 * @param[in]		bextra_volume	extra volume status
 */
inline gboolean voicecall_core_change_audio_path(voicecall_engine_t *pcall_engine, voicecall_audio_path_t audio_path, gboolean bextra_volume);

/**
 * This function sets the voice call audio volume for the given audio path type
 *
 * @return		returns TRUE in success , FALSE otherwise
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		tapi_snd_path		audio path for the volume to be set
 * @param[in]		vol_level			volume level
 */
inline gboolean voicecall_core_set_audio_volume(voicecall_engine_t *pcall_engine, voicecall_audio_path_t tapi_snd_path, int vol_level);

/**
 * This function retreives the voice call audio volume for the given audio path type
 *
 * @return		returns TRUE in success , FALSE otherwise
 * @param[in]		pcall_engine			Handle to voicecall engine
 * @param[in]		audio_path_type		audio path for the volume to be retreived
 */
inline gboolean voicecall_core_get_audio_volume(voicecall_engine_t *pcall_engine, voicecall_audio_path_t audio_path_type);

/**
 * This function set the voice call audio mute status
 *
 * @return		returns TRUE in success , FALSE otherwise
 * @param[in]		pcall_engine			Handle to voicecall engine
 * @param[in]		bvoice_mute			mute status
 */
inline gboolean voicecall_core_set_audio_mute_status(voicecall_engine_t *pcall_engine, gboolean bvoice_mute);

/**
 * This function retreives the first active call among the available active calls
 *
 * @return		Returns TRUE if success or FALSE on failure
 * @param[in]		pcall_engine			Handle to voicecall engine
 * @param[out]	pcall_handle		call handle of the active call
 */
inline gboolean voicecall_core_get_zuhause(voicecall_engine_t *pcall_engine, gboolean * bzuhause);

/**
 * This function retreives the Voicecall Engine's State
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]	pcall_engine	Handle to voicecall engine
 * @param[in]	call_handle	Call handle of the call for which the call object is retrieved
 * @param[out]	pcall_object	Pointer to the retrived call object info
 */
inline gboolean voicecall_core_get_call_object(voicecall_engine_t *pcall_engine, int call_handle, call_vc_call_objectinfo_t * pcall_object);

/**
 * This function sends response to sat engine
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]	pcall_engine	Handle to voicecall engine
 * @param[in]		sat_rqst_resp_type sat rqst/resp type to be set by the client
 * @param[in]		sat_response_type sat response type to be sent to sat
 */
inline gboolean voicecall_core_send_sat_response(voicecall_engine_t *pcall_engine, voicecall_engine_sat_rqst_resp_type sat_rqst_resp_type, call_vc_sat_reponse_type_t sat_response_type);

/**
 * This function retreives the number of active call members
 *
 * @return		Returns TRUE if success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[out]	pactive_member_num	number of active call members available
 */
inline gboolean voicecall_core_get_active_call_member(call_vc_core_state_t *pcall_core, int *pactive_member_num);

/**
 * This function checks whether possible to make conference call
 *
 * @return		Returns TRUE If Conference call can be made or FALSE if not
 * @param[in]		papp_document	Handle to Application Document
 */
inline gboolean voicecall_core_is_conf_call_possible(call_vc_core_state_t *pcall_core);

/**
 * This function checks whether possible to transfer call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 */
inline gboolean voicecall_core_is_transfer_call_possible(call_vc_core_state_t *pcall_core);

/**
 * This function checks whether the given code is a valid Supplementary Services Code
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		pszInput			Number to be verified
 */
inline gboolean voicecall_core_is_valid_sscode(call_vc_core_state_t *pcall_core, const char *pszInput);

#ifdef _CPHS_DEFINED_
/**
 * This function gets the cphs status from the engine
 *
 * @return		TRUE if queried status is enabled, FALSE otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		csp_service		csp service to be queried
 */
inline gboolean voicecall_core_get_cphs_csp_status(call_vc_core_state_t *pcall_core, voicecall_cphs_csp_service csp_service);
#endif

/**
 * This function informs the Voicecall Engine that current SS operation has been completed
 *
 * @return		Returns TRUE if all the calls are ended or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall cor
 */
inline gboolean voicecall_core_set_check_ss_on_end(call_vc_core_state_t *pcall_core);

/**
 * This function informs the Voicecall Engine that current SS operation has been completed
 *
 * @return		Returns TRUE if all the calls are ended or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
inline void voicecall_core_extract_phone_number(const char *source_tel_number, char *phone_number, const int buf_size);

/************************
*  inline function END
**************************/
void voicecall_core_set_status(call_vc_core_state_t *pcall_core, call_vc_core_flags_t core_flags, gboolean bstatus);
gboolean voicecall_core_get_status(call_vc_core_state_t *pcall_core, call_vc_core_flags_t core_flags);

/**
 * This function starts the supplementary services while on call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		incall_ss_state		state of the In Call Supplementary Service
 */
gboolean voicecall_core_start_incall_ss(call_vc_core_state_t *pcall_core, call_vc_core_incall_ss_state_t incall_ss_state);

gboolean voicecall_core_set_to_default(call_vc_core_state_t *pcall_core);

/**
 * This function initialize voicecall core
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
int voicecall_core_init(call_vc_core_state_t *pcall_core);

/**
 * This function prepares a voice call with the given data
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		bemergency		emergency call or not from dialer
 * @param[in]
 */
gboolean voicecall_core_setup_call(call_vc_core_state_t *pcall_core, gboolean bemergency);

/**
 * This function makes the actual voicecall prepared by the #voicecall_core_setup_call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_make_call(call_vc_core_state_t *pcall_core);
gboolean voicecall_core_process_sat_setup_call(vcall_engine_sat_setup_call_info_t *sat_setup_call_info);

/**
 * This function processed incoming call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]
*/
gboolean voicecall_core_process_incoming_call(call_vc_core_incoming_info_t *incoming_call_info);

/**
 * This function answers an incoming call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]
*/
gboolean voicecall_core_answer_call(call_vc_core_state_t *pcall_core, gboolean auto_accept);

/**
 * This function answers an incoming call  according to the given type
 *
 * @return		Returns TRUE -if answer is sucess, FALSE - otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_answer_call_bytype(call_vc_core_state_t *pcall_core, voicecall_answer_type_t answer_type);

/**
* This function rejects an incoming call
*
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pcall_core		Handle to voicecall core
* @param[in]		bUDUB			TRUE - set UDUB, FALSE - reject call
*/
gboolean voicecall_core_reject_mt(call_vc_core_state_t *pcall_core, gboolean bUDUB);

/**
 * This function ends the call by state
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_end_call(call_vc_core_state_t *pcall_core);

/**
 * This function ends the call corresponding to the given call handle
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		call_handle		handle of the call to be ended
 */
gboolean voicecall_core_end_call_by_handle(call_vc_core_state_t *pcall_core, int call_handle);

/**
 * This function ends all available calls
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_end_all_calls(call_vc_core_state_t *pcall_core);

/**
 * This function ends all available active calls
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_end_all_active_calls(call_vc_core_state_t *pcall_core);

/**
 * This function ends all available held calls
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_end_all_held_calls(call_vc_core_state_t *pcall_core);

/**
 * This function cancel outgoing call
 *
 * @return		Returns TRUE -if answer is sucess, FALSE - otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_cancel_call(call_vc_core_state_t *pcall_core);

/**
 * This function process hold/retrive/swap conntected call
 *
 * @return		Returns TRUE -if answer is sucess, FALSE - otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_process_hold_call(call_vc_core_state_t *pcall_core);

/**
 * This function sets up a conference call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_setup_conference(call_vc_core_state_t *pcall_core);

/**
 * This function splits the call corressponding to the given call handle and makes a private call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		call_handle		Call to be splitted from the conference call
 */
gboolean voicecall_core_make_private_call(call_vc_core_state_t *pcall_core, int call_handle);

/**
 * This function transfers the call from active call to the held call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_transfer_calls(call_vc_core_state_t *pcall_core);

/**
 * This function sends a dtmf string
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		dtmf_string		dtmf string to be sent
 */
gboolean voicecall_core_send_dtmf(call_vc_core_state_t *pcall_core, char *dtmf_string);

/**
* This function stops sound alert in case of reject with msg
*
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pcall_core		Handle to voicecall core
*/
gboolean voicecall_core_stop_alert(call_vc_core_state_t *pcall_core);

/**
* This function proce bt headset msg.
*
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pcall_core		Handle to voicecall core
* @param[in]		bheadset			TRUE or FALSE
*/
inline gboolean voicecall_core_prepare_redial(call_vc_core_state_t *pcall_core, int call_handle);
gboolean voicecall_core_start_redial(call_vc_core_state_t *pcall_core, int manual_redial);
gboolean voicecall_core_stop_redial(call_vc_core_state_t *pcall_core);

/**
 * This function parses the in call supplementary services string and returns the in call ss to be used
 *
 * @return		Returns in call ss state #vcui_app_incall_ss_state_t
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		number			number to be parsed
 */
call_vc_core_incall_ss_state_t voicecall_core_parse_incall_ss_string(call_vc_core_state_t *pcall_core, char *number);

/**
 * This function starts the supplementary services while on call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		incall_ss_state		state of the In Call Supplementary Service
 */
gboolean voicecall_core_start_incall_ss(call_vc_core_state_t *pcall_core, call_vc_core_incall_ss_state_t incall_ss_state);

/**
 * This function processed the supplementary services while on call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		pdialled_number		dial number
*/
void voicecall_core_process_incall_ss(call_vc_core_state_t *pcall_core, char *pdialled_number);

/**
 *** This function sends the dtmf number
 ***
 *** @return              Returns TRUE on success
 *** @param[in]           puser_data	pointer to user data
 **/
gboolean voicecall_core_send_phone_number_dtmf(gpointer puser_data);

/**
 *** This function change sound path
 ***
 *** @return              Returns TRUE on success
 *** @param[in]           sound_path
 **/
gboolean voicecall_core_change_sound_path(call_vc_core_state_t *pcall_core, voicecall_snd_audio_type_t sound_path);
gboolean voicecall_core_get_sound_path(call_vc_core_state_t *pcall_core, int *sound_path);

/**
 * This function processed the supplementary services while on call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 */
void voicecall_core_process_dtmf_send_status(call_vc_core_state_t *pcall_core, gboolean bsuccess);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _VOICE_CALL_CORE_H_ */
