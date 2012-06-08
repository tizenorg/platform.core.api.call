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


#ifndef __VC_CORE_ENGINE_STATUS_H_
#define __VC_CORE_ENGINE_STATUS_H_

#include "vc-core-engine-types.h"
#include "vc-core-error.h"

/**
* This function checks whether the given incoming call is a restricted call or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle		Call handle of the call to be checked
* @param[out]	pbrestricted		Pointer to the restricted name mode
* @remarks		pvoicecall_agent and prestricted cannot be NULL.
*				This API shall only be used with the incoming call handle before it is connected
*/
voicecall_error_t _vc_core_engine_status_isrestricted_call(voicecall_engine_t *pvoicecall_agent, int call_handle, gboolean *pbrestricted);

/**
* This function retrieves the call object belongs to the given call handle
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle	Call handle of the call for which the call object is retrieved
* @param[out]	pcall_object	Pointer to the retrived call object info
* @remarks		pvoicecall_agent and pcall_object cannot be NULL.
*/
voicecall_error_t _vc_core_engine_status_get_call_object(voicecall_engine_t *pvoicecall_agent, int call_handle, call_vc_call_objectinfo_t *pcall_object);

/**
* This function retrieves the inout state of the engine
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	pio_state			Contains the Engine InOut state on return
* @remarks		pvoicecall_agent and pio_state cannot be NULL.
* @see			_vc_core_engine_change_engine_iostate
*/
voicecall_error_t _vc_core_engine_status_get_engine_iostate(voicecall_engine_t *pvoicecall_agent, int *pio_state);

/**
* This function checks whether connected call exists or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_type			call type
* @param[out]	bcall_exists		TRUE - if call exists of given given type, FALSE otherwise
* @remarks		pvoicecall_agent and bcall_exists cannot be NULL.
*/
voicecall_error_t _vc_core_engine_status_isexists_call_bytype(voicecall_engine_t *pvoicecall_agent, voicecall_call_type_t call_type, gboolean *bcall_exists);

/**
* This function retrieves the total number of call members available with the engine
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent		Handle to voicecall engine
* @param[out]	ptotal_call_member	Contains the total call member availalbe in engine on return
* @remarks		pvoicecall_agent and ptotal_call_member cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_get_call_member_count(voicecall_engine_t *pvoicecall_agent, int *ptotal_call_member);

/**
* This function retrieves the total number of call members with the given connected call type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent		Handle to voicecall engine
* @param[in]	connected_call_type	connected call type
* @param[out]	pmember_num		Contains the number of call members available with the given connected call type on return
* @remarks		pvoicecall_agent and pmember_num cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_get_call_member_info(voicecall_engine_t *pvoicecall_agent, voicecall_connected_call_type_t connected_call_type, int *pmember_num);

/**
* This function retrieves the call handle according to the given call type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_type		call type
* @param[out]	pcall_handle	Contains the call handle on success
* @remarks		pvoicecall_agent and pcall_handle cannot be NULL
*				In case of multiple connected calls available, it will retreive the first connected call
*/
voicecall_error_t _vc_core_engine_status_get_call_handle_bytype(voicecall_engine_t *pvoicecall_agent, voicecall_call_type_t call_type, int *pcall_handle);

/**
* This function checks if active calls and/or held call exists or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	pactive_calls		Set to TRUE if active calls exist
* @param[out]	pheld_calls		Set to TRUE if held calls exist
* @remarks		pvoicecall_agent,pactive_calls and pheld_calls cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_isexists_any_call(voicecall_engine_t *pvoicecall_agent, gboolean *pactive_calls, gboolean *pheld_calls);

/**
* This function retreives the cphs csp status
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent		Handle to Voicecall Engine
* @param[in]	csp_service		csp name
* @param[out]	pbcsp_status		Contains TRUE if given csp service is enabled,FALSE  otherwise
* @remarks		pvoicecall_agent and pbcsp_status cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_get_cphs_csp_status(voicecall_engine_t *pvoicecall_agent, voicecall_cphs_csp_service csp_service, gboolean *pbcsp_status);

/**
* This function checks if the call is emergency call for the given outgoing call index
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to Voicecall Engine
* @param[in]	mo_call_index		Index of the outgoing call
* @param[out]	pbemergency_call	Contains TRUE if the call is emergency call,FALSE  otherwise
* @remarks		pvoicecall_agent and pbemergency_call cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_check_emergency_byindex(voicecall_engine_t *pvoicecall_agent, int mo_call_index, gboolean *pbemergency_call);

/**
* This function checks and returns the FDN status
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pcall_agent	Handle to Voicecall Engine
* @param[out]	bfdn_enabled	TRUE - if FDN is enabled, FALSE  otherwise
* @remarks		pvoicecall_agent and bfdn_anabled cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_isenabled_fdn(voicecall_engine_t *pcall_agent, gboolean *bfdn_enabled);

/**
* This function checks the possiblity of transfering calls
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent		Handle to Voicecall Engine
* @param[out]	pbtransfer_calls	Contains TRUE if call transfer is possible, FALSE otherwise
* @remarks		pvoicecall_agent and pbtransfer_calls cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_transfer_call_possible(voicecall_engine_t *pvoicecall_agent, gboolean *pbtransfer_calls);

/**
* This function checks the possiblity of making conference calls
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to Voicecall Engine
* @param[out]	pbconf_call		Contains TRUE if conference call is possible , FALSE otherwise
* @remarks		pvoicecall_agent and pbconf_call cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_conf_call_possible(voicecall_engine_t *pvoicecall_agent, gboolean *pbconf_call);

/**
* This function retreives the call state of the given call
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle		Call handle of particular call
* @param[out]	pcall_state		Contains the call state of the given call handle
* @remarks		pvoicecall_agent and pcall_state cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_get_call_state_byhandle(voicecall_engine_t *pvoicecall_agent, int call_handle, voicecall_call_state_t *pcall_state);

/**
* This function checks if any call is ending and retrieves its call number if it is ending
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	bcall_ending		TRUE if any call is being ended, FALSE otherwise
* @remarks		pvoicecall_agent and bcall_ending cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_any_call_ending(voicecall_engine_t *pvoicecall_agent, gboolean *bcall_ending);

/**
* This function checks whther engine is busy in processing any events or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	bbusy	TRUE - if engine is not busy in processing any events, FALSE - otherwise
* @remarks		pvoicecall_agent and bbusy cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_engine_busy(voicecall_engine_t *pvoicecall_agent, gboolean *bbusy);

/**
* This function sets the given flag to engine for processing during call end
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	end_flag			End Flag to be set
* @remarks		pvoicecall_agent and bsscode cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_set_end_flag(voicecall_engine_t *pvoicecall_agent, voicecall_end_flag_t end_flag);

/**
* This function checks whether the given string is MMI string or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	pinput_number	Input string to be verified
* @param[out]	bsscode			TRUE - if the given string is a valid ss code, FALSE otherwise
* @remarks		pvoicecall_agent and bsscode cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_isvalid_ss_code(voicecall_engine_t *pvoicecall_agent, const char *pinput_number, gboolean *bsscode);

/**
* This function checks the if it is in zuhause area or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	psat_icon_data	Pointer carrying the zuhause status
* @remarks		pvoicecall_agent and psat_icon_data cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_zuhause_area(voicecall_engine_t *pvoicecall_agent, gboolean *bzuhause);

voicecall_error_t _vc_core_engine_status_dump_call_details(voicecall_engine_t *pvoicecall_agent);

void _vc_core_engine_status_set_download_call(voicecall_engine_t *pvoicecall_agent, gboolean b_download_call);
gboolean _vc_core_engine_status_get_download_call(voicecall_engine_t *pvoicecall_agent);

#endif				/* __VC_CORE_ENGINE_STATUS_H_ */
