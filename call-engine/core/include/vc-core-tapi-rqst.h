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


#ifndef __VC_CORE_TAPI_RQST_H_
#define __VC_CORE_TAPI_RQST_H_

#include "vc-core-util.h"
#include "vc-core-callagent.h"

 /**
 * This function prepares for a call setup
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_prepare_setup_call(call_vc_callagent_state_t *pcall_agent);

  /**
 * This function sets up an outgoing call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_setup_call(call_vc_callagent_state_t *pcall_agent);

/*
 * Function to answer/release calls,
 */
 /**
 * This function answers the call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 * @param[in]		answer_type	call answer type #voicecall_answer_type_t
 * @param[out]	error_code	Error code
 */
gboolean _vc_core_tapi_rqst_answer_call(call_vc_callagent_state_t *pcall_agent, voicecall_answer_type_t answer_type, int *error_code);

/**
 * This function checks and prepares to accept a call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_response_call(call_vc_callagent_state_t *pcall_agent);

/**
 * This function rejects a mobile terminated call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 * @param[in]		budub		TRUE - User Determined User Busy, FALSE - otherwise
 * @param[out]	error_code	Error code
 */
gboolean _vc_core_tapi_rqst_reject_mt_call(call_vc_callagent_state_t *pcall_agent, gboolean budub, int *error_code);

/**
 * This function ends a call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_end_call(call_vc_callagent_state_t *pcall_agent);

/**
 * This function releases active calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_active_calls(call_vc_callagent_state_t *pcall_agent);

/**
 * This function releases held calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_held_calls(call_vc_callagent_state_t *pcall_agent);

/**
 * This function releases the incoming call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_incoming_call(call_vc_callagent_state_t *pcall_agent);

/**
 * This function releases outgoing call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_outgoing_call(call_vc_callagent_state_t *pcall_agent);

/**
 * This function releases all calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_all_calls(call_vc_callagent_state_t *pcall_agent);

/**
 * This function releases the call associate with the given call handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 * @param[in]		call_handle	handle of the call to be ended
 */
gboolean _vc_core_tapi_rqst_end_call_by_callhandle(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle);

/**
 * This function holds a call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_hold_call(call_vc_callagent_state_t *pcall_agent);

/**
 * This function retrieves a call from hold
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_retrieve_call(call_vc_callagent_state_t *pcall_agent);

/**
 * This function swaps held and active calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_swap_calls(call_vc_callagent_state_t *pcall_agent);

/**
 * This function joins two calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_join_calls(call_vc_callagent_state_t *pcall_agent);

/**
 * This function splits the members of a call given its call handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 * @param[in]		call_handle	Call handle for a call the members of which has to be split
 */
gboolean _vc_core_tapi_rqst_private_call(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle);

/**
 * This function transfers call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_transfer_call(call_vc_callagent_state_t *pcall_agent);

/**
 * This function sends the given string as dtmf
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 * @param[in]		dtmf_string	dtmf string
 */
gboolean _vc_core_tapi_rqst_start_dtmf(call_vc_callagent_state_t *pcall_agent, char *dtmf_string);

#endif				/* __VC_CORE_TAPI_RQST_H_ */
