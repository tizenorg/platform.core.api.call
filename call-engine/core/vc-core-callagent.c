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

#include "vc-core-callagent.h"
#include "vc-core-engine.h"
#include "vc-core-ccbs.h"
#include "vc-core-tapi-rqst.h"
#include "vc-core-tapi-evnt.h"
#include "vc-core-svcall.h"
#include <assert.h>

/* For Debug Information, Inout state name string constant */
char *gszcall_vc_inout_state[VC_INOUT_STATE_MAX_NUM] = {
	"NONE",
	"OUTGOING_START",
	"OUTGOING_WAIT_HOLD",
	"OUTGOING_WAIT_ORIG",
	"OUTGOING_WAIT_ALERT",
	"OUTGOING_WAIT_CONNECTED",
	"OUTGOING_WAIT_RELEASE",
	"OUTGOING_ABORTED",
	"OUTGOING_SHOW_REDIALCAUSE",
	"OUTGOING_WAIT_REDIAL",
	"OUTGOING_SHOW_RETRY_CALLBOX",
	"OUTGOING_END",
	"INCOME_SELFEVENT_WAIT",
	"INCOME_START",
	"INCOME_BOX",
	"INCOME_WAIT_REDIRECTCNF",
	"INCOME_WAIT_REDIRECT_END",
	"INCOME_WAIT_CONNECTED",
	"INCOME_WAIT_HOLD_CONNECTED",
	"INCOME_WAIT_RELEASE_ACTIVE_CONNECTED",
	"INCOME_WAIT_HOLD",
	"INCOME_WAIT_RELEASE_ACTIVECALL",
	"INCOME_WAIT_RELEASE_HOLDCALL",
	"INCOME_WAIT_RELEASE",
	"INCOME_END"
};

/* For Debug Information, Call Agent State name string constant */
char *gszcall_vc_ca_state[CALL_VC_CA_STATE_MAX_NUM] = {
	"CA_STATE_NORMAL",
	"CA_STATE_SPLIT_CALLBOX",
	"CA_STATE_WAIT_SPLIT",
	"CA_STATE_DROP_CALLBOX",
	"CA_STATE_WAIT_DROP",
#ifdef SWAP_SUPPORT
	"CA_STATE_WAIT_SWAP_HOLD_OR_ACTIVATE",
	"CA_STATE_WAIT_SWAP_HOLD",
	"CA_STATE_WAIT_SWAP_ACTIVATE",
#endif				/*                */
	"CA_STATE_WAIT_SWAP",
	"CA_STATE_WAIT_HOLD",
	"CA_STATE_WAIT_UNHOLD",
	"CA_STATE_WAIT_JOIN",
	"CA_STATE_WAIT_TRANSFER_CNF",
	"CA_STATE_WAIT_TRANSFER_CALLEND",
	"CA_STATE_WAIT_RELEASE_ALL_ACTIVECALL",
	"CA_STATE_WAIT_RELEASE_ALL_HOLDCALL",
	"CA_STATE_SENDMSG_CALLBOX",
	"CA_STATE_VIEW_CONTACT_DETAIL_CALLBOX",
	"CA_STATE_SAVE_TO_CONTACT_CALLBOX",
	"CA_STATE_SS_WAIT_RELEASE_ALL_ACTIVECALL",
	"CA_STATE_WAIT_RELEASE_ALL_CALLS",
	"CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SETUP",
	"CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SWITCH_TO_VIDEO_CALL"
};

/**
 * This function intializes the call agent
 *
 * @return		Pointer to call agent state.
 */
call_vc_callagent_state_t *_vc_core_ca_init_agent()
{
	call_vc_callagent_state_t *pcall_agent = NULL;
	pcall_agent = (call_vc_callagent_state_t *)calloc(1, sizeof(call_vc_callagent_state_t));
	if (NULL == pcall_agent) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Voicecall Engine Initialization Failed: MEM ALLOC Failure");
		return NULL;
	}

	/*Initialize Call Manager */
	_vc_core_call_manager_init(&pcall_agent->call_manager);

	/* Initialize Call Agent Flags */
	_vc_core_ca_init_data(pcall_agent);

#ifdef _CCBS_DEFINED_
	/* Init CCBS Info */
	_vc_core_init_ccbs_info(pcall_agent);
#endif

	/* Init CPHS Info */
#ifdef _CPHS_DEFINED_
	_vc_core_svcall_init_cphs_info(pcall_agent);
#endif
	return pcall_agent;
}

/**
 * This function intializes the callagent data
 *
 * @return		void
 * @param[in]		pagent					Pointer to the call agent structure
 */
void _vc_core_ca_init_data(call_vc_callagent_state_t *pagent)
{
	VOICECALL_RETURN_IF_FAIL(pagent != NULL);
	pagent->bonly_sos_call = FALSE;
	pagent->callagent_state = CALL_VC_CA_STATE_NORMAL;
	pagent->io_state = VC_INOUT_STATE_NONE;
	pagent->bis_no_sim = FALSE;
	pagent->aoc_ppm = VC_INVALID_PPM;
}

/**
 * This function finalizes the call agent
 *
 * @return		Returns void
 * @param[in]		pcall_agent Pointer to the call agent structure
 */
void _vc_core_ca_finish_agent(call_vc_callagent_state_t *pcall_agent)
{
	if (NULL != pcall_agent) {
		free(pcall_agent);
		pcall_agent = NULL;
	}
}

/**
 * This function changes the in out state of the call agent
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pagent		Pointer to the call agent structure
 * @param[in]		new_state		The new i/o state that should be set
 * @see			_vc_core_ca_change_agent_state
 */
gboolean _vc_core_ca_change_inout_state(call_vc_callagent_state_t *pagent, voicecall_inout_state_t new_state)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pagent != NULL);

	CALL_ENG_DEBUG(ENG_ERR, "%s(%d) --> %s(%d)", gszcall_vc_inout_state[pagent->io_state], pagent->io_state, gszcall_vc_inout_state[new_state], new_state);
	pagent->io_state = new_state;
	return TRUE;
}

/**
 * This function changes the in call agent state
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pAgent		Pointer to the call agent structure
 * @param[in]		new_state		The new call agent state that should be set
 * @see			_vc_core_ca_change_inout_state
 */
gboolean _vc_core_ca_change_agent_state(call_vc_callagent_state_t *pAgent, call_vc_ca_state_t new_state)
{
	VOICECALL_RETURN_FALSE_IF_FAIL((new_state >= CALL_VC_CA_STATE_NORMAL && new_state < CALL_VC_CA_STATE_MAX_NUM));
	CALL_ENG_DEBUG(ENG_DEBUG, "%s(%d) --> %s(%d)", gszcall_vc_ca_state[pAgent->callagent_state], pAgent->callagent_state, gszcall_vc_ca_state[new_state], new_state);
	pAgent->callagent_state = new_state;
	return TRUE;
}

/**
 * This function checks if all the call members have terminated or not
 *
 * @return		Returns TRUE if no call members exist, FALSE otherwise
 * @param[in]		pAgent		Pointer to the call agent structure
 */
gboolean _vc_core_ca_check_end(call_vc_callagent_state_t *pAgent)
{
	gboolean result = FALSE;
	VOICECALL_RETURN_FALSE_IF_FAIL(pAgent != NULL);
	if ((_vc_core_cm_get_call_member_count(&pAgent->call_manager) == 0) && (pAgent->io_state == VC_INOUT_STATE_NONE)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TRUE");
		result = TRUE;
	}

	else {
		CALL_ENG_DEBUG(ENG_DEBUG, "FALSE");
		result = FALSE;
	}
	return result;
}


/**
 * This function sends the response to the SAT engine
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]	pagent				Pointer to the call agent structure
 * @param[in]	sat_rqst_resp_type	sat rquest/response type used by the client
 * @param[in]	sat_response_type	response to be sent to sat
 */
gboolean _vc_core_ca_send_sat_response(call_vc_callagent_state_t *pagent, voicecall_engine_sat_rqst_resp_type sat_rqst_resp_type, call_vc_sat_reponse_type_t sat_response_type)
{
	call_vc_satsetup_info_t *pcall_vc_satcall_info = (call_vc_satsetup_info_t *) &(pagent->call_manager.setupcall_info.satcall_setup_info);
	TelSatAppsRetInfo_t call_vc_sat_response = {0,};
	TapiResult_t error_code;
	CALL_ENG_DEBUG(ENG_DEBUG, "sat_rqst_resp_type: %d, sat_response_type: %d", sat_rqst_resp_type, sat_response_type);
	switch (sat_rqst_resp_type) {
	case SAT_RQST_SETUP_CALL:
		{
			TelSatCallRetInfo_t sat_engine_ret_call = {0,};
			switch (sat_response_type) {
			case CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND:
				sat_engine_ret_call.resp = TAPI_SAT_R_ME_UNABLE_TO_PROCESS_COMMAND;
				sat_engine_ret_call.bIsTapiCauseExist = FALSE;
				sat_engine_ret_call.tapiCause = TAPI_CAUSE_UNKNOWN;
				sat_engine_ret_call.meProblem = TAPI_SAT_ME_PROBLEM_ME_BUSY_ON_CALL;
				sat_engine_ret_call.bIsOtherInfoExist = FALSE;
				sat_engine_ret_call.permanentCallCtrlProblem = TAPI_SAT_CC_PROBLEM_NO_SPECIFIC_CAUSE;
				break;
			case CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND:
				sat_engine_ret_call.resp = TAPI_SAT_R_NETWORK_UNABLE_TO_PROCESS_COMMAND;
				sat_engine_ret_call.bIsTapiCauseExist = TRUE;
				sat_engine_ret_call.tapiCause = TAPI_CAUSE_BUSY;
				sat_engine_ret_call.meProblem = TAPI_SAT_ME_PROBLEM_NO_SERVICE;
				sat_engine_ret_call.bIsOtherInfoExist = FALSE;
				sat_engine_ret_call.permanentCallCtrlProblem = TAPI_SAT_CC_PROBLEM_NO_SPECIFIC_CAUSE;
				break;
			case CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND_WITHOUT_CAUSE:
				sat_engine_ret_call.resp = TAPI_SAT_R_NETWORK_UNABLE_TO_PROCESS_COMMAND;
				sat_engine_ret_call.bIsTapiCauseExist = FALSE;
				sat_engine_ret_call.tapiCause = TAPI_CAUSE_UNKNOWN;
				sat_engine_ret_call.meProblem = TAPI_SAT_ME_PROBLEM_NO_SPECIFIC_CAUSE;
				sat_engine_ret_call.bIsOtherInfoExist = FALSE;
				sat_engine_ret_call.permanentCallCtrlProblem = TAPI_SAT_CC_PROBLEM_NO_SPECIFIC_CAUSE;
				break;
			case CALL_VC_ME_CONTROL_PERMANENT_PROBLEM:
				sat_engine_ret_call.resp = TAPI_SAT_R_INTRCTN_WITH_CC_OR_SMS_CTRL_PRMNT_PRBLM;
				sat_engine_ret_call.bIsTapiCauseExist = FALSE;
				sat_engine_ret_call.tapiCause = TAPI_CAUSE_UNKNOWN;
				sat_engine_ret_call.meProblem = TAPI_SAT_ME_PROBLEM_ACCESS_CONTROL_CLASS_BAR;
				sat_engine_ret_call.bIsOtherInfoExist = FALSE;
				sat_engine_ret_call.permanentCallCtrlProblem = TAPI_SAT_CC_PROBLEM_ACTION_NOT_ALLOWED;
				break;
			case CALL_VC_ME_CLEAR_DOWN_BEFORE_CONN:
				sat_engine_ret_call.resp = TAPI_SAT_R_USER_CLEAR_DOWN_CALL_BEFORE_CONN;
				sat_engine_ret_call.bIsTapiCauseExist = FALSE;
				sat_engine_ret_call.tapiCause = TAPI_CAUSE_UNKNOWN;
				sat_engine_ret_call.meProblem = TAPI_SAT_ME_PROBLEM_ME_BUSY_ON_CALL;
				sat_engine_ret_call.bIsOtherInfoExist = FALSE;
				sat_engine_ret_call.permanentCallCtrlProblem = TAPI_SAT_CC_PROBLEM_NO_SPECIFIC_CAUSE;
				break;
			case CALL_VC_ME_RET_SUCCESS:
				sat_engine_ret_call.resp = TAPI_SAT_R_SUCCESS;
				sat_engine_ret_call.bIsTapiCauseExist = TRUE;
				sat_engine_ret_call.tapiCause = TAPI_CAUSE_SUCCESS;
				sat_engine_ret_call.meProblem = TAPI_SAT_ME_PROBLEM_NO_SPECIFIC_CAUSE;
				sat_engine_ret_call.bIsOtherInfoExist = FALSE;
				sat_engine_ret_call.permanentCallCtrlProblem = TAPI_SAT_CC_PROBLEM_NO_SPECIFIC_CAUSE;
				break;
			default:
				return FALSE;
				break;
			}
			call_vc_sat_response.commandType = TAPI_SAT_CMD_TYPE_SETUP_CALL;
			call_vc_sat_response.commandId = pcall_vc_satcall_info->satengine_setupcall_data.commandId;
			memset(&(call_vc_sat_response.appsRet.setupCall), 0, sizeof(call_vc_sat_response.appsRet.setupCall));
			memcpy(&(call_vc_sat_response.appsRet.setupCall), &sat_engine_ret_call, sizeof(call_vc_sat_response.appsRet.setupCall));
		}
		break;
	case SAT_RQST_SEND_DTMF:
		{
			TelSatDtmfRetInfo_t sat_engine_ret_dtmf;
			switch (sat_response_type) {
			case CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND:
				sat_engine_ret_dtmf.resp = TAPI_SAT_R_ME_UNABLE_TO_PROCESS_COMMAND;
				break;
			case CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND:
				sat_engine_ret_dtmf.resp = TAPI_SAT_R_NETWORK_UNABLE_TO_PROCESS_COMMAND;
				break;
			case CALL_VC_ME_CONTROL_PERMANENT_PROBLEM:
				sat_engine_ret_dtmf.resp = TAPI_SAT_R_INTRCTN_WITH_CC_OR_SMS_CTRL_PRMNT_PRBLM;
				break;
			case CALL_VC_ME_CLEAR_DOWN_BEFORE_CONN:
				sat_engine_ret_dtmf.resp = TAPI_SAT_R_USER_CLEAR_DOWN_CALL_BEFORE_CONN;
				break;
			case CALL_VC_ME_RET_SUCCESS:
				sat_engine_ret_dtmf.resp = TAPI_SAT_R_SUCCESS;
				break;
			default:
				return FALSE;
				break;
			}
			call_vc_sat_response.commandType = TAPI_SAT_CMD_TYPE_SEND_DTMF;
			call_vc_sat_response.commandId = pcall_vc_satcall_info->satengine_dtmf_data.commandId;
			memset(&(call_vc_sat_response.appsRet.sendDtmf), 0, sizeof(call_vc_sat_response.appsRet.sendDtmf));
			memcpy(&(call_vc_sat_response.appsRet.sendDtmf), &sat_engine_ret_dtmf, sizeof(call_vc_sat_response.appsRet.sendDtmf));

			/*Reset SAT DATA after sending response */
			pcall_vc_satcall_info->satengine_dtmf_data.bIsHiddenMode = FALSE;
			CALL_ENG_DEBUG(ENG_ERR, "SAT Hidden mode has been reset");
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Invalid SAT Rquest Response Type");
		break;
	}
	error_code = tel_send_sat_app_exec_result(&call_vc_sat_response);
	if (error_code != TAPI_API_SUCCESS) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Error tel_send_sat_app_exec_result():%#X", error_code);
		return FALSE;
	}

	else {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_send_sat_app_exec_result: Success");
	}
	return TRUE;
}

/**
 * This function checks whether outgoing call is possible
 *
 * @return		This function returns TRUE if outgoing call is possible or else FALSE
 * @param[in]		pagent			Pointer to the call agent structure
 * @param[in]		bemergency_number	TRUE - if outgoing call being made is emergency call or else FALSE
 */
gboolean _vc_core_ca_is_mocall_possible(call_vc_callagent_state_t *pagent, gboolean bemergency_number)
{
	gboolean bactive_call = FALSE;
	gboolean bheld_call = FALSE;
	int member_num = 0;
	VOICECALL_RETURN_FALSE_IF_FAIL(pagent != NULL);
	bactive_call = _vc_core_cm_isexists_active_call(&pagent->call_manager);
	bheld_call = _vc_core_cm_isexists_held_call(&pagent->call_manager);
	if (pagent->io_state != VC_INOUT_STATE_NONE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "io_state=%d with FALSE ..", pagent->io_state);
		return FALSE;
	}

	/* If it is emergency number, the call can be made by disconnecting all calls */
	if (bemergency_number) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Emergency number with TRUE..");
		return TRUE;
	}

	/* Mo is impossile when both active and hold call exist */
	if (bactive_call && bheld_call) {
		CALL_ENG_DEBUG(ENG_DEBUG, "ended with FALSE ..");
		return FALSE;
	}

	else {
		member_num = _vc_core_cm_get_call_member_count(&pagent->call_manager);

#ifdef _CPHS_DEFINED_
		if (bactive_call && _vc_core_svcall_cphs_csp_get_status(VC_CPHS_CSP_HOLD) == FALSE) {
			CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_svcall_cphs_csp_get_status : Hold is not possible");
			return FALSE;
		}
#endif				/* */
		/* Mo is impossile when member is more than max */
		if (member_num >= (VC_MAX_CALL_GROUP_MEMBER + 1)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_ca_is_mocall_possible: ended with FALSE ..");
			return FALSE;
		}

		else {
			CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_ca_is_mocall_possible: ended with TRUE ..");
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * This function ends all the active calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]	pagent	Handle to voicecall engine
 */
gboolean _vc_core_ca_end_active_calls(call_vc_callagent_state_t *pagent)
{
	gboolean result = FALSE;
	VOICECALL_RETURN_FALSE_IF_FAIL(pagent != NULL);
	if (TRUE == _vc_core_cm_isexists_active_call(&pagent->call_manager)) {
		result = _vc_core_tapi_rqst_release_active_calls(pagent);
		if (TRUE == result) {
			_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_ACTIVECALL);
		}
	}
	return result;
}

/**
 * This function ends all the calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]	pagent	Handle to voicecall engine
 */
gboolean _vc_core_ca_end_all_calls(call_vc_callagent_state_t *pagent)
{
	gboolean result = FALSE;
	VOICECALL_RETURN_FALSE_IF_FAIL(pagent != NULL);
	result = _vc_core_tapi_rqst_release_all_calls(pagent);
	if (TRUE == result) {
		_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS);
	}
	return result;
}

/**
 * This function ends all the held calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]	pagent        Handle to voicecall engine
 */
gboolean _vc_core_ca_end_held_calls(call_vc_callagent_state_t *pagent)
{
	gboolean result = FALSE;
	VOICECALL_RETURN_FALSE_IF_FAIL(pagent != NULL);
	if (TRUE == _vc_core_cm_isexists_held_call(&pagent->call_manager)) {
		result = _vc_core_tapi_rqst_release_held_calls(pagent);
		if (TRUE == result) {
			_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_HOLDCALL);
		}
	}
	return result;
}

/**
 * This function checks whether private call is possible or not
 *
 * @return		This function returns TRUE if private is possible or else FALSE
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_ca_is_private_call_possible(call_vc_callagent_state_t *pcall_agent)
{
	gboolean active_calls = FALSE, held_calls = FALSE;
	int active_call_member = 0;
	_vc_core_cm_isexists_call_ingroup(&pcall_agent->call_manager, &active_calls, &held_calls);
	active_call_member = _vc_core_cm_get_active_call_count(&pcall_agent->call_manager);
	if (TRUE == active_calls && FALSE == held_calls) {
		if (active_call_member > 1) {
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * This function checks whether call transfer is possible
 *
 * @return		This function returns TRUE if transfer is possible or else FALSE
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_ca_is_transfer_call_possible(call_vc_callagent_state_t *pcall_agent)
{
	gboolean active_calls = FALSE;
	gboolean held_calls = FALSE;
	int total_call_member = 0;

#ifdef _CPHS_DEFINED_
	if (FALSE == _vc_core_svcall_cphs_csp_get_status(pcall_agent, VC_CPHS_CSP_CT)) {
		return FALSE;
	}
#endif				/* */
	_vc_core_cm_isexists_call_ingroup(&pcall_agent->call_manager, &active_calls, &held_calls);
	total_call_member = _vc_core_cm_get_call_member_count(&pcall_agent->call_manager);

	/* The Explicit Call Transfer (ECT) function should be invoked in association with two existing calls which 1) one is answered and in the held state and 2) the other is answered and active or alerting. */
	if (3 == total_call_member) {
		if ((TRUE == active_calls) && (TRUE == held_calls) && (VC_INVALID_CALL_INDEX != pcall_agent->call_manager.mtcall_index)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Transfer possible..1 active, 1 held, 1 waiting..");
			return TRUE;
		}
	}

	else if (2 == total_call_member) {
		if ((TRUE == active_calls) && (TRUE == held_calls) && (VC_INVALID_CALL_INDEX == pcall_agent->call_manager.mtcall_index)) {
			return TRUE;
		}

		else if ((FALSE == active_calls) && (TRUE == held_calls) && (VC_INVALID_CALL_INDEX != pcall_agent->call_manager.mtcall_index)) {
			return TRUE;
		}

		else if ((FALSE == active_calls) && (TRUE == held_calls) && (-1 != pcall_agent->call_manager.setupcall_info.mocall_index)	/*Outgoing call exists */
			 && (VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED == pcall_agent->io_state)) {
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * This function checks whether conference call is possible
 *
 * @return		This function returns TRUE if transfer is possible or else FALSE
 * @param[in]	pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_ca_is_conf_call_possible(call_vc_callagent_state_t *pcall_agent)
{
	gboolean active_calls = FALSE;
	gboolean held_calls = FALSE;
	int total_call_member = 0;

#ifdef _CPHS_DEFINED_
	if (FALSE == _vc_core_svcall_cphs_csp_get_status(pcall_agent, VC_CPHS_CSP_MPTY)) {
		return FALSE;
	}
#endif				/* */
	_vc_core_cm_isexists_call_ingroup(&pcall_agent->call_manager, &active_calls, &held_calls);
	total_call_member = _vc_core_cm_get_call_member_count(&pcall_agent->call_manager);

	/* Joining call is impossile when active or hold call doesn't exist */
	if ((FALSE == active_calls) || (FALSE == held_calls)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Join Impossible...");
		return FALSE;
	}

	else {

		/*Joining call is impossile when member is more than max
		 * if ( total_call_member >= (CALL_VC_CALL_GROUP_MEMBER_MAX + 1)) : Max Number in Group + Another Call*/
		if (total_call_member > VC_MAX_CALL_GROUP_MEMBER) {	/*Logic Changed from above line for same condition */
			CALL_ENG_DEBUG(ENG_DEBUG, "Ended with FALSE...");
			return FALSE;
		}

		else {
			CALL_ENG_DEBUG(ENG_DEBUG, "Ended with TRUE...");
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * This function clears the data of a connected call givenits call handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]	pcall_agent	Handle to voicecall engine
 * @param[in]	call_handle	Call handle of the connected call to be cleared
 */
gboolean _vc_core_ca_clear_connected_call(call_vc_callagent_state_t *pcall_agent, int call_handle)
{
	call_vc_call_objectinfo_t call_object;
	gboolean remove = FALSE;
	int group_index = -1;
	int grp_mem_num = 0;
	int i = 0;
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(call_handle >= 0);
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &call_object);
	group_index = _vc_core_cm_get_group_index(&pcall_agent->call_manager, (call_vc_handle) call_handle);
	if (group_index == -1) {
		CALL_ENG_DEBUG(ENG_DEBUG, "ERROR: Not exist");
		return FALSE;
	}
	grp_mem_num = _vc_core_cm_get_member_count_ingroup(&pcall_agent->call_manager, group_index);
	for (i = 0; i < grp_mem_num; i++) {
		if ((_vc_core_cm_get_call_state_ingroup_byposition(&pcall_agent->call_manager, group_index, i) != VC_CALL_STATE_ENDED) && (_vc_core_cm_get_call_state_ingroup_byposition(&pcall_agent->call_manager, group_index, i) != VC_CALL_STATE_ENDED_FINISH)) {
			remove = TRUE;
		}
	}
	if (remove) {
		_vc_core_cm_remove_call_object(&pcall_agent->call_manager, call_handle);
	}

	else {
		gboolean clear_end_call = TRUE;
		_vc_core_cm_change_call_state(&call_object, VC_CALL_STATE_ENDED_FINISH);
		_vc_core_cm_set_call_object(&pcall_agent->call_manager, &call_object);
		for (i = 0; i < grp_mem_num; i++) {
			if (_vc_core_cm_get_call_state_ingroup_byposition(&pcall_agent->call_manager, group_index, i) != VC_CALL_STATE_ENDED_FINISH)
				clear_end_call = FALSE;
		}
		if (clear_end_call) {
			_vc_core_cm_clear_endcall_member(&pcall_agent->call_manager);
		}
	}
	return TRUE;
}
