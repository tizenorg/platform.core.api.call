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


#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <glib.h>
#include "vc-core-util.h"
#include "vc-core-callmanager.h"
#include "vc-core-callagent.h"

char *gaszCallStateName[VC_CALL_STATE_MAX_NUM] = {
	"CALL_NONE",
	"CALL_INCOME",
	"CALL_REJECTED",
	"CALL_PREPARE_OUTGOING",
	"CALL_OUTGOING",
	"CALL_OUTGOING_ORIG",
	"CALL_OUTGOING_ALERT",
	"CALL_CANCELLED",
	"CALL_CONNECTED",
	"CALL_RELEASE_WAIT",
	"CALL_ENDED",
	"CALL_REDIAL",
	"CALL_ENDED_FINISH",
};

/*Local Function Declarations*/
/**
* This function handles sat engine notification
*
* @internal
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		callStatusInfo	Handle to call manager
* @param[in]		call_handle	telephony call handle
* @param[out]	pGroup		group to which the call_handle belongs
* @param[in]		pPos			positoin in the pGroup in which the call details of call handle is maintained
*/
static gboolean __call_vc_cm_gp_get_groupcall_pos(call_vc_manager_t *callStatusInfo, call_vc_handle call_handle, int *pGroup, int *pPos);

/**
 * This function removes a call member to a group
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	The call handle to be removed
 * @see			_vc_core_cm_add_call_member_togroup
 */
static gboolean __vc_core_cm_remove_call_member_fromgroup(call_vc_manager_t *pMng, call_vc_handle call_handle);

/*Function Definitions*/
/**
 * This function initializes the call manager
 *
 * @return		void
 * @param[out]	pMng		Pointer to the call manager structure
 */
void _vc_core_call_manager_init(call_vc_manager_t *pMng)
{
	VOICECALL_RETURN_IF_FAIL(pMng != NULL);
	int index = 0;

	memset(pMng, 0, sizeof(call_vc_manager_t));

	/* init call handle */
	for (index = 0; index < VC_MAX_CALL_MEMBER; index++) {
		/*CALL_ENG_DEBUG(ENG_DEBUG, "%d",index);*/
		pMng->callobject_info[index].call_handle = VC_TAPI_INVALID_CALLHANDLE;
		pMng->callobject_info[index].aoc_ccm = VC_INVALID_CALL_COST;
		pMng->callobject_info[index].call_id = VC_INVALID_CALL_ID;
		pMng->callobject_info[index].call_type = VC_CALL_ORIG_TYPE_NORMAL;
	}

	pMng->setupcall_info.mocall_index = VC_INVALID_CALL_INDEX;
	pMng->setupcall_info.call_control_type = CALL_VC_SAT_CC_NONE;
	pMng->mtcall_index = VC_INVALID_CALL_INDEX;

	CALL_ENG_DEBUG(ENG_DEBUG, "[pMng->callobject_info initialized]");

}

/**
 * This function clears the information of a given call state
 *
 * @return		void
 * @param[in]	pcall_object	Pointer to the call object which has to be cleared
 */
void _vc_core_cm_clear_call_object(call_vc_call_objectinfo_t *pcall_object)
{
	VOICECALL_RETURN_IF_FAIL(pcall_object != NULL);
	memset(pcall_object, 0, sizeof(call_vc_call_objectinfo_t));

	pcall_object->state = VC_CALL_STATE_NONE;
	pcall_object->call_handle = VC_TAPI_INVALID_CALLHANDLE;
	pcall_object->aoc_ccm = VC_INVALID_CALL_COST;
	pcall_object->call_id = VC_INVALID_CALL_ID;
	pcall_object->call_type = VC_CALL_ORIG_TYPE_NORMAL;

#ifdef _CPHS_DEFINED_
	pcall_object->alsLine = VC_CALL_CPHS_ALS_NONE;
#endif

	pcall_object->bccbs_call = FALSE;

}

/**
* This function clears the information of the callobject available in the given index
 *
* @return		TRUE, if the object cleared, otherwise FALSE
* @param[in]	pMng		Pointer to the call manager structure
* @param[in]	index		Index of the call object to be cleared
 */
gboolean _vc_core_cm_clear_call_object_byindex(call_vc_manager_t *pMng, int index)
{
	call_vc_call_objectinfo_t call_object;
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(index >= 0 && index <= 7);

	if (TRUE == _vc_core_cm_get_call_object_byindex(pMng, index, &call_object)) {
		_vc_core_cm_clear_call_object(&call_object);
		return TRUE;
	}

	return FALSE;
}

/**
 * This function adds the given call object info to a free slot.
 *
 * @return		This function returns the call object index on success and -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pcall_object	Pointer to the call object info structure that need to be added
 * @see			_vc_core_cm_set_call_object, _vc_core_cm_remove_call_object
 */
int _vc_core_cm_add_call_object(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object)
{
	int index = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL(pcall_object != NULL);

	for (index = 0; index < VC_MAX_CALL_MEMBER; index++) {
		if (pMng->callobject_info[index].state == VC_CALL_STATE_NONE) {
			memcpy(&pMng->callobject_info[index], pcall_object, sizeof(call_vc_call_objectinfo_t));
			return index;
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Insufficient buffer i = %d..", index);

	return VC_TAPI_INVALID_CALLHANDLE;
}

/**
 * This function removes the specified call object info of a given call handle
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	Handle of the call object to be removed
 * @see			_vc_core_cm_add_call_object, _vc_core_cm_set_call_object
 */
gboolean _vc_core_cm_remove_call_object(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int i = 0;
	int groupIndex = -1;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE);

	CALL_ENG_DEBUG(ENG_DEBUG, "[Call Handle = %d]", call_handle);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (pMng->callobject_info[i].call_handle == call_handle) {
			if (pMng->setupcall_info.mocall_index == i) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Removing MO Call Info >> i=%d", i);
				pMng->setupcall_info.mocall_index = VC_INVALID_CALL_INDEX;
				pMng->setupcall_info.no_service_state = FALSE;
			} else if (pMng->mtcall_index == i) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Removing MT Call Info >> i=%d", i);
				pMng->mtcall_index = VC_INVALID_CALL_INDEX;
			} else {
				groupIndex = _vc_core_cm_get_group_index(pMng, call_handle);
				__vc_core_cm_remove_call_member_fromgroup(pMng, call_handle);
			}
			_vc_core_cm_clear_call_object(&(pMng->callobject_info[i]));
			break;
		}

	}

	return TRUE;
}

/**
 * This function retrieves the given call object info
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	Call handle of the object info to be retrieved
 * @param[out]	pcall_object	Pointer to the call object info structure that should be retrieved
 * @see			_vc_core_cm_add_call_object, _vc_core_cm_remove_call_object, _vc_core_cm_set_call_object
 */
gboolean _vc_core_cm_get_call_object(call_vc_manager_t *pMng, call_vc_handle call_handle, call_vc_call_objectinfo_t *pcall_object)
{
	int i = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE);
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_object != NULL);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (pMng->callobject_info[i].call_handle == call_handle) {
			memcpy(pcall_object, &pMng->callobject_info[i], sizeof(call_vc_call_objectinfo_t));
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * This function retrieves the given call object info
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		index		index of the object info to be retrieved
 * @param[out]	pcall_object	Pointer to the call object info structure that should be retrieved
 * @see			_vc_core_cm_add_call_object, _vc_core_cm_remove_call_object, _vc_core_cm_set_call_object
 */
gboolean _vc_core_cm_get_call_object_byindex(call_vc_manager_t *pMng, int index, call_vc_call_objectinfo_t *pcall_object)
{
	int i = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(index >= 0 && index <= 7);
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_object != NULL);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (i == index) {
			memcpy(pcall_object, &pMng->callobject_info[i], sizeof(call_vc_call_objectinfo_t));
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * This function retrieves the call state of a given call handle
 *
 * @return		This function returns the call state
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	Call handle for which the state has to be retrieved
 */
voicecall_call_state_t _vc_core_cm_get_call_state(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int i = 0;

	VOICECALL_RETURN_VALUE_IF_FAIL(pMng != NULL, VC_CALL_STATE_NONE);
	VOICECALL_RETURN_VALUE_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE, VC_CALL_STATE_NONE);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (pMng->callobject_info[i].call_handle == call_handle) {
			return pMng->callobject_info[i].state;
		}
	}

	return VC_CALL_STATE_NONE;
}

 /**
 * This function retrieves the number of call members in various states of a call
 *
 * @return		Number of call members available
 * @param[in]		pMng		Pointer to the call manager structure
 */
int _vc_core_cm_get_call_member_count(call_vc_manager_t *pMng)
{
	int i = 0;
	int nCount = 0;

	VOICECALL_RETURN_ZERO_IF_FAIL(pMng != NULL);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (pMng->callobject_info[i].state != VC_CALL_STATE_NONE)
			nCount++;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "%d", nCount);

	return nCount;
}

/**
 * This function sets the given call object info to the given slot
 *
 * @return		This function returns the call object index on success and -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pcall_object	Pointer to the call object info structure that need to be modified
 * @see			_vc_core_cm_add_call_object, _vc_core_cm_remove_call_object, _vc_core_cm_get_call_object
 */
int _vc_core_cm_set_call_object(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object)
{
	int i = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL(pcall_object != NULL);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (pMng->callobject_info[i].call_handle == pcall_object->call_handle) {
			CALL_ENG_DEBUG(ENG_DEBUG, "[Call Handle = %d]", pcall_object->call_handle);
			memcpy(&pMng->callobject_info[i], pcall_object, sizeof(call_vc_call_objectinfo_t));
			return i;
		}
	}

	return VC_TAPI_INVALID_CALLHANDLE;
}

/**
 * This function returns the number of calls in a given call state
 *
 * @return		This function returns the number of calls in the given state
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		state		The call state the number of which has to be retrieved
 */
int _vc_core_cm_get_call_state_num(call_vc_manager_t *pMng, voicecall_call_state_t state)
{
	int i = 0;
	int count = 0;

	VOICECALL_RETURN_ZERO_IF_FAIL(pMng != NULL);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (pMng->callobject_info[i].state == state) {
			count++;
		}
	}
	return count;
}

static gboolean __call_vc_cm_gp_get_groupcall_pos(call_vc_manager_t *callStatusInfo, call_vc_handle call_handle, int *pGroup, int *pPos)
{
	int group = 0;
	int pos = 0;
	gboolean bFound = FALSE;
	call_vc_groupinfo_t *pcall_group_info = NULL;
	call_vc_call_objectinfo_t *pcall_object_info = NULL;

	VOICECALL_RETURN_FALSE_IF_FAIL(callStatusInfo != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE);
	VOICECALL_RETURN_FALSE_IF_FAIL(pGroup != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(pPos != NULL);

	/*Optimization: Avoiding Long dereferencing inside loops */
	pcall_group_info = callStatusInfo->callgroup_info;
	pcall_object_info = callStatusInfo->callobject_info;
	for (group = 0; group < CALL_VC_CALL_GROUP_MAX; group++) {
		CALL_ENG_DEBUG(ENG_DEBUG, "callStatusInfo->callgroup_info[%d].num:%d", group, callStatusInfo->callgroup_info[group].num);
		for (pos = 0; pos < pcall_group_info[group].num; pos++) {
			int v = pcall_group_info[group].callobject_index[pos];
			if (pcall_object_info[v].call_handle == call_handle) {
				bFound = TRUE;
				break;
			}
		}
		if (bFound)
			break;
	}

	if (bFound == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "fail to get");
		return FALSE;
	}

	*pGroup = group;
	*pPos = pos;

	return TRUE;

}

/**
 * This function adds a call member to a group
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callobject_index	The object index of the call object to be added
 * @see			__vc_core_cm_remove_call_member_fromgroup
 */
gboolean _vc_core_cm_add_call_member_togroup(call_vc_manager_t *pMng, int callobject_index)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng->callgroup_info[CALL_VC_CALL_GROUP_1].state == CALL_VC_GROUP_STATE_NONE);

	CALL_ENG_DEBUG(ENG_DEBUG, "member index=%d", callobject_index);

	/* Every Member should be added to a new group when added for the first time. First check whether Group 0 has any members.
	   If Group 0 has any previous members then move all members in the Group 0 to  Group 1 and Group 0 must be in held state
	   as the current call is going to be active in the new group. Then add this new call to the Group 0 and Group 0 becomes active group */
	if (pMng->callgroup_info[CALL_VC_CALL_GROUP_0].state != CALL_VC_GROUP_STATE_NONE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Copying Group 0 data to Group 1");
		VOICECALL_RETURN_FALSE_IF_FAIL(pMng->callgroup_info[CALL_VC_CALL_GROUP_0].state == CALL_VC_GROUP_STATE_HOLD);
		memcpy(&(pMng->callgroup_info[CALL_VC_CALL_GROUP_1]), &(pMng->callgroup_info[CALL_VC_CALL_GROUP_0]), sizeof(call_vc_groupinfo_t));
	}

	pMng->callgroup_info[CALL_VC_CALL_GROUP_0].num = 1;
	pMng->callgroup_info[CALL_VC_CALL_GROUP_0].callobject_index[0] = callobject_index;
	pMng->callgroup_info[CALL_VC_CALL_GROUP_0].state = CALL_VC_GROUP_STATE_ACTIVE;

	/*If the currently added call is Incoming or Outgoing call then make the Incoming/Outgoing status
	   invalid since the call has been connected and added to call group */
	if (callobject_index == pMng->setupcall_info.mocall_index) {
		pMng->setupcall_info.mocall_index = VC_INVALID_CALL_INDEX;
		pMng->setupcall_info.no_service_state = FALSE;
	} else if (callobject_index == pMng->mtcall_index) {
		pMng->mtcall_index = VC_INVALID_CALL_INDEX;
	} else {
		return FALSE;
	}

	return TRUE;

}

/**
 * This function removes a call member to a group
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	The call handle to be removed
 * @see			_vc_core_cm_add_call_member_togroup
 */
gboolean __vc_core_cm_remove_call_member_fromgroup(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int group = 0;
	int pos = 0;
	int i = 0;
	call_vc_groupinfo_t *pcall_group_info = NULL;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE);

	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d", call_handle);

	/*Findout the Index Group number and the index posistion in the group for the given call handle */
	if (__call_vc_cm_gp_get_groupcall_pos(pMng, call_handle, &group, &pos) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "fail to remove");
		return FALSE;
	}

	/*Optimization: Avoiding Long dereferencing inside loops */
	pcall_group_info = pMng->callgroup_info;
	for (i = pos + 1; i < pMng->callgroup_info[group].num; i++) {
		pcall_group_info[group].callobject_index[i - 1] = pcall_group_info[group].callobject_index[i];
	}

	pcall_group_info[group].num--;

	/*If the number of members in the group become 0, then make the group status to none */
	if (pcall_group_info[group].num == 0) {
		pcall_group_info[group].state = CALL_VC_GROUP_STATE_NONE;

		/*If Group 0 becomes none and Group 1 has any  members , then move the Group 1 members to Group 0
		   Always the Group 0 is prefered group by default, this logic is to simplify all other search mehods. */
		if ((group == 0) && (pcall_group_info[CALL_VC_CALL_GROUP_1].num > 0)) {
			memcpy(&(pcall_group_info[0]), &(pcall_group_info[1]), sizeof(call_vc_groupinfo_t));
			memset(&(pcall_group_info[1]), 0, sizeof(call_vc_groupinfo_t));
		}
	}

	return TRUE;

}

/**
 * This function clears the call object information of the ended and finished calls
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 */
gboolean _vc_core_cm_clear_endcall_member(call_vc_manager_t *pMng)
{
	int i = 0;
	call_vc_call_objectinfo_t *pcall_object_info = NULL;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "...");

	/*Optimization: Avoiding Long dereferencing inside loops */
	pcall_object_info = pMng->callobject_info;
	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if ((pcall_object_info[i].state == VC_CALL_STATE_ENDED) || (pcall_object_info[i].state == VC_CALL_STATE_ENDED_FINISH)) {
			__vc_core_cm_remove_call_member_fromgroup(pMng, pcall_object_info[i].call_handle);

			_vc_core_cm_clear_call_object(&pcall_object_info[i]);
		}
	}
	return TRUE;
}

/**
 * This function sets the state of a given group
 *
 * @return		void
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		The call group for which the state needs to be changed
 * @param[in]		state		The state to be set
 * @see			_vc_core_cm_swap_group_state, _vc_core_cm_get_call_pos_ingroup
 */
void _vc_core_cm_set_group_state(call_vc_manager_t *pMng, int nGroup, call_vc_groupstate_t state)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "group=%d, state=%d", nGroup, state);

	VOICECALL_RETURN_IF_FAIL(pMng != NULL);
	/*Only Group 0 and Group 1 are possible */
	VOICECALL_RETURN_IF_FAIL((nGroup >= CALL_VC_CALL_GROUP_0 && nGroup < CALL_VC_CALL_GROUP_MAX));

	pMng->callgroup_info[nGroup].state = state;
}

/**
 * This function retrieves the index of the group given the call handle
 *
 * @return		Returns group index on success and -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	The for which the group index has to be retrieved
 * @see			_vc_core_cm_set_group_state, _vc_core_cm_get_call_pos_ingroup
 */
int _vc_core_cm_get_group_index(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int group = 0;
	int index = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE);

	if (__call_vc_cm_gp_get_groupcall_pos(pMng, call_handle, &group, &index) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "fail");
		return VC_TAPI_INVALID_CALLHANDLE;
	}

	return group;
}

/**
 * This function retrieves the index/position of the call info in the object info table for a given call handle
 *
 * @return		Returns index on success and -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group ID
 * @param[in]		call_handle	The call handle for which the call index has to be retrieved
 * @see			_vc_core_cm_get_group_index
 */
int _vc_core_cm_get_call_pos_ingroup(call_vc_manager_t *pMng, int nGroup, call_vc_handle call_handle)
{
	int i = 0;
	int idx = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL((nGroup >= CALL_VC_CALL_GROUP_0 && nGroup < CALL_VC_CALL_GROUP_MAX));
	VOICECALL_RETURN_INVALID_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE);

	for (i = 0; i < pMng->callgroup_info[nGroup].num; i++) {
		idx = pMng->callgroup_info[nGroup].callobject_index[i];
		if (pMng->callobject_info[idx].call_handle == call_handle)
			return i;
	}
	return VC_TAPI_INVALID_CALLHANDLE;
}

/**
 * This function swaps the state of the groups
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_set_group_state
 */
gboolean _vc_core_cm_swap_group_state(call_vc_manager_t *pMng)
{
	int i = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_HOLD)
			pMng->callgroup_info[i].state = CALL_VC_GROUP_STATE_ACTIVE;
		else if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_ACTIVE)
			pMng->callgroup_info[i].state = CALL_VC_GROUP_STATE_HOLD;
	}
	return TRUE;
}

/**
 * This function joins the groups
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_split_group
 */
gboolean _vc_core_cm_join_group(call_vc_manager_t *pMng)
{
	int callIndex[VC_MAX_CALL_MEMBER];
	int i = 0;
	int j = 0;
	int nCount = 0;
	int totalMember = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	totalMember = _vc_core_cm_get_total_members_ingroup(pMng);

	CALL_ENG_DEBUG(ENG_DEBUG, "Total Members: %d", totalMember);

	for (i = CALL_VC_CALL_GROUP_MAX - 1; i >= 0; i--) {
		for (j = 0; j < pMng->callgroup_info[i].num; j++) {
			callIndex[nCount] = pMng->callgroup_info[i].callobject_index[j];
			nCount++;
		}
	}

	VOICECALL_RETURN_FALSE_IF_FAIL(nCount <= VC_MAX_CALL_GROUP_MEMBER);

	for (i = 0; i < nCount; i++) {
		pMng->callgroup_info[CALL_VC_CALL_GROUP_0].callobject_index[i] = callIndex[i];
	}

	pMng->callgroup_info[CALL_VC_CALL_GROUP_0].num = nCount;
	if (nCount == 0)
		pMng->callgroup_info[CALL_VC_CALL_GROUP_0].state = CALL_VC_GROUP_STATE_NONE;
	else
		pMng->callgroup_info[CALL_VC_CALL_GROUP_0].state = CALL_VC_GROUP_STATE_ACTIVE;

	pMng->callgroup_info[CALL_VC_CALL_GROUP_1].num = 0;
	pMng->callgroup_info[CALL_VC_CALL_GROUP_1].state = CALL_VC_GROUP_STATE_NONE;

	return TRUE;

}

/**
 * This function splits the group given a call handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	Call handle
 * @see			_vc_core_cm_join_group
 */
gboolean _vc_core_cm_split_group(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int group = 0;
	int pos = 0;
	int i = 0;
	int count = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE);

	if (__call_vc_cm_gp_get_groupcall_pos(pMng, call_handle, &group, &pos) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "fail");
		return FALSE;
	}

	VOICECALL_RETURN_FALSE_IF_FAIL(group == 0);
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng->callgroup_info[CALL_VC_CALL_GROUP_1].state == CALL_VC_GROUP_STATE_NONE);

	CALL_ENG_DEBUG(ENG_DEBUG, "group=%d, pos=%d, Call Handle = %d, title=%s", group, pos, call_handle, pMng->callobject_info[pMng->callgroup_info[group].callobject_index[pos]].tel_number);

	for (i = 0; i < pMng->callgroup_info[0].num; i++) {
		if (i != pos) {
			pMng->callgroup_info[CALL_VC_CALL_GROUP_1].callobject_index[count] = pMng->callgroup_info[CALL_VC_CALL_GROUP_0].callobject_index[i];
			count++;
		}
	}

	pMng->callgroup_info[CALL_VC_CALL_GROUP_1].num = count;

	if (count == 0)
		pMng->callgroup_info[CALL_VC_CALL_GROUP_1].state = CALL_VC_GROUP_STATE_NONE;
	else
		pMng->callgroup_info[CALL_VC_CALL_GROUP_1].state = CALL_VC_GROUP_STATE_HOLD;

	pMng->callgroup_info[CALL_VC_CALL_GROUP_0].num = 1;
	pMng->callgroup_info[CALL_VC_CALL_GROUP_0].state = CALL_VC_GROUP_STATE_ACTIVE;
	pMng->callgroup_info[CALL_VC_CALL_GROUP_0].callobject_index[0] = pMng->callgroup_info[CALL_VC_CALL_GROUP_0].callobject_index[pos];

	return TRUE;

}

/**
 * This function retreives the group state for the given group index
 *
 * @return		This function returns the group state
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		group index
 */
call_vc_groupstate_t _vc_core_cm_get_group_state(call_vc_manager_t *pMng, int nGroup)
{
	VOICECALL_RETURN_VALUE_IF_FAIL(pMng != NULL, CALL_VC_GROUP_STATE_NONE);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nGroup, 0, 1, CALL_VC_GROUP_STATE_NONE);

	return pMng->callgroup_info[nGroup].state;
}

/**
 * This function retrieves the number of members in a group
 *
 * @return		Returns number of members in the group
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 */
int _vc_core_cm_get_member_count_ingroup(call_vc_manager_t *pMng, int nGroup)
{
	VOICECALL_RETURN_ZERO_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nGroup, 0, 1, CALL_VC_GROUP_STATE_NONE);

	return pMng->callgroup_info[nGroup].num;
}

/**
 * This function retrieves the total number of members in all the groups
 *
 * @return		Returns the total number of members
 * @param[in]		pMng		Pointer to the call manager structure
 */
int _vc_core_cm_get_total_members_ingroup(call_vc_manager_t *pMng)
{
	VOICECALL_RETURN_ZERO_IF_FAIL(pMng != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Members in Group 0: %d, Members in Group 1: %d", pMng->callgroup_info[CALL_VC_CALL_GROUP_0].num, pMng->callgroup_info[CALL_VC_CALL_GROUP_1].num);
	return (pMng->callgroup_info[CALL_VC_CALL_GROUP_0].num + pMng->callgroup_info[CALL_VC_CALL_GROUP_1].num);
}

/**
 * This function retrieves the number of connected members in a group
 *
 * @return		Returns number of connected members in the group
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 */
int _vc_core_cm_get_connected_member_count_ingroup(call_vc_manager_t *pMng, int nGroup)
{
	int i = 0;
	int callIndex = 0;
	int count = 0;

	VOICECALL_RETURN_ZERO_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nGroup, 0, 1, CALL_VC_GROUP_STATE_NONE);

	for (i = 0; i < pMng->callgroup_info[nGroup].num; i++) {
		callIndex = pMng->callgroup_info[nGroup].callobject_index[i];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			count++;
		}
	}
	return count;

}

/**
 * This function retrieves the total number of groups
 *
 * @return		Returns the total number of groups
 * @param[in]		pMng		Pointer to the call manager structure
 */
int _vc_core_cm_get_group_count(call_vc_manager_t *pMng)
{
	int num = 0;
	int i = 0;

	VOICECALL_RETURN_ZERO_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		CALL_ENG_DEBUG(ENG_DEBUG, "pMng->callgroup_info[%d].num = %d", i, pMng->callgroup_info[i].num);

		if (pMng->callgroup_info[i].num != 0)
			num++;
	}
	return num;
}

/**
 * This function retrieves the call state of a given group and object position
 *
 * @return		Returns call state of the object
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 * @param[in]		nPos			Call object index
 */
voicecall_call_state_t _vc_core_cm_get_call_state_ingroup_byposition(call_vc_manager_t *pMng, int nGroup, int nPos)
{
	int memIndex = 0;

	VOICECALL_RETURN_VALUE_IF_FAIL(pMng != NULL, VC_CALL_STATE_NONE);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nGroup, 0, 1, VC_CALL_STATE_NONE);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nPos, 0, 4, VC_CALL_STATE_NONE);

	if (pMng->callgroup_info[nGroup].state == CALL_VC_GROUP_STATE_NONE) {
		return VC_CALL_STATE_NONE;
	}

	if (nPos >= pMng->callgroup_info[nGroup].num) {
		return VC_CALL_STATE_NONE;
	}

	memIndex = pMng->callgroup_info[nGroup].callobject_index[nPos];

	return pMng->callobject_info[memIndex].state;
}

/**
 * This function retrieves the call object info of a given group and object position
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 * @param[in]		nPos			Call object index
 * @param[out]	pcall_object	Pointer to the call object info structure
 */
gboolean _vc_core_cm_get_call_info_ingroup_byposition(call_vc_manager_t *pMng, int nGroup, int nPos, call_vc_call_objectinfo_t *pcall_object)
{
	int memIndex = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nGroup, 0, 1, FALSE);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nPos, 0, 4, FALSE);

	if (pMng->callgroup_info[nGroup].state == CALL_VC_GROUP_STATE_NONE)
		return FALSE;
	if (nPos >= pMng->callgroup_info[nGroup].num)
		return FALSE;

	memIndex = pMng->callgroup_info[nGroup].callobject_index[nPos];

	memcpy(pcall_object, &pMng->callobject_info[memIndex], sizeof(call_vc_call_objectinfo_t));

	return TRUE;
}

/**
 * This function checks if connected call exists
 *
 * @return		Returns TRUE if call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_isexits_outgoing_call, _vc_core_cm_isexists_incoming_call
 */
gboolean _vc_core_cm_isexists_connected_call(call_vc_manager_t *pMng)
{
	int i = 0;
	int group = 0;
	int callIndex = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (group = 0; group < CALL_VC_CALL_GROUP_MAX; group++) {
		for (i = 0; i < pMng->callgroup_info[group].num; i++) {
			callIndex = pMng->callgroup_info[group].callobject_index[i];
			if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
				return TRUE;
			}
		}
	}
	return FALSE;

}

/**
 * This function retrieves the group state for a given call ID of an
 *
 * @return		This function returns the group state
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callID		The call ID of the object
 */
call_vc_groupstate_t _vc_core_cm_get_group_state_callid(call_vc_manager_t *pMng, int callID)
{
	int i = 0;
	int nGroup = 0;
	int callIndex = 0;

	VOICECALL_RETURN_VALUE_IF_FAIL(pMng != NULL, CALL_VC_GROUP_STATE_NONE);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(callID, 1, 7, FALSE);

	for (nGroup = 0; nGroup < CALL_VC_CALL_GROUP_MAX; nGroup++) {
		if (pMng->callgroup_info[nGroup].state == CALL_VC_GROUP_STATE_NONE)
			continue;

		for (i = 0; i < pMng->callgroup_info[nGroup].num; i++) {
			callIndex = pMng->callgroup_info[nGroup].callobject_index[i];
			if (pMng->callobject_info[callIndex].call_id == callID) {
				return pMng->callgroup_info[nGroup].state;
			}
		}
	}

	return CALL_VC_GROUP_STATE_NONE;

}

/**
 * This function clears the outgoing call information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_set_outgoing_call
 */
gboolean _vc_core_cm_clear_outgoing_call(call_vc_manager_t *pMng)
{

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	if (pMng->setupcall_info.mocall_index != VC_INVALID_CALL_INDEX) {
		_vc_core_cm_clear_call_object(&(pMng->callobject_info[pMng->setupcall_info.mocall_index]));
		pMng->setupcall_info.mocall_index = VC_INVALID_CALL_INDEX;
		pMng->setupcall_info.no_service_state = FALSE;
		pMng->setupcall_info.call_control_type = CALL_VC_SAT_CC_NONE;

		CALL_ENG_DEBUG(ENG_DEBUG, "remove outgoing call info");
		return TRUE;
	}
	return FALSE;
}

/**
 * This function sets the out going call index
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callIndex		Call index that needs to be set
 * @see			_vc_core_cm_clear_outgoing_call
 */
gboolean _vc_core_cm_set_outgoing_call(call_vc_manager_t *pMng, int callIndex)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	if (pMng->setupcall_info.mocall_index != VC_INVALID_CALL_INDEX) {
		CALL_ENG_DEBUG(ENG_DEBUG, "previous mo call not cleard!");
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "index=%d", callIndex);

	pMng->setupcall_info.mocall_index = callIndex;

	return TRUE;
}

/**
 * This function retrieves the outgoing call handle
 *
 * @return		Returns out going call handle on success and TAPI_INVALID_CALLHANDLE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 */
call_vc_handle _vc_core_cm_get_outgoing_call_handle(call_vc_manager_t *pMng)
{
	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);

	if (pMng->setupcall_info.mocall_index == VC_INVALID_CALL_INDEX) {
		return VC_TAPI_INVALID_CALLHANDLE;
	}

	return pMng->callobject_info[pMng->setupcall_info.mocall_index].call_handle;
}

/**
 * This function retrieves the outgoing call information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[out]	pcall_object	Pointer to the call object info structure
 * @see			_vc_core_cm_set_outgoing_call_info
 */
gboolean _vc_core_cm_get_outgoing_call_info(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	if (pMng->setupcall_info.mocall_index == VC_INVALID_CALL_INDEX)
		return FALSE;
	memcpy(pcall_object, &pMng->callobject_info[pMng->setupcall_info.mocall_index], sizeof(call_vc_call_objectinfo_t));

	return TRUE;
}

/**
 * This function sets the given outgoing call information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pcall_object	Pointer to the call object info structure
 * @see			_vc_core_cm_get_outgoing_call_info
 */
gboolean _vc_core_cm_set_outgoing_call_info(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	if (pMng->setupcall_info.mocall_index == VC_INVALID_CALL_INDEX)
		return FALSE;
	memcpy(&pMng->callobject_info[pMng->setupcall_info.mocall_index], pcall_object, sizeof(call_vc_call_objectinfo_t));

	return TRUE;
}

/**
 * This function checks if outgoing call exists
 *
 * @return		Returns TRUE if outgoing call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_isexists_incoming_call, _vc_core_cm_isexists_connected_call
 */
gboolean _vc_core_cm_isexits_outgoing_call(call_vc_manager_t *pMng)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	if (pMng->setupcall_info.mocall_index == VC_INVALID_CALL_INDEX)
		return FALSE;
	else
		return TRUE;
}

/**
 * This function sets the incoming call index
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callIndex		Call index of the incoming call
 */
gboolean _vc_core_cm_set_incoming_call(call_vc_manager_t *pMng, int callIndex)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	if (pMng->mtcall_index != VC_INVALID_CALL_INDEX) {
		CALL_ENG_DEBUG(ENG_DEBUG, "previous mo call not cleard!");
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "index=%d", callIndex);
	pMng->mtcall_index = callIndex;

	return TRUE;
}

/**
* This function retrieves the call handle of the incoming call
*
* @return		Returns the call handle if success, TAPI_INVALID_CALLHANDLE on failure
* @param[in]		pMng		Pointer to the call manager structure
* @see			_vc_core_cm_get_incoming_call_info
*/
call_vc_handle _vc_core_cm_get_incoming_call_handle(call_vc_manager_t *pMng)
{
	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);

	if (pMng->mtcall_index == VC_INVALID_CALL_INDEX) {
		return VC_TAPI_INVALID_CALLHANDLE;
	}

	return pMng->callobject_info[pMng->mtcall_index].call_handle;
}

/**
 * This function retrieves the call information of the incoming call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[out]	pcall_object	Pointer to the call object info structure
 * @see			_vc_core_cm_get_incoming_call_info, __vc_core_cm_set_incoming_call_info
 */
gboolean _vc_core_cm_get_incoming_call_info(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_object != NULL);

	call_handle = _vc_core_cm_get_incoming_call_handle(pMng);
	if (VC_TAPI_INVALID_CALLHANDLE == call_handle) {
		return FALSE;
	}
	return _vc_core_cm_get_call_object(pMng, call_handle, pcall_object);
}

/**
 * This function sets the call information of the incoming call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pcall_object	Pointer to the call object info structure
 * @see			_vc_core_cm_get_incoming_call_info
 */
gboolean __vc_core_cm_set_incoming_call_info(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_object != NULL);

	if (VC_INVALID_CALL_INDEX == pMng->mtcall_index) {
		return FALSE;
	}
	memcpy(&pMng->callobject_info[pMng->mtcall_index], pcall_object, sizeof(call_vc_call_objectinfo_t));

	return TRUE;
}

/**
 * This function checks if incoming call exists
 *
 * @return		Returns TRUE if call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_isexits_outgoing_call, _vc_core_cm_isexists_connected_call
 */
gboolean _vc_core_cm_isexists_incoming_call(call_vc_manager_t *pMng)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	CALL_ENG_DEBUG(ENG_DEBUG, "[pMng=0x%x]", pMng);

	CALL_ENG_DEBUG(ENG_DEBUG, "[pMng->mtcall_index=%d]", pMng->mtcall_index);

	if (pMng->mtcall_index == VC_INVALID_CALL_INDEX)
		return FALSE;
	else
		return TRUE;
}

/**
 * This function retrieves the call handle of a given group and object position
 *
 * @return		Returns call handle of the object
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 * @param[in]		nPos			Call object index
 */
call_vc_handle _vc_core_cm_get_call_handle_ingroup_byposition(call_vc_manager_t *pMng, int nGroup, int nPos)
{
	int callIndex = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nGroup, 0, 1, VC_TAPI_INVALID_CALLHANDLE);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nPos, 0, 4, VC_TAPI_INVALID_CALLHANDLE);
	VOICECALL_RETURN_INVALID_IF_FAIL(pMng->callgroup_info[nGroup].state != CALL_VC_GROUP_STATE_NONE);
	VOICECALL_RETURN_INVALID_IF_FAIL(nPos <= pMng->callgroup_info[nGroup].num);

	callIndex = pMng->callgroup_info[nGroup].callobject_index[nPos];

	return pMng->callobject_info[callIndex].call_handle;
}

/**
 * This function retrieves the call handle of a given call ID
 *
 * @return		Returns call handle of the object
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callID		Call ID for a given call object
 */
call_vc_handle _vc_core_cm_get_call_handle_ingroup_bycallId(call_vc_manager_t *pMng, int callID)
{
	int i = 0;
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(callID, 1, 7, VC_TAPI_INVALID_CALLHANDLE);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (pMng->callobject_info[i].call_id == callID) {
			return pMng->callobject_info[i].call_handle;
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "return VC_TAPI_INVALID_CALLHANDLE");

	return call_handle;

}

/**
 * This function retrieves the call handle of a given telephone number
 *
 * @return		Returns call handle of the object
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		tel_number	Telephone number of a call object
 */
call_vc_handle __vc_core_cm_get_call_handle_ingroup_bynumber(call_vc_manager_t *pMng, char *tel_number)
{
	int group = 0;
	int i = 0;
	int callIndex = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL(tel_number != NULL);

	if (strlen(tel_number) == 0) {
		return VC_TAPI_INVALID_CALLHANDLE;
	}

	for (group = 0; group < CALL_VC_CALL_GROUP_MAX; group++) {
		for (i = 0; i < pMng->callgroup_info[group].num; i++) {
			callIndex = pMng->callgroup_info[group].callobject_index[i];
			if (strcmp(pMng->callobject_info[callIndex].tel_number, tel_number) == 0) {
				return pMng->callobject_info[callIndex].call_handle;
			}
		}
	}

	return VC_TAPI_INVALID_CALLHANDLE;

}

/**
 * This function retrieves the call handle of the first active call
 *
 * @return		Returns the call handle on sucess, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pCall		Pointer to the call handle where the retrieved call handle need to be stored
 * @see			_vc_core_cm_get_next_active_call_handle
 */
int _vc_core_cm_get_first_active_call_handle(call_vc_manager_t *pMng, call_vc_handle *pCall)
{
	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL(pCall != NULL);

	if (_vc_core_cm_get_total_members_ingroup(pMng) == 0 && _vc_core_cm_isexits_outgoing_call(pMng)) {
		*pCall = _vc_core_cm_get_outgoing_call_handle(pMng);
		return VC_TAPI_INVALID_CALLHANDLE;

	}

	return _vc_core_cm_get_next_active_call_handle(pMng, pCall, 0);
}

/**
 * This function retrieves the call handle of the first held call
 *
 * @return		Returns the call handle on sucess, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pCall		Pointer to the call handle where the retrieved call handle need to be stored
 * @see			_vc_core_cm_get_next_held_call_handle
 */
int _vc_core_cm_get_first_held_call_handle(call_vc_manager_t *pMng, call_vc_handle *pCall)
{
	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL(pCall != NULL);

	return _vc_core_cm_get_next_held_call_handle(pMng, pCall, 0);
}

/**
 * This function retrieves the next active call handle
 *
 * @return		Returns the call handle on sucess, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pCall		Pointer to the call handle where the retrieved call handle need to be stored
 * @param[in]		nPos			Current position of the call object index
 * @see			_vc_core_cm_get_first_active_call_handle
 */
int _vc_core_cm_get_next_active_call_handle(call_vc_manager_t *pMng, call_vc_handle *pCall, int nPos)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL(pCall != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_ACTIVE)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		*pCall = VC_TAPI_INVALID_CALLHANDLE;
		return VC_TAPI_INVALID_CALLHANDLE;
	}

	for (k = nPos; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			*pCall = pMng->callobject_info[callIndex].call_handle;
			return k + 1;
		}
	}

	*pCall = VC_TAPI_INVALID_CALLHANDLE;

	return VC_TAPI_INVALID_CALLHANDLE;

}

/**
 * This function retrieves the next held call handle
 *
 * @return		Returns the call handle on sucess, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pCall		Pointer to the call handle where the retrieved call handle need to be stored
 * @param[in]		nPos			Current position of the call object index
 * @see			_vc_core_cm_get_first_held_call_handle
 */
int _vc_core_cm_get_next_held_call_handle(call_vc_manager_t *pMng, call_vc_handle *pCall, int nPos)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_INVALID_IF_FAIL(pCall != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_HOLD)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		*pCall = VC_TAPI_INVALID_CALLHANDLE;
		return VC_TAPI_INVALID_CALLHANDLE;
	}

	for (k = nPos; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			*pCall = pMng->callobject_info[callIndex].call_handle;
			return k + 1;
		}
	}

	*pCall = VC_TAPI_INVALID_CALLHANDLE;

	return VC_TAPI_INVALID_CALLHANDLE;

}

/**
 * This function checks if active call exists
 *
 * @return		Returns TRUE if call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_isexists_held_call
 */
gboolean _vc_core_cm_isexists_active_call(call_vc_manager_t *pMng)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_ACTIVE)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		return FALSE;
	}

	for (k = 0; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "returns TRUE");
			return TRUE;
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "returns FALSE");

	return FALSE;

}

/**
 * This function checks if held call exists
 *
 * @return		Returns TRUE if call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_isexists_active_call
 */
gboolean _vc_core_cm_isexists_held_call(call_vc_manager_t *pMng)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_HOLD)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		return FALSE;
	}

	for (k = 0; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "returns TRUE");

			return TRUE;
		}
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "returns FALSE");
	return FALSE;
}

/**
 * This function retrieves the number of active calls
 *
 * @return		Returns number of active calls
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_held_call_count
 */
int _vc_core_cm_get_active_call_count(call_vc_manager_t *pMng)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;
	int count = 0;

	VOICECALL_RETURN_ZERO_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_ACTIVE)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		return 0;
	}

	count = 0;
	for (k = 0; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			count++;
		}
	}
	return count;
}

/**
 * This function retrieves the number of held calls
 *
 * @return		Returns number of held calls
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_held_call_count
 */
int _vc_core_cm_get_held_call_count(call_vc_manager_t *pMng)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;
	int count = 0;

	VOICECALL_RETURN_ZERO_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_HOLD)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		return 0;
	}

	count = 0;
	for (k = 0; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			count++;
		}
	}
	return count;
}

/**
 * This function retrieves the group index of active calls
 *
 * @return		Returns the group index
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_held_group_index
 */
int _vc_core_cm_get_active_group_index(call_vc_manager_t *pMng)
{
	int i = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (_vc_core_cm_get_group_state(pMng, i) == CALL_VC_GROUP_STATE_ACTIVE)
			return i;
	}
	return VC_TAPI_INVALID_CALLHANDLE;
}

/**
 * This function retrieves the group index of held calls
 *
 * @return		Returns the group index
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_active_group_index
 */
int _vc_core_cm_get_held_group_index(call_vc_manager_t *pMng)
{
	int i = 0;

	VOICECALL_RETURN_INVALID_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (_vc_core_cm_get_group_state(pMng, i) == CALL_VC_GROUP_STATE_HOLD)
			return i;
	}
	return VC_TAPI_INVALID_CALLHANDLE;

}

 /**
 * This function changes the state of the given onject info
 *
 * @return		void
 * @param[in]		info			Pointer to the call object into structure for which the state has to be changed
 * @param[in]		callState		State that needs to be set
 */
void inline _vc_core_cm_change_call_state(call_vc_call_objectinfo_t *info, voicecall_call_state_t callState)
{
	VOICECALL_RETURN_IF_FAIL(info != NULL);
	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d..%s(%d)-->%s(%d))", info->call_handle, gaszCallStateName[info->state], info->state, gaszCallStateName[callState], callState);
	info->state = callState;
}

/**
 * This function dumps the complete information in the call manager
 *
 * @return		void
 * @param[in]		info		Pointer to the call manager structure
 */
void _vc_core_cm_test_dump(call_vc_manager_t *info)
{
	int i = 0;
	int j = 0;
	int count = 0;
	char szBuffer[320];

	VOICECALL_RETURN_IF_FAIL(info != NULL);
	CALL_ENG_DEBUG(ENG_ERR, "************START*****************CALL_DETAILS_DUMP************START*****************");

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		char szTemp[10];
		memset(szTemp, 0, sizeof(szTemp));
		if (info->callgroup_info[i].state == CALL_VC_GROUP_STATE_NONE) {
			snprintf(szTemp, 5, "%s", "NONE");
		} else if (info->callgroup_info[i].state == CALL_VC_GROUP_STATE_ACTIVE) {
			snprintf(szTemp, 7, "%s", "ACTIVE");
		} else if (info->callgroup_info[i].state == CALL_VC_GROUP_STATE_HOLD) {
			snprintf(szTemp, 5, "%s", "HOLD");
		}

		snprintf(szBuffer, (27 + sizeof(i) + sizeof(int) + sizeof(szTemp)), "__________ group=%d(num=%d) %s", i, info->callgroup_info[i].num, szTemp);
		CALL_ENG_DEBUG(ENG_ERR, "%s", szBuffer);
		for (j = 0; j < info->callgroup_info[i].num; j++) {
			snprintf(szBuffer, (50 + sizeof(j) + sizeof(int) + sizeof(int) + VC_PHONE_NUMBER_LENGTH_MAX),
				 "             __pos=%d, bufindex=%d, Call Handle = %d, [%s], state:[%d]",
				 j, info->callgroup_info[i].callobject_index[j],
				 info->callobject_info[info->callgroup_info[i].callobject_index[j]].call_handle,
				 info->callobject_info[info->callgroup_info[i].callobject_index[j]].tel_number, info->callobject_info[info->callgroup_info[i].callobject_index[j]].state);
			CALL_ENG_DEBUG(ENG_ERR, "%s", szBuffer);
		}
	}
	if (info->setupcall_info.mocall_index == VC_INVALID_CALL_INDEX) {
		CALL_ENG_DEBUG(ENG_ERR, "_______ MO call not exist");
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "_______ MO callIndex=%d, Call Handle = %d, %s",
			       info->setupcall_info.mocall_index, info->callobject_info[info->setupcall_info.mocall_index].call_handle, info->callobject_info[info->setupcall_info.mocall_index].tel_number);
	}
	if (info->mtcall_index == VC_INVALID_CALL_INDEX) {
		CALL_ENG_DEBUG(ENG_ERR, "_______ MT call not exist");
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "_______ MT callIndex=%d, Call Handle = %d, [%s], state:[%d]",
			       info->mtcall_index, info->callobject_info[info->mtcall_index].call_handle, info->callobject_info[info->mtcall_index].tel_number, info->callobject_info[info->mtcall_index].state);
	}
	count = 0;
	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if (info->callobject_info[i].call_handle != VC_TAPI_INVALID_CALLHANDLE) {
			count++;
		}
	}
	CALL_ENG_DEBUG(ENG_ERR, "____Call Handle Num=%d", count);
	CALL_ENG_DEBUG(ENG_ERR, "**************END*****************CALL_DETAILS_DUMP**************END*****************");
}

#ifdef UNUSED_APIS
unsigned long call_vc_cm_search_oldcall(call_vc_manager_t *pMng, gboolean activegroup)
{
	int index = 0;
	int totalmember = 0;
	int k = 0;
	int min = 0;
	unsigned long mintmconnected = 0;

	VOICECALL_RETURN_VALUE_IF_FAIL(pMng != NULL, 0);
	if (activegroup == TRUE) {
		index = _vc_core_cm_get_active_group_index(pMng);
	} else if (activegroup == FALSE) {
		index = _vc_core_cm_get_held_group_index(pMng);
	}

	totalmember = pMng->callgroup_info[index].num;

	if (totalmember > 1) {
		for (k = 1; k < totalmember; k++) {
			if (pMng->callobject_info[pMng->callgroup_info[index].callobject_index[k]].connected_time < pMng->callobject_info[pMng->callgroup_info[index].callobject_index[min]].connected_time) {
				min = k;
			}
		}
		mintmconnected = pMng->callobject_info[pMng->callgroup_info[index].callobject_index[min]].connected_time;
	} else if (totalmember == 1)
		mintmconnected = pMng->callobject_info[pMng->callgroup_info[index].callobject_index[0]].connected_time;

	CALL_ENG_DEBUG(ENG_DEBUG, "index = %d", index);
	CALL_ENG_DEBUG(ENG_DEBUG, "totalmember = %d", totalmember);
	CALL_ENG_DEBUG(ENG_DEBUG, "min = %d", min);
	CALL_ENG_DEBUG(ENG_DEBUG, "min tmconnected = %d", mintmconnected);

	return mintmconnected;

}

unsigned long call_vc_cm_search_next_oldcall(call_vc_manager_t *pMng, gboolean activegroup, call_vc_handle call_handle)
{
	int index = 0;
	int totalmember = 0;
	int i = 0;
	int k = 0;
	int min = 0;
	int j = 0;
	int searchinfonum = 0;
	unsigned long mintmconnected = 0xFFFF;
	unsigned long searchInfo[VC_MAX_CALL_GROUP_MEMBER];

	VOICECALL_RETURN_VALUE_IF_FAIL(pMng != NULL, 0);
	CALL_ENG_DEBUG(ENG_DEBUG, "ended call_handle = %d", call_handle);

	if (activegroup == TRUE) {
		index = _vc_core_cm_get_active_group_index(pMng);
	} else if (activegroup == FALSE) {
		index = _vc_core_cm_get_held_group_index(pMng);
	}

	totalmember = pMng->callgroup_info[index].num;

	CALL_ENG_DEBUG(ENG_DEBUG, "totalmember = %d", totalmember);

	for (i = 0; i < totalmember; i++) {
		if (pMng->callobject_info[pMng->callgroup_info[index].callobject_index[i]].call_handle != call_handle) {
			searchInfo[j] = pMng->callobject_info[pMng->callgroup_info[index].callobject_index[i]].connected_time;
			CALL_ENG_DEBUG(ENG_DEBUG, "searchInfo[%d] = %d", j, searchInfo[j]);
			j = j + 1;
		}
	}

	searchinfonum = j;

	CALL_ENG_DEBUG(ENG_DEBUG, "searchinfonum = %d", searchinfonum);

	if (searchinfonum > 1) {
		for (k = 1; k < searchinfonum; k++) {
			if (searchInfo[k] < searchInfo[min]) {
				min = k;
			}
		}
		mintmconnected = searchInfo[min];
	} else if (searchinfonum == 1)
		mintmconnected = searchInfo[0];

	CALL_ENG_DEBUG(ENG_DEBUG, "index = %d", index);
	CALL_ENG_DEBUG(ENG_DEBUG, "min = %d", min);
	CALL_ENG_DEBUG(ENG_DEBUG, "min tmconnected = %d", mintmconnected);

	return mintmconnected;

}

unsigned long call_vc_cm_gp_get_calltm_connected(call_vc_manager_t *pMng, int nGroup, int nPos)
{
	int callIndex = 0;
	VOICECALL_RETURN_VALUE_IF_FAIL(pMng != NULL, 0);
	VOICECALL_RETURN_VALUE_IF_FAIL((nGroup >= CALL_VC_CALL_GROUP_0 && nGroup < CALL_VC_CALL_GROUP_MAX), 0);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(nPos, 0, 4, 0);

	callIndex = pMng->callgroup_info[nGroup].callobject_index[nPos];
	return pMng->callobject_info[callIndex].connected_time;
}

gboolean call_vc_cm_search_activecall(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_ACTIVE)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		return FALSE;
	}
	for (k = 0; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			if (call_handle == pMng->callobject_info[callIndex].call_handle)
				return TRUE;
		}
	}
	return FALSE;

}

gboolean call_vc_cm_search_activecall_ctinfo(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;
	gboolean bctinfo_found = FALSE;
	gboolean bctinfo_atleast_one_found = FALSE;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_ACTIVE)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		return FALSE;
	}
	for (k = 0; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			bctinfo_found = pMng->callobject_info[callIndex].bctinfo_found;
			CALL_ENG_DEBUG(ENG_DEBUG, "pMng->callobject_info[%d].bctinfo_found (%d).", callIndex, pMng->callobject_info[callIndex].bctinfo_found);
		}

		bctinfo_atleast_one_found = bctinfo_found || bctinfo_atleast_one_found;

	}

	return bctinfo_atleast_one_found;

}

gboolean call_vc_cm_search_holdcall(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int i = 0;
	int k = 0;
	int callIndex = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_HOLD)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		return FALSE;
	}
	for (k = 0; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			if (call_handle == pMng->callobject_info[callIndex].call_handle)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean call_vc_cm_search_holdcall_ctinfo(call_vc_manager_t *pMng, call_vc_handle call_handle)
{
	int i = 0;
	int k = 0 callIndex;
	gboolean bctinfo_found = FALSE;
	gboolean bCtInfoAtLeastOnde = FALSE;

	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (i = 0; i < CALL_VC_CALL_GROUP_MAX; i++) {
		if (pMng->callgroup_info[i].state == CALL_VC_GROUP_STATE_HOLD)
			break;
	}
	if (i == CALL_VC_CALL_GROUP_MAX) {
		return FALSE;
	}
	for (k = 0; k < pMng->callgroup_info[i].num; k++) {
		callIndex = pMng->callgroup_info[i].callobject_index[k];
		if ((pMng->callobject_info[callIndex].call_handle != VC_TAPI_INVALID_CALLHANDLE) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[callIndex].state != VC_CALL_STATE_ENDED_FINISH)) {
			bctinfo_found = pMng->callobject_info[callIndex].bctinfo_found;
			CALL_ENG_DEBUG(ENG_DEBUG, "pMng->callobject_info[%d].bctinfo_found (%d).", callIndex, pMng->callobject_info[callIndex].bctinfo_found);
		}

		bCtInfoAtLeastOnde = bctinfo_found || bCtInfoAtLeastOnde;
	}
	return bCtInfoAtLeastOnde;
}
#endif

/**
 * This function retrieves an available call ID
 *
 * @return		Call ID on success, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 */
int _vc_core_cm_get_new_callId(call_vc_manager_t *pMng)
{
	gboolean bCheck[VC_MAX_CALL_ID];
	int i = 0;

	VOICECALL_RETURN_VALUE_IF_FAIL(pMng != NULL, VC_TAPI_INVALID_CALLHANDLE);

	memset(bCheck, 0, VC_MAX_CALL_ID);
	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if ((pMng->callobject_info[i].state != VC_CALL_STATE_ENDED) && (pMng->callobject_info[i].state != VC_CALL_STATE_ENDED_FINISH)) {
			VOICECALL_RETURN_INVALID_IF_FAIL(((pMng->callobject_info[i].call_id >= 0) && (pMng->callobject_info[i].call_id <= VC_MAX_CALL_ID)));
			if (pMng->callobject_info[i].call_id != 0) {
				bCheck[pMng->callobject_info[i].call_id - 1] = TRUE;
			}
		}
	}

	for (i = 0; i < VC_MAX_CALL_ID; i++) {
		if (bCheck[i] == FALSE) {
			CALL_ENG_DEBUG(ENG_DEBUG, "CallID=%d", i + 1);
			return i + 1;
		}
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "fail to get id!");
	return VC_TAPI_INVALID_CALLHANDLE;
}

/**
 * This function checks if active/ held exists
 *
 * @return		Returns TRUE if success else FALSE
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[out]	active_calls	TRUE if active call exists, FALSE if held call exists
 * @param[out]	held_calls	TRUE if held call exists, FALSE if held call exists
 * @see			_vc_core_cm_isexits_outgoing_call, _vc_core_cm_isexists_connected_call
 */
gboolean _vc_core_cm_isexists_call_ingroup(call_vc_manager_t *pMng, int *active_calls, int *held_calls)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(active_calls != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(held_calls != NULL);

	*active_calls = _vc_core_cm_isexists_active_call(pMng);
	*held_calls = _vc_core_cm_isexists_held_call(pMng);

	CALL_ENG_DEBUG(ENG_DEBUG, "act_calls :%d", *active_calls);
	CALL_ENG_DEBUG(ENG_DEBUG, "hld_calls :%d", *held_calls);

	return TRUE;
}

/**
 * This function changes the call state of the call object corresponds to the given call handle
 *
 * @return		Returns TRUE if success else FALSE
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[out]	call_handle	Call Handle of the Callobject to be modified
 * @param[out]	callState		New call state to be set
 */
gboolean _vc_core_cm_change_call_object_state(call_vc_manager_t *pMng, call_vc_handle call_handle, voicecall_call_state_t callState)
{
	call_vc_call_objectinfo_t callobject_info;
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	if (VC_TAPI_INVALID_CALLHANDLE == call_handle)
		return FALSE;

	_vc_core_cm_clear_call_object(&callobject_info);

	if (!_vc_core_cm_get_call_object(pMng, call_handle, &callobject_info))
		return FALSE;

	_vc_core_cm_change_call_state(&callobject_info, callState);

	_vc_core_cm_set_call_object(pMng, &callobject_info);

	return TRUE;

}

gboolean _vc_core_cm_get_ending_call_info(call_vc_manager_t *pMng)
{
	int i = 0;
	gboolean bcall_ending = FALSE;
	VOICECALL_RETURN_FALSE_IF_FAIL(pMng != NULL);

	for (i = 0; i < VC_MAX_CALL_MEMBER; i++) {
		if ((pMng->callobject_info[i].state == VC_CALL_STATE_ENDED) || (pMng->callobject_info[i].state == VC_CALL_STATE_ENDED_FINISH)) {

			CALL_ENG_DEBUG(ENG_DEBUG, "Tel(%s) is ending!!", pMng->callobject_info[i].tel_number);
			bcall_ending = TRUE;
			break;
		}
	}
	return bcall_ending;
}
