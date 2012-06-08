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


#include <sys/time.h>
#include "vc-core-tapi-rqst.h"
#include "vc-core-util.h"
#include "vc-core-engine-types.h"

static int gcall_vc_callmember_count = 0;
static gboolean gcall_vc_callend_wait = FALSE;

#ifdef _CALL_LONG_DTMF
static char gsz_dtmf_buffer[TAPI_CALL_DIALDIGIT_LEN_MAX + 1];
static int gdtmf_headindex = 0;
static int gdtmf_tailindex = 0;
static call_vc_dtmf_bufferstate_t gdtmf_buffer_state = CALL_VC_DTMF_BUF_NONE;
static gboolean glong_dtmf_mode = FALSE;
#endif

/*Local Function Declerations */
/**
* This function splits the given call from the conference call , if available
*
* @internal
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		call_handle		handle of the call to be splitted
*/
static gboolean __call_vc_split_member(call_vc_handle call_handle);

 /**
 * This function prepares for a call setup
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_prepare_setup_call(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_call_objectinfo_t callobject_info = { 0 };

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "IO State =%d, Agent State: %d", pcall_agent->io_state, pcall_agent->callagent_state);

	_vc_core_cm_clear_call_object(&callobject_info);
	if ((_vc_core_cm_get_outgoing_call_info(&pcall_agent->call_manager, &callobject_info) == FALSE) || (strlen(callobject_info.tel_number) == 0) || (pcall_agent->callagent_state != CALL_VC_CA_STATE_NORMAL)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "MO Call Not Possible: Number: %s", callobject_info.tel_number);
		return FALSE;
	} else if (_vc_core_cm_isexists_active_call(&pcall_agent->call_manager) && _vc_core_cm_isexists_held_call(&pcall_agent->call_manager)) {
		/*Emergency calls should be established, even when active or hold calls exists */
		if (TRUE == callobject_info.bemergency_number) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SETUP);
			_vc_core_tapi_rqst_release_all_calls(pcall_agent);
			return TRUE;
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "Active&Held call exist!");
			return FALSE;
		}
	} else if (_vc_core_cm_isexists_active_call(&pcall_agent->call_manager)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "There is active call when trying new call...");

		/* Change the In Out State accordingly after Hold Call */
		if (_vc_core_tapi_rqst_hold_call(pcall_agent) == FALSE) {
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_NONE);
			return FALSE;
		} else {
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_OUTGOING_WAIT_HOLD);
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_HOLD);
			return TRUE;
		}
	} else {
		if (_vc_core_tapi_rqst_setup_call(pcall_agent) == FALSE) {
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_NONE);
			CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_tapi_rqst_prepare_setup_call:Fail to _vc_core_tapi_rqst_setup_call");
			return FALSE;
		} else {
			/* Wait for the TAPI_EVENT_CALL_ORIG */
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_OUTGOING_WAIT_ORIG);
			return TRUE;
		}
	}

	return FALSE;
}

 /**
 * This function sets up an outgoing call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_setup_call(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	TelCallSetupParams_t setupCallInfo;
/*	TelCallCugInfo_t pCugInfo = {0,};*/
	call_vc_call_objectinfo_t callobject_info = { 0 };
/*	TelCallIdentityMode_t	identityMode = TAPI_CALL_IDENTITY_DEFAULT;*/
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;
	clock_t start;
	clock_t end;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	memset(&setupCallInfo, 0, sizeof(TelCallSetupParams_t));

	/* Get the Outgoing Call Info */
	_vc_core_cm_clear_call_object(&callobject_info);
	if (_vc_core_cm_get_outgoing_call_info(&pcall_agent->call_manager, &callobject_info) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call info does not exist!");
		return FALSE;
	}
	/* set setupCallInfo structure for call setup */
	if (callobject_info.bemergency_number == TRUE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Emergency call!");
		setupCallInfo.CallType = TAPI_CALL_TYPE_E911;
		/*setupCallInfo.Ecc = callobject_info.ecc_category;
		CALL_ENG_DEBUG(ENG_DEBUG,"Emergency call, ecc_category:[%d]!", callobject_info.ecc_category);*/
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Normal call!");
		setupCallInfo.CallType = TAPI_CALL_TYPE_VOICE;
	}

	/*Set the Call Object MO Flag as TRUE */
	callobject_info.mo = TRUE;

	/* cli setting */
	if (_vc_core_util_extract_call_number_without_cli(callobject_info.source_tel_number, setupCallInfo.szNumber, sizeof(setupCallInfo.szNumber)) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No proper number = %s", callobject_info.source_tel_number);
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "tapi callnum=[%s]", setupCallInfo.szNumber);


	/* CUG settings */
#ifdef _TAPI_CUG_
	setupCallInfo.pCugInfo = &pCugInfo;
	if (FALSE == callobject_info.cug_info.bcug_used) {
		setupCallInfo.pCugInfo->bCugFlag = FALSE;
	} else {
		setupCallInfo.pCugInfo->bCugFlag = TRUE;
		/*if the index is 0, use pref cug, so no cug index */
		if (0 == callobject_info.cug_info.cug_index) {
			setupCallInfo.pCugInfo->Option = TAPI_CALL_CUG_NO_INFO;
			setupCallInfo.pCugInfo->Index = 0;
		} else {
			if ((FALSE == callobject_info.cug_info.bpref_cug) && (FALSE == callobject_info.cug_info.boa_cug)) {
				setupCallInfo.pCugInfo->Option = TAPI_CALL_CUG_SUPRESS_OA_AND_CUG;
			} else if (FALSE == callobject_info.cug_info.bpref_cug) {
				setupCallInfo.pCugInfo->Option = TAPI_CALL_CUG_SUPRESS_PRF_CUG;
			} else if (FALSE == callobject_info.cug_info.boa_cug) {
				setupCallInfo.pCugInfo->Option = TAPI_CALL_CUG_SUPRESS_OA;
			} else {
				setupCallInfo.pCugInfo->Option = TAPI_CALL_CUG_NO_INFO;
			}
			setupCallInfo.pCugInfo->Index = callobject_info.cug_info.cug_index;
		}
	}
#endif

#ifdef _CPHS_DEFINED_
	if (TRUE == _vc_core_svcall_cphs_csp_get_status(pcall_agent, VC_CPHS_CSP_ALS)) {
		if (callobject_info.setupBy == VC_CALL_SETUP_BY_MAILBOX) {
			if (callobject_info.alsLine == VC_CALL_CPHS_ALS_LINE1) {
				tel_set_call_act_line(TAPI_CALL_ACTIVE_LINE1, &ReqId);
			} else if (callobject_info.alsLine == VC_CALL_CPHS_ALS_LINE2) {
				tel_set_call_act_line(TAPI_CALL_ACTIVE_LINE2, &ReqId);
			} else {
				CALL_ENG_DEBUG(ENG_DEBUG, "callobject_info.alsLine invalid value=%d", callobject_info.alsLine);
				tel_set_call_act_line(TAPI_CALL_ACTIVE_LINE1, &ReqId);
			}
		} else {
			/*read the line information from the dynamic flags */
			voice_call_cphs_alsline_t als_line;
			callobject_info.alsLine = _vc_core_svcall_get_cphs_als_active_line(pcall_agent);
			tel_set_call_act_line(callobject_info.alsLine);
			_vc_core_cm_set_outgoing_call_info(&pcall_agent->call_manager, &callobject_info);
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "ACtive Line Set is TAPI_ACTIVE_LINE1");
		tel_set_call_act_line(TAPI_CALL_ACTIVE_LINE1, &ReqId);
	}
#endif

	CALL_ENG_DEBUG(ENG_DEBUG, "call_type = %d", setupCallInfo.CallType);

	CALL_ENG_DEBUG(ENG_DEBUG, "Call Type by Source: %d", callobject_info.call_type);
	if (VC_CALL_ORIG_TYPE_SAT == callobject_info.call_type) {
		/*setupCallInfo.bRequestedBySAT = TRUE;*/
	} else {
		/*setupCallInfo.bRequestedBySAT = FALSE;*/
	}
	/*CALL_ENG_DEBUG(ENG_DEBUG,"Call Initiated by SAT: %d",setupCallInfo.bRequestedBySAT);*/

	CALL_ENG_KPI("tel_exe_call_mo start");
	/*This Function originates MO Call set-up, This is asynchronous function */
	tapi_err = tel_exe_call_mo(&setupCallInfo, (TS_UINT *) &call_handle, &ReqId);
	CALL_ENG_KPI("tel_exe_call_mo done");

	CALL_ENG_DEBUG(ENG_DEBUG, "ReqId is = %d", ReqId);

	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tapi_call_setup failed: Error Code: %d", tapi_err);
		return FALSE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "call_handle = %d", call_handle);

		/* Set the Call Handle to the CallbObject for future reference */
		callobject_info.call_handle = call_handle;

		_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_OUTGOING);
		callobject_info.call_id = _vc_core_cm_get_new_callId(&pcall_agent->call_manager);

		_vc_core_cm_set_outgoing_call_info(&pcall_agent->call_manager, &callobject_info);

		CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);
	}
	return TRUE;
}

 /**
 * This function answers the call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 * @param[in]		answer_type	call answer type #voicecall_answer_type_t
 * @param[out]	error_code	Error code
 */
gboolean _vc_core_tapi_rqst_answer_call(call_vc_callagent_state_t *pcall_agent, voicecall_answer_type_t answer_type, int *error_code)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	/*Encapsulates Errors and Warnings from TAPI Library */
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(error_code != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if ((VC_INOUT_STATE_INCOME_WAIT_CONNECTED == pcall_agent->io_state)
	    || (VC_INOUT_STATE_INCOME_WAIT_HOLD_CONNECTED == pcall_agent->io_state)
	    || (VC_INOUT_STATE_INCOME_WAIT_RELEASE_ACTIVE_CONNECTED == pcall_agent->io_state))
		/*||(VC_INOUT_STATE_INCOME_WAIT_RELEASE_HOLDCALL == pcall_agent->io_state))*/
	{
		CALL_ENG_DEBUG(ENG_DEBUG, "Answer Call Request Already Made");
		*error_code = ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
		return FALSE;
	}

	/*
	   Process the answer call request only when the state is in income and it is not ended.
	   This must be checked as both incoming event and incoming end event from tapi are added to g_idle_add
	   so any change in state should be noted before accepting the call
	*/
	if ((VC_INOUT_STATE_INCOME_BOX != pcall_agent->io_state) || (VC_INOUT_STATE_INCOME_END == pcall_agent->io_state)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "IO State not in VC_INOUT_STATE_INCOME_BOX, Current state: %d", pcall_agent->io_state);
		*error_code = ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
		return FALSE;
	}

	_vc_core_cm_clear_endcall_member(&pcall_agent->call_manager);
	call_handle = _vc_core_cm_get_incoming_call_handle(&pcall_agent->call_manager);

	CALL_ENG_DEBUG(ENG_DEBUG, "answer_type = %d,Incoming call Handle: %d", answer_type, call_handle);
	if (VC_TAPI_INVALID_CALLHANDLE == call_handle) {
		*error_code = ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
		return FALSE;
	}

	switch (answer_type) {
	case VC_ANSWER_NORMAL:
		{
			/*Answer a call by accepting or rejecting a call */
			tapi_err = tel_answer_call(call_handle, TAPI_CALL_ANSWER_ACCEPT, &ReqId);
			if (TAPI_API_SUCCESS == tapi_err) {
				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_CONNECTED);
			}
		}
		break;
	case VC_ANSWER_HOLD_ACTIVE_AND_ACCEPT:
		{
			/*Answer a call by accepting or rejecting a call */
			tapi_err = tel_answer_call(call_handle, TAPI_CALL_ANSWER_HOLD_AND_ACCEPT, &ReqId);
			if (TAPI_API_SUCCESS == tapi_err) {
				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_HOLD_CONNECTED);
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_HOLD);
			}
		}
		break;
	case VC_ANSWER_RELEASE_ACTIVE_AND_ACCEPT:
		{
			/*Answer a call by accepting or rejecting a call */
			tapi_err = tel_answer_call(call_handle, TAPI_CALL_ANSWER_REPLACE, &ReqId);
			if (TAPI_API_SUCCESS == tapi_err) {
				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_RELEASE_ACTIVE_CONNECTED);
			}
		}
		break;
	case VC_ANSWER_RELEASE_HOLD_AND_ACCEPT:
		{
			/* first end held call and then accept incoming */
			if (TRUE == _vc_core_tapi_rqst_release_held_calls(pcall_agent)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_tapi_rqst_release_held_calls returns TRUE");
				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_RELEASE_HOLDCALL);
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_HOLDCALL);
			}
			return TRUE;
		}
		break;
#ifdef RELEASE_ALL_AND_ACCEPT_SUPPORT
	case VC_ANSWER_RELEASE_ALL_AND_ACCEPT:
		{
			/* first (end held call) and then ( release accept and accept ) */
			if (TRUE == _vc_core_tapi_rqst_release_held_calls(pcall_agent)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_tapi_rqst_release_held_calls returns TRUE");
				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_RELEASE_HOLDCALL_FOR_ALL_RELEASE);
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_HOLDCALL);
			}
			return TRUE;
		}
		break;
#endif
	default:
		{
			*error_code = ERROR_VOICECALL_NOT_SUPPORTED;
			return FALSE;
		}
	}

	if (TAPI_API_SUCCESS != tapi_err) {
		_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_NONE);
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_answer_call failed ,Error Code:%d", tapi_err);
		*error_code = ERROR_VOICECALL_ANSWER_FAILED;
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_answer_call failed ,Engine Error Code:%d", *error_code);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function checks and prepares to accept a call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_response_call(call_vc_callagent_state_t *pcall_agent)
{
	gboolean active_call, held_call, incoming_call;
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	active_call = _vc_core_cm_isexists_active_call(&pcall_agent->call_manager);
	held_call = _vc_core_cm_isexists_held_call(&pcall_agent->call_manager);
	incoming_call = _vc_core_cm_isexists_incoming_call(&pcall_agent->call_manager);

	if (active_call && held_call) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Show EndCallChoice Box");
		return TRUE;
	} else if (active_call) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Case of bActiceCall...");
		/* if there are active call, after hold call can accept mtc */
		/* set the flag and return */
		/*Although TapiHold failed, keep on going( because , when active call is ended, TapiHold failed then ansercall is possible... only when Tapihold succeed, state is changed to WAIT_HOLD*/
		if (_vc_core_tapi_rqst_hold_call(pcall_agent) == TRUE) {
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_HOLD);
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_HOLD);
			return TRUE;
		}
	} else if (incoming_call == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Call Available");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Calling tapi_call_respond_recall(call_handle = %d, TRUE) ...", call_handle);

	return TRUE;
}

/**
 * This function releases active calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_active_calls(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	int pReqId = VC_RQSTID_DEFAULT;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int nPos;
	int nCount = 0;
	call_vc_call_objectinfo_t callobject_info = { 0 };

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
#ifdef NEW_TAPI_API
	nPos = _vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &call_handle);

	if (call_handle != -1) {
		CALL_ENG_DEBUG(ENG_DEBUG, "First Active call_handle = %d", call_handle);
#ifdef SINGLE_CALL_END
		if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) == 1) {
			CALL_ENG_DEBUG(ENG_DEBUG, "End Single call..");

			/* Use ReleaseAll api in case single call is ended - this is caused by modem limitation */
			tapi_err = tel_release_call_all(&pReqId);
		} else
#endif
		{
			tapi_err = tel_release_call_all_active(&pReqId);
		}

		if (TAPI_API_SUCCESS != tapi_err) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Release All Active Failed call_handle=%d Error Code:%d...", call_handle, tapi_err);
			return FALSE;
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "pReqId = %d", pReqId);
			while (nPos != -1) {
				_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info);
				_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_RELEASE_WAIT);
				_vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);

				nPos = _vc_core_cm_get_next_active_call_handle(&pcall_agent->call_manager, &call_handle, nPos);
			}

			return TRUE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Active calls available");
		return FALSE;
	}
#else
	gcall_vc_callmember_count = 0;

	nPos = _vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &call_handle);

	_vc_core_cm_clear_call_object(&callobject_info);
	while (nPos != -1) {
		_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info);

		if (callobject_info.state == VC_CALL_STATE_CONNECTED) {
#ifdef SINGLE_CALL_END
			if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) == 1) {
				CALL_ENG_DEBUG(ENG_DEBUG, "End Single call..");

				/* Use ReleaseAll api in case single call is ended - this is caused by modem limitation */
				tapi_err = tel_release_call_all(&pReqId);
			} else
#endif
			{
				/*Releases the call identified by Call Handle irrespective of call is hold or active state */
				tapi_err = tel_release_call(call_handle, &pReqId);
			}

			if (TAPI_API_SUCCESS != tapi_err) {
				CALL_ENG_DEBUG(ENG_DEBUG, "tapi_call_relese Failed call_handle=%d Error Code:%d...", call_handle, tapi_err);
				return FALSE;
			} else {
				_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_RELEASE_WAIT);
				_vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);
				nCount++;
			}
		}
		nPos = _vc_core_cm_get_next_active_call_handle(&pcall_agent->call_manager, &call_handle, nPos);
	}

	gcall_vc_callmember_count = nCount;
	if (gcall_vc_callmember_count > 0) {
		gcall_vc_callend_wait = TRUE;
		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "There is no active call to release..");
		return FALSE;
	}
#endif
}

/**
 * This function releases held calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_held_calls(call_vc_callagent_state_t *pcall_agent)
{
	int nPos;
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	call_vc_call_objectinfo_t callobject_info = { 0 };
	int nCount = 0;
	int pReqId = VC_RQSTID_DEFAULT;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
#ifdef NEW_TAPI_API
	nPos = _vc_core_cm_get_first_held_call_handle(&pcall_agent->call_manager, &call_handle);

	if (call_handle != -1) {
		CALL_ENG_DEBUG(ENG_DEBUG, "First Held call_handle = %d", call_handle);
#ifdef SINGLE_CALL_END
		if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) == 1) {
			CALL_ENG_DEBUG(ENG_DEBUG, "End Single call..");

			/* Use ReleaseAll api in case single call is ended - this is caused by modem limitation */
			tapi_err = tel_release_call_all(&pReqId);
		} else
#endif
		{
			tapi_err = tel_release_call_all_held(&pReqId);
		}

		if (TAPI_API_SUCCESS != tapi_err) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Release All Held Failed call_handle=%d Error Code:%d...", call_handle, tapi_err);
			return FALSE;
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "pReqId = %d", pReqId);
			while (nPos != -1) {
				_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info);
				_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_RELEASE_WAIT);
				_vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);

				nPos = _vc_core_cm_get_next_held_call_handle(&pcall_agent->call_manager, &call_handle, nPos);
			}

			return TRUE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Held calls available");
		return FALSE;
	}
#else
	gcall_vc_callmember_count = 0;

	nPos = _vc_core_cm_get_first_held_call_handle(&pcall_agent->call_manager, &call_handle);

	_vc_core_cm_clear_call_object(&callobject_info);
	while (nPos != -1) {
		_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info);

		if (callobject_info.state == VC_CALL_STATE_CONNECTED) {
#ifdef SINGLE_CALL_END
			if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) == 1) {
				CALL_ENG_DEBUG(ENG_DEBUG, "End Single call..");

				/* Use ReleaseAll api in case single call is ended - this is caused by modem limitation */
				tapi_err = tel_release_call_all(&pReqId);
			} else
#endif
			{
				/*Releases the call identified by Call Handle irrespective of call is hold or active state */
				tapi_err = tel_release_call(call_handle, &pReqId);
			}

			if (TAPI_API_SUCCESS != tapi_err) {
				CALL_ENG_DEBUG(ENG_DEBUG, "tapi_call_relese Failed call_handle=%d Error Code:%d...", call_handle, tapi_err);
				return FALSE;
			} else {
				_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_RELEASE_WAIT);
				_vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);
				nCount++;
			}
		}
		nPos = _vc_core_cm_get_next_held_call_handle(&pcall_agent->call_manager, &call_handle, nPos);
	}

	gcall_vc_callmember_count = nCount;
	if (gcall_vc_callmember_count > 0) {
		gcall_vc_callend_wait = TRUE;
		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "There is no held call to release..");
		return FALSE;
	}
#endif
}

/**
 * This function releases all calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_all_calls(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	int pReqId = VC_RQSTID_DEFAULT;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &call_handle);
	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle:[%d]..", call_handle);

	if (VC_TAPI_INVALID_CALLHANDLE == call_handle) {
		_vc_core_cm_get_first_held_call_handle(&pcall_agent->call_manager, &call_handle);
	}

	if (VC_TAPI_INVALID_CALLHANDLE == call_handle) {
		CALL_ENG_DEBUG(ENG_DEBUG, "invalid call handle");
		return FALSE;
	}

	/*Releases All calls irrespective of call is in hold or active state */
	tapi_err = tel_release_call_all(&pReqId);
	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_release_call_all failed: Error _Code: %d", tapi_err);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function releases the incoming call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_incoming_call(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	int pReqId = VC_RQSTID_DEFAULT;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	call_handle = _vc_core_cm_get_incoming_call_handle(&pcall_agent->call_manager);
	if (VC_TAPI_INVALID_CALLHANDLE == call_handle)
		return FALSE;

#ifdef SINGLE_CALL_END
	if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) == 1) {
		CALL_ENG_DEBUG(ENG_DEBUG, "End Single call..");

		/* Use ReleaseAll api in case single call is ended - this is caused by modem limitation */
		tapi_err = tel_release_call_all(&pReqId);
	} else
#endif
	{
		/*Releases the call identified by Call Handle irrespective of call is hold or active state */
		tapi_err = tel_release_call(call_handle, &pReqId);
	}

	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_release_call() call_handle=%d Failed, Error Code: %d...", call_handle, tapi_err);
		return FALSE;
	}

	_vc_core_cm_change_call_object_state(&pcall_agent->call_manager, call_handle, VC_CALL_STATE_REJECTED);

	return TRUE;

}

/**
 * This function releases outgoing call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_release_outgoing_call(call_vc_callagent_state_t *pcall_agent)
{

	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	int pReqId = VC_RQSTID_DEFAULT;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	call_handle = _vc_core_cm_get_outgoing_call_handle(&pcall_agent->call_manager);

	if (VC_TAPI_INVALID_CALLHANDLE == call_handle)
		return FALSE;

#ifdef SINGLE_CALL_END
	if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) == 1) {
		CALL_ENG_DEBUG(ENG_DEBUG, "End Single call..");

		/* Use ReleaseAll api in case single call is ended - this is caused by modem limitation */
		tapi_err = tel_release_call_all(&pReqId);
	} else
#endif
	{
		/*Releases the call identified by Call Handle irrespective of call is hold or active state */
		tapi_err = tel_release_call(call_handle, &pReqId);
	}

	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_release_call Failed call_handle=%d Error Code:%d", call_handle, tapi_err);
		return FALSE;
	}

	_vc_core_cm_change_call_object_state(&pcall_agent->call_manager, call_handle, VC_CALL_STATE_CANCELLED);

	return TRUE;
}

/**
 * This function holds a call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_hold_call(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	/*Get the current active call handle and hold it*/
	_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &call_handle);
	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "ERROR: No active call available");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "calling tapi_call_hold, Current active call = %d...", call_handle);

	/* Hold the call */
	/*Puts the given call on hold */
	tapi_err = tel_hold_call(call_handle, &ReqId);
	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_hold_call() Failed Error Code: %d", tapi_err);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function retrieves a call from hold
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_retrieve_call(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;

	CALL_ENG_DEBUG(ENG_DEBUG, "...");
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	_vc_core_cm_get_first_held_call_handle(&pcall_agent->call_manager, &call_handle);
	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Hold Call Error...");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Cur held call_handle = %d.", call_handle);
	/* activate the call */
	/*This function retrieves the held call */
	tapi_err = tel_retrieve_call(call_handle, &ReqId);
	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_retrieve_call() Failed, Error Code: %d", tapi_err);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function swaps held and active calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_swap_calls(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle active_call = VC_TAPI_INVALID_CALLHANDLE, held_call = VC_TAPI_INVALID_CALLHANDLE;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &active_call);
	if (active_call == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Active Call...");
		return FALSE;
	}

	_vc_core_cm_get_first_held_call_handle(&pcall_agent->call_manager, &held_call);
	if (held_call == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_tapi_rqst_swap_calls: No Hold Call...");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Current Active call = %d, Hold call :%d", active_call, held_call);

	tapi_err = tel_swap_call(active_call, held_call, &ReqId);
	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_swap_call() Failed, Error Code: %d", tapi_err);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function joins two calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_join_calls(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle active_call = VC_TAPI_INVALID_CALLHANDLE, held_call = VC_TAPI_INVALID_CALLHANDLE;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &active_call);
	if (active_call == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Active Call");
		return FALSE;
	}

	_vc_core_cm_get_first_held_call_handle(&pcall_agent->call_manager, &held_call);
	if (held_call == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Hold Call");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Current Active call = %d, Hold call :%d", active_call, held_call);
	/*This functions joins given two calls */
	tapi_err = tel_join_call(active_call, held_call, &ReqId);
	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_join_call() Failed, Error Code: %d", tapi_err);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function splits the members of a call given its call handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent structure
 * @param[in]		call_handle	Call handle for a call the members of which has to be split
 */
gboolean _vc_core_tapi_rqst_private_call(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle)
{

	if (TRUE == __call_vc_split_member(call_handle)) {
		_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_SPLIT);
		return TRUE;
	} else {
		_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		return FALSE;
	}
}

static gboolean __call_vc_split_member(call_vc_handle call_handle)
{
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;

	CALL_ENG_DEBUG(ENG_DEBUG, "...");
	VOICECALL_RETURN_FALSE_IF_FAIL(call_handle != VC_TAPI_INVALID_CALLHANDLE);

	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle to be splited : %d", call_handle);

	/*Splits a private call from multiparty call. */
	tapi_err = tel_split_call(call_handle, &ReqId);
	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_split_call() Failed, Error Code: %d", tapi_err);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function transfers call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent structure
 */
gboolean _vc_core_tapi_rqst_transfer_call(call_vc_callagent_state_t *pcall_agent)
{
	call_vc_handle active_call = VC_TAPI_INVALID_CALLHANDLE;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &active_call);
	if (active_call == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Active Call...");

		active_call = _vc_core_cm_get_outgoing_call_handle(&pcall_agent->call_manager);
		if (VC_TAPI_INVALID_CALLHANDLE == active_call) {
			CALL_ENG_DEBUG(ENG_DEBUG, "No Outgoing Call...");
			return FALSE;
		} else
			CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call exist..!!");
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "active_call = %d ", active_call);

	/*An explicit call transfer by connecting the two parties where in one party being
	   active (active state) and another party being held (held state) */
	tapi_err = tel_exe_call_explicit_transfer(active_call, &ReqId);
	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_exe_call_explicit_transfer() Failed, Error Code: %d", tapi_err);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function sends the given string as dtmf
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent structure
 * @param[in]		dtmf_string	dtmf string
 */
gboolean _vc_core_tapi_rqst_start_dtmf(call_vc_callagent_state_t *pcall_agent, char *dtmf_string)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int ReqId = VC_RQSTID_DEFAULT;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Start DTMF!! string = %s", dtmf_string);

	_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &call_handle);

	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Active Call Handle..");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "string = %s", dtmf_string);

	/* start DTMF */
	/*This function sends one or more DTMF digits during call */
	tapi_err = tel_send_call_dtmf(dtmf_string, &ReqId);

	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tapi_call_dtmf Failed, Error Code: %d", tapi_err);
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, " Ended...");

	return TRUE;

}

#ifdef _SEND_UUSINFO_
/**
 * This function sends user to user information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle	The call handle for which the information has to be sent
 * @param[in]		pszUusData	User data
 */
gboolean call_vc_send_uusinfo(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, char *pszUusData)
{
	TelCallUusInfo_t uusInfo;
	int nPos;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	memset(&uusInfo, 0, sizeof(TelCallUusInfo_t));

	uusInfo.bMoreDataPresent = FALSE;	/*more_data_present = FALSE;*/
	uusInfo.ProtocolType = TAPI_CALL_UUS_PROTO_SPECIFIC;
	uusInfo.UusType = TAPI_CALL_UUS_1;
	uusInfo.UusDataLen = strlen(pszUusData);

	_vc_core_util_strcpy((char *)uusInfo.UusData, sizeof(uusInfo.UusData), pszUusData);

	nPos = _vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &call_handle);
	while (nPos != -1) {
		/*TAPI API not available to send user info */
		/*tapi doen't supprot this api.*/
		/******************************************************************************************
		tapi_err = tapi_call_send_user_info(call_handle, &uusInfo);
		if (TAPI_API_SUCCESS != tapi_err) {
			CALL_ENG_DEBUG(ENG_DEBUG, "tapi_call_send_user_info() Failed,Error Code: %d", tapi_err);
			return FALSE;
		}
		******************************************************************************************/
		nPos = _vc_core_cm_get_next_active_call_handle(&pcall_agent->call_manager, &call_handle, nPos);
	}

	return TRUE;
}
#endif

/**
 * This function releases the call associate with the given call handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent			Pointer to the call agent state
 * @param[in]		call_handle	handle of the call to be ended
 */
gboolean _vc_core_tapi_rqst_end_call_by_callhandle(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle)
{
	call_vc_call_objectinfo_t callobject_info;
	int pReqId = VC_RQSTID_DEFAULT;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	if (FALSE == _vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Call Object not available for call handle: %d", call_handle);
		return FALSE;

	}

	if (VC_CALL_STATE_CONNECTED == callobject_info.state) {
#ifdef SINGLE_CALL_END
		if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) == 1) {
			CALL_ENG_DEBUG(ENG_DEBUG, "End Single call..");

			/* Use ReleaseAll api in case single call is ended - this is caused by modem limitation */
			tapi_err = tel_release_call_all(&pReqId);
		} else
#endif
		{
			/*Releases the call identified by Call Handle irrespective of call is hold or active state */
			tapi_err = tel_release_call(call_handle, &pReqId);
		}

		if (TAPI_API_SUCCESS != tapi_err) {
			CALL_ENG_DEBUG(ENG_DEBUG, "tapi_call_release Failed Call Handle: %d, Error Code: %d", call_handle, tapi_err);
			return FALSE;
		}

		_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_RELEASE_WAIT);
		_vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);
		return TRUE;
	}

	return FALSE;
}

/**
 * This function ends a call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 */
gboolean _vc_core_tapi_rqst_end_call(call_vc_callagent_state_t *pcall_agent)
{
	gboolean ret_val = FALSE;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	switch (pcall_agent->callagent_state) {
	case CALL_VC_CA_STATE_NORMAL:
	case CALL_VC_CA_STATE_WAIT_HOLD:
	case CALL_VC_CA_STATE_WAIT_UNHOLD:
		{

			if (_vc_core_cm_isexists_active_call(&pcall_agent->call_manager)) {
				ret_val = _vc_core_tapi_rqst_release_active_calls(pcall_agent);
				if (TRUE == ret_val) {
					_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_ACTIVECALL);
				}
			} else if (_vc_core_cm_isexists_held_call(&pcall_agent->call_manager)) {
				ret_val = _vc_core_tapi_rqst_release_held_calls(pcall_agent);
				if (TRUE == ret_val) {
					_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_HOLDCALL);
				}
			} else {
				_vc_core_cm_clear_endcall_member(&pcall_agent->call_manager);
			}
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "No Action defined for current call agent state: %d", pcall_agent->callagent_state);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function rejects a mobile terminated call
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent	Pointer to the call agent state
 * @param[in]		budub		TRUE - User Determined User Busy, FALSE - otherwise
 * @param[out]	error_code	Error code
 */
gboolean _vc_core_tapi_rqst_reject_mt_call(call_vc_callagent_state_t *pcall_agent, gboolean budub, int *error_code)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	int pReqId = VC_RQSTID_DEFAULT;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "budub = %d", budub);
	if (_vc_core_cm_isexists_incoming_call(&pcall_agent->call_manager) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Incoming call does not exist");
		*error_code = ERROR_VOICECALL_NO_INCOMING_CALL_EXIST;
		return FALSE;
	}

	if (pcall_agent->io_state == VC_INOUT_STATE_INCOME_WAIT_RELEASE) {
		CALL_ENG_DEBUG(ENG_DEBUG, " io_state is already VC_INOUT_STATE_INCOME_WAIT_RELEASE");
		/*return TRUE since call release has been already done and it is waiting for the release*/
		return TRUE;
	}

	if (pcall_agent->io_state == VC_INOUT_STATE_NONE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "return FALSE io_state=%d", pcall_agent->io_state);
		*error_code = ERROR_VOICECALL_ENGINE_STATE_NONE;
		return FALSE;
	}

	call_handle = _vc_core_cm_get_incoming_call_handle(&pcall_agent->call_manager);

	if (TRUE == budub) {
		/*Reject the Call for User Busy Scenario */
		tapi_err = tel_answer_call(call_handle, TAPI_CALL_ANSWER_REJECT, &pReqId);

		if (TAPI_API_SUCCESS != tapi_err) {
			CALL_ENG_DEBUG(ENG_DEBUG, " tel_answer_call failed: %d", tapi_err);
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_NONE);
			*error_code = ERROR_VOICECALL_TAPI_ERROR;
			return FALSE;
		}
	} else {
		/*Release the call to end it normally */
		if (FALSE == _vc_core_tapi_rqst_release_incoming_call(pcall_agent)) {
			CALL_ENG_DEBUG(ENG_ERR, "Release Incoming Call Failed");
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_NONE);
			*error_code = ERROR_VOICECALL_TAPI_ERROR;
			return FALSE;
		}
	}

	_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_RELEASE);
	return TRUE;
}
