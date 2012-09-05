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


#ifndef __VC_CORE_ENGINE_H_
#define __VC_CORE_ENGINE_H_

#include "vc-core-engine-types.h"
#include "vc-core-error.h"
#include "vc-core-engine-group.h"
#include "vc-core-engine-status.h"
#include <stdbool.h>
#include <tapi_common.h>

/*Voicecall Engine Exposed API's */

void _vc_core_engine_handle_sat_events_cb(void *sat_setup_call_data, void *userdata);

void _vc_core_engine_handle_incoming_tapi_events(void *mt_data, void *userdata);

/**
 * This function initializes the voicecall engine
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[out]	pcall_agent_out	Pointer to the address of call agent
* @param[in]		pcallback_func	Call back function
 * @param[in]		puser_data		Data set by user
* @remarks		pcall_agent_out and pcallback_func cannot be NULL.
 */
voicecall_error_t _vc_core_engine_init(voicecall_engine_t **pcall_agent_out, voicecall_cb pcallback_func, void *puser_data);

/**
* This function prepares the call setup info structure for making call
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	        Handle to voicecall engine
* @param[in]	psetup_call_info	Pointer to the call setup info structure.
* @remarks		pvoicecall_agent and psetup_call_info cannot be NULL
*				Only on successfull completion of this API, _vc_core_engine_make_call can be made
* @see			See following API's also
*				-_vc_core_engine_make_call
*				-voicecall_clear_prepared_call
 */
voicecall_error_t _vc_core_engine_prepare_call(voicecall_engine_t *pvoicecall_agent, voicecall_setup_info_t *psetup_call_info);

/**
* This function establishes an outgoing call with the details prepared using _vc_core_engine_prepare_call
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent    Handle to voicecall engine
* @param[in]	mo_call_index		Index of the prepare mo call
* @param[out]	pcall_handle		Handle of the MO Call Made
* @remarks		pvoicecall_agent and pcall_handle cannot be NULL
* @see			_vc_core_engine_end_call
 */
voicecall_error_t _vc_core_engine_make_call(voicecall_engine_t *pvoicecall_agent, int mo_call_index, int *pcall_handle);

/**
* This function answers a call according to the given answer type
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	answer_type		The answer type to be used
* @remarks		pvoicecall_agent and pcall_handle cannot be NULL
* @see			_vc_core_engine_reject_call
 */
voicecall_error_t _vc_core_engine_answer_call(voicecall_engine_t *pvoicecall_agent, voicecall_answer_type_t answer_type);

/**
* This function rejects the incoming call if any
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	budub			User Determined User Busy - TRUE, Else - FALSE
* @exception		In case of exceptions return value contains appropriate error code.
* @remarks		pvoicecall_agent cannot be NULL
* @see			_vc_core_engine_answer_call
*/
voicecall_error_t _vc_core_engine_reject_call(voicecall_engine_t *pvoicecall_agent, gboolean budub);

/**
* This function ends the call according to the given end call type
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	end_call_type		End Call Type
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also following functions
*				- _vc_core_engine_make_call
*				- _vc_core_engine_end_call_byhandle
*				- _vc_core_engine_end_call_bycallId
*				.
 */
voicecall_error_t _vc_core_engine_end_call(voicecall_engine_t *pvoicecall_agent, _vc_core_engine_end_call_type_t end_call_type);

/**
* This function ends only the connected call corresponding to the given call handle
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle		Call handle of the call to be ended
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also following functions
*				- _vc_core_engine_make_call
*				- _vc_core_engine_end_call
*				- _vc_core_engine_end_call_bycallId
 */
voicecall_error_t _vc_core_engine_end_call_byhandle(voicecall_engine_t *pvoicecall_agent, int call_handle);

/**
* This function ends a call corresponding to the given call ID
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_id			call id of the call to be ended
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also following functions
*				- _vc_core_engine_make_call
*				- _vc_core_engine_end_call
*				- _vc_core_engine_end_call_byhandle
 */
voicecall_error_t _vc_core_engine_end_call_bycallId(voicecall_engine_t *pvoicecall_agent, int call_id);

/**
* This function puts the active call if any on hold
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	        Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
* @see			voicecall_retreive_call
 */
voicecall_error_t _vc_core_engine_hold_call(voicecall_engine_t *pvoicecall_agent);

/**
* This function retrieves/activates the held call
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
* @see			_vc_core_engine_hold_call
 */
voicecall_error_t _vc_core_engine_retrieve_call(voicecall_engine_t *pvoicecall_agent);

/**
* This function swaps the active and held calls if any available
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also the following APIs
*				- _vc_core_engine_hold_call
*				- _vc_core_engine_retrieve_call
*				.
 */
voicecall_error_t _vc_core_engine_swap_calls(voicecall_engine_t *pvoicecall_agent);

/**
* This function does the explicit call transfer
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
 */
voicecall_error_t _vc_core_engine_transfer_calls(voicecall_engine_t *pvoicecall_agent);

/**
* This function sets up a conference beween the currently available active and held calls
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	        Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also the following APIs
*				- _vc_core_engine_private_call
*				- _vc_core_engine_private_call_by_callid
*				.
 */
voicecall_error_t _vc_core_engine_setup_conference(voicecall_engine_t *pvoicecall_agent);

/**
* This function makes a private call to the given call member from the currently available active conference call
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle		call handle of the call to be made private
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also the following APIs
*				- _vc_core_engine_setup_conference
*				- _vc_core_engine_private_call_by_callid
*				.
 */
voicecall_error_t _vc_core_engine_private_call(voicecall_engine_t *pvoicecall_agent, int call_handle);

/**
* This function makes a private call to the call member corressponding to the given call id.
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_id			Call ID of the call to be made private
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also the following APIs
*				- _vc_core_engine_setup_conference
*				- _vc_core_engine_private_call
*				.
 */
voicecall_error_t _vc_core_engine_private_call_by_callid(voicecall_engine_t *pvoicecall_agent, int call_id);

/**
* This function sends the given dtmf digits
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	pdtmf_string		dtmf digits to be sent
* @remarks		pvoicecall_agent and pdtmf_string cannot be NULL
 */
voicecall_error_t _vc_core_engine_send_dtmf(voicecall_engine_t *pvoicecall_agent, char *pdtmf_string);

/**
* This function clears the data of the given call type.
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_type	call type
* @param[in]	call_handle	Call handle of the connected call to be cleared
* @remarks		This will clear the call data only when the call data are currently not being used
*				i,e) the data will be cleared only if the corresponding call is ended or the call data is not used at all.
*				call_handle argument is required only in case of connected call, Engine ignores call_handle for other
*				call types.
 */
voicecall_error_t _vc_core_engine_finalize_call(voicecall_engine_t *pvoicecall_agent, voicecall_call_type_t call_type, int call_handle);

/**
* This function changes the inout state of the engine to the given state
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	io_state			Inout state to be set
* @remarks		pvoicecall_agent cannot be NULL
* @see			_vc_core_engine_status_get_engine_iostate
 */
voicecall_error_t _vc_core_engine_change_engine_iostate(voicecall_engine_t *pvoicecall_agent, int io_state);

/**
 * This function extracts the call number from the given number
 *
 * @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		source_tel_number		source number
 * @param[out]	phone_number		target number
 * @param[in]		buf_size				target number buffer size
 */
voicecall_error_t _vc_core_engine_extract_phone_number(const char *source_tel_number, char *phone_number, const int buf_size);

/**
* This function finalizes the voiecall engine and removes all allocated resources
 *
* @return		nothing
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @remarks		pvoicecall_agent cannot be NULL
 */
void _vc_core_engine_engine_finish(voicecall_engine_t *pvoicecall_agent);

/**
* This function changes the voice audio path
 *
 * @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
 * @param[in]		audio_path		audio path to be changed
 * @param[in]		bextra_volume		TRUE - extra volume on, FALSE - extra volume off
 * @remarks		pvoicecall_agent cannot be NULL
 */
voicecall_error_t _vc_core_engine_change_audio_path(voicecall_engine_t *pvoicecall_agent, voicecall_audio_path_t audio_path, gboolean bextra_volume);

/**
* This function sets the voice call audio volume for the given audio path type
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[in]		audio_path_type		audio path for the volume to be set
* @param[in]		vol_level			volume level
* @remarks		pvoicecall_agent cannot be NULL
 */
voicecall_error_t _vc_core_engine_set_audio_volume(voicecall_engine_t *pvoicecall_agent, voicecall_audio_path_t audio_path_type, voicecall_audio_volume_t vol_level);

/**
* This function sets the modem mute status
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[in]		bmute_audio				TRUE - mure audio, FALSE - unmute audio
* @remarks		pvoicecall_agent cannot be NULL
*/
voicecall_error_t _vc_core_engine_set_audio_mute(voicecall_engine_t *pvoicecall_agent, gboolean bmute_audio);

/**
* This function retreives the voice call audio volume for the given audio path type
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[in]		audio_path_type		audio path for the volume to be retreived
* @remarks		pvoicecall_agent cannot be NULL
 */
voicecall_error_t _vc_core_engine_get_audio_volume(voicecall_engine_t *pvoicecall_agent, voicecall_audio_path_t audio_path_type);


#ifdef _SAT_MENU_
/**
 * This function requests SAT Engine to setup SIM services Menu
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @remarks		Voicecall Engine only requests the SAT engine to display the menu.
 */
voicecall_error_t voicecall_request_sat_menu(voicecall_engine_t *pvoicecall_agent);

/**
* This function retreives the SIM Menu Title from the SAT Engine
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[out]	title					contains the sat menu title on sucess
 */
voicecall_error_t voicecall_request_sat_menu_title(voicecall_engine_t *pvoicecall_agent, char *title);
#endif

/**
* This function prepares the engine for the redial call. It preserves the previsouly made call object to be used for the next make call
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
 * @param[in]		call_handle		call handle
* @remarks		If this API is used, _vc_core_engine_prepare_call is not reqired for making the call again. The last prepared call details will
*				be used for the redialling. Application has to just use _vc_core_engine_make_call API to redial the call
 */
voicecall_error_t _vc_core_engine_prepare_redial(voicecall_engine_t *pvoicecall_agent, int call_handle);

#ifdef _OLD_SAT_
/**
 * This function checks whether SAT redial duration is valid
 *
 * @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
 * @param[out]	bredial_duration	Contains TRUE if SAT redial duration is enabled, FALSE otherwise
 * @remarks		pvoicecall_agent and bredial_duration cannot be NULL
 */
voicecall_error_t voicecall_get_sat_redial_duration_status(voicecall_engine_t *pvoicecall_agent, gboolean *bredial_duration);

/**
 * This function sets the current duration and retireives the modified remaining SAT redial duration
 *
 * @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
 * @param[in]		pvoicecall_agent		Handle to Voicecall Engine
 * @param[out]	remaining_duration		remaining sat duration
 * @remarks		pvoicecall_agent and remaining_duration cannot be NULL
 */
voicecall_error_t voicecall_get_set_sat_remaining_duration(voicecall_engine_t *pvoicecall_agent, long *remaining_duration);
#endif

voicecall_error_t _vc_core_engine_get_sat_dtmf_hidden_mode(voicecall_engine_t *pvoicecall_agent, gboolean *bhidden_mode);

/**
* This function sends response to sat based on the given sat response type
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	sat_rqst_resp_type	sat rqst/response type sent by client
* @param[in]	sat_response_type	sat response type to be sent to SAT
 */
voicecall_error_t _vc_core_engine_send_sat_response(voicecall_engine_t *pvoicecall_agent, voicecall_engine_sat_rqst_resp_type sat_rqst_resp_type, call_vc_sat_reponse_type_t sat_response_type);

voicecall_error_t _vc_core_engine_set_to_default_values(voicecall_engine_t *pvoicecall_agent);
voicecall_error_t _vc_core_engine_check_incoming_handle(voicecall_engine_t *pvoicecall_agent, int call_id);

/* Tapi response call back */
void _vc_core_engine_dial_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_answer_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_end_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_hold_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_active_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_swap_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_join_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_split_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_transfer_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_dtmf_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_set_volume_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_get_volume_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_set_sound_path_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_set_mute_status_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
void _vc_core_engine_get_aoc_info_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data);
/* Tapi response call back end */

#endif				/* __VC_CORE_ENGINE_H_ */
