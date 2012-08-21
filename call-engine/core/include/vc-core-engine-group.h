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


#ifndef __VC_CORE_ENGINE_GROUP_H_
#define __VC_CORE_ENGINE_GROUP_H_

#include "vc-core-engine-types.h"
#include "vc-core-error.h"

/**
* This function retrieves the number of connected call members in the given group
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	group_index		Group index of the group
* @param[out]	pmember_num	Pointer to the retrieved number of connected members
* @remarks		pvoicecall_agent and pmember_num cannot be NULL.
*/
voicecall_error_t _vc_core_engine_group_get_connected_member_count(voicecall_engine_t *pvoicecall_agent, int group_index, int *pmember_num);

/**
* This function retrives the number of groups in which atleast one call member is available
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	pnum_enabled_group	Pointer to the number of enabled groups
* @remarks		pvoicecall_agent and pnum_enabled_group cannot be NULL.
*/
voicecall_error_t _vc_core_engine_group_get_enabled_group_count(voicecall_engine_t *pvoicecall_agent, int *pnum_enabled_group);

/**
* This function retrieves the group indices of active and held calls
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	pact_grp_index	Set to the group index of active calls
* @param[out]	pheld_grp_index	Set to the group index of held calls
* @remarks		pvoicecall_agent, pact_grp_index and pheld_grp_index cannot be NULL.
*/
voicecall_error_t _vc_core_engine_group_get_group_indices(voicecall_engine_t *pvoicecall_agent, int *pact_grp_index, int *pheld_grp_index);

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
voicecall_error_t _vc_core_engine_group_get_call_handle_byposition(voicecall_engine_t *pvoicecall_agent, int group_index, int pos, int *pcall_handle);

/**
* This function checks if connected call exists in a given group
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	group_index		Group index of the group
* @param[out]	bcall_exists		Contains TRUE if call exists in the given group, FALSE otherwise
* @remarks		pvoicecall_agent and bcall_exists cannot be NULL.
*/
voicecall_error_t _vc_core_engine_group_isexists_connected_call_ingroup(voicecall_engine_t *pvoicecall_agent, int group_index, gboolean *bcall_exists);

#endif				/*__VC_CORE_ENGINE_GROUP_H_*/
