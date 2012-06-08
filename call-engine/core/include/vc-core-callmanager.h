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


#ifndef __VC_CORE_CALL_MANAGER_H_
#define __VC_CORE_CALL_MANAGER_H_

#include "vc-core-util.h"
#include "vc-core-engine-types.h"

#ifdef CALLDETAILS_DUMP
#define CALL_VC_DUMP_CALLDETAILS(call_manager) _vc_core_cm_test_dump(call_manager)
#else
#define CALL_VC_DUMP_CALLDETAILS(call_manager)
#endif

/*
* This enumeration defines the Call Group
*/
typedef enum __call_vc_group_t {
	CALL_VC_CALL_GROUP_0,
	CALL_VC_CALL_GROUP_1,
	CALL_VC_CALL_GROUP_MAX
} call_vc_group_t;

/**
* This enumeration has types of sat call control
*/
typedef enum {
	CALL_VC_SAT_CC_NONE,					/**< No SAT call control */
	CALL_VC_SAT_CC_ALLOWED,				/**< SAT call control Allowed */
	CALL_VC_SAT_CC_NOT_ALLOWED,			/**< SAT call control Not Allowed */
	CALL_VC_SAT_CC_ALLOWED_WITH_MODIFIED	/**< SAT call control allowed with modification */
} call_vc_callcontrol_type_t;

/**
 * This structure is used to handle the call group information
 */
typedef struct {
	int num;											/**< The number of members in the group */
	int callobject_index[VC_MAX_CALL_GROUP_MEMBER];		/**< Index to the call object for each member */
	call_vc_groupstate_t state;							/**< State of the group */
} call_vc_groupinfo_t;

/**
 * This structure is used to handle SAT setup information
 */
typedef struct {
	TelSatSetupCallIndCallData_t satengine_setupcall_data;	/**< Sat call setup proactive command*/
	TelSatCallCtrlIndData_t satengine_callctrl_data;	/**< Sat call control confirm data*/
	TelSatSendDtmfIndDtmfData_t satengine_dtmf_data;	/**< Sat send dtmf data*/
	int satengine_event_type;								/**< Event type of Sat engine */
	gboolean redial;												/**< Redial yes? or No? */
	TelCallCause_t tapi_cause;								/**< Tapi Success / Error Cause */
	call_vc_handle call_handle;								/**< Tapi call handle */
	gboolean bduration;												/**< Duration */
	unsigned long remaining_duration;							/**< Remaining duration */
	/*guchar	sat_rgb_data[64*64];*/
	guchar *psat_rgb_data;
} call_vc_satsetup_info_t;

/**
 * This structure is used to handle call setup information
 */
typedef struct {
	int mocall_index;												/**< Index for originated call */
	gboolean no_service_state;												/**< No service? */
	call_vc_callcontrol_type_t call_control_type;							/**< SAT call control type */
	char modified_number[VC_PHONE_NUMBER_LENGTH_MAX];				      /**< Phone number */
	voicecall_call_orig_type_t call_type;										/**< Type of the voice call */

	call_vc_satsetup_info_t satcall_setup_info;							/**<applies when call_type is VC_CALL_ORIG_TYPE_SAT */
} call_vc_setupcall_info_t;

/**
 *  This structure holds the status of call members
 */
typedef struct {
	call_vc_call_objectinfo_t callobject_info[VC_MAX_CALL_MEMBER];		 /**< CallObject having details of Single Call */
	call_vc_groupinfo_t callgroup_info[CALL_VC_CALL_GROUP_MAX];			 /**< Maintains the Call Group Details */
	call_vc_setupcall_info_t setupcall_info;								 /**< MO Call Details */
	int mtcall_index;												 /**< Index for the MT Call Object */
} call_vc_manager_t;

/**
 * This function initializes the call manager
 *
 * @return		void
 * @param[out]	pMng		Pointer to the call manager structure
 */
void _vc_core_call_manager_init(call_vc_manager_t *pMng);

/**
 * This function adds the given call object info to a free slot.
 *
 * @return		This function returns the call object index on success and -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pcall_object	Pointer to the call object info structure that need to be added
 * @see			_vc_core_cm_set_call_object, _vc_core_cm_remove_call_object
 */
int _vc_core_cm_add_call_object(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object);

/**
 * This function sets the given call object info to the given slot
 *
 * @return		This function returns the call object index on success and -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pcall_object	Pointer to the call object info structure that need to be modified
 * @see			_vc_core_cm_add_call_object, _vc_core_cm_remove_call_object, _vc_core_cm_get_call_object
 */
int _vc_core_cm_set_call_object(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object);

/**
 * This function removes the specified call object info of a given call handle
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	Handle of the call object to be removed
 * @see			_vc_core_cm_add_call_object, _vc_core_cm_set_call_object
 */
gboolean _vc_core_cm_remove_call_object(call_vc_manager_t *pMng, call_vc_handle call_handle);

/**
 * This function retrieves the given call object info
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	Call handle of the object info to be retrieved
 * @param[out]	pcall_object	Pointer to the call object info structure that should be retrieved
 * @see			_vc_core_cm_add_call_object, _vc_core_cm_remove_call_object, _vc_core_cm_set_call_object
 */
gboolean _vc_core_cm_get_call_object(call_vc_manager_t *pMng, call_vc_handle call_handle, call_vc_call_objectinfo_t *pcall_object);

/**
 * This function retrieves the given call object info for the given index
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		index		index of the object info to be retrieved
 * @param[out]	pcall_object	Pointer to the call object info structure that should be retrieved
 * @see			_vc_core_cm_add_call_object, _vc_core_cm_remove_call_object, _vc_core_cm_set_call_object
 */
gboolean _vc_core_cm_get_call_object_byindex(call_vc_manager_t *pMng, int index, call_vc_call_objectinfo_t *pcall_object);

/**
 * This function retrieves the call state of a given call handle
 *
 * @return		This function returns the call state
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	Call handle for which the state has to be retrieved
 */
voicecall_call_state_t _vc_core_cm_get_call_state(call_vc_manager_t *pMng, call_vc_handle call_handle);

/**
 * This function returns the number of calls in a given call state
 *
 * @return		This function returns the number of calls in the given state
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		state		The call state the number of which has to be retrieved
 */
int _vc_core_cm_get_call_state_num(call_vc_manager_t *pMng, voicecall_call_state_t state);

/**
 * This function clears the information of a given call state
 *
 * @return		void
 * @param[in]	pcall_object	Pointer to the call object which has to be cleared
 */
void _vc_core_cm_clear_call_object(call_vc_call_objectinfo_t *pcall_object);

/**
* This function clears the information of the callobject available in the given index
*
* @return		TRUE, if the object cleared, otherwise FALSE
* @param[in]	pMng		Pointer to the call manager structure
* @param[in]	index		Index of the call object to be cleared
*/
gboolean _vc_core_cm_clear_call_object_byindex(call_vc_manager_t *pMng, int index);

/**
 * This function clears the call object information of the ended and finished calls
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 */
gboolean _vc_core_cm_clear_endcall_member(call_vc_manager_t *pMng);

/**
 * This function adds a call member to a group
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callobject_index	The object index of the call object to be added
 * @see			__vc_core_cm_remove_call_member_fromgroup
 */
gboolean _vc_core_cm_add_call_member_togroup(call_vc_manager_t *pMng, int callobject_index);

/**
 * This function retreives the group state for the given group index
 *
 * @return		This function returns the group state
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		group index
 */
call_vc_groupstate_t _vc_core_cm_get_group_state(call_vc_manager_t *pMng, int nGroup);

/**
 * This function sets the state of a given group
 *
 * @return		void
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		The call group for which the state needs to be changed
 * @param[in]		state		The state to be set
 * @see			_vc_core_cm_swap_group_state, _vc_core_cm_get_call_pos_ingroup
 */
void _vc_core_cm_set_group_state(call_vc_manager_t *pMng, int nGroup, call_vc_groupstate_t state);

/**
 * This function retrieves the index of the group given the call handle
 *
 * @return		Returns group index on success and -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	The for which the group index has to be retrieved
 * @see			_vc_core_cm_set_group_state, _vc_core_cm_get_call_pos_ingroup
 */
int _vc_core_cm_get_group_index(call_vc_manager_t *pMng, call_vc_handle call_handle);

/**
 * This function retrieves the index/position of the call info in the object info table for a given call handle
 *
 * @return		Returns index on success and -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group ID
 * @param[in]		call_handle	The call handle for which the call index has to be retrieved
 * @see			_vc_core_cm_get_group_index
 */
int _vc_core_cm_get_call_pos_ingroup(call_vc_manager_t *pMng, int nGroup, call_vc_handle call_handle);

/**
 * This function swaps the state of the groups
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_set_group_state
 */
gboolean _vc_core_cm_swap_group_state(call_vc_manager_t *pMng);

/**
 * This function joins the groups
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_split_group
 */
gboolean _vc_core_cm_join_group(call_vc_manager_t *pMng);

/**
 * This function splits the group given a call handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		call_handle	Call handle
 * @see			_vc_core_cm_join_group
 */
gboolean _vc_core_cm_split_group(call_vc_manager_t *pMng, call_vc_handle call_handle);

/**
 * This function retrieves the total number of members in all the groups
 *
 * @return		Returns the total number of members
 * @param[in]		pMng		Pointer to the call manager structure
 */
int _vc_core_cm_get_total_members_ingroup(call_vc_manager_t *pMng);

/**
 * This function retrieves the total number of groups
 *
 * @return		Returns the total number of groups
 * @param[in]		pMng		Pointer to the call manager structure
 */
int _vc_core_cm_get_group_count(call_vc_manager_t *pMng);

/**
 * This function clears the outgoing call information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_set_outgoing_call
 */
gboolean _vc_core_cm_clear_outgoing_call(call_vc_manager_t *pMng);

/**
 * This function sets the out going call index
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callIndex		Call index that needs to be set
 * @see			_vc_core_cm_clear_outgoing_call
 */
gboolean _vc_core_cm_set_outgoing_call(call_vc_manager_t *pMng, int callIndex);

/**
 * This function retrieves the outgoing call handle
 *
 * @return		Returns out going call handle on success and TAPI_INVALID_CALLHANDLE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 */
call_vc_handle _vc_core_cm_get_outgoing_call_handle(call_vc_manager_t *pMng);

/**
 * This function retrieves the outgoing call information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[out]	pcall_object	Pointer to the call object info structure
 * @see			_vc_core_cm_set_outgoing_call_info
 */
gboolean _vc_core_cm_get_outgoing_call_info(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object);

/**
 * This function sets the given outgoing call information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pcall_object	Pointer to the call object info structure
 * @see			_vc_core_cm_get_outgoing_call_info
 */
gboolean _vc_core_cm_set_outgoing_call_info(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object);

/**
 * This function sets the incoming call index
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callIndex		Call index of the incoming call
 */
gboolean _vc_core_cm_set_incoming_call(call_vc_manager_t *pMng, int callIndex);

/**
 * This function retrieves the call handle of the incoming call
 *
 * @return		Returns the call handle if success, TAPI_INVALID_CALLHANDLE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_incoming_call_info
 */
call_vc_handle _vc_core_cm_get_incoming_call_handle(call_vc_manager_t *pMng);

/**
 * This function retrieves the call information of the incoming call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[out]	pcall_object	Pointer to the call object info structure
 * @see			_vc_core_cm_get_incoming_call_info, call_vc_cm_set_incomingcall_info
 */
gboolean _vc_core_cm_get_incoming_call_info(call_vc_manager_t *pMng, call_vc_call_objectinfo_t *pcall_object);

/**
 * This function checks if incoming call exists
 *
 * @return		Returns TRUE if call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			call_vc_cm_outgoing_call_exist, _vc_core_cm_isexists_connected_call
 */
gboolean _vc_core_cm_isexists_incoming_call(call_vc_manager_t *pMng);

/**
 * This function checks if active/ held exists
 *
 * @return		Returns TRUE if success else FALSE
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[out]	active_calls	TRUE if active call exists, FALSE if held call exists
 * @param[out]	held_calls	TRUE if held call exists, FALSE if held call exists
 * @see			call_vc_cm_outgoing_call_exist, _vc_core_cm_isexists_connected_call
 */
gboolean _vc_core_cm_isexists_call_ingroup(call_vc_manager_t *pMng, int *active_calls, int *held_calls);

/**
 * This function changes the call state of the call object corresponds to the given call handle
 *
 * @return		Returns TRUE if success else FALSE
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[out]	call_handle	Call Handle of the Callobject to be modified
 * @param[out]	callState		New call state to be set
 */
gboolean _vc_core_cm_change_call_object_state(call_vc_manager_t *pMng, call_vc_handle call_handle, voicecall_call_state_t callState);

/**
 * This function returns the status of currently being ended call
 *
 * @return		Returns TRUE if any calls are being ended, FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 */
gboolean _vc_core_cm_get_ending_call_info(call_vc_manager_t *pMng);

/**
 * This function retrieves the number of members in a group
 *
 * @return		Returns number of members in the group
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 */
int _vc_core_cm_get_member_count_ingroup(call_vc_manager_t *pMng, int nGroup);

/**
 * This function retrieves the number of connected members in a group
 *
 * @return		Returns number of connected members in the group
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 */
int _vc_core_cm_get_connected_member_count_ingroup(call_vc_manager_t *pMng, int nGroup);

/**
 * This function retrieves the call object info of a given group and object position
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 * @param[in]		nPos			Call object index
 * @param[out]	pcall_object	Pointer to the call object info structure
 */
gboolean _vc_core_cm_get_call_info_ingroup_byposition(call_vc_manager_t *pMng, int nGroup, int nPos, call_vc_call_objectinfo_t *pcall_object);

/**
 * This function retrieves the call state of a given group and object position
 *
 * @return		Returns call state of the object
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 * @param[in]		nPos			Call object index
 */
voicecall_call_state_t _vc_core_cm_get_call_state_ingroup_byposition(call_vc_manager_t *pMng, int nGroup, int nPos);

/**
 * This function retrieves the call handle of a given group and object position
 *
 * @return		Returns call handle of the object
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		nGroup		Group index
 * @param[in]		nPos			Call object index
 */
call_vc_handle _vc_core_cm_get_call_handle_ingroup_byposition(call_vc_manager_t *pMng, int nGroup, int nPos);

/**
 * This function retrieves the call handle of a given call ID
 *
 * @return		Returns call handle of the object
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callID		Call ID for a given call object
 */
call_vc_handle _vc_core_cm_get_call_handle_ingroup_bycallId(call_vc_manager_t *pMng, int callID);

/**
 * This function checks if connected call exists
 *
 * @return		Returns TRUE if call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			call_vc_cm_outgoing_call_exist, _vc_core_cm_isexists_incoming_call
 */
gboolean _vc_core_cm_isexists_connected_call(call_vc_manager_t *pMng);

/**
 * This function retrieves the group state for a given call ID of an
 *
 * @return		This function returns the group state
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		callID		The call ID of the object
 */
call_vc_groupstate_t _vc_core_cm_get_group_state_callid(call_vc_manager_t *pMng, int callID);

/**
 * This function retrieves the call handle of the first active call
 *
 * @return		Returns the call handle on sucess, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pCall		Pointer to the call handle where the retrieved call handle need to be stored
 * @see			_vc_core_cm_get_next_active_call_handle
 */
int _vc_core_cm_get_first_active_call_handle(call_vc_manager_t *pMng, call_vc_handle *pCall);

/**
 * This function retrieves the call handle of the first held call
 *
 * @return		Returns the call handle on sucess, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pCall		Pointer to the call handle where the retrieved call handle need to be stored
 * @see			_vc_core_cm_get_next_held_call_handle
 */
int _vc_core_cm_get_first_held_call_handle(call_vc_manager_t *pMng, call_vc_handle *pCall);

/**
 * This function retrieves the next active call handle
 *
 * @return		Returns the call handle on sucess, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pCall		Pointer to the call handle where the retrieved call handle need to be stored
 * @param[in]		nPos			Current position of the call object index
 * @see			_vc_core_cm_get_first_active_call_handle
 */
int _vc_core_cm_get_next_active_call_handle(call_vc_manager_t *pMng, call_vc_handle *pCall, int nPos);

/**
 * This function retrieves the next held call handle
 *
 * @return		Returns the call handle on sucess, -1 on failure
 * @param[in]		pMng		Pointer to the call manager structure
 * @param[in]		pCall		Pointer to the call handle where the retrieved call handle need to be stored
 * @param[in]		nPos			Current position of the call object index
 * @see			_vc_core_cm_get_first_held_call_handle
 */
int _vc_core_cm_get_next_held_call_handle(call_vc_manager_t *pMng, call_vc_handle *pCall, int nPos);

/**
 * This function checks if active call exists
 *
 * @return		Returns TRUE if call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_isexists_held_call
 */
gboolean _vc_core_cm_isexists_active_call(call_vc_manager_t *pMng);

/**
 * This function checks if held call exists
 *
 * @return		Returns TRUE if call exist FALSE otherwise
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_isexists_active_call
 */
gboolean _vc_core_cm_isexists_held_call(call_vc_manager_t *pMng);

/**
 * This function retrieves the number of active calls
 *
 * @return		Returns number of active calls
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_held_call_count
 */
int _vc_core_cm_get_active_call_count(call_vc_manager_t *pMng);

/**
 * This function retrieves the number of held calls
 *
 * @return		Returns number of held calls
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_held_call_count
 */
int _vc_core_cm_get_held_call_count(call_vc_manager_t *pMng);

/**
 * This function retrieves the group index of active calls
 *
 * @return		Returns the group index
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_held_group_index
 */
int _vc_core_cm_get_active_group_index(call_vc_manager_t *pMng);

/**
 * This function retrieves the group index of held calls
 *
 * @return		Returns the group index
 * @param[in]		pMng		Pointer to the call manager structure
 * @see			_vc_core_cm_get_active_group_index
 */
int _vc_core_cm_get_held_group_index(call_vc_manager_t *pMng);

/**
 * This function changes the state of the given onject info
 *
 * @return		void
 * @param[in]		info			Pointer to the call object into structure for which the state has to be changed
 * @param[in]		callState		State that needs to be set
 */
void inline _vc_core_cm_change_call_state(call_vc_call_objectinfo_t *info, voicecall_call_state_t callState);

/**
 * This function dumps the complete information in the call manager
 *
 * @return		void
 * @param[in]		info		Pointer to the call manager structure
 */
void _vc_core_cm_test_dump(call_vc_manager_t *info);

/*========================================================================================*/
/**
 * This function retrieves the number of call members in various states of a call
 *
 * @return		Number of call members available
 * @param[in]		pMng		Pointer to the call manager structure
 */
int _vc_core_cm_get_call_member_count(call_vc_manager_t *pMng);

#ifdef UNUSED_APIS
unsigned long call_vc_cm_search_oldcall(call_vc_manager_t *pMng, gboolean activegroup);
unsigned long call_vc_cm_search_next_oldcall(call_vc_manager_t *pMng, gboolean activegroup, call_vc_handle call_handle);
unsigned long call_vc_cm_gp_get_calltm_connected(call_vc_manager_t *pMng, int nGroup, int nPos);
gboolean call_vc_cm_search_activecall(call_vc_manager_t *pMng, call_vc_handle call_handle);
gboolean call_vc_cm_search_activecall_ctinfo(call_vc_manager_t *pMng, call_vc_handle call_handle);
gboolean call_vc_cm_search_holdcall(call_vc_manager_t *pMng, call_vc_handle call_handle);
gboolean call_vc_cm_search_holdcall_ctinfo(call_vc_manager_t *pMng, call_vc_handle call_handle);
#endif

/**
* This function retrieves an available call ID
*
* @return		Call ID on success, -1 on failure
* @param[in]		pMng		Pointer to the call manager structure
*/
int _vc_core_cm_get_new_callId(call_vc_manager_t *pMng);
#endif				/* __VC_CORE_CALL_MANAGER_H_ */
