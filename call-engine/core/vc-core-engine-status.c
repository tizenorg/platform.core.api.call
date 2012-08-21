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


/* Call Module File Includes */
#include "vc-core-engine.h"
#include "vc-core-callagent.h"
#include "vc-core-callmanager.h"
#include "vc-core-util.h"
#include "vc-core-tapi-evnt.h"
#include "vc-core-tapi-rqst.h"
#include "vc-core-svcall.h"
#include "vc-core-engine-status.h"

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
voicecall_error_t _vc_core_engine_status_isrestricted_call(voicecall_engine_t *pvoicecall_agent, int call_handle, gboolean *pbrestricted)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_call_objectinfo_t call_object;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(call_handle >= 0, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pbrestricted != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	_vc_core_cm_clear_call_object(&call_object);
	if (TRUE == _vc_core_cm_get_call_object(&pagent->call_manager, call_handle, &call_object)) {
		*pbrestricted = call_object.brestricted_namemode;
		return ERROR_VOICECALL_NONE;
	}

	*pbrestricted = FALSE;
	return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

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
voicecall_error_t _vc_core_engine_status_get_calling_namemode(voicecall_engine_t *pvoicecall_agent, int call_handle, gboolean *bcalling_namemode)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_call_objectinfo_t call_object;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(call_handle >= 0, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bcalling_namemode != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	_vc_core_cm_clear_call_object(&call_object);
	if (TRUE == _vc_core_cm_get_call_object(&pagent->call_manager, call_handle, &call_object)) {
		if (call_object.bcalling_namemode == CALL_VC_CALLING_NAME_MODE_AVAILABLE) {
			*bcalling_namemode = TRUE;
		} else {
			*bcalling_namemode = FALSE;
		}
		return ERROR_VOICECALL_NONE;
	}

	*bcalling_namemode = FALSE;
	return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

/**
* This function retrieves the call object belongs to the given call handle
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle	Call handle of the call for which the call object is retrieved
* @param[out]	pcall_object	Pointer to the retrived call object info
* @remarks		pvoicecall_agent and pcall_object cannot be NULL.
*/
voicecall_error_t _vc_core_engine_status_get_call_object(voicecall_engine_t *pvoicecall_agent, int call_handle, call_vc_call_objectinfo_t *pcall_object)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(call_handle >= 0, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_object != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*Clear the call object */
	_vc_core_cm_clear_call_object(pcall_object);

	if (TRUE == _vc_core_cm_get_call_object(&pagent->call_manager, call_handle, pcall_object)) {
		return ERROR_VOICECALL_NONE;
	}

	pcall_object = NULL;
	return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

/**
* This function retrieves the inout state of the engine
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	pio_state			Contains the Engine InOut state on return
* @remarks		pvoicecall_agent and pio_state cannot be NULL.
* @see			_vc_core_engine_change_engine_iostate
*/
voicecall_error_t _vc_core_engine_status_get_engine_iostate(voicecall_engine_t *pvoicecall_agent, int *pio_state)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pio_state != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	*pio_state = pagent->io_state;
	CALL_ENG_DEBUG(ENG_DEBUG, "io_state = [%d]", pagent->io_state);
	return ERROR_VOICECALL_NONE;
}

/**
* This function checks whether connected call exists or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_type			call type
* @param[out]	bcall_exists		TRUE - if call exists of given given type, FALSE otherwise
* @remarks		pvoicecall_agent and bcall_exists cannot be NULL.
*/
voicecall_error_t _vc_core_engine_status_isexists_call_bytype(voicecall_engine_t *pvoicecall_agent, voicecall_call_type_t call_type, gboolean *bcall_exists)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bcall_exists != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*CALL_ENG_DEBUG(ENG_DEBUG,"call_type = %d", call_type);*/
	*bcall_exists = FALSE;
	switch (call_type) {
	case VC_INCOMING_CALL:
		{
			if (VC_INVALID_CALL_INDEX != pagent->call_manager.mtcall_index) {
				CALL_ENG_DEBUG(ENG_DEBUG, "incoming call exits...");

				*bcall_exists = TRUE;
				return ERROR_VOICECALL_NONE;
			}
		}
		break;
	case VC_OUTGOING_CALL:
		{
			if (VC_INVALID_CALL_INDEX != pagent->call_manager.setupcall_info.mocall_index) {
				CALL_ENG_DEBUG(ENG_DEBUG, "outgoing call exits...");

				*bcall_exists = TRUE;
				return ERROR_VOICECALL_NONE;
			}
		}
		break;
	case VC_CONNECTED_CALL:
		{
			*bcall_exists = _vc_core_cm_isexists_connected_call(&pagent->call_manager);
			if (*bcall_exists == TRUE) {
				CALL_ENG_DEBUG(ENG_DEBUG, "connected call exits...");
			}
			return ERROR_VOICECALL_NONE;
		}
		break;
	default:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Invalid call type..");
			return ERROR_VOICECALL_INVALID_CALL_TYPE;
		}
	}

	return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

/**
* This function retrieves the total number of call members currently available with the engine
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent		Handle to voicecall engine
* @param[out]	ptotal_call_member	Contains the total call member availalbe in engine on return
* @remarks		pvoicecall_agent and ptotal_call_member cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_get_call_member_count(voicecall_engine_t *pvoicecall_agent, int *ptotal_call_member)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(ptotal_call_member != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	*ptotal_call_member = _vc_core_cm_get_call_member_count(&pagent->call_manager);

	return ERROR_VOICECALL_NONE;
}

/**
* This function retrieves the total number of call members with the given connected call type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent		Handle to voicecall engine
* @param[in]	connected_call_type	connected call type
* @param[out]	pmember_num		Contains the number of call members available with the given connected call type on return
* @remarks		pvoicecall_agent and pmember_num cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_get_call_member_info(voicecall_engine_t *pvoicecall_agent, voicecall_connected_call_type_t connected_call_type, int *pmember_num)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pmember_num != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	switch (connected_call_type) {
	case VC_ACTIVE_CALL:
		{
			*pmember_num = _vc_core_cm_get_active_call_count(&pagent->call_manager);
		}
		break;
	case VC_HELD_CALL:
		{
			*pmember_num = _vc_core_cm_get_held_call_count(&pagent->call_manager);
		}
		break;
	default:
		*pmember_num = 0;
		return ERROR_VOICECALL_INVALID_CALL_TYPE;
	}

	return ERROR_VOICECALL_NONE;
}

/**
* This function retrieves the call handle according to the given call type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_type		call type
* @param[out]	pcall_handle		Contains the call handle on success
* @remarks		pvoicecall_agent and pcall_handle cannot be NULL
*				In case of multiple connected calls available, it will retreive the first connected call
*/
voicecall_error_t _vc_core_engine_status_get_call_handle_bytype(voicecall_engine_t *pvoicecall_agent, voicecall_call_type_t call_type, int *pcall_handle)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_manager_t *pcall_manager = NULL;
	call_vc_handle call_handle = -1;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_handle != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	pcall_manager = (call_vc_manager_t *) &pagent->call_manager;

	switch (call_type) {
	case VC_INCOMING_CALL:
		{
			call_handle = _vc_core_cm_get_incoming_call_handle(pcall_manager);
		}
		break;
	case VC_OUTGOING_CALL:
		{
			call_handle = _vc_core_cm_get_outgoing_call_handle(pcall_manager);
		}
		break;
	case VC_CONNECTED_CALL:
		{
			_vc_core_cm_get_first_active_call_handle(pcall_manager, &call_handle);
			if (-1 == call_handle) {
				_vc_core_cm_get_first_held_call_handle(pcall_manager, &call_handle);
			}
		}
		break;
	default:
		return ERROR_VOICECALL_INVALID_CALL_TYPE;
	}

	if (-1 == call_handle) {
		return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
	}

	*pcall_handle = call_handle;
	return ERROR_VOICECALL_NONE;
}

/**
* This function checks if active calls and/or held call exists or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	pactive_calls		Set to TRUE if active calls exist
* @param[out]	pheld_calls		Set to TRUE if held calls exist
* @remarks		pvoicecall_agent,pactive_calls and pheld_calls cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_isexists_any_call(voicecall_engine_t *pvoicecall_agent, gboolean *pactive_calls, gboolean *pheld_calls)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_manager_t *pcall_manager = NULL;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pactive_calls != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pheld_calls != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	pcall_manager = &pagent->call_manager;

	*pactive_calls = _vc_core_cm_isexists_active_call(pcall_manager);
	*pheld_calls = _vc_core_cm_isexists_held_call(pcall_manager);

	return ERROR_VOICECALL_NONE;
}

/**
* This function retreives the cphs csp status
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[in]		csp_service		csp name
* @param[out]	pbcsp_status		Contains TRUE if given csp service is enabled,FALSE  otherwise
* @remarks		pvoicecall_agent and pbcsp_status cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_get_cphs_csp_status(voicecall_engine_t *pvoicecall_agent, voicecall_cphs_csp_service csp_service, gboolean *pbcsp_status)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pbcsp_status != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
#ifdef _CPHS_DEFINED_
	*pbcsp_status = _vc_core_svcall_cphs_csp_get_status(pcall_agent, csp_service);
#else
	*pbcsp_status = (VC_CPHS_CSP_ALS == csp_service) ? FALSE : TRUE;
#endif
	return ERROR_VOICECALL_NONE;
}

/**
* This function checks if the call is emergency call for the given outgoing call index
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent	Handle to Voicecall Engine
* @param[in]		mo_call_index		Index of the outgoing call
* @param[out]	pbemergency_call	Contains TRUE if the call is emergency call,FALSE  otherwise
* @remarks		pvoicecall_agent and pbemergency_call cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_check_emergency_byindex(voicecall_engine_t *pvoicecall_agent, int mo_call_index, gboolean *pbemergency_call)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_manager_t *pcall_manager = NULL;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pbemergency_call != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(mo_call_index, 0, 7, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*Assign Default Value */
	*pbemergency_call = FALSE;

	pcall_manager = (call_vc_manager_t *) &pagent->call_manager;

	CALL_VC_DUMP_CALLDETAILS(pcall_manager);
	CALL_ENG_DEBUG(ENG_DEBUG, "call_index = %d, bemergency_number = %d", mo_call_index, pcall_manager->callobject_info[mo_call_index].bemergency_number);

	*pbemergency_call = pcall_manager->callobject_info[mo_call_index].bemergency_number;
	return ERROR_VOICECALL_NONE;
}

/**
* This function checks the possiblity of transfering calls
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent	Handle to Voicecall Engine
* @param[out]	pbtransfer_calls	Contains TRUE if call transfer is possible, FALSE otherwise
* @remarks		pvoicecall_agent and pbtransfer_calls cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_transfer_call_possible(voicecall_engine_t *pvoicecall_agent, gboolean *pbtransfer_calls)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pbtransfer_calls != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	*pbtransfer_calls = _vc_core_ca_is_transfer_call_possible(pcall_agent);

	return ERROR_VOICECALL_NONE;
}

/**
* This function checks the possiblity of making conference calls
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent	Handle to Voicecall Engine
* @param[out]	pbconf_call		Contains TRUE if conference call is possible , FALSE otherwise
* @remarks		pvoicecall_agent and pbconf_call cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_conf_call_possible(voicecall_engine_t *pvoicecall_agent, gboolean *pbconf_call)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	gboolean active_calls = FALSE;
	gboolean held_calls = FALSE;
	int total_call_member = 0;
	gboolean bconf = FALSE;

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pbconf_call != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

#ifdef _CPHS_DEFINED_
	if (_vc_core_svcall_cphs_csp_get_status(pcall_agent, VC_CPHS_CSP_MPTY)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_svcall_cphs_csp_get_status returned TRUE");
		*pbconf_call = TRUE;
	} else {
		*pbconf_call = FALSE;
		return ERROR_VOICECALL_NONE;
	}
#endif

	_vc_core_cm_isexists_call_ingroup(&pcall_agent->call_manager, &active_calls, &held_calls);
	total_call_member = _vc_core_cm_get_call_member_count(&pcall_agent->call_manager);

	/* Joining call is impossile when !active or !hold call exist */
	if (!active_calls || !held_calls) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Join Impossible...");
		bconf = FALSE;
	} else {
		/* Joining call is impossile when member is more than max
		Max Number in Group + Another Call*/
		if (total_call_member >= (VC_MAX_CALL_GROUP_MEMBER + 1)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Ended with FALSE...");
			bconf = FALSE;
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "Ended with TRUE...");
			bconf = TRUE;
		}
	}

	*pbconf_call = bconf;
	return ERROR_VOICECALL_NONE;

}

/**
* This function retreives the call state of the given call
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle		Call handle of particular call
* @param[out]	pcall_state		Contains the call state of the given call handle
* @remarks		pvoicecall_agent and pcall_state cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_get_call_state_byhandle(voicecall_engine_t *pvoicecall_agent, int call_handle, voicecall_call_state_t *pcall_state)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_call_objectinfo_t call_object = { 0 };

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_CALL_HANDLE);
	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_state != NULL, ERROR_VOICECALL_INVALID_CALL_HANDLE);
	VOICECALL_RETURN_VALUE_IF_FAIL(call_handle >= 0, ERROR_VOICECALL_INVALID_CALL_HANDLE);

	_vc_core_cm_clear_call_object(&call_object);
	if (TRUE == _vc_core_cm_get_call_object(&pagent->call_manager, call_handle, &call_object)) {
		*pcall_state = call_object.state;
		return ERROR_VOICECALL_NONE;
	}

	*pcall_state = VC_CALL_STATE_NONE;
	return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

#if	0	/*unused*/
/**
* This function retrieves call handle of the any one of the calls of given type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent		Handle to voicecall engine
* @param[in]	connected_call_type	Connected call type
* @param[out]	pcall_handle			Contains the Call handle on return
* @remarks		pvoicecall_agent and pcall_handle cannot be NULL
*/
voicecall_error_t voicecall_get_any_call_handle(voicecall_engine_t *pvoicecall_agent, voicecall_connected_call_type_t connected_call_type, int *pcall_handle)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_handle callhandle = -1;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_handle != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	switch (connected_call_type) {
	case VC_ACTIVE_CALL:
		{
			_vc_core_cm_get_first_active_call_handle(&pagent->call_manager, &callhandle);
		}
		break;
	case VC_HELD_CALL:
		{
			_vc_core_cm_get_first_held_call_handle(&pagent->call_manager, &callhandle);
		}
		break;
	default:
		return ERROR_VOICECALL_INVALID_CALL_TYPE;
	}

	if (-1 != callhandle) {
		*pcall_handle = (int)callhandle;
		return ERROR_VOICECALL_NONE;
	}

	return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}
#endif

/**
* This function checks if any call is ending and retrieves its call number if it is ending
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	bcall_ending		TRUE if any call is being ended, FALSE otherwise
* @remarks		pvoicecall_agent and bcall_ending cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_any_call_ending(voicecall_engine_t *pvoicecall_agent, gboolean *bcall_ending)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bcall_ending != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	*bcall_ending = FALSE;
	*bcall_ending = _vc_core_cm_get_ending_call_info(&pagent->call_manager);

	return ERROR_VOICECALL_NONE;
}

/**
* This function checks whther engine is busy in processing any events or waiting for any events to process
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[out]	bbusy	TRUE - if engine is busy in processing any events or waiting for any events, FALSE - otherwise
* @remarks		pvoicecall_agent and bbusy cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_engine_busy(voicecall_engine_t *pvoicecall_agent, gboolean *bbusy)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bbusy != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	CALL_ENG_DEBUG(ENG_DEBUG, "Current Call Agent State:%d", pagent->callagent_state);
	if (CALL_VC_CA_STATE_NORMAL == pagent->callagent_state) {
		*bbusy = FALSE;
	} else {
		*bbusy = TRUE;
	}

	return ERROR_VOICECALL_NONE;
}

/**
* This function sets the given flag to engine for processing during call end.
* This function has to be used after calling the end call API
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	end_flag			End Flag to be set
* @remarks		pvoicecall_agent and bsscode cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_set_end_flag(voicecall_engine_t *pvoicecall_agent, voicecall_end_flag_t end_flag)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	switch (end_flag) {
	case VC_RETREIVE_CALL_ON_MOCALL_END:
		{
			pagent->callagent_state = CALL_VC_CA_STATE_SS_WAIT_RELEASE_ALL_ACTIVECALL;
		}
		break;
	default:
		return ERROR_VOICECALL_INVALID_ARGUMENTS;
	}

	return ERROR_VOICECALL_NONE;
}

/**
* This function checks whether the given string is MMI string or not
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	pinput_number	Input string to be verified
* @param[out]	bsscode			TRUE - if the given string is a valid ss code, FALSE otherwise
* @remarks		pvoicecall_agent and bsscode cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_isvalid_ss_code(voicecall_engine_t *pvoicecall_agent, const char *pinput_number, gboolean *bsscode)
{
	int strLen = 0;
	call_vc_ss_si_format si_format = SS_SI_FORMAT_INVALID;

	VOICECALL_RETURN_VALUE_IF_FAIL(pvoicecall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pinput_number != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bsscode != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	si_format = SS_SI_FORMAT_VALID;
	strLen = strlen(pinput_number);

	CALL_ENG_DEBUG(ENG_DEBUG, "strLen = %d", strLen);

	if (strLen > 2) {
		/* Deactivate :  "#...#" */
		/* Erase :               "##...#" */
		if (((pinput_number[0] == '#') && (pinput_number[strLen - 1] == '#')) || ((pinput_number[0] == '+') && (pinput_number[1] == '#') && (pinput_number[strLen - 1] == '#'))) {
			si_format = _vc_core_util_check_si_format(pinput_number);
			*bsscode = TRUE;
			return ERROR_VOICECALL_NONE;
		}

		/* Activate :           "*...#" */
		/* Interrogate :        "*#...#" */
		/* Register :           "**...#" */
		if ((('*' == pinput_number[0]) && ('#' == pinput_number[strLen - 1])) || (('+' == pinput_number[0]) && ('*' == pinput_number[1]) && ('#' == pinput_number[strLen - 1]))) {
			si_format = _vc_core_util_check_si_format(pinput_number);
			*bsscode = TRUE;
			return ERROR_VOICECALL_NONE;
		}
	} else if (2 == strLen) {
/*This will be covered, once the operator requirements are clear*/
#ifdef MCC_USA_SS_CODE
		unsigned long mcc = 0;
		tapi_network_info_t networkInfo;

		memset(&networkInfo, 0x00, sizeof(tapi_network_info_t));

		tapi_get_network_info(&networkInfo);

		networkInfo.sysid.sysid = (networkInfo.sysid.sysid >> 16);
		mcc = (networkInfo.sysid.sysid & 0x0000ffff);
		CALL_ENG_DEBUG(ENG_DEBUG, "mcc (%d)!!", mcc);

		/*CALL_NETWORK_MCC_USA */
		if (mcc == 0x136) {
			if (strncmp(pinput_number, "08", 2) == 0 || strncmp(pinput_number, "00", 2) == 0)
				*bsscode = FALSE;
		}
#endif
		/*08 is not a ss string.*/
		if (strncmp(pinput_number, "08", 2) == 0) {
			*bsscode = FALSE;
		} else {
			/*All two digit number should be handled as ss string during call.*/
			CALL_ENG_DEBUG(ENG_DEBUG, "two digit number... returns TRUE");
			*bsscode = TRUE;
		}

		return ERROR_VOICECALL_NONE;
	} else if (1 == strLen) {
		if (('#' == pinput_number[0]) || ('+' == pinput_number[0]) || ('*' == pinput_number[0]) || ('7' == pinput_number[0])) {
			*bsscode = TRUE;
			return ERROR_VOICECALL_NONE;
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "invalid sscode... returns FALSE");
	*bsscode = FALSE;

	return ERROR_VOICECALL_NONE;
}

voicecall_error_t _vc_core_engine_status_dump_call_details(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	CALL_ENG_DEBUG(ENG_DEBUG, "");
	_vc_core_cm_test_dump(&pagent->call_manager);
	return ERROR_VOICECALL_NONE;
}

/**
* This function checks the possiblity of making private calls
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent	Handle to Voicecall Engine
* @param[out]	bzuhause		Contains TRUE if zuhause area, FALSE otherwise
* @remarks		pvoicecall_agent and pbprivate_call cannot be NULL
*/
voicecall_error_t _vc_core_engine_status_is_zuhause_area(voicecall_engine_t *pvoicecall_agent, gboolean *bzuhause)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bzuhause != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	*bzuhause = _vc_core_util_check_zuhause_status();
	return ERROR_VOICECALL_NONE;
}

/**
* This function checks the possiblity of making private calls
*
* @param[in]		pvoicecall_agent	Handle to Voicecall Engine
* @param[out]		b_download_call		Contains TRUE if zuhause area, FALSE otherwise
* @remarks			pvoicecall_agent and pbprivate_call cannot be NULL
*/
void _vc_core_engine_status_set_download_call(voicecall_engine_t *pvoicecall_agent, gboolean b_download_call)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	if (pagent == NULL) {
		CALL_ENG_DEBUG(ENG_ERR, "pagent is NULL! [PROBLEM] !!!");
		return;
	}

	pagent->bdownload_call = b_download_call;

}

gboolean _vc_core_engine_status_get_download_call(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	return pcall_agent->bdownload_call;
}
