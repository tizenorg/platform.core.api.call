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


#include "vc-core-engine-group.h"
#include "vc-core-callagent.h"
#include "vc-core-engine.h"
#include "vc-core-ccbs.h"
#include "vc-core-tapi-rqst.h"
#include "vc-core-tapi-evnt.h"
#include "vc-core-svcall.h"

/**
* This function retrieves the number of connected call members in the given group
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	group_index		Group index of the group
* @param[out]	pmember_num	Pointer to the retrieved number of connected members
* @remarks		pvoicecall_agent and pmember_num cannot be NULL.
*				group_index  shall only take 0 or 1  as input values
*/
voicecall_error_t _vc_core_engine_group_get_connected_member_count(voicecall_engine_t *pvoicecall_agent, int group_index, int *pmember_num)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pmember_num != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	*pmember_num = _vc_core_cm_get_connected_member_count_ingroup(&pagent->call_manager, group_index);

	return ERROR_VOICECALL_NONE;
}

/**
* This function retrives the number of groups in which atleast one call member is available
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	pnum_enabled_group	Pointer to the number of enabled groups
* @remarks		pvoicecall_agent and pnum_enabled_group cannot be NULL.
*/
voicecall_error_t _vc_core_engine_group_get_enabled_group_count(voicecall_engine_t *pvoicecall_agent, int *pnum_enabled_group)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pnum_enabled_group != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	*pnum_enabled_group = _vc_core_cm_get_group_count(&pagent->call_manager);

	return ERROR_VOICECALL_NONE;
}

/**
* This function retrieves the group indices of active and held calls
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	pact_grp_index	Set to the group index of active calls
* @param[out]	pheld_grp_index	Set to the group index of held calls
* @remarks		pvoicecall_agent, pact_grp_index and pheld_grp_index cannot be NULL.
*/
voicecall_error_t _vc_core_engine_group_get_group_indices(voicecall_engine_t *pvoicecall_agent, int *pact_grp_index, int *pheld_grp_index)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pact_grp_index != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pheld_grp_index != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	*pact_grp_index = _vc_core_cm_get_active_group_index(&pagent->call_manager);
	*pheld_grp_index = _vc_core_cm_get_held_group_index(&pagent->call_manager);

	return ERROR_VOICECALL_NONE;
}

/**
* This function retrieves the call handle of the call member given its groupe index and position in the group
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	group_index	Group index of the group
* @param[in]	pos		Position of the call in the group
* @param[out]	pcall_handle	Pointer to the call handle to be retrieved
* @remarks		pvoicecall_agent and pcall_handle cannot be NULL.
*/
voicecall_error_t _vc_core_engine_group_get_call_handle_byposition(voicecall_engine_t *pvoicecall_agent, int group_index, int pos, int *pcall_handle)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	int callhandle = -1;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_handle != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(group_index, 0, 1, ERROR_VOICECALL_INVALID_ARGUMENTS)
	    VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(pos, 0, 4, ERROR_VOICECALL_INVALID_ARGUMENTS)

	    callhandle = _vc_core_cm_get_call_handle_ingroup_byposition(&pagent->call_manager, group_index, pos);;

	if (callhandle != -1) {
		*pcall_handle = callhandle;
		return ERROR_VOICECALL_NONE;
	}

	return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

/**
* This function checks if connected call exists in a given group
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	group_index	Group index of the group
* @param[out]	bcall_exists TRUE - if call exists, FALSE otherwise
* @remarks		pvoicecall_agent and bcall_exists cannot be NULL.
*				group_index  shall only take 0 or 1  as input values
*/
voicecall_error_t _vc_core_engine_group_isexists_connected_call_ingroup(voicecall_engine_t *pvoicecall_agent, int group_index, gboolean *bcall_exists)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bcall_exists != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(group_index, 0, 1, ERROR_VOICECALL_INVALID_ARGUMENTS);

	if (CALL_VC_GROUP_STATE_NONE == _vc_core_cm_get_group_state(&pagent->call_manager, group_index)) {
		*bcall_exists = FALSE;
	}

	if (0 == _vc_core_cm_get_connected_member_count_ingroup(&pagent->call_manager, group_index)) {
		*bcall_exists = FALSE;
	} else {
		*bcall_exists = TRUE;
	}

	return ERROR_VOICECALL_NONE;
}
