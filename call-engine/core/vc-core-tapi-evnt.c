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


#include <string.h>
#include <assert.h>
#include "vc-core-tapi-evnt.h"
#include "vc-core-callmanager.h"
#include "vc-core-util.h"
#include "vc-core-tapi-rqst.h"

/**
* This function processes the current io wait states if  available
*
* @internal
* @return		if wait states are processed, FALSE otherwise
* @param[in]		pcall_agent		handle to voicecall agent structure
*/
static gboolean __call_vc_process_wait_state_success_events(call_vc_callagent_state_t *pcall_agent);

/**
* This function requests the TAPI for the ppm value for callcost
*
* @internal
* @return		void
* @param[in]		pcall_agent		handle to voicecall agent structure
*/
static void __call_vc_get_aoc_ppm_value(call_vc_callagent_state_t *pcall_agent);
static gboolean __call_vc_get_aoc_ppm_value_idle_cb(gpointer pdata);
static gboolean __call_vc_download_call_timer_cb(gpointer pdata);

/**
 * This function retreives the voicecall engine specific end cause type for the given TAPI end cause type
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		type				TAPI event type
 * @param[in]		cause			TAPI call end cause
 * @param[out]	end_cause_type	voicecall engine end cause
 */
void _vc_core_tapi_event_get_end_cause_type(call_vc_callagent_state_t *pcall_agent, const char *noti_id, TelTapiEndCause_t cause, voice_call_end_cause_type_t *end_cause_type)
{
	VOICECALL_RETURN_IF_FAIL(end_cause_type != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "EventType:[%s],EndCause:%d", noti_id, cause);

	if (strcmp(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_IDLE) == 0) {
		switch (cause) {
		case TAPI_CC_CAUSE_NORMAL_UNSPECIFIED:
			*end_cause_type = VC_ENDCAUSE_CALL_DISCONNECTED;
			break;
		case TAPI_CC_CAUSE_FACILITY_REJECTED:
			*end_cause_type = VC_ENDCAUSE_CALL_FAILED;
			break;
		case TAPI_CC_CAUSE_QUALITY_OF_SERVICE_UNAVAILABLE:
		case TAPI_CC_CAUSE_ACCESS_INFORMATION_DISCARDED:
		case TAPI_CC_CAUSE_BEARER_CAPABILITY_NOT_AUTHORISED:
		case TAPI_CC_CAUSE_BEARER_CAPABILITY_NOT_PRESENTLY_AVAILABLE:
		case TAPI_CC_CAUSE_SERVICE_OR_OPTION_NOT_AVAILABLE:
		case TAPI_CC_CAUSE_BEARER_SERVICE_NOT_IMPLEMENTED:
		case TAPI_CC_CAUSE_PROTOCOL_ERROR_UNSPECIFIED:

			*end_cause_type = VC_ENDCAUSE_CALL_ENDED;
			break;

		case TAPI_CC_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED:
			*end_cause_type = VC_ENDCAUSE_CALL_SERVICE_NOT_ALLOWED;
			break;

		case TAPI_CC_CAUSE_OPERATOR_DETERMINED_BARRING:
			*end_cause_type = VC_ENDCAUSE_CALL_BARRED;
			break;
		case TAPI_REJECT_CAUSE_MM_REJ_NO_SERVICE:
			*end_cause_type = VC_ENDCAUSE_NO_SERVICE;
			break;

		case TAPI_REJECT_CAUSE_CONGESTTION:
		case TAPI_REJECT_CAUSE_CNM_REJ_NO_RESOURCES:
		case TAPI_CC_CAUSE_SWITCHING_EQUIPMENT_CONGESTION:	/* Match as NW_BUSY*/
			*end_cause_type = VC_ENDCAUSE_NW_BUSY;
			break;

		case TAPI_REJECT_CAUSE_NETWORK_FAILURE:
		case TAPI_REJECT_CAUSE_MSC_TEMPORARILY_NOT_REACHABLE:
			*end_cause_type = VC_ENDCAUSE_NW_FAILED;
			break;

		case TAPI_REJECT_CAUSE_IMEI_NOT_ACCEPTED:
			{
				unsigned long mcc = 0;

				_vc_core_util_get_mcc(&mcc);

				if (mcc == CALL_NETWORK_MCC_UK) {
					*end_cause_type = VC_ENDCAUSE_IMEI_REJECTED;	/*Display "Please verify SIM or insert valid SIM"*/
				} else {
					*end_cause_type = VC_ENDCAUSE_NW_FAILED;	/*Display Network unavailable*/
				}
			}
			break;

		case TAPI_CC_CAUSE_NO_ROUTE_TO_DEST:
		case TAPI_CC_CAUSE_TEMPORARY_FAILURE:
		case TAPI_CC_CAUSE_NETWORK_OUT_OF_ORDER:
		case TAPI_CC_CAUSE_REQUESTED_CIRCUIT_CHANNEL_NOT_AVAILABLE:
		case TAPI_CC_CAUSE_NO_CIRCUIT_CHANNEL_AVAILABLE:
		case TAPI_CC_CAUSE_DESTINATION_OUT_OF_ORDER:
			*end_cause_type = VC_ENDCAUSE_SERVICE_TEMP_UNAVAILABLE;
			break;
		case TAPI_CC_CAUSE_NO_USER_RESPONDING:
		case TAPI_CC_CAUSE_USER_ALERTING_NO_ANSWER:
			*end_cause_type = VC_ENDCAUSE_USER_DOESNOT_RESPOND;
			break;

		case TAPI_CC_CAUSE_ACM_GEQ_ACMMAX:
			*end_cause_type = VC_ENDCAUSE_NO_CREDIT;
			break;

		case TAPI_CC_CAUSE_CALL_REJECTED:
			if (pcall_agent->barring_ind_type == VC_BARR_IND_ALL)
				*end_cause_type = VC_ENDCAUSE_CALL_BARRED;
			else
				*end_cause_type = VC_ENDCAUSE_USER_UNAVAILABLE;

			pcall_agent->barring_ind_type = VC_BARR_IND_NONE;
			break;

		case TAPI_CC_CAUSE_USER_BUSY:
			*end_cause_type = VC_ENDCAUSE_USER_BUSY;
			break;

		case TAPI_CC_CAUSE_USER_NOT_MEMBER_OF_CUG:
			*end_cause_type = VC_ENDCAUSE_WRONG_GROUP;
			break;

		case TAPI_CC_CAUSE_INVALID_NUMBER_FORMAT:

			*end_cause_type = VC_ENDCAUSE_INVALID_NUMBER_FORMAT;
			break;

		case TAPI_CC_CAUSE_UNASSIGNED_NUMBER:
			*end_cause_type = VC_ENDCAUSE_UNASSIGNED_NUMBER;
			break;

		case TAPI_CC_CAUSE_NUMBER_CHANGED:
			*end_cause_type = VC_ENDCAUSE_NUMBER_CHANGED;
			break;

		case TAPI_CALL_END_NO_CAUSE:
		default:
			*end_cause_type = VC_ENDCAUSE_CALL_ENDED;

			CALL_ENG_DEBUG(ENG_ERR, "Call Ended or Default Cause Value: %d", cause);
			break;
		}
	} else {
		*end_cause_type = VC_ENDCAUSE_CALL_FAILED;
		CALL_ENG_DEBUG(ENG_DEBUG, "Invalid call end cause or error !!");
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Engine End Cause: %d", (int)*end_cause_type);
}

void _vc_core_tapi_event_copy_incoming_call_data(call_vc_callagent_state_t *pcall_agent, TelCallIncomingCallInfo_t *callInfo, call_vc_call_objectinfo_t *pcallobject_info)
{
#ifdef _CPHS_DEFINED_
	if (_vc_core_svcall_cphs_csp_get_status(pcall_agent, VC_CPHS_CSP_ALS)) {
		TelCallActiveLine_t activeLine = callInfo->ActiveLine;

		if (activeLine == TAPI_CALL_ACTIVE_LINE1) {
			pcallobject_info->alsLine = VC_CALL_CPHS_ALS_LINE1;
		} else if (activeLine == TAPI_CALL_ACTIVE_LINE2) {
			pcallobject_info->alsLine = VC_CALL_CPHS_ALS_LINE2;
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "undefined currentLine=%d", activeLine);
		}
	}
#endif

	_vc_core_util_strcpy(pcallobject_info->calling_name, sizeof(pcallobject_info->calling_name), callInfo->CallingNameInfo.szNameData);
	pcallobject_info->bcalling_namemode = callInfo->CallingNameInfo.NameMode;

	/* If BCD number exists, number should be shown even though number restriction is set */
	if (strlen(callInfo->szCallingPartyNumber) > 0) {
		pcallobject_info->brestricted_namemode = FALSE;

		if (callInfo->szCallingPartyNumber[0] == '*') {
			pcallobject_info->bccbs_call = TRUE;
			_vc_core_util_strcpy(pcallobject_info->tel_number, sizeof(pcallobject_info->tel_number), callInfo->szCallingPartyNumber + 1);
		} else {
			_vc_core_util_strcpy(pcallobject_info->tel_number, sizeof(pcallobject_info->tel_number), callInfo->szCallingPartyNumber);
		}

		/* check callInfo->name_mode if override category isn't supported.*/
	} else {		/* If BCD number doesn't exist, cause_of_no_cli value should be checked to decide its presentation */

		pcallobject_info->brestricted_namemode = TRUE;

/*code clean: #ifndef _ARM_SP*/
		pcallobject_info->name_mode = callInfo->CliCause;

		CALL_ENG_DEBUG(ENG_DEBUG, "no_cli_cause = %d, name_mode = %d...", callInfo->CliCause, pcallobject_info->name_mode);
	}
}

/**
 * This function handles the incoming event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		Incoming call handle
 * @param[in]		callInfo			Incoming call info associated with the incoming call
 */
gboolean _vc_core_tapi_event_handle_incoming_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallIncomingCallInfo_t *callInfo)
{
	call_vc_call_objectinfo_t callobject_info;
	int callIndex;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d, number = %s..", call_handle, callInfo->szCallingPartyNumber);

	/* setting the new member info */
	_vc_core_cm_clear_call_object(&callobject_info);

	callobject_info.call_handle = call_handle;
	callobject_info.call_id = _vc_core_cm_get_new_callId(&pcall_agent->call_manager);
	callobject_info.bincoming_call_is_fwded = callInfo->fwded;

	/* setting the incom call telephone number */
	CALL_ENG_DEBUG(ENG_DEBUG, "[callobject_info.call_handle=%d], FWDED Call: %d", callobject_info.call_handle, callobject_info.bincoming_call_is_fwded);

	/*Copy Incoming call data in to the callobject */
	_vc_core_tapi_event_copy_incoming_call_data(pcall_agent, callInfo, &callobject_info);

	/*Change the Call Object Call State*/
	_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_INCOME);

	/*Add the newly created callobject to the Call Manager*/
	callIndex = _vc_core_cm_add_call_object(&pcall_agent->call_manager, &callobject_info);
	if (callIndex != -1) {
		/*Set the Call Manager's MT Call Index as CallObject Index*/
		if (FALSE == _vc_core_cm_set_incoming_call(&pcall_agent->call_manager, callIndex)) {
			CALL_ENG_DEBUG(ENG_ERR, "Problem in accpeting the incoming call, Current Call Details");
			CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);
			return FALSE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "Problem in adding the call to the call manager");
		CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);
	}

	_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_START);

	return TRUE;
}

/**
 * This function handles TAPI alert event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		Call Handle associated with the alert event
 */
gboolean _vc_core_tapi_event_handle_alert_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	/* Verify call handle */
	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d", call_handle);

	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_ERR, "INVALID_CALLHANDLE Error");
		return FALSE;
	}

	_vc_core_cm_change_call_object_state(&pcall_agent->call_manager, call_handle, VC_CALL_STATE_OUTGOING_ALERT);

	/* Change Inout state to "wait connected" before sending event to client so the same state is
	   reflected in client side also */
	_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED);

	/*Send Alert Event to Client*/
	_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_OUTGOING_ALERT, call_handle, 0, NULL);
	return TRUE;
}

/**
 * This function handles TAPI origination event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		Call Handle associated with the alert event
 */
gboolean _vc_core_tapi_event_handle_originated_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(call_handle > 0);

	/* Verify call handle */
	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d", call_handle);

	_vc_core_cm_change_call_object_state(&pcall_agent->call_manager, call_handle, VC_CALL_STATE_OUTGOING_ORIG);

	/* Send Alert Event to Client */
	_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_OUTGOING_ORIG, call_handle, 0, NULL);

	/* Change Inout state to "wait Alert" */
	_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_OUTGOING_WAIT_ALERT);

	/* send response here.. not connected ind. Disable checking gcf status. */
#ifndef _vc_core_ca_send_sat_response_ORIG
	if (TRUE == _vc_core_util_check_gcf_status())
#endif
	{
		call_vc_call_objectinfo_t callobject_info;

		_vc_core_cm_clear_call_object(&callobject_info);

		/*Process Connected Event*/
		/* Get the member info and chage info */
		_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info);

		if (callobject_info.call_type == VC_CALL_ORIG_TYPE_SAT) {
			_vc_core_ca_send_sat_response(pcall_agent, SAT_RQST_SETUP_CALL, CALL_VC_ME_RET_SUCCESS);
		}
	}

	return TRUE;
}

/**
 * This function handles the call end event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		type				TAPI Event Type
 * @param[in]		handle			Call Handle of the call being ended
 * @param[in]		cause			TAPI End Cause
 */
gboolean _vc_core_tapi_event_handle_call_end_event(call_vc_callagent_state_t *pcall_agent, const char * noti_id, call_vc_handle handle, TelTapiEndCause_t cause)
{
	gboolean active_call = FALSE;
	gboolean held_call = FALSE;
	int error_code = 0;
	call_vc_call_objectinfo_t callobject_info;
	voicecall_call_state_t prev_callstate;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	_vc_core_cm_get_call_object(&pcall_agent->call_manager, handle, &callobject_info);

	prev_callstate = callobject_info.state;

	_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_ENDED);

	/* set end cause text*/
	_vc_core_tapi_event_get_end_cause_type(pcall_agent, noti_id, cause, &callobject_info.end_cause_type);

	/*Set the modified CallObject to the Call Manager*/
	_vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);

	/*All calls are disconnected, so stop call timer*/
	if (_vc_core_cm_isexists_connected_call(&pcall_agent->call_manager) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Connected Call Exist");
	}

	active_call = _vc_core_cm_isexists_active_call(&pcall_agent->call_manager);
	held_call = _vc_core_cm_isexists_held_call(&pcall_agent->call_manager);

	switch (pcall_agent->callagent_state) {
	case CALL_VC_CA_STATE_WAIT_JOIN:
		if ((FALSE == active_call) || (FALSE == held_call)) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		}
		break;
	case CALL_VC_CA_STATE_WAIT_HOLD:
		if (FALSE == active_call) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		}
		break;
	case CALL_VC_CA_STATE_WAIT_UNHOLD:
		if (FALSE == held_call) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		}
		break;
	case CALL_VC_CA_STATE_WAIT_SWAP:
		if ((FALSE == active_call) || (FALSE == held_call)) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		}
		break;
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_ACTIVECALL:
#ifdef AUTO_RETREIVE_HELD_CALL
		if (FALSE == active_call) {
			/* todo call: IF Ear MIC is Inserted && */
			if ((TRUE == held_call) && (pcall_agent->io_state == VC_INOUT_STATE_NONE) && \
					(TRUE == _vc_core_tapi_rqst_retrieve_call(pcall_agent))) {
				_vc_core_cm_clear_endcall_member(&pcall_agent->call_manager);

				_vc_core_ca_send_event_to_client(pcall_agent, VC_ACTION_CALL_END_HELD_RETREIVED, handle, 0, NULL);

				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_UNHOLD);
			} else {
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
			}
		}
#else
		if (FALSE == active_call) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "Still Active Calls are available");
		}
#endif
		break;
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_HOLDCALL:
		if (FALSE == held_call) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		}
		break;
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS:
		if ((active_call == FALSE) && (held_call == FALSE)) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		}
		break;
	case CALL_VC_CA_STATE_SS_WAIT_RELEASE_ALL_ACTIVECALL:
		CALL_ENG_DEBUG(ENG_DEBUG, "Retrieve held call on active call end");
		if (FALSE == active_call) {
			if ((TRUE == held_call) && (TRUE == _vc_core_tapi_rqst_retrieve_call(pcall_agent))) {
				_vc_core_cm_clear_endcall_member(&pcall_agent->call_manager);
				_vc_core_ca_send_event_to_client(pcall_agent, VC_ACTION_CALL_END_HELD_RETREIVED, handle, 0, NULL);
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_UNHOLD);
			} else {
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
			}
		}
		break;
	case CALL_VC_CA_STATE_WAIT_DROP:
		if (VC_CALL_STATE_RELEASE_WAIT == prev_callstate) {
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		}

		break;

	case CALL_VC_CA_STATE_SPLIT_CALLBOX:
	case CALL_VC_CA_STATE_DROP_CALLBOX:
	case CALL_VC_CA_STATE_SENDMSG_CALLBOX:
	case CALL_VC_CA_STATE_SAVE_TO_CONTACT_CALLBOX:
	case CALL_VC_CA_STATE_VIEW_CONTACT_DETAIL_CALLBOX:
		_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
		break;
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SWITCH_TO_VIDEO_CALL:
		if ((FALSE == active_call) && (FALSE == held_call)) {
			char tel_number[VC_PHONE_NUMBER_LENGTH_MAX];

			memset(tel_number, 0, VC_PHONE_NUMBER_LENGTH_MAX);
		}
		break;
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SETUP:
		{
			if ((active_call == FALSE) && (held_call == FALSE)) {
				call_vc_call_objectinfo_t callInfo;
				call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;

				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				if (_vc_core_tapi_rqst_setup_call(pcall_agent) == FALSE) {
					if (_vc_core_cm_get_outgoing_call_info(&pcall_agent->call_manager, &callInfo)) {
						/*Send response only if call type is sat */
						if (callInfo.call_type == VC_CALL_ORIG_TYPE_SAT) {
							CALL_ENG_DEBUG(ENG_DEBUG, "SAT_CALL fail to setup call");
							_vc_core_ca_send_sat_response(pcall_agent, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
						}
					}
					break;
				}

				/*Update CallManager State */
				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_OUTGOING_WAIT_ORIG);
				call_handle = _vc_core_cm_get_outgoing_call_handle(&pcall_agent->call_manager);
				CALL_ENG_DEBUG(ENG_DEBUG, "Deffered Outgoing Call Handle = %d", call_handle);

				/*Inform Client about the Deferred Outgoing */
				_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_OUTGOING, call_handle, 0, NULL);
			}
		}
		break;
	case CALL_VC_CA_STATE_WAIT_TRANSFER_CNF:
		{
			_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
			CALL_ENG_DEBUG(ENG_DEBUG, "Waiting transfer canceled!");
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Action not defined for this call agent state:%d", pcall_agent->callagent_state);
	}

	/* if waiting call end for mtc */
	switch (pcall_agent->io_state) {
	case VC_INOUT_STATE_INCOME_WAIT_HOLD:

		if (active_call == FALSE) {
			CALL_ENG_DEBUG(ENG_DEBUG, "call to hold is disconnected..proceed to answer call");
			/*It will be better to show end of call
			_vc_core_tapi_rqst_answer_call( pcall_agent);*/
		}

		break;

	case VC_INOUT_STATE_INCOME_WAIT_RELEASE_ACTIVECALL:
		if (active_call == FALSE) {
			/*_vc_core_tapi_rqst_answer_call( pcall_agent);*/
		}
		break;
	case VC_INOUT_STATE_INCOME_WAIT_RELEASE_HOLDCALL:
		if (held_call == FALSE) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Answering call after ending held calls");

			/*Change the state to Income box, so the answer API will process it */
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_BOX);

			/* end held call and then accept incoming call */
			_vc_core_tapi_rqst_answer_call(pcall_agent, VC_ANSWER_HOLD_ACTIVE_AND_ACCEPT, &error_code);
		}
		break;
#ifdef RELEASE_ALL_AND_ACCEPT_SUPPORT
	case VC_INOUT_STATE_INCOME_WAIT_RELEASE_HOLDCALL_FOR_ALL_RELEASE:
		if (held_call == FALSE) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Answering(rel and accept) call after ending held calls");

			/*Change the state to Income box, so the answer API will process it */
			_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_BOX);

			/* end held call and then accept incoming call */
			_vc_core_tapi_rqst_answer_call(pcall_agent, VC_ANSWER_RELEASE_ACTIVE_AND_ACCEPT, &error_code);
		}
		break;
#endif
	case VC_INOUT_STATE_OUTGOING_WAIT_HOLD:
		_vc_core_cm_clear_outgoing_call(&pcall_agent->call_manager);
		_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_OUTGOING_ABORTED);
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "No Actions defined for IO State: %d", pcall_agent->io_state);
	}

	if (VC_CALL_ORIG_TYPE_SAT == callobject_info.call_type) {
		if (pcall_agent->call_manager.setupcall_info.satcall_setup_info.psat_rgb_data != NULL) {
			free(pcall_agent->call_manager.setupcall_info.satcall_setup_info.psat_rgb_data);
			pcall_agent->call_manager.setupcall_info.satcall_setup_info.psat_rgb_data = NULL;
		}
	}

	return TRUE;
}

static gboolean __call_vc_process_wait_state_success_events(call_vc_callagent_state_t *pcall_agent)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	switch (pcall_agent->io_state) {
	case VC_INOUT_STATE_INCOME_WAIT_HOLD_CONNECTED:
		{
			int grp_index;
			call_vc_handle active_handle = VC_TAPI_INVALID_CALLHANDLE;

			if (pcall_agent->callagent_state == CALL_VC_CA_STATE_WAIT_HOLD) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Separate HOLD_CNF event not arrived for HOLD and ACCEPT request, holding the call in the CONNECTED event");
				grp_index = _vc_core_cm_get_active_group_index(&pcall_agent->call_manager);

				if (-1 == grp_index) {
					CALL_ENG_DEBUG(ENG_DEBUG, " ERROR:No Active Grp Index :grp_index = %d, Active state has been already held, check io state", grp_index);
					return TRUE;
				}

				_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &active_handle);

				_vc_core_cm_set_group_state(&pcall_agent->call_manager, grp_index, CALL_VC_GROUP_STATE_HOLD);

				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);

				_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_HELD, active_handle, 0, NULL);

				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_CONNECTED);
			}
		}
		break;

#ifdef _INCOME_WAIT_CONNECTED_
	/*currently not applied, Fix to change the group state to hold, if the second call is connected before getting the holdcnf resp from tapi */
	case VC_INOUT_STATE_INCOME_WAIT_CONNECTED:
		{
			/*Check for incoming call exists */
			if (VC_INVALID_CALL_INDEX == pcall_agent->call_manager.mtcall_index) {
				CALL_ENG_DEBUG(ENG_DEBUG, "No Incoming call exists");
				return TRUE;
			}

			if (0 == _vc_core_cm_get_connected_member_count_ingroup(&pcall_agent->call_manager, 0)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "No Previous Connected Members");
				return TRUE;
			}

			/*Connected Event for the incoming call is arrived, when waiting for hold confirmation
			   So change the active call state to hold before processing the connected indication */
			if (CALL_VC_CA_STATE_WAIT_HOLD == pcall_agent->callagent_state) {
				int grp_index;
				/*call_vc_handle active_handle =TAPI_INVALID_CALLHANDLE;*/
				grp_index = _vc_core_cm_get_active_group_index(&pcall_agent->call_manager);

				if (-1 == grp_index) {
					CALL_ENG_DEBUG(ENG_DEBUG, " ERROR:No Active Grp Index :grp_index = %d, Active state has been already held, check io state", grp_index);
					return TRUE;
				}
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				/*_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager,&active_handle);*/
				_vc_core_cm_set_group_state(&pcall_agent->call_manager, grp_index, CALL_VC_GROUP_STATE_HOLD);
			} else if (CALL_VC_CA_STATE_WAIT_UNHOLD == pcall_agent->callagent_state) {
				int grp_index;
				call_vc_handle held_handle = VC_TAPI_INVALID_CALLHANDLE;
				grp_index = _vc_core_cm_get_held_group_index(&pcall_agent->call_manager);
				if (-1 == grp_index) {
					CALL_ENG_DEBUG(ENG_DEBUG, " ERROR:No Active Grp Index :grp_index = %d, Active state has been already held, check io state", grp_index);
					return TRUE;
				}
				_vc_core_cm_get_first_held_call_handle(&pcall_agent->call_manager, &held_handle);
				_vc_core_cm_set_group_state(&pcall_agent->call_manager, grp_index, CALL_VC_GROUP_STATE_ACTIVE);

			}
		}
		break;
#endif
	case VC_INOUT_STATE_INCOME_WAIT_RELEASE_ACTIVE_CONNECTED:
		{
			call_vc_handle active_handle = VC_TAPI_INVALID_CALLHANDLE;;
			CALL_ENG_DEBUG(ENG_DEBUG, "VC_INOUT_STATE_INCOME_WAIT_RELEASE_ACTIVE_CONNECTED");
			_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &active_handle);
			CALL_ENG_DEBUG(ENG_DEBUG, "Active Call Handle = %d", active_handle);

			/*_vc_core_tapi_event_handle_call_end_event(pcall_agent, TAPI_EVENT_CALL_END, active_handle, TAPI_CALL_END_NO_CAUSE);*/
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Action not defined for this wait io state: %d", pcall_agent->io_state);
	}

	return TRUE;
}

/**
 * This function handles the TAPI call connect event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle assoicated with the call being connected
 */
gboolean _vc_core_tapi_event_handle_call_connect_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle)
{
	call_vc_call_objectinfo_t callobject_info;
	int index;
	int connectednum = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	/* Verify call handle */
	CALL_ENG_DEBUG(ENG_DEBUG, "Started, Call Handle = %d", call_handle);

	_vc_core_cm_clear_call_object(&callobject_info);
	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_ERR, "INVALID_CALLHANDLE Error");
		return FALSE;
	}

	/*First Process the wait state events before processing the connect event */
	CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);
	__call_vc_process_wait_state_success_events(pcall_agent);
	CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);

	/*Process Connected Event*/
	/* Get the member info and chage info */
	_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info);

	/*
	 **     Reqeust from SAT. When GCF field is ON, send response when get a setup confirm.
	 **     To send response to SAT faster...
	 */
#ifndef _vc_core_ca_send_sat_response_ORIG
	if (FALSE == _vc_core_util_check_gcf_status()) {
		if (callobject_info.call_type == VC_CALL_ORIG_TYPE_SAT) {
			_vc_core_ca_send_sat_response(pcall_agent, SAT_RQST_SETUP_CALL, CALL_VC_ME_RET_SUCCESS);
		}
	}
#endif

	_vc_core_cm_change_call_state(&callobject_info, VC_CALL_STATE_CONNECTED);

	VOICECALL_RETURN_FALSE_IF_FAIL(callobject_info.call_id != VC_INVALID_CALL_ID);

	index = _vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);
	VOICECALL_RETURN_FALSE_IF_FAIL(index != -1);

	connectednum = _vc_core_cm_get_connected_member_count_ingroup(&pcall_agent->call_manager, 0) + _vc_core_cm_get_connected_member_count_ingroup(&pcall_agent->call_manager, 1);
	CALL_ENG_DEBUG(ENG_DEBUG, "Connected Member Num before adding Connected call to group is :%d", connectednum);

	/*If any previous calls are not cleared after end, clear it here so makes provision to add one more call to the call manager */
	_vc_core_cm_clear_endcall_member(&pcall_agent->call_manager);

	/*Add the conneted call to the active group */
	if (FALSE == _vc_core_cm_add_call_member_togroup(&pcall_agent->call_manager, index)) {
		CALL_ENG_DEBUG(ENG_ERR, "Call Object not added to the Group, [PROBLEM] !!!");
	}

	CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);

	/* Send Connected Event to the Client */
	_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_CONNECTED, callobject_info.call_handle, 0, NULL);

	/* Once the Call is connected, change the InOut state to None */
	_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_NONE);

	/*Getting AOC PPM Value is delayed to make the current flow faster by adding it to g_idle_add */
	g_idle_add(__call_vc_get_aoc_ppm_value_idle_cb, pcall_agent);

	if (_vc_core_engine_status_get_download_call(pcall_agent) == TRUE) {
		g_timeout_add(9000, __call_vc_download_call_timer_cb, pcall_agent);

		_vc_core_util_download_test_call("downloadcall_success");
	}

	return TRUE;
}

/**
 * This function handles call hold event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle assoicated with the call being held
 * @param[in]		status			TAPI cause incase of hold failed
 */
gboolean _vc_core_tapi_event_handle_call_held_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallCause_t status)
{
	call_vc_call_objectinfo_t callobject_info;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);

	/* Verify call handle */
	CALL_ENG_DEBUG(ENG_DEBUG, "Started, Call Handle = %d, status = %d", call_handle, status);

	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "INVALID_CALLHANDLE Error");
		return FALSE;
	}

	if (TAPI_CAUSE_SUCCESS != status) {
		switch (pcall_agent->io_state) {
		case VC_INOUT_STATE_OUTGOING_WAIT_HOLD:
			{
				/* Recover and reset related state variable */
				if (_vc_core_cm_get_outgoing_call_info(&pcall_agent->call_manager, &callobject_info) == FALSE) {
					CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call info does not exist");
					assert(0);
				}

				if ((callobject_info.call_type == VC_CALL_ORIG_TYPE_SAT) && \
						((pcall_agent->call_manager.setupcall_info.satcall_setup_info.satengine_setupcall_data.calltype == \
								TAPI_SAT_SETUP_CALL_PUT_ALL_OTHER_CALLS_ON_HOLD) || \
								(pcall_agent->call_manager.setupcall_info.satcall_setup_info.satengine_setupcall_data.calltype == \
										TAPI_SAT_SETUP_CALL_PUT_ALL_OTHER_CALLS_ON_HOLD_WITH_REDIAL))) {

					/* Terminal Response for Option B */
					/* _vc_core_ca_send_sat_response(pcall_agent, SAT_RQST_SETUP_CALL, CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND); */
					/* Terminal Response for Option A */
					_vc_core_ca_send_sat_response(pcall_agent, SAT_RQST_SETUP_CALL, CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND_WITHOUT_CAUSE);

				}
				_vc_core_cm_clear_outgoing_call(&pcall_agent->call_manager);

				_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_OUTGOING_ABORTED, VC_ENDCAUSE_CALL_ENDED, 0, NULL);

				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_NONE);
			}
			break;

		case VC_INOUT_STATE_INCOME_WAIT_HOLD:
			break;

		case VC_INOUT_STATE_NONE:
			break;

		default:
			CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_tapi_event_handle_call_held_event(fail): Not allowed io_state=%d", pcall_agent->io_state);
			break;
		}

		switch (pcall_agent->callagent_state) {
#ifdef SWAP_SUPPORT
		case CALL_VC_CA_STATE_WAIT_SWAP_HOLD_OR_ACTIVATE:
			{
				/* Reset the Agent State */
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_SWAP_FAILED, 0, NULL);
			}
			break;
#endif
		case CALL_VC_CA_STATE_WAIT_SWAP:
			{
				/* Reset the Agent State */
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_SWAP_FAILED, 0, NULL);
			}
			break;
		case CALL_VC_CA_STATE_WAIT_HOLD:
			{
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_HOLD_FAILED, 0, NULL);
			}
			break;
		case CALL_VC_CA_STATE_SS_WAIT_RELEASE_ALL_ACTIVECALL:
			{
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
			}
			break;
		case CALL_VC_CA_STATE_NORMAL:
		default:	/*Fall Through */
			CALL_ENG_DEBUG(ENG_DEBUG, "Not allowed callagent_state=%d", pcall_agent->callagent_state);
			return FALSE;
			break;
		}
	} else {
		int index;
		int grp_state = 0;

		/* Get the Group Index */
		index = _vc_core_cm_get_group_index(&pcall_agent->call_manager, call_handle);
		grp_state = _vc_core_cm_get_group_state(&pcall_agent->call_manager, index);

		CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d, GP_Index:%d, Cur Gp State: %d", call_handle, index, grp_state);

		switch (pcall_agent->io_state) {
		case VC_INOUT_STATE_OUTGOING_WAIT_HOLD:
			{
				/* Change the Group State */
				_vc_core_cm_set_group_state(&pcall_agent->call_manager, index, CALL_VC_GROUP_STATE_HOLD);
				CALL_ENG_DEBUG(ENG_DEBUG, "Gropu Index: %d , set to GROUP_STATE_HOLD", index);

				/* Send Call Held Event to the Client */
				_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_HELD, call_handle, 0, NULL);

				if (_vc_core_tapi_rqst_setup_call(pcall_agent) == FALSE) {
					_vc_core_cm_clear_outgoing_call(&pcall_agent->call_manager);
					_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_OUTGOING_ABORTED);
				} else {
					call_vc_handle outgoing_call_handle = VC_TAPI_INVALID_CALLHANDLE;

					_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_OUTGOING_WAIT_ORIG);
					outgoing_call_handle = _vc_core_cm_get_outgoing_call_handle(&pcall_agent->call_manager);
					CALL_ENG_DEBUG(ENG_DEBUG, "Deffered Outgoing Call Handle = %d", outgoing_call_handle);
					_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_OUTGOING, outgoing_call_handle, 0, NULL);
				}

				if (pcall_agent->callagent_state == CALL_VC_CA_STATE_WAIT_HOLD)
					_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				return TRUE;
			}
			break;
		case VC_INOUT_STATE_INCOME_WAIT_HOLD:
			{
				int error_code;

				/*Change the Group State*/
				_vc_core_cm_set_group_state(&pcall_agent->call_manager, index, CALL_VC_GROUP_STATE_HOLD);
				CALL_ENG_DEBUG(ENG_DEBUG, "Gropu Index: %d , set to GROUP_STATE_HOLD", index);

				if (_vc_core_tapi_rqst_answer_call(pcall_agent, VC_ANSWER_NORMAL, &error_code)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "Voicecall Answered");
					/*TODO: Inform client to update ui */
				}
				if (pcall_agent->callagent_state == CALL_VC_CA_STATE_WAIT_HOLD) {
					_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				}
			}
			break;
		case VC_INOUT_STATE_INCOME_WAIT_HOLD_CONNECTED:
			{
				CALL_ENG_DEBUG(ENG_DEBUG, "Hold arrived for Hold and Accept Event, Current IO State:%d", pcall_agent->io_state);
				/*Change state to wait_connected */
				_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_WAIT_CONNECTED);
			}
			break;
		case VC_INOUT_STATE_NONE:
		default:	/*Fall Through */
			CALL_ENG_DEBUG(ENG_DEBUG, "Not allowed io_state=%d", pcall_agent->io_state);
			break;
		}

		switch (pcall_agent->callagent_state) {
#ifdef SWAP_SUPPORT
		case CALL_VC_CA_STATE_WAIT_SWAP_HOLD_OR_ACTIVATE:
			{
				int held_call_num = 0;
				int act_call_num = 0;

				held_call_num = _vc_core_cm_get_held_call_count(&pcall_agent->call_manager);
				act_call_num = _vc_core_cm_get_active_call_count(&pcall_agent->call_manager);
				if ((held_call_num > 0 && act_call_num <= 0) || (act_call_num > 0 && held_call_num <= 0)) {
					if (CALL_VC_GROUP_STATE_HOLD != grp_state) {
						/* Set the State to HOLD and inform client */
						_vc_core_cm_set_group_state(&pcall_agent->call_manager, index, CALL_VC_GROUP_STATE_HOLD);
						_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_HELD, 0, 0, NULL);
					}
					_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				} else {
					_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_SWAP);
				}
			}
			break;
#endif
		case CALL_VC_CA_STATE_WAIT_SWAP:
			{
				/*Always reset the agent state as the event for this wait state is arrived */
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);

				/*Swap the state only if the group state of the given call handle is not in hold state already
				   as this is a hold confirmation event */
				if (CALL_VC_GROUP_STATE_HOLD != grp_state) {
					_vc_core_cm_swap_group_state(&pcall_agent->call_manager);
					_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_SWAP, call_handle, 0, NULL);
				}
			}
			break;
		case CALL_VC_CA_STATE_WAIT_HOLD:
			{
				/* Change the Group State */
				_vc_core_cm_set_group_state(&pcall_agent->call_manager, index, CALL_VC_GROUP_STATE_HOLD);
				CALL_ENG_DEBUG(ENG_DEBUG, "Group Index: %d , set to GROUP_STATE_HOLD", index);

				/* Change Call Agent State */
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);

				_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_HELD, call_handle, 0, NULL);
			}
			break;
		case CALL_VC_CA_STATE_SS_WAIT_RELEASE_ALL_ACTIVECALL:
			{
				gboolean active_call = _vc_core_cm_isexists_active_call(&pcall_agent->call_manager);
				gboolean held_call = _vc_core_cm_isexists_held_call(&pcall_agent->call_manager);

				/*Upon waiting for  the success event for hold request,
				   Other calls might have been released during this time, so the held call need to retrieved */
				if ((active_call == FALSE) && (held_call == TRUE)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "Unhold call");
					if (_vc_core_tapi_rqst_retrieve_call(pcall_agent) == TRUE) {
						_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_UNHOLD);
					}
				}
			}
			break;
		case CALL_VC_CA_STATE_NORMAL:
		default:	/*Fall Through */
			CALL_ENG_DEBUG(ENG_DEBUG, "Not allowed callagent_state=%d", pcall_agent->callagent_state);
			return FALSE;
			break;
		}
	}

	return TRUE;
}

/**
* This function handles TAPI call activate/retrieve event
*
* @return		Returns TRUE on success and FALSE on failure
* @param[in]		pcall_agent		Pointer to the call agent state
* @param[in]		call_handle		call handle associated with the call being retrieved
* @param[in]		status			TAPI cause in case of retrieve failed
*/
gboolean _vc_core_tapi_event_handle_call_retrieve_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallCause_t status)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d, status = %d", call_handle, status);

	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "INVALID_CALLHANDLE Error");
		return FALSE;
	}

	if (TAPI_CAUSE_SUCCESS != status) {
		switch (pcall_agent->callagent_state) {
#ifdef SWAP_SUPPORT
		case CALL_VC_CA_STATE_WAIT_SWAP_HOLD_OR_ACTIVATE:
			{
				/*Reset the Agent State*/
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_SWAP_FAILED, 0, NULL);
			}
			break;
#endif
		case CALL_VC_CA_STATE_WAIT_SWAP:
			{
				/*Reset the Call Agent State*/
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_SWAP_FAILED, 0, NULL);
			}
			break;
		case CALL_VC_CA_STATE_WAIT_UNHOLD:
			{
				/*Reset the Agent State*/
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_RETREIVE_FAILED, 0, NULL);
			}
			break;
		default:
			CALL_ENG_DEBUG(ENG_DEBUG, "Not allowed callagent_state=%d", pcall_agent->callagent_state);
			return FALSE;
			break;
		}

	} else {
		int index;
		int cur_grp_state = 0;

		/*Get the Group Index and current group status */
		index = _vc_core_cm_get_group_index(&pcall_agent->call_manager, call_handle);
		cur_grp_state = _vc_core_cm_get_group_state(&pcall_agent->call_manager, index);

		CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d, GP_Index:%d, Cur Gp State: %d", call_handle, index, cur_grp_state);

		switch (pcall_agent->callagent_state) {
#ifdef SWAP_SUPPORT
		case CALL_VC_CA_STATE_WAIT_SWAP_HOLD_OR_ACTIVATE:
			{
				int held_call_num = 0;
				int act_call_num = 0;

				held_call_num = _vc_core_cm_get_held_call_count(&pcall_agent->call_manager);
				act_call_num = _vc_core_cm_get_active_call_count(&pcall_agent->call_manager);
				if ((held_call_num > 0 && act_call_num <= 0) || (act_call_num > 0 && held_call_num <= 0)) {
					if (CALL_VC_GROUP_STATE_ACTIVE != cur_grp_state) {
						/* Set  the State ACTIVE and inform client */
						_vc_core_cm_set_group_state(&pcall_agent->call_manager, index, CALL_VC_GROUP_STATE_ACTIVE);
						_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_RETREIVED, 0, 0, NULL);
					}
					_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);
				} else {
					_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_WAIT_SWAP);
				}
			}
			break;
#endif
		case CALL_VC_CA_STATE_WAIT_SWAP:
			{
				/* Always reset the agent state as the event for the wait state is arrived */
				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);

				/*Change the state only when the current state of the given handle is not in ACTIVE state */
				if (CALL_VC_GROUP_STATE_ACTIVE != cur_grp_state) {
					_vc_core_cm_swap_group_state(&pcall_agent->call_manager);
					_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_SWAP, call_handle, 0, NULL);
				}
			}
			break;
		case CALL_VC_CA_STATE_WAIT_UNHOLD:
			{
				int index;
				/* get the member info and chage info */
				index = _vc_core_cm_get_group_index(&pcall_agent->call_manager, call_handle);
				_vc_core_cm_set_group_state(&pcall_agent->call_manager, index, CALL_VC_GROUP_STATE_ACTIVE);

				_vc_core_ca_change_agent_state(pcall_agent, CALL_VC_CA_STATE_NORMAL);

				/*Send Call Retreived Event to Client*/
				_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_RETREIVED, call_handle, 0, NULL);
			}
			break;
		default:
			CALL_ENG_DEBUG(ENG_DEBUG, "Not allowed callagent_state=%d", pcall_agent->callagent_state);
			/*return FALSE if the event is not handled here, because end call memeber will be unnecessarily cleared if returned TRUE */
			return FALSE;
			break;
		}

	}

	return TRUE;
}

/**
 * This function handles call join/conference event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle assoicated with the call being joined
 * @param[in]		status			tapi cause incase of join failed
 */
gboolean _vc_core_tapi_event_handle_call_join_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallCause_t status)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d, status = %d", call_handle, status);

	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "INVALID_CALLHANDLE Error");
		return FALSE;
	}

	if (TAPI_CAUSE_SUCCESS != status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Join fail and return..");
		_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_SETUP_CONF_FAILED, 0, NULL);
		return TRUE;
	}

	CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);

	/* get the member info and chage info */
	_vc_core_cm_join_group(&pcall_agent->call_manager);

	CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);

	_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_SETUP_CONF, 0, 0, NULL);
	return TRUE;
}

/**
 * This function handles call split/private call event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		call handle associated with the call being made as private call
 * @param[in]		status			TAPI cause in case of split failed
 */
gboolean _vc_core_tapi_event_handle_call_split_event(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallCause_t status)
{
	call_vc_call_objectinfo_t callobject_info;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d, status = %d", call_handle, status);

	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "INVALID_CALLHANDLE Error");
		_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_SPLIT_CONF_FAILED, 0, NULL);
		return FALSE;
	}

	if (TAPI_CAUSE_SUCCESS != status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Split fail and return..");
		_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, ERROR_VOICECALL_SPLIT_CONF_FAILED, 0, NULL);
		return FALSE;
	}

	/* get the member info and chage info */
	_vc_core_cm_clear_call_object(&callobject_info);
	_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info);

	_vc_core_cm_split_group(&pcall_agent->call_manager, call_handle);

	_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_SPLIT_CONF, call_handle, 0, NULL);	/* Added Call handle */
	return TRUE;
}

/**
* This function handles the call transfer event
*
* @return		Returns TRUE on success and FALSE on failure
* @param[in]		pcall_agent		Pointer to the call agent state
* @param[in]		status			TAPI cause in case of hold failed
*/
gboolean _vc_core_tapi_event_handle_call_transfer_event(call_vc_callagent_state_t *pcall_agent, TelCallCause_t status)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Started, status = %d", status);

	if (TAPI_CAUSE_SUCCESS != status) {
		int err_cause = 0;
		CALL_ENG_DEBUG(ENG_DEBUG, "TapiTransfer Failed");
		if (VC_TAPI_INVALID_CALLHANDLE == _vc_core_cm_get_incoming_call_handle(&pcall_agent->call_manager)
		    && VC_TAPI_INVALID_CALLHANDLE == _vc_core_cm_get_outgoing_call_handle(&pcall_agent->call_manager)) {
			err_cause = ERROR_VOICECALL_TRANSFER_FAILED;

		} else {
			if (_vc_core_cm_isexists_incoming_call(&pcall_agent->call_manager)) {
				err_cause = ERROR_VOICECALL_TRANSFER_FAILED;
			} else if (VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED == pcall_agent->io_state) {
				err_cause = ERROR_VOICECALL_TRANSFER_FAILED;
			}
		}

		_vc_core_ca_send_event_to_client(pcall_agent, VC_ERROR_OCCURED, err_cause, 0, NULL);
		CALL_ENG_DEBUG(ENG_DEBUG, "Transfer failed and return..");

		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Transfer success!");
		_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_TRANSFERRED, 0, 0, NULL);
	}
	/*todo ss: Check Updating flag gbpsh_voicecall_command_transfer*/
	return TRUE;
}

/**
* This function handles the TAPI connected line indication handle
*
* @return		Returns TRUE on success and FALSE on failure
* @param[in]		pcall_agent				Pointer to the call agent state
* @param[in]		call_handle				TAPI Call Handle associated with connected line indication
* @param[in]		connected_number_info	Connected Number Details
*/
gboolean _vc_core_tapi_event_connected_line_ind_handle(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallConnectedNumberInfo_t *connected_number_info)
{
	call_vc_call_objectinfo_t callobject_info;
	call_vc_handle active_call_handle = VC_TAPI_INVALID_CALLHANDLE;
	gboolean bConnectedCall = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, ":Number(%s)", connected_number_info->number);

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_ERR, " INVALID_CALLHANDLE Error");
		return FALSE;
	}

	_vc_core_cm_clear_call_object(&callobject_info);

	if (_vc_core_cm_get_outgoing_call_info(&pcall_agent->call_manager, &callobject_info)) {
		if (callobject_info.call_handle != call_handle) {
			CALL_ENG_DEBUG(ENG_ERR, "It is not outging call(Call Handle = %d, MO Call Handle =%d)", call_handle, callobject_info.call_handle);
			return FALSE;
		}
	} else {
		if (_vc_core_cm_get_first_active_call_handle(&pcall_agent->call_manager, &active_call_handle) == -1) {
			CALL_ENG_DEBUG(ENG_ERR, "No Active Calls(Call Handle = %d)", call_handle);
			return FALSE;
		}

		if (active_call_handle != call_handle) {
			CALL_ENG_DEBUG(ENG_ERR, "Call Handle Mismatch(Call Handle = %d)", call_handle);
			return FALSE;
		}

		_vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info);
		bConnectedCall = TRUE;
	}

	switch (connected_number_info->name_mode) {
	case TAPI_CALL_NAME_AVAIL:	/*if sznumber of the callInfo is null, set Anoymous*/

		if (0 == strlen(connected_number_info->number)) {
			/*Forwarded Call number is unknown*/
			memset(callobject_info.connected_telnumber, 0, sizeof(callobject_info.connected_telnumber));
		} else {
			_vc_core_util_strcpy(callobject_info.connected_telnumber, sizeof(callobject_info.connected_telnumber), connected_number_info->number);
		}

		break;

	case TAPI_CALL_NAME_RESTRICTED:
	case TAPI_CALL_NAME_AVAIL_RESTRICTED:	/*withheld*/
		/*Forwarded Call number is unknown*/
		memset(callobject_info.connected_telnumber, 0, sizeof(callobject_info.connected_telnumber));
		break;

	case TAPI_CALL_NAME_UNAVAIL:	/*Anoymous*/
	default:
		/*Forwarded Call number is unknown*/
		memset(callobject_info.connected_telnumber, 0, sizeof(callobject_info.connected_telnumber));
		break;
	}

	/*Set the modified call object to call manager*/
	_vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);

	/*Send Event to the Client*/
	_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_SS_CONNECT_LINE_IND, callobject_info.call_handle, 0, (void *)callobject_info.connected_telnumber);

	return TRUE;
}

/**
 * This function handles the AOC Event
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Pointer to the call agent state
 * @param[in]		call_handle		Call Handle
 * @param[in]		ptapi_aoc_info	AOC info associated with the AOC Event
 */
gboolean _vc_core_tapi_event_handle_aoc(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, TelCallAocInfo_t *ptapi_aoc_info)
{
	call_vc_call_objectinfo_t callobject_info;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "Started, Call Handle = %d", call_handle);
	if (call_handle == VC_TAPI_INVALID_CALLHANDLE) {
		CALL_ENG_DEBUG(ENG_ERR, "INVALID_CALLHANDLE Error");
		return FALSE;
	}

	/* get the member info and chage info */
	_vc_core_cm_clear_call_object(&callobject_info);
	if (FALSE == _vc_core_cm_get_call_object(&pcall_agent->call_manager, call_handle, &callobject_info)) {
		CALL_ENG_DEBUG(ENG_ERR, "Call Object Not available");
		return FALSE;
	}

	/* Store the call cost TAPI info */
	callobject_info.aoc_ccm = ptapi_aoc_info->CCM;
	memcpy((void *)callobject_info.aoc_currency, (const void *)ptapi_aoc_info->szCurrency, VC_AOC_CURRENCY_CODE_LEN_MAX);

	/* Set the modified call object to the Call Mangaer */
	_vc_core_cm_set_call_object(&pcall_agent->call_manager, &callobject_info);

	_vc_core_ca_send_event_to_client(pcall_agent, VC_CALL_IND_AOC, call_handle, 0, &callobject_info);
	return TRUE;

}

static gboolean __call_vc_get_aoc_ppm_value_idle_cb(gpointer pdata)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pdata;
	__call_vc_get_aoc_ppm_value(pcall_agent);
	return FALSE;
}

static gboolean __call_vc_download_call_timer_cb(gpointer pdata)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pdata;
	_vc_core_ca_end_all_calls(pcall_agent);
	return FALSE;
}

static void __call_vc_get_aoc_ppm_value(call_vc_callagent_state_t *pcall_agent)
{
	TapiResult_t tapi_error = TAPI_API_SUCCESS;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (pcall_agent->aoc_ppm == VC_INVALID_PPM) {
		tapi_error = tel_get_ss_aoc_info(pcall_agent->tapi_handle, TAPI_SS_AOC_TYPE_PUC, _vc_core_engine_get_aoc_info_cb, NULL);

		if (tapi_error != TAPI_API_SUCCESS) {
			CALL_ENG_DEBUG(ENG_ERR, "TAPI Error: %x", tapi_error);
		}
	}
}
