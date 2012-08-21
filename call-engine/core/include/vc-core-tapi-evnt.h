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


#ifndef __VC_CORE_TAPI_EVENT_H_
#define __VC_CORE_TAPI_EVENT_H_

#include "vc-core-util.h"
#include "vc-core-callagent.h"
#include "vc-core-engine.h"

/**
 * This function handles the tapi connected line indication handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 * @param[in]		call_handle			Tapi Call Handle associtated with connected line indication
 */
gboolean _vc_core_tapi_event_connected_line_ind_handle(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallConnectedNumberInfo_t *connected_number_info);

/**
 * This function handles the AOC Event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		Pointet to SAT engine return call structure
 * @param[in]		ptapi_aoc_info		AOC info associated with the AOC Event
 */
gboolean _vc_core_tapi_event_handle_aoc(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallAocInfo_t *pTapiAocInfo);

/**
 * This function retreives the voicecall engine specific end cause type for the given tapi end cause type
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		noti_id				tapi event type
 * @param[in]		cause			tapi call end cause
 * @param[out]	end_cause_type	voicecall engine end cause
 */
void _vc_core_tapi_event_get_end_cause_type(call_vc_callagent_state_t *pcall_agent, const char *noti_id, TelTapiEndCause_t cause, voice_call_end_cause_type_t *end_cause_type);

/**
* This function Copies Telephony incoming call data to voice call incoming call data
*
* @return		void
* @param[in]		pcall_agent		Handle to voicecall agent
* @param[in]		callInfo			Telephony Incoming Call Data
* @param[out]	pcallobject_info	Voicecall Engine Incoming Call Dta
*/
void _vc_core_tapi_event_copy_incoming_call_data(call_vc_callagent_state_t *pcall_agent, TelCallIncomingCallInfo_t *callInfo, call_vc_call_objectinfo_t *pcallobject_info);

/**
 * This function handles the incoming event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		Incoming call handle
 * @param[in]		callInfo			Incoming call info associated with the incoming call
 */
gboolean _vc_core_tapi_event_handle_incoming_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallIncomingCallInfo_t *callInfo);

/**
 * This function handles tapi origination event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		Call Handle associated with the alert event
 */
gboolean _vc_core_tapi_event_handle_originated_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle);

/**
 * This function handles tapi alert event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		Call Handle associated with the alert event
 */
gboolean _vc_core_tapi_event_handle_alert_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle);

/**
 * This function handles the call end event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		type				Tapi Event Type
 * @param[in]		handle			Call Handle of the call being ended
 * @param[in]		cause			Tapi End Cause
 */
gboolean _vc_core_tapi_event_handle_call_end_event(call_vc_callagent_state_t *pcall_agent, const char * noti_id, call_vc_handle handle, TelTapiEndCause_t cause);

/**
 * This function handles the tapi call connect event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle assoicated with the call being connected
 */
gboolean _vc_core_tapi_event_handle_call_connect_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle);

/**
 * This function handles call hold event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle assoicated with the call being held
 * @param[in]		status			tapi cause incase of hold failed
 */
gboolean _vc_core_tapi_event_handle_call_held_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallCause_t status);

/**
 * This function handles tapi call activate/retreive event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle assoicated with the call being held
 * @param[in]		status			tapi cause incase of hold failed
 */
gboolean _vc_core_tapi_event_handle_call_retrieve_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallCause_t status);

/**
 * This function handles call join/conference event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle assoicated with the call being held
 * @param[in]		status			tapi cause incase of hold failed
 */
gboolean _vc_core_tapi_event_handle_call_join_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallCause_t status);

/**
 * This function handles call split/private call event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle assoicated with the call being held
 * @param[in]		status			tapi cause incase of hold failed
 */
gboolean _vc_core_tapi_event_handle_call_split_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallCause_t status);

/**
 * This function handles the call transfer event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		status			tapi cause incase of hold failed
 */
gboolean _vc_core_tapi_event_handle_call_transfer_event(call_vc_callagent_state_t *pcall_agent, TelCallCause_t status);

#endif				/* __VC_CORE_TAPI_EVENT_H_ */
