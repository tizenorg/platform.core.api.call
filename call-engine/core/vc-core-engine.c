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
#include <errno.h>
#include <ITapiModem.h>

/* Call Module File Includes */
#include "vc-core-engine.h"
#include "vc-core-callagent.h"
#include "vc-core-callmanager.h"
#include "vc-core-util.h"
#include "vc-core-tapi-evnt.h"
#include "vc-core-tapi-rqst.h"
#include "vc-core-svcall.h"
#include "vc-core-engine-status.h"
#include "vc-core-ecc.h"

/*Global Variable Declerations */

/*Incoming Call Details used for Self Event*/
static call_vc_handle gincoming_call_handle = VC_TAPI_INVALID_CALLHANDLE;
static TelCallIncomingCallInfo_t gincoming_call_info;

/*Initialization Global Variables*/
static gboolean gphone_init_finished = FALSE;
static call_vc_handle gphone_rejected_call = VC_TAPI_INVALID_CALLHANDLE;

static call_vc_callagent_state_t *gpcall_agent_for_callback = NULL;	/*jspark event subscribe & callback function*/

/* SAT call detail used for self event */
static int gsat_event_type = 0;
static int gtype = 0;
static void *gpresult = NULL;
static TelSatSetupCallIndCallData_t gSatSetupCallInfo;
//static TelSatSendDtmfIndDtmfData_t gSatSendDtmfInfo;

/*Local Function Declarations*/
/**
 * This function handles the end event for outgoing call
 *
 * @internal
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Handle to the call agent
 * @param[in]		call_handle		call handle of the outgoing call
 * @param[in]		type				type of the tapi event
 * @param[in]		tapi_cause		tapi cause
 */
static gboolean __call_vc_outgoingcall_endhandle(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle, const char *type, TelTapiEndCause_t tapi_cause);
/**
 * This function handles the end event for incoming call
 *
 * @internal
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent		Handle to the call agent
 * @param[in]		call_handle		call handle of the outgoing call
 */
static gboolean __call_vc_incomingcall_endhandle(call_vc_callagent_state_t *pcall_agent, call_vc_handle call_handle);
/**
 * This function checks whether the call agent is in any of the wait state
 *
 * @internal
 * @return		Returns TRUE on if call agent is in wait state or FALSE
 * @param[in]		pcall_agent		Handle to the call agent
 */
static gboolean __call_vc_is_callagent_waitstate(call_vc_callagent_state_t *pcall_agent);
/**
 * This function creates call details for outgoign call with the given setup info
 *
 * @internal
 * @return		Returns TRUE on if call agent is in wait state or FALSE
 * @param[in]		pagent			Handle to the call agent
 * @param[in]		psetup_call_info	Setup info
 * @param[out]	pcall_object		Call object containing call details
 * @param[out]	error_code		Error Code
 */
static gboolean __call_vc_create_outgoing_callinfo(call_vc_callagent_state_t *pagent, voicecall_setup_info_t *psetup_call_info, call_vc_call_objectinfo_t *pcall_object, int *error_code);

/**
 * This function handles all telephony events
 *
 * @internal
 * @return		void
* @param[in]		tapi_handle		tapi handle
* @param[in]		noti_id			event type
* @param[in]		data				tapi event data
* @param[in]		userdata			user callback data
 */

static void __call_vc_handle_tapi_events(TapiHandle *handle, const char *noti_id, void *data, void *user_data);

/**
* This function handles sat engine notification
*
* @internal
* @return		void
* @param[in]		tapi_handle		tapi handle
* @param[in]		noti_id			event type
* @param[in]		data				tapi event data
* @param[in]		userdata			user callback data
*/
static void __call_vc_handle_sat_engine_events_cb(TapiHandle *handle, const char *noti_id, void *data, void *user_data);

/**
* This function subscribes for all notifications required for voicecall engine
*
* @internal
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pcall_agent		handle to voicecall agent structure
*/
static gboolean __call_vc_subscribe_call_events(call_vc_callagent_state_t *pcall_agent);

/**
* This function handles telephony initialized notifications
*
* @internal
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		tapi_handle		tapi handle
* @param[in]		noti_id			event type
* @param[in]		data				tapi event data
* @param[in]		userdata			user callback data
*/
static void __call_vc_tapi_initialized_cb(TapiHandle *handle, const char *noti_id, void *data, void *user_data);

/**
* This function subscribes for telephony call notifications
*
* @internal
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pcall_agent		handle to voicecall agent structure
*/
static gboolean __call_vc_subscribe_tapi_event(call_vc_callagent_state_t *pcall_agent);

/**
* This function request the engine to setup a sat call
*
* @internal
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pagent		handle to voicecall agent structure
* @param[in]		mo_call_index		call index of the mo call prepared for sat call
* @param[in]		ret_code		Error code
* @exception		ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED
*/
static gboolean __call_vc_request_sat_call(call_vc_callagent_state_t *pagent, int mo_call_index, int *ret_code);

/**
* This function hadles the events from the sat engine
*
* @internal
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pcall_agent		handle to voicecall agent structure
* @param[in]		sat_event		event type (sat request / sat response)
* @param[in]		sat_event_type	sat event sub type
* @param[in]		type				event type received from sat
* @param[in]		result			data received from sat
*/
static gboolean __call_vc_handle_sat_engine_events(call_vc_callagent_state_t *pcall_agent, int sat_event, int sat_event_type, int type, void *result);

/**
* This function request the engine to setup a normal voice call
*
* @internal
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pagent		handle to voicecall agent structure
* @param[in]		mo_call_index		call index of the mo call prepared for sat call
* @param[in]		ret_code		Error code
* @exception		ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED
*/
static gboolean __call_vc_request_call(call_vc_callagent_state_t *pagent, int mo_call_index, int *ret_code);

/**
* This function serves as the callback function for the incoming call idle add function
*
* @return		Returns TRUE - if the callback has to be called again, FALSE otherwise
* @param[in]		puser_data	data set by the user
*/
static gboolean __call_vc_incoming_idle_cb(gpointer puser_data);
static gboolean __call_vc_incoming_call_end_idle_cb(gpointer puser_data);
static gboolean __call_vc_reject_call_idle_cb(gpointer puser_data);
static gboolean __call_vc_reject_call_full_idle_cb(gpointer puser_data);

/**
* This function serves as the callback function for the SAT idle add function
*
* @return		Returns TRUE - if the callback has to be called again, FALSE otherwise
* @param[in]		puser_data	data set by the user
*/
static gboolean __call_vc_sat_idle_cb(gpointer puser_data);

/**
 * This function checks the voicecall engine's idle status and send VC_ACTION_NO_ACTIVE_TASK to client if engine is idle
 *
 * @return		void
 * @param[in]		pcall_agent	Pointer to the call agent structure
 */
static void __vc_core_check_engine_active_task(call_vc_callagent_state_t *pcall_agent);

/**
 * This function checks whether dtmf is possible
 *
 * @return		This function returns TRUE if dtmf is possible or else FALSE
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
static gboolean __vc_core_is_dtmf_possible(call_vc_callagent_state_t *pcall_agent);

/**
* This function initializes the voicecall engine
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[out]	pcall_agent_out	Pointer to the address of call agent
* @param[in]	pcallback_func	Call back function
* @param[in]	puser_data		Data set by user
* @remarks		pcall_agent_out and pcallback_func cannot be NULL.
*				Output Parameter pcall_agent_out should be initialized to NULL
*/
voicecall_error_t _vc_core_engine_init(voicecall_engine_t **pcall_agent_out, voicecall_cb pcallback_func, void *puser_data)
{
	call_vc_callagent_state_t *pcall_agent = NULL;

	CALL_ENG_DEBUG(ENG_DEBUG, "Voicecall Engine");

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent_out != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(*pcall_agent_out == NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*Initialize Call Agent*/
	pcall_agent = _vc_core_ca_init_agent();
	if (NULL == pcall_agent) {
		return ERROR_VOICECALL_MEMALLOC_FAILURE;
	}

	if (FALSE == __call_vc_subscribe_call_events(pcall_agent)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Event Subscrition Failed");
		_vc_core_ca_finish_agent(pcall_agent);
		return ERROR_VOICECALL_NOTI_FAILED;
	}
	/*Store the Callback Pointer*/
	pcall_agent->client_callback = pcallback_func;
	pcall_agent->puser_data = puser_data;

	/*Calls to Tapi Should not be made in the application initialization, because this code will be executed if application is launched during
	an incoming call also. so DBus Lock up can happen.*/
	gphone_init_finished = TRUE;

	/*Init Success, Assign Output Parameters*/
	*pcall_agent_out = (voicecall_engine_t *)pcall_agent;

	return ERROR_VOICECALL_NONE;
}

void _vc_core_engine_handle_sat_events_cb(void *sat_setup_call_data, void *userdata)
{
	char *data = sat_setup_call_data;

	call_vc_callagent_state_t *pcall_agent = gpcall_agent_for_callback;

	CALL_ENG_DEBUG(ENG_DEBUG, "sat event callback.");

	memset(&gSatSetupCallInfo, 0, sizeof(TelSatSetupCallIndCallData_t));
	memcpy(&gSatSetupCallInfo, data, sizeof(TelSatSetupCallIndCallData_t));

	gsat_event_type = SAT_RQST_SETUP_CALL;
	//gtype = TAPI_EVENT_SAT_SETUP_CALL_IND;
	gpresult = &gSatSetupCallInfo;
	g_idle_add(__call_vc_sat_idle_cb, pcall_agent);

	CALL_ENG_DEBUG(ENG_DEBUG, "Call back Ends and returning..");
}

static void __call_vc_handle_sat_engine_events_cb(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
//	call_vc_callagent_state_t *pcall_agent = gpcall_agent_for_callback;

	CALL_ENG_DEBUG(ENG_DEBUG, "sat event callback.");

#if 0
	int event_type = event->EventType;
	/*int status = event->Status;*/
	char *data = event->pData;

	switch (event_type) {
	case TAPI_EVENT_SAT_SETUP_CALL_IND:
		{
			/*it will be processed in _vc_core_engine_handle_sat_events_cb().*/
			return;
		}
		break;

	case TAPI_EVENT_SAT_SEND_DTMF_IND:
		{
			/* async for SAT sync noti */
			memset(&gSatSendDtmfInfo, 0, sizeof(TelSatSendDtmfIndDtmfData_t));
			memcpy(&gSatSendDtmfInfo, data, sizeof(TelSatSendDtmfIndDtmfData_t));

			gsat_event_type = SAT_RQST_SEND_DTMF;
			gtype = event_type;
			gpresult = &gSatSendDtmfInfo;

			g_idle_add(__call_vc_sat_idle_cb, pcall_agent);
		}
		break;

	case TAPI_EVENT_SAT_CALL_CONTROL_IND:
		{
			__call_vc_handle_sat_engine_events(pcall_agent, VC_ACTION_SAT_RESPONSE, SAT_RESP_SETUP_CALL, event_type, data);
		}
		break;

	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Default: event_type = %d", event_type);
		break;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Call back Ends and returning..");

	/*Send Process Termintate Event to the Client */
	if ((event_type == TAPI_EVENT_SAT_SETUP_CALL_IND) || (event_type == TAPI_EVENT_SAT_SEND_DTMF_IND)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Do not check an active task here");
	} else {
		__vc_core_check_engine_active_task(pcall_agent);
	}
#endif
}

/*Subscribe Noti Events*/
static gboolean __call_vc_subscribe_call_events(call_vc_callagent_state_t *pcall_agent)
{
	CALL_ENG_KPI("__call_vc_subscribe_tapi_event start");
	/* Subscribe Tapi Events */
	if (FALSE == __call_vc_subscribe_tapi_event(pcall_agent)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Noti subscribe to Tapi Events failed");
		return FALSE;

	}
	CALL_ENG_KPI("__call_vc_subscribe_tapi_event done");

	CALL_ENG_DEBUG(ENG_DEBUG, "Noti Subscription Sucess");
	return TRUE;
}

/**
 * This function sends response event to the registered client
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent	Pointer to the call agent structure
 * @param[in]		event		response event type
 * @param[in]		param1		param 1 to be passed to the client
 * @param[in]		param2		param 2 to be passed to the client
 * @param[in]		param3		param 3 to be passed to the client
 */
gboolean _vc_core_ca_send_event_to_client(call_vc_callagent_state_t *pcall_agent, int event, int param1, int param2, void *param3)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "Sending Event to Client");
	if (pcall_agent->client_callback != NULL) {
		return pcall_agent->client_callback(event, param1, param2, param3, pcall_agent->puser_data);
	}
	return FALSE;
}

static void __call_vc_tapi_initialized_cb(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	TelSimCardStatus_t sim_status = 0;
	TelSimCardType_t card_type = 0;
	int sim_changed = 0;

	VOICECALL_RETURN_IF_FAIL(data != NULL);
	tapi_power_phone_power_status_t *power = (tapi_power_phone_power_status_t *)data;

	if (power != TAPI_PHONE_POWER_STATUS_ON) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Modem is not available");
		return;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "gphone_init_finished %d", gphone_init_finished);

	if (gphone_init_finished != TRUE) {
		gphone_init_finished = TRUE;

		CALL_ENG_DEBUG(ENG_DEBUG, "Query Card Status ..");

		memset(&sim_status, 0, sizeof(sim_status));
		tapi_err = tel_get_sim_init_info(pagent->tapi_handle, &sim_status, &sim_changed);

		if (TAPI_API_SUCCESS != tapi_err) {
			CALL_ENG_DEBUG(ENG_DEBUG, "tel_get_sim_init_info failed.. tapi_err = %d", tapi_err);
			pagent->bis_no_sim = TRUE;
		} else {

			tapi_err = tel_get_sim_type(pagent->tapi_handle, &card_type);
			if (TAPI_API_SUCCESS == tapi_err) {
				CALL_ENG_DEBUG(ENG_DEBUG, "tel_get_sim_type failed.. tapi_err = %d", tapi_err);
			}

			pagent->card_type = card_type;

			CALL_ENG_DEBUG(ENG_DEBUG, "card_status = %d, card_type = %d", sim_status, pagent->card_type);
			/*Telephony team's reqeust..*/
			switch (sim_status) {
			case TAPI_SIM_STATUS_CARD_NOT_PRESENT:	/* = 0x01, <  Card not present */
			case TAPI_SIM_STATUS_CARD_REMOVED:	/* =0x0b, <  Card removed **/
				pagent->bis_no_sim = TRUE;
				break;
			case TAPI_SIM_STATUS_CARD_ERROR:	/* = 0x00, < Bad card / On the fly SIM gone bad **/
			case TAPI_SIM_STATUS_SIM_INITIALIZING:	/* = 0x02, <  Sim is Initializing state **/
			case TAPI_SIM_STATUS_SIM_INIT_COMPLETED:	/* = 0x03, <  Sim Initialization ok **/
			case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:	/* = 0x04, <  PIN  required state **/
			case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:	/* = 0x05, <  PUK required state **/
			case TAPI_SIM_STATUS_CARD_BLOCKED:	/* = 0x06,              <  PIN/PUK blocked(permanently blocked- All the attempts for PIN/PUK failed) **/
			case TAPI_SIM_STATUS_SIM_NCK_REQUIRED:	/* = 0x07,              <  Network Control Key required state **/
			case TAPI_SIM_STATUS_SIM_NSCK_REQUIRED:	/* = 0x08,              <  Network Subset Control Key required state **/
			case TAPI_SIM_STATUS_SIM_SPCK_REQUIRED:	/* = 0x09,              <  Service Provider Control Key required state **/
			case TAPI_SIM_STATUS_SIM_CCK_REQUIRED:	/* = 0x0a,              <  Corporate Control Key required state **/
			case TAPI_SIM_STATUS_SIM_LOCK_REQUIRED:
				pagent->bis_no_sim = FALSE;
				break;
			default:
				CALL_ENG_DEBUG(ENG_DEBUG, "Unknown Card_status = %d", sim_status);
				pagent->bis_no_sim = TRUE;
				break;
			}
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "gphone_init_finished is TRUE.");
	}

	/*As a Backup, whenever this event is arrived, just update this variable*/
	gphone_init_finished = TRUE;
}

static gboolean __call_vc_subscribe_tapi_event(call_vc_callagent_state_t *pcall_agent)
{
	int index = 0;

	TapiResult_t api_err = TAPI_API_SUCCESS;
	int num_event = 0;

	pcall_agent->tapi_handle = tel_init(NULL);
	if (pcall_agent->tapi_handle != NULL) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_init() success.");
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_init() failed.");
		return FALSE;
	}

	const char *call_event_list[] = {
		TAPI_NOTI_VOICE_CALL_STATUS_IDLE,
		TAPI_NOTI_VOICE_CALL_STATUS_ACTIVE,
		TAPI_NOTI_VOICE_CALL_STATUS_DIALING,
		TAPI_NOTI_VOICE_CALL_STATUS_ALERT,
		TAPI_NOTI_VOICE_CALL_STATUS_INCOMING,
		TAPI_NOTI_VOICE_CALL_STATUS_WAITING,
		TAPI_NOTI_CALL_INFO_CALL_CONNECTED_LINE,
		TAPI_NOTI_CALL_INFO_WAITING,
		TAPI_NOTI_CALL_INFO_CUG,
		TAPI_NOTI_CALL_INFO_FORWARDED,
		TAPI_NOTI_CALL_INFO_BARRED_INCOMING,
		TAPI_NOTI_CALL_INFO_BARRED_OUTGOING,
		TAPI_NOTI_CALL_INFO_DEFLECTED,
		TAPI_NOTI_CALL_INFO_CLIR_SUPPRESSION_REJECT,
		TAPI_NOTI_CALL_INFO_FORWARD_UNCONDITIONAL,
		TAPI_NOTI_CALL_INFO_FORWARD_CONDITIONAL,
		TAPI_NOTI_CALL_INFO_CALL_LINE_IDENTITY,
		TAPI_NOTI_CALL_INFO_CALL_NAME_INFORMATION,
		TAPI_NOTI_CALL_INFO_FORWARDED_CALL,
		TAPI_NOTI_CALL_INFO_CUG_CALL,
		TAPI_NOTI_CALL_INFO_DEFLECTED_CALL,
		TAPI_NOTI_CALL_INFO_TRANSFERED_CALL,
		TAPI_NOTI_CALL_INFO_HELD,
		TAPI_NOTI_CALL_INFO_ACTIVE,
		TAPI_NOTI_CALL_INFO_JOINED,
		TAPI_NOTI_CALL_INFO_RELEASED_ON_HOLD,
		TAPI_NOTI_CALL_INFO_TRANSFER_ALERT,
		TAPI_NOTI_CALL_INFO_TRANSFERED,
		TAPI_NOTI_CALL_SOUND_WBAMR,
	};
	num_event = sizeof(call_event_list) / sizeof(int);
	for (index = 0; index < num_event; index++) {
		api_err = tel_register_noti_event(pcall_agent->tapi_handle, call_event_list[index], __call_vc_handle_tapi_events, NULL);
		if (api_err != TAPI_API_SUCCESS) {
			CALL_ENG_DEBUG(ENG_DEBUG, "tel_register_noti_event() failed.. event id:[%s], api_err:[%d]", call_event_list[index], api_err);
			return FALSE;
		}
	}

	const char *ready_event_list[] = {
		TAPI_NOTI_MODEM_POWER,
	};
	num_event = sizeof(ready_event_list) / sizeof(int);
	for (index = 0; index < num_event; index++) {
		api_err = tel_register_noti_event(pcall_agent->tapi_handle, ready_event_list[index], __call_vc_tapi_initialized_cb, NULL);
		if (api_err != TAPI_API_SUCCESS) {
			CALL_ENG_DEBUG(ENG_DEBUG, "tel_register_noti_event() failed.. event id:[%s], api_err:[%d]", ready_event_list[index], api_err);
			return FALSE;
		}
	}

#if 0
	const char *sat_event_list[] = {
		"SETUP CALL"/*TAPI_EVENT_SAT_SETUP_CALL_IND*/,
		"CALL CONTROL" /*TAPI_EVENT_SAT_CALL_CONTROL_IND*/,
		"SEND DTMF" /*TAPI_EVENT_SAT_SEND_DTMF_IND*/,	/*CALL_VC_SIMATK_EVENT_MAX_NUM : 3*/
	};
	num_event = sizeof(sat_event_list) / sizeof(int);
	for (index = 0; index < num_event; index++) {
		api_err = tel_register_noti_event(pcall_agent->tapi_handle, sat_event_list[index], __call_vc_handle_sat_engine_events_cb, NULL);
		if (api_err != TAPI_API_SUCCESS) {
			CALL_ENG_DEBUG(ENG_DEBUG, "tel_register_noti_event() failed.. event id:[%s], api_err:[%d]", sat_event_list[index], api_err);
			return FALSE;
		}
	}
#endif

	gpcall_agent_for_callback = pcall_agent;

	CALL_ENG_DEBUG(ENG_DEBUG, "Subscribe to TAPI Success");

	return TRUE;

}

static gboolean __call_vc_create_outgoing_callinfo(call_vc_callagent_state_t *pagent, voicecall_setup_info_t *psetup_call_info, call_vc_call_objectinfo_t *pcall_object, int *error_code)
{
	char call_number[VC_PHONE_NUMBER_LENGTH_MAX] = { 0, };

	CALL_ENG_DEBUG(ENG_DEBUG, "call_type=%d", psetup_call_info->call_type);

	_vc_core_cm_clear_call_object(pcall_object);

	/*Update CallObjects state to Prepare Outgoing*/
	pcall_object->state = VC_CALL_STATE_PREPARE_OUTGOING;

	/*Update Call|Objects Call Type*/
	pcall_object->call_type = psetup_call_info->call_type;

	/*Update Call Objects Identity*/
	pcall_object->identity_mode = psetup_call_info->identity_mode;

	/*Differentiate the call by its source of origination*/
	switch (pcall_object->call_type) {
	case VC_CALL_ORIG_TYPE_EMERGENCY:
		{
			pcall_object->bemergency_number = TRUE;
			_vc_core_util_strcpy(pcall_object->source_tel_number, sizeof(pcall_object->source_tel_number), psetup_call_info->source_tel_number);
			_vc_core_util_strcpy(pcall_object->tel_number, sizeof(pcall_object->tel_number), psetup_call_info->tel_number);
			return TRUE;
		}
		break;
	case VC_CALL_ORIG_TYPE_PINLOCK:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "PIN LOCK!!!!");
			_vc_core_util_strcpy(pcall_object->source_tel_number, sizeof(pcall_object->source_tel_number), psetup_call_info->source_tel_number);
			_vc_core_util_strcpy(pcall_object->tel_number, sizeof(pcall_object->tel_number), psetup_call_info->tel_number);
		}
		break;
	case VC_CALL_ORIG_TYPE_NOSIM:	/*no sim (pagent->bis_no_sim == TRUE)*/
		{
			_vc_core_util_strcpy(pcall_object->source_tel_number, sizeof(pcall_object->source_tel_number), psetup_call_info->source_tel_number);
			_vc_core_util_strcpy(pcall_object->tel_number, sizeof(pcall_object->tel_number), psetup_call_info->tel_number);
		}
		break;
	case VC_CALL_ORIG_TYPE_NORMAL:
	case VC_CALL_ORIG_TYPE_SAT:
		{
			_vc_core_util_strcpy(pcall_object->source_tel_number, sizeof(pcall_object->source_tel_number), psetup_call_info->source_tel_number);
			_vc_core_util_strcpy(pcall_object->tel_number, sizeof(pcall_object->tel_number), psetup_call_info->tel_number);
		}
		break;
	case VC_CALL_ORIG_TYPE_VOICEMAIL:
		{
			_vc_core_util_strcpy(pcall_object->source_tel_number, sizeof(pcall_object->source_tel_number), psetup_call_info->source_tel_number);
			_vc_core_util_strcpy(pcall_object->tel_number, sizeof(pcall_object->tel_number), psetup_call_info->tel_number);
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Invalide Call Type: %d", pcall_object->call_type);
		*error_code = ERROR_VOICECALL_INVALID_CALL_TYPE;
		return FALSE;
	}

	/*Copy CUG Details */
	memcpy(&pcall_object->cug_info, &psetup_call_info->cug_info, sizeof(voicecall_cug_info_t));

	/*Check for Emergency Number */
	_vc_core_util_extract_call_number(pcall_object->tel_number, call_number, sizeof(call_number));
	pcall_object->bemergency_number = _vc_core_ecc_check_emergency_number(pagent->tapi_handle, pagent->card_type, call_number, pagent->bis_no_sim, &pcall_object->ecc_category);

	CALL_ENG_DEBUG(ENG_DEBUG, "no_sim=%d, emergency_number=%d", pagent->bis_no_sim, pcall_object->bemergency_number);

	if (VC_CALL_ORIG_TYPE_PINLOCK == pcall_object->call_type && pcall_object->bemergency_number == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "PIN/PUK Lock and number is no emergency number, call cannot be made");
		*error_code = ERROR_VOICECALL_EMERGENCY_CALLS_ONLY;
		return FALSE;
	}

	if (pagent->bis_no_sim == TRUE && pcall_object->bemergency_number == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Sim Not available and number is no emergency number, call cannot be made");
		*error_code = ERROR_VOICECALL_CALL_IMPOSSIBLE_NOSIM_NOEMERGNUM;
		return FALSE;
	}

	if (TRUE == pcall_object->bemergency_number) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Number is Emergency Number, Change call type to emergency");
		psetup_call_info->call_type = pcall_object->call_type = VC_CALL_ORIG_TYPE_EMERGENCY;
		psetup_call_info->ecc_category = pcall_object->ecc_category;
	}

	return TRUE;
}

static gboolean __call_vc_request_sat_call(call_vc_callagent_state_t *pagent, int mo_call_index, int *ret_code)
{
	call_vc_call_objectinfo_t callInfo;
	TelSatCmdQualiSetupCall_t sat_request_type = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "mo_call_index = %d", mo_call_index);

	VOICECALL_RETURN_FALSE_IF_FAIL(pagent->call_manager.setupcall_info.mocall_index == mo_call_index);

	if (FALSE == _vc_core_cm_get_outgoing_call_info(&pagent->call_manager, &callInfo)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call info does not exist!");
		*ret_code = ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
		return FALSE;
	}

	sat_request_type = pagent->call_manager.setupcall_info.satcall_setup_info.satengine_setupcall_data.calltype;

	CALL_ENG_DEBUG(ENG_DEBUG, " SAT_CALL sat_request_type=%d", sat_request_type);

	switch (sat_request_type) {
	case TAPI_SAT_SETUP_CALL_IF_ANOTHER_CALL_NOT_BUSY_WITH_REDIAL:
		{
			pagent->call_manager.setupcall_info.satcall_setup_info.redial = TRUE;
		}
	case TAPI_SAT_SETUP_CALL_IF_ANOTHER_CALL_NOT_BUSY:	/*Fall Through*/
		{
			/* one is the call set up by SAT, so more than 1 means another call exists */
			if (_vc_core_cm_get_call_member_count(&pagent->call_manager) > 1) {
				/* voice call agent is busy */
				CALL_ENG_DEBUG(ENG_DEBUG, "Call Exists, SAT call connot be continued");
				_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
				_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
				*ret_code = ERROR_VOICECALL_CALL_NOT_ALLOWED;
				return FALSE;
			}

			_vc_core_cm_set_outgoing_call_info(&pagent->call_manager, &callInfo);

			if (_vc_core_tapi_rqst_prepare_setup_call(pagent) == FALSE) {
				CALL_ENG_DEBUG(ENG_DEBUG, "SAT_CALL fail to setup call");
				_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
				_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
				_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_OUTGOING_ABORTED);
				*ret_code = ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED;
				return FALSE;
			} else {
				CALL_ENG_DEBUG(ENG_DEBUG, "Prepare Setup Call Success");
			}
		}
		break;
	case TAPI_SAT_SETUP_CALL_PUT_ALL_OTHER_CALLS_ON_HOLD_WITH_REDIAL:
		{
			pagent->call_manager.setupcall_info.satcall_setup_info.redial = TRUE;
		}
	case TAPI_SAT_SETUP_CALL_PUT_ALL_OTHER_CALLS_ON_HOLD:	/*Fall Through */
		{
			if (_vc_core_cm_isexists_active_call(&pagent->call_manager) && _vc_core_cm_isexists_held_call(&pagent->call_manager)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Both Activee & Hld call exists, SAT Call cannot be continued");
				_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
				_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
				*ret_code = ERROR_VOICECALL_CALL_NOT_ALLOWED;
				return FALSE;
			}
			if (_vc_core_cm_isexists_connected_call(&pagent->call_manager)) {
				if (FALSE == _vc_core_tapi_rqst_prepare_setup_call(pagent)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "SAT_CALL hold fail");
					_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
					_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
					_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_OUTGOING_ABORTED);
					*ret_code = ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED;
					return FALSE;
				}
			} else {
				/*If no other calls to hold, setup the call directly */
				if (FALSE == _vc_core_tapi_rqst_setup_call(pagent)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "SAT_CALL fail to setup call");
					_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
					_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
					_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_OUTGOING_ABORTED);
					*ret_code = ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED;
					return FALSE;
				}
			}
		}
		break;
	case TAPI_SAT_SETUP_CALL_DISCONN_ALL_OTHER_CALLS_WITH_REDIAL:
		{
			pagent->call_manager.setupcall_info.satcall_setup_info.redial = TRUE;
		}
	case TAPI_SAT_SETUP_CALL_DISCONN_ALL_OTHER_CALLS:	/*Fall Through */
		{
			if (_vc_core_cm_isexists_active_call(&pagent->call_manager) || _vc_core_cm_isexists_held_call(&pagent->call_manager)) {
				/*Disconnect all calls and setup call */
				_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SETUP);
				_vc_core_tapi_rqst_release_all_calls(pagent);
			} else {
				if (FALSE == _vc_core_tapi_rqst_prepare_setup_call(pagent)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "SAT_CALL fail to setup call");
					_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
					_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
					_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_OUTGOING_ABORTED);
					*ret_code = ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED;
					return FALSE;
				}
			}
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Action not defined for this SAT Request Type");
		return FALSE;
	}

	return TRUE;
}

static gboolean __call_vc_request_call(call_vc_callagent_state_t *pagent, int mo_call_index, int *ret_code)
{
	call_vc_call_objectinfo_t callInfo;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	VOICECALL_RETURN_FALSE_IF_FAIL(pagent->call_manager.setupcall_info.mocall_index == mo_call_index);

	_vc_core_cm_clear_call_object(&callInfo);
	if (_vc_core_cm_get_outgoing_call_info(&pagent->call_manager, &callInfo) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call info does not exist!");
		*ret_code = ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
		return FALSE;
	}
	/*Switch according to the CallType requested*/
	switch (callInfo.call_type) {
	case VC_CALL_ORIG_TYPE_NORMAL:
	case VC_CALL_ORIG_TYPE_EMERGENCY:
	case VC_CALL_ORIG_TYPE_NOSIM:
	case VC_CALL_ORIG_TYPE_SAT:
	case VC_CALL_ORIG_TYPE_PINLOCK:

		/*Set the updated Object Info to the Call Manager*/
		_vc_core_cm_set_outgoing_call_info(&pagent->call_manager, &callInfo);

		if (_vc_core_tapi_rqst_prepare_setup_call(pagent) == FALSE) {
			_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
			_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_OUTGOING_ABORTED);
			*ret_code = ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED;
			CALL_ENG_DEBUG(ENG_DEBUG, "Prepare Setup Call Failed");

			return FALSE;
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "Prepare Setup Call Success");
		}

		break;
	default:
		*ret_code = ERROR_VOICECALL_INVALID_CALL_TYPE;
		CALL_ENG_DEBUG(ENG_DEBUG, "Not defined call type=%d", pagent->call_manager.setupcall_info.call_type);
		return FALSE;
		break;
	}

	return TRUE;
}

/**
* This function prepares the call setup info structure for making call
 *
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent    Handle to voicecall engine
* @param[in]	psetup_call_info	Pointer to the call setup info structure.
* @remarks		pvoicecall_agent and psetup_call_info cannot be NULL
*				Only on successfull completion of this API, _vc_core_engine_make_call can be made
* @see			See following API's also
*				-_vc_core_engine_make_call
*				-voicecall_clear_prepared_call
*
 */
voicecall_error_t _vc_core_engine_prepare_call(voicecall_engine_t *pvoicecall_agent, voicecall_setup_info_t *psetup_call_info)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_call_objectinfo_t callobject_info;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	TelSimCardStatus_t sim_status;
	TelSimCardType_t card_type = 0;
	int sim_changed = 0;
	int status = 0;
	int nIndex = 0;
	int error_code = 0;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(psetup_call_info != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	if (FALSE == _vc_core_util_isvalid_telephone_number(psetup_call_info->tel_number)) {
		return ERROR_VOICECALL_INVALID_TELEPHONE_NUMBER;
	}

	CALL_ENG_KPI("tel_check_modem_power_status start");
	tapi_err = tel_check_modem_power_status(pagent->tapi_handle, &status);
	CALL_ENG_KPI("tel_check_modem_power_status done");
	if (TAPI_API_SUCCESS != tapi_err || 1 /* offline */ == status || 2 /* Error */== status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Tapi not initialized");
		return ERROR_VOICECALL_PHONE_NOT_INITIALIZED;
	}
	/*Check the Call Engines IO State*/
	if (pagent->io_state != VC_INOUT_STATE_NONE && pagent->io_state != VC_INOUT_STATE_OUTGOING_SHOW_RETRY_CALLBOX) {
		CALL_ENG_DEBUG(ENG_DEBUG, "pagent->io_state != VC_INOUT_STATE_NONE..io_state=%d", pagent->io_state);
		return ERROR_VOICECALL_ENGINE_STATE_NOT_NONE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Checking for SIM Availablity");

	/*memset(&sim_status, 0, sizeof(sim_status));*/
	CALL_ENG_KPI("tel_get_sim_init_info start");
	tapi_err = tel_get_sim_init_info(pagent->tapi_handle, &sim_status, &sim_changed);
	CALL_ENG_KPI("tel_get_sim_init_info done");

	if (TAPI_API_SUCCESS != tapi_err) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_get_sim_init_info failed.. tapi_err = %d", tapi_err);

		pagent->bis_no_sim = TRUE;
		psetup_call_info->call_type = VC_CALL_ORIG_TYPE_NOSIM;
	} else {
		CALL_ENG_KPI("tel_get_sim_type start");
		tapi_err = tel_get_sim_type(pagent->tapi_handle, &card_type);
		if (TAPI_API_SUCCESS != tapi_err) {
			CALL_ENG_DEBUG(ENG_DEBUG, "tel_get_sim_type failed.. tapi_err = %d", tapi_err);
		}
		CALL_ENG_KPI("tel_get_sim_type done");
		pagent->card_type = card_type;

		CALL_ENG_DEBUG(ENG_DEBUG, "card_status = %d, card_type = %d", sim_status, pagent->card_type);
		switch (sim_status) {
		case TAPI_SIM_STATUS_CARD_NOT_PRESENT:	/* = 0x01, <  Card not present **/
		case TAPI_SIM_STATUS_CARD_REMOVED:	/* =0x0b, <  Card removed **/
			pagent->bis_no_sim = TRUE;
			break;
		case TAPI_SIM_STATUS_CARD_ERROR:	/* = 0x00, < Bad card / On the fly SIM gone bad **/
		case TAPI_SIM_STATUS_SIM_INITIALIZING:	/* = 0x02, <  Sim is Initializing state **/
		case TAPI_SIM_STATUS_SIM_INIT_COMPLETED:	/* = 0x03, <  Sim Initialization ok **/
		case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:	/* = 0x04, <  PIN  required state **/
		case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:	/* = 0x05, <  PUK required state **/
		case TAPI_SIM_STATUS_CARD_BLOCKED:	/* = 0x06,              <  PIN/PUK blocked(permanently blocked- All the attempts for PIN/PUK failed) **/
		case TAPI_SIM_STATUS_SIM_NCK_REQUIRED:	/* = 0x07,              <  Network Control Key required state **/
		case TAPI_SIM_STATUS_SIM_NSCK_REQUIRED:	/* = 0x08,              <  Network Subset Control Key required state **/
		case TAPI_SIM_STATUS_SIM_SPCK_REQUIRED:	/* = 0x09,              <  Service Provider Control Key required state **/
		case TAPI_SIM_STATUS_SIM_CCK_REQUIRED:	/* = 0x0a,              <  Corporate Control Key required state **/
		case TAPI_SIM_STATUS_SIM_LOCK_REQUIRED:
			pagent->bis_no_sim = FALSE;
			break;
		default:
			CALL_ENG_DEBUG(ENG_DEBUG, "Unknown Card_status = %d", sim_status);
			pagent->bis_no_sim = TRUE;
			break;
		}
	}

	/*Prepare Outgoing Call Info*/
	_vc_core_cm_clear_call_object(&callobject_info);
	if (__call_vc_create_outgoing_callinfo(pagent, psetup_call_info, &callobject_info, &error_code) == FALSE) {
		return error_code;
	}

	/* Check for MO Call Possiblity */
	if (_vc_core_ca_is_mocall_possible(pagent, callobject_info.bemergency_number) == FALSE) {
		return ERROR_VOICECALL_CALL_NOT_ALLOWED;
	}

	/* Add the prepared call object to the CallManager */
	nIndex = _vc_core_cm_add_call_object(&pagent->call_manager, &callobject_info);
	if (nIndex != -1) {
		/*If there is a previously made MO Call, clear that before setting the new mo call */
		_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
		_vc_core_cm_set_outgoing_call(&pagent->call_manager, nIndex);
		psetup_call_info->mo_call_index = nIndex;
	} else {
		return ERROR_VOICECALL_CALL_NOT_ALLOWED;
	}

	return ERROR_VOICECALL_NONE;
}

/**
* This function clears the data of the given call type.
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_type		call type
* @param[in]	call_handle		Call handle of the connected call to be cleared
* @remarks		This will clear the call data only when the call data are currently not being used
*				i,e) the data will be cleared only if the corresponding call is ended or the call data is not used at all.
*				call_handle argument is required only in case of connected call, Engine ignores call_handle for other
*				call types.
*/
voicecall_error_t _vc_core_engine_finalize_call(voicecall_engine_t *pvoicecall_agent, voicecall_call_type_t call_type, int call_handle)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	gboolean bret_val = FALSE;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	CALL_ENG_DEBUG(ENG_DEBUG, "call_type = %d", call_type);

	switch (call_type) {
	case VC_OUTGOING_CALL:
		{
			if ((VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED == pagent->io_state) || (VC_INOUT_STATE_OUTGOING_WAIT_ORIG == pagent->io_state) || (VC_INOUT_STATE_OUTGOING_WAIT_ALERT == pagent->io_state)) {
				return ERROR_VOICECALL_CALL_IS_IN_PROGRESS;
			}

			bret_val = _vc_core_cm_clear_outgoing_call(&pagent->call_manager);
		}
		break;
	case VC_CONNECTED_CALL:
		{
			call_vc_call_objectinfo_t call_object;

			if (TRUE == _vc_core_cm_get_call_object(&pagent->call_manager, call_handle, &call_object)) {
				if (VC_CALL_STATE_ENDED == call_object.state) {
					bret_val = _vc_core_ca_clear_connected_call(pagent, call_handle);
				} else {
					return ERROR_VOICECALL_CALL_IS_IN_PROGRESS;
				}
			}
		}
		break;
	case VC_INCOMING_CALL:
		{
			return ERROR_VOICECALL_NOT_SUPPORTED;
		}
		break;
	default:
		return ERROR_VOICECALL_INVALID_CALL_TYPE;
	}

	return (TRUE == bret_val) ? ERROR_VOICECALL_NONE : ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

/**
* This function establishes an outgoing call with the details prepared using _vc_core_engine_prepare_call
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent    Handle to voicecall engine
* @param[in]	mo_call_index		Index of the prepare mo call
* @param[out]	pcall_handle		Handle of the MO Call Made
* @remarks		pvoicecall_agent and pcall_handle cannot be NULL
* @see			_vc_core_engine_end_call
*/
voicecall_error_t _vc_core_engine_make_call(voicecall_engine_t *pvoicecall_agent, int mo_call_index, int *pcall_handle)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	gboolean ret_val = FALSE;
	call_vc_call_objectinfo_t call_object;
	int error_code = ERROR_VOICECALL_NONE;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(mo_call_index >= 0 && mo_call_index <= 7, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_handle != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	CALL_ENG_DEBUG(ENG_DEBUG, "...");

	_vc_core_cm_clear_call_object(&call_object);
	if (_vc_core_cm_get_outgoing_call_info(&pagent->call_manager, &call_object) == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call info does not exist!");
		return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
	}

	if (VC_CALL_ORIG_TYPE_SAT == call_object.call_type) {
		ret_val = __call_vc_request_sat_call(pagent, mo_call_index, &error_code);
	} else {
		ret_val = __call_vc_request_call(pagent, mo_call_index, &error_code);
	}

	if ((TRUE == ret_val) && (pagent->io_state == VC_INOUT_STATE_OUTGOING_WAIT_ALERT || pagent->io_state == VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED || pagent->io_state == VC_INOUT_STATE_OUTGOING_WAIT_ORIG)) {
		*pcall_handle = _vc_core_cm_get_outgoing_call_handle(&pagent->call_manager);
		CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call Handle: %d", *pcall_handle);

	}

	return (TRUE == ret_val) ? ERROR_VOICECALL_NONE : error_code;
}

static gboolean __call_vc_handle_sat_engine_events(call_vc_callagent_state_t *pcall_agent, int sat_event, int sat_event_type, int type, void *result)
{
	call_vc_callagent_state_t *pagent = pcall_agent;
	call_vc_satsetup_info_t *psatsetup_info = NULL;
	voicecall_sat_callinfo_t call_vc_sat_callinfo;

	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "sat_event: %d, sat_event_type: %d, command_type: %d", sat_event, sat_event_type, type);

	psatsetup_info = (call_vc_satsetup_info_t *) &(pagent->call_manager.setupcall_info.satcall_setup_info);

	switch (sat_event) {
	case VC_ACTION_SAT_REQUEST:
		{
			switch (sat_event_type) {
			case SAT_RQST_SETUP_CALL:
				{
					CALL_ENG_DEBUG(ENG_DEBUG, "SAT_RQST_SETUP_CALL SAT Event is recieved...");

					if ((pagent->io_state != VC_INOUT_STATE_NONE) || (pagent->callagent_state != CALL_VC_CA_STATE_NORMAL)) {
						CALL_ENG_DEBUG(ENG_DEBUG, "SAT_CALL unable to process command!");
						_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
						return FALSE;
					}

					/*Initialize Call Setup Data */
					psatsetup_info->psat_rgb_data = NULL;
					memset(&(psatsetup_info->satengine_setupcall_data), 0, sizeof(TelSatSetupCallIndCallData_t));
					memcpy(&(psatsetup_info->satengine_setupcall_data), result, sizeof(TelSatSetupCallIndCallData_t));
					psatsetup_info->satengine_event_type = type;
					memset(&call_vc_sat_callinfo, 0, sizeof(voicecall_sat_callinfo_t));

					_vc_core_util_strcpy(call_vc_sat_callinfo.call_number, sizeof(call_vc_sat_callinfo.call_number), (char *)psatsetup_info->satengine_setupcall_data.callNumber.string);
					_vc_core_util_strcpy(call_vc_sat_callinfo.disp_text, sizeof(call_vc_sat_callinfo.disp_text), (char *)psatsetup_info->satengine_setupcall_data.dispText.string);
					call_vc_sat_callinfo.duration = psatsetup_info->satengine_setupcall_data.duration;
					if (call_vc_sat_callinfo.duration > 0) {
						psatsetup_info->bduration = TRUE;

						/*Store the duration, this remaining duration will be reduced in subsequent redial attempts */
						psatsetup_info->remaining_duration = call_vc_sat_callinfo.duration;
					}
					CALL_ENG_DEBUG(ENG_DEBUG, "Voice call set up request sent to voice call application psatsetup_info->bduration = %d", psatsetup_info->bduration);
					CALL_ENG_DEBUG(ENG_DEBUG, "psatsetup_info->remaining_duration = %lu", psatsetup_info->remaining_duration);
					CALL_ENG_DEBUG(ENG_DEBUG, "sat call type = %d", psatsetup_info->satengine_setupcall_data.calltype);

					_vc_core_ca_send_event_to_client(pagent, VC_ACTION_SAT_REQUEST, SAT_RQST_SETUP_CALL, 0, &call_vc_sat_callinfo);
				}
				break;
			case SAT_RQST_SEND_DTMF:
				{
					memset(&(psatsetup_info->satengine_dtmf_data), 0, sizeof(TelSatSendDtmfIndDtmfData_t));
					memcpy(&(psatsetup_info->satengine_dtmf_data), result, sizeof(TelSatSendDtmfIndDtmfData_t));
					psatsetup_info->satengine_event_type = type;

					if (FALSE == __vc_core_is_dtmf_possible(pagent)) {
						_vc_core_ca_send_sat_response(pagent, SAT_RQST_SEND_DTMF, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
					} else {
						memset(&call_vc_sat_callinfo, 0, sizeof(voicecall_sat_callinfo_t));

						_vc_core_util_strcpy(call_vc_sat_callinfo.call_number, sizeof(call_vc_sat_callinfo.call_number), (char *)psatsetup_info->satengine_dtmf_data.dtmfString.string);
						call_vc_sat_callinfo.bsat_hidden = psatsetup_info->satengine_dtmf_data.bIsHiddenMode;
						_vc_core_ca_send_event_to_client(pagent, VC_ACTION_SAT_REQUEST, SAT_RQST_SEND_DTMF, 0, &call_vc_sat_callinfo);
					}
				}
				break;
			}
		}
		break;
	case VC_ACTION_SAT_RESPONSE:
		{
			call_vc_call_objectinfo_t objectInfo;
			switch (sat_event_type) {
			case SAT_RESP_SETUP_CALL:
				{
					if (_vc_core_cm_get_outgoing_call_info(&pagent->call_manager, &objectInfo) == FALSE) {
						CALL_ENG_DEBUG(ENG_DEBUG, "SAT_CALL_CONTROL Outgoing call info does not exist..");
						_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_NONE);
						break;
					}
					memset(&(psatsetup_info->satengine_callctrl_data), 0, sizeof(psatsetup_info->satengine_callctrl_data));
					memcpy(&(psatsetup_info->satengine_callctrl_data), result, sizeof(psatsetup_info->satengine_callctrl_data));

					switch (psatsetup_info->satengine_callctrl_data.callCtrlResult)	{
					case TAPI_SAT_CALL_CTRL_R_ALLOWED_NO_MOD:
						{
							CALL_ENG_DEBUG(ENG_DEBUG, "ret=TAPI_SAT_CALL_CTRL_R_ALLOWED_NO_MOD");
							pagent->call_manager.setupcall_info.call_control_type = CALL_VC_SAT_CC_ALLOWED;
						}
						break;
					case TAPI_SAT_CALL_CTRL_R_NOT_ALLOWED:
						{
							CALL_ENG_DEBUG(ENG_DEBUG, "ret=TAPI_SAT_CALL_CTRL_R_NOT_ALLOWED");
							pagent->call_manager.setupcall_info.call_control_type = CALL_VC_SAT_CC_NOT_ALLOWED;
							_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_NONE);

							if (objectInfo.call_type == VC_CALL_ORIG_TYPE_SAT) {
								_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
								_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_CONTROL_PERMANENT_PROBLEM);
							} else {
								_vc_core_cm_clear_outgoing_call(&pagent->call_manager);
							}

							memset(&call_vc_sat_callinfo, 0, sizeof(voicecall_sat_callinfo_t));
							call_vc_sat_callinfo.sat_mo_call_ctrl_res = CALL_NOT_ALLOWED;
							_vc_core_ca_send_event_to_client(pagent, VC_ACTION_SAT_RESPONSE, SAT_RESP_SETUP_CALL, 0, &call_vc_sat_callinfo);
						}
						break;
					case TAPI_SAT_CALL_CTRL_R_ALLOWED_WITH_MOD:
						{
							gboolean bsscode = FALSE;

							CALL_ENG_DEBUG(ENG_DEBUG, "ret=TAPI_SAT_CALL_CTRL_R_ALLOWED_WITH_MOD");
							pagent->call_manager.setupcall_info.call_control_type = CALL_VC_SAT_CC_ALLOWED_WITH_MODIFIED;

							memset(objectInfo.connected_telnumber, 0, sizeof(objectInfo.connected_telnumber));
							memset(objectInfo.connected_name, 0, sizeof(objectInfo.connected_name));

							_vc_core_util_strcpy(objectInfo.connected_telnumber, sizeof(objectInfo.connected_telnumber), (char *)psatsetup_info->satengine_callctrl_data.u.callCtrlCnfCallData.address.string);
							_vc_core_util_strcpy(objectInfo.connected_name, sizeof(objectInfo.connected_name), (char *)psatsetup_info->satengine_callctrl_data.dispData.string);

							/*Prepare the data to be sent to the client */
							memset(&call_vc_sat_callinfo, 0, sizeof(voicecall_sat_callinfo_t));
							_vc_core_util_strcpy(call_vc_sat_callinfo.call_number, sizeof(call_vc_sat_callinfo.call_number), objectInfo.connected_telnumber);
							_vc_core_util_strcpy(call_vc_sat_callinfo.disp_text, sizeof(call_vc_sat_callinfo.disp_text), objectInfo.connected_name);

							/* when call number is changed as SS string */
							_vc_core_engine_status_isvalid_ss_code((voicecall_engine_t *)pcall_agent, objectInfo.connected_telnumber, &bsscode);
							if (TRUE == bsscode) {
								_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_NONE);

								call_vc_sat_callinfo.sat_mo_call_ctrl_res = CALL_CHANGED_TO_SS;
								CALL_ENG_DEBUG(ENG_DEBUG, "VC Call Control Response Event: %d", call_vc_sat_callinfo.sat_mo_call_ctrl_res);
							} else {
								objectInfo.bemergency_number = _vc_core_ecc_check_emergency_number(pagent->tapi_handle, pagent->card_type, objectInfo.connected_telnumber, pagent->bis_no_sim, &objectInfo.ecc_category);
								_vc_core_cm_set_outgoing_call_info(&pagent->call_manager, &objectInfo);

								call_vc_sat_callinfo.duration = psatsetup_info->satengine_setupcall_data.duration;
								call_vc_sat_callinfo.sat_mo_call_ctrl_res = CALL_ALLOWED_WITH_MOD;
								CALL_ENG_DEBUG(ENG_DEBUG, "VC Call Control Response Event: %d", call_vc_sat_callinfo.sat_mo_call_ctrl_res);
							}

							/*Send the Call Control response event to the client */
							_vc_core_ca_send_event_to_client(pagent, VC_ACTION_SAT_RESPONSE, SAT_RESP_SETUP_CALL, 0, &call_vc_sat_callinfo);
						}
						break;
					default:
						CALL_ENG_DEBUG(ENG_DEBUG, "SAT_CALL_CONTROL - not defined return code");
						break;
					}
				}
				break;
			default:
				CALL_ENG_DEBUG(ENG_DEBUG, "Invalid Sat Event Type");
				break;
			}
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Invalid SAT Event");
		return FALSE;
	}
	return TRUE;
}

static gboolean __call_vc_incoming_idle_cb(gpointer puser_data)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)puser_data;
	call_vc_call_objectinfo_t callobject_info;

	CALL_ENG_DEBUG(ENG_DEBUG, "Handling Incoming call in Idle Callback");

	if (FALSE == _vc_core_cm_get_incoming_call_info(&pagent->call_manager, &callobject_info)) {
		CALL_ENG_DEBUG(ENG_ERR, "Incoming Call Info not available");
		return FALSE;
	}

	/*If the Incoming End event arrived before processing the Incoming request, then donot change the state to Incombox */
	if (pagent->io_state != VC_INOUT_STATE_INCOME_END) {
		_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_INCOME_BOX);
	}

	/* Send Incoming Call Event to Client */
	/*_vc_core_ca_send_event_to_client(pagent, VC_CALL_INCOM, callobject_info.call_handle, callobject_info.bincoming_call_is_fwded, callobject_info.tel_number);*/
	_vc_core_ca_send_event_to_client(pagent, VC_CALL_INCOM, callobject_info.call_handle, 0, callobject_info.tel_number);

	/* Reset Incoming Call Details */
	gincoming_call_handle = VC_TAPI_INVALID_CALLHANDLE;
	memset(&gincoming_call_info, 0, sizeof(TelCallIncomingCallInfo_t));

	/*Check whether the incoming call is accepted or rejected by
	   cheking the Incoming callobjects status */
	if (_vc_core_cm_get_call_state(&pagent->call_manager, callobject_info.call_handle) != VC_CALL_STATE_INCOME) {
		CALL_ENG_DEBUG(ENG_DEBUG, "[Call :%d] not in VC_CALL_STATE_INCOME state", callobject_info.call_handle);
		return FALSE;
	}

	/*Always Return FALSE from this g_idle callback, so it will not be called again */
	return FALSE;
}

/*Rejects the call, only if the call state is in rejected state*/
static gboolean __call_vc_reject_call_full_idle_cb(gpointer puser_data)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)puser_data;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	call_vc_call_objectinfo_t call_object;

	CALL_ENG_DEBUG(ENG_DEBUG, "Rejecting the [Call:%d] in IDLE CB", gincoming_call_handle);

	if (gincoming_call_handle != -1) {
		_vc_core_cm_get_call_object(&pcall_agent->call_manager, gincoming_call_handle, &call_object);

		if (VC_CALL_STATE_REJECTED == call_object.state) {
			/*Answer the incoming call by accepting or rejecting the call */
			tapi_err = tel_answer_call(pcall_agent->tapi_handle, gincoming_call_handle, TAPI_CALL_ANSWER_REJECT, _vc_core_engine_answer_call_resp_cb, NULL);
			if (TAPI_API_SUCCESS != tapi_err) {
				CALL_ENG_DEBUG(ENG_ERR, "tel_answer_call failed, Error: %d", tapi_err);
			}

			gincoming_call_handle = -1;
		}
	}
	return FALSE;
}

/*Always reject the call, if the reject call handle is valid*/
static gboolean __call_vc_reject_call_idle_cb(gpointer puser_data)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)puser_data;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;

	CALL_ENG_DEBUG(ENG_DEBUG, "Rejecting the [Call Handle :%d] in IDLE CB", gincoming_call_handle);

	if (gphone_rejected_call != -1) {
		/*Answer the incoming call by accepting or rejecting the call */
		tapi_err = tel_answer_call(pcall_agent->tapi_handle, gphone_rejected_call, TAPI_CALL_ANSWER_REJECT, _vc_core_engine_answer_call_resp_cb, NULL);
		if (TAPI_API_SUCCESS != tapi_err) {
			CALL_ENG_DEBUG(ENG_ERR, "tel_answer_call failed, Error: %d", tapi_err);
		}
		gphone_rejected_call = -1;
	}
	return FALSE;
}

static gboolean __call_vc_sat_idle_cb(gpointer puser_data)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)puser_data;

	TelSatSetupCallIndCallData_t temp_data = { 0, };

	memset(&temp_data, 0, sizeof(TelSatSetupCallIndCallData_t));
	memcpy(&temp_data, gpresult, sizeof(TelSatSetupCallIndCallData_t));
	CALL_ENG_DEBUG(ENG_DEBUG, "[SAT] call_data->calltype = 0x%x", temp_data.calltype);
	CALL_ENG_DEBUG(ENG_DEBUG, "[SAT] call_data->dispText.string = %s", temp_data.dispText.string);
	CALL_ENG_DEBUG(ENG_DEBUG, "[SAT] call_data->callNumber.string = %s", temp_data.callNumber.string);
	CALL_ENG_DEBUG(ENG_DEBUG, "[SAT] call_data->duration = %d", (int)temp_data.duration);

	__call_vc_handle_sat_engine_events(pagent, VC_ACTION_SAT_REQUEST, gsat_event_type, gtype, gpresult);

	/*Free the result after copying the data */
	/*g_free(gpresult);*/
	gpresult = NULL;
	/*Always Return FALSE from this g_idle callback, so it will not be called again */
	return FALSE;
}

void _vc_core_engine_handle_incoming_tapi_events(void *mt_data, void *userdata)
{
	char *data = mt_data;

	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;

	VOICECALL_RETURN_IF_FAIL(pagent != NULL);

	int current_mt_call_handle = -1;
	CALL_ENG_DEBUG(ENG_DEBUG, "event_type == TAPI_EVENT_CALL_INCOM_IND...");

	/*Safety Check to avoid the mutiple incoming noti for the same call */
	current_mt_call_handle = _vc_core_cm_get_incoming_call_handle(&pagent->call_manager);
	CALL_ENG_DEBUG(ENG_ERR, "current_mt_call_handle = %d", current_mt_call_handle);

	if (current_mt_call_handle != VC_TAPI_INVALID_CALLHANDLE) {
		TelCallIncomingCallInfo_t mt_call_info;
		CALL_ENG_DEBUG(ENG_ERR, "Already an Incoming Call exits ,Problem in accpeting the incoming call, Current Call Details");
		CALL_VC_DUMP_CALLDETAILS(&pagent->call_manager);
		CALL_ENG_DEBUG(ENG_ERR, "****************Currently received call details *************************");

		memset(&mt_call_info, 0, sizeof(TelCallIncomingCallInfo_t));
		memcpy(&mt_call_info, data, sizeof(TelCallIncomingCallInfo_t));
		CALL_ENG_DEBUG(ENG_ERR, "****************call handle = [%d] *************************", mt_call_info.CallHandle);
		CALL_ENG_DEBUG(ENG_ERR, "****************call Number = [%s] *************************", mt_call_info.szCallingPartyNumber);
		CALL_ENG_DEBUG(ENG_ERR, "**************** Ignoring this incoming notification *************************");
		return;
	}

	memset(&gincoming_call_info, 0, sizeof(TelCallIncomingCallInfo_t));
	memcpy(&gincoming_call_info, data, sizeof(TelCallIncomingCallInfo_t));
	gincoming_call_handle = gincoming_call_info.CallHandle;

	CALL_ENG_DEBUG(ENG_DEBUG, "CallHandle = %d,  Number = %s", gincoming_call_info.CallHandle, gincoming_call_info.szCallingPartyNumber);

	/* Reject the Incoming call */
	if (FALSE == gphone_init_finished) {
		CALL_ENG_DEBUG(ENG_ERR, "Phone is not initialized, So reject the Call");

		gphone_rejected_call = gincoming_call_handle;

		/*Reject the Call in the Idle Callback */
		g_idle_add_full(G_PRIORITY_HIGH_IDLE, __call_vc_reject_call_idle_cb, pagent, NULL);

		return;
	}

	/* Check the IO State before accepting the call */
	switch (pagent->io_state) {
	case VC_INOUT_STATE_OUTGOING_WAIT_HOLD:
	case VC_INOUT_STATE_OUTGOING_WAIT_ALERT:
	case VC_INOUT_STATE_OUTGOING_WAIT_ORIG:
	case VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED:
	case VC_INOUT_STATE_INCOME_END:	/*If the Previous End event is still not prcocessed then reject the call */
		{
			call_vc_call_objectinfo_t objectInfo;

			/* setting the new member info */
			_vc_core_cm_clear_call_object(&objectInfo);
			objectInfo.call_handle = gincoming_call_handle;
			_vc_core_cm_change_call_state(&objectInfo, VC_CALL_STATE_REJECTED);

			/* add new member info */
			_vc_core_cm_add_call_object(&pagent->call_manager, &objectInfo);

			/*Reject the Call in the Idle Callback */
			g_idle_add_full(G_PRIORITY_HIGH_IDLE, __call_vc_reject_call_full_idle_cb, pagent, NULL);

			return;
		}
		break;
		/*If Outgoing call is in any of the following wait state during an Incoming Event Cancel the Outgoing Call */
	case VC_INOUT_STATE_OUTGOING_WAIT_RELEASE:	/*If Outgoing call is in any of the following wait state during an Incoming Event Cancel the Outgoing Call */
	case VC_INOUT_STATE_OUTGOING_ABORTED:
	case VC_INOUT_STATE_OUTGOING_SHOW_REDIALCAUSE:
	case VC_INOUT_STATE_OUTGOING_WAIT_REDIAL:
	case VC_INOUT_STATE_OUTGOING_SHOW_RETRY_CALLBOX:
		{
			int mo_call_handle = -1;
			mo_call_handle = _vc_core_cm_get_outgoing_call_handle(&pagent->call_manager);
			_vc_core_cm_remove_call_object(&pagent->call_manager, mo_call_handle);

			/* Inform the Client that waiting outgoing call are cleaned up to accept the incoming call , */
			_vc_core_ca_send_event_to_client(pagent, VC_ACTION_INCOM_FORCE, mo_call_handle, 0, NULL);
		}
	default:
		break;
	}

	/*If Incoming End event is still pending, First Process the incoming end indication before processing the new
	   Incoming Call */
	if (VC_INOUT_STATE_INCOME_END == pagent->io_state) {
		int mt_call_handle = -1;

		CALL_ENG_DEBUG(ENG_ERR, "Previous Incoming End Call Not processed, Processing Here");
		mt_call_handle = _vc_core_cm_get_incoming_call_handle(&pagent->call_manager);
		if (mt_call_handle != -1) {
			__call_vc_incomingcall_endhandle(pagent, mt_call_handle);
		}
	}

	/* Handle Incoming Call */
	if (TRUE == _vc_core_tapi_event_handle_incoming_event(pagent, gincoming_call_handle, &gincoming_call_info)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Using Idle Add Full with G_PRIORITY_HIGH_IDLE for processing Incoming Call");
		g_idle_add_full(G_PRIORITY_HIGH_IDLE, __call_vc_incoming_idle_cb, pagent, NULL);
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "MT CALL event processed done.");
	return;

}

static void __call_vc_handle_tapi_events(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{

	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;

	VOICECALL_RETURN_IF_FAIL(pagent != NULL);

	CALL_ENG_DEBUG(ENG_WARN, "event_type:[%s]", noti_id);

	/* Process TAPI events */
	if (strcmp(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_IDLE) == 0) {
		/* End IND */
		call_vc_call_objectinfo_t objectInfo;
		voicecall_call_state_t present_call_state = VC_CALL_STATE_NONE;
		call_vc_handle incoming_call_handle = VC_TAPI_INVALID_CALLHANDLE;
		TelTapiEndCause_t tapi_cause = TAPI_CALL_END_NO_CAUSE;
		call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
		TelCallStatusIdleNoti_t callIdleInfo;

		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_VOICE_CALL_STATUS_IDLE..");

		/* tapicallback data =  TelCallStatusIdleNoti_t */
		if (data != NULL) {
			memset(&callIdleInfo, 0, sizeof(TelCallStatusIdleNoti_t));
			memcpy(&callIdleInfo, data, sizeof(TelCallStatusIdleNoti_t));

			call_handle = callIdleInfo.id;
			tapi_cause = callIdleInfo.cause;
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d,end cause:%d", call_handle, tapi_cause);

		/*the end of incoming call rejected by callagent, because the call had come before the phone is initialized */
		if (call_handle == gphone_rejected_call) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Rejected call..phone not initialized");

			gphone_rejected_call = VC_TAPI_INVALID_CALLHANDLE;

			/*If no more calls available, End the Application */
			__vc_core_check_engine_active_task(pagent);
			return;
		}

		incoming_call_handle = _vc_core_cm_get_incoming_call_handle(&pagent->call_manager);
		present_call_state = _vc_core_cm_get_call_state(&pagent->call_manager, call_handle);

		CALL_ENG_DEBUG(ENG_DEBUG, "New Call Handle = %d, Already registered MT call handle : %d", call_handle, incoming_call_handle);
		switch (present_call_state) {
		case VC_CALL_STATE_NONE:
		case VC_CALL_STATE_ENDED:
		case VC_CALL_STATE_ENDED_FINISH:
			{
				CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d state is %d", call_handle, present_call_state);
				/*If no more calls available, End the Application */
				__vc_core_check_engine_active_task(pagent);
				return;
			}
			break;
		case VC_CALL_STATE_REJECTED:
			{
				/*End of incoming call (not registered as incoming call in CallAgent) rejected by callagent */
				if (incoming_call_handle != call_handle) {
					_vc_core_cm_remove_call_object(&pagent->call_manager, call_handle);
					CALL_ENG_DEBUG(ENG_DEBUG, "end of call rejected by callagent");

					/*If no more calls available, End the Application */
					__vc_core_check_engine_active_task(pagent);
					return;
				}
			}
			break;
		default:
			break;
		}

		/*End of the call rejected by user or by callagent (when hold is failed) */
		if ((VC_INOUT_STATE_INCOME_WAIT_RELEASE == pagent->io_state) && (incoming_call_handle == call_handle)) {
			_vc_core_cm_remove_call_object(&pagent->call_manager, call_handle);

			/*Change the In Out state to None*/
			_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_NONE);

			/*Notify Client about rejected Event*/
			_vc_core_ca_send_event_to_client(pagent, VC_CALL_REJECTED_END, call_handle, 0, NULL);

			return;
		}

		/*End of Incoming Call */
		if (incoming_call_handle == call_handle) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Adding Incoming End Event to Idle Callback");
			_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_INCOME_END);
			/*Make sure that the End Indication is processed always after the Incoming Indication , as both are
			   processed in Idle Add Callbacks */
			g_idle_add(__call_vc_incoming_call_end_idle_cb, pagent);
			return;
		}

		/*End of Outgoing Call */
		if (_vc_core_cm_get_outgoing_call_handle(&pagent->call_manager) == call_handle) {
			__call_vc_outgoingcall_endhandle(pagent, call_handle, TAPI_NOTI_VOICE_CALL_STATUS_IDLE, tapi_cause);
			return;
		}

		/*End of Normal Connected Call */
		_vc_core_tapi_event_handle_call_end_event(pagent, noti_id, call_handle, tapi_cause);

		CALL_VC_DUMP_CALLDETAILS(&pagent->call_manager);
		_vc_core_cm_clear_call_object(&objectInfo);
		if (FALSE == _vc_core_cm_get_call_object(&pagent->call_manager, call_handle, &objectInfo)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Call Already Cleared for Call Handle = %d", call_handle);

			/*
			 * Because of _vc_core_tapi_rqst_answer_call( .., VC_ANSWER_HOLD_ACTIVE_AND_ACCEPT,.. ) inside _vc_core_tapi_event_handle_call_end_event(),
			 * pagent->call_manager is cleared. so, we didn't send VC_CALL_NORMAL_END to call-ui.
			 * so we should send this event to call-ui.
			 */
			{
				voice_call_end_cause_type_t end_cause_type;
				_vc_core_tapi_event_get_end_cause_type(pagent, noti_id, tapi_cause, &end_cause_type);
				_vc_core_ca_send_event_to_client(pagent, VC_CALL_NORMAL_END, call_handle, end_cause_type, NULL);
			}
		} else {
			_vc_core_ca_send_event_to_client(pagent, VC_CALL_NORMAL_END, objectInfo.call_handle, objectInfo.end_cause_type, NULL);
		}
	} else if (strcmp(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_ACTIVE) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "event_type == TAPI_NOTI_VOICE_CALL_STATUS_ACTIVE...");
		call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
		TelCallStatusActiveNoti_t callActiveInfo;

		if (data != NULL) {
			memset(&callActiveInfo, 0, sizeof(TelCallStatusActiveNoti_t));
			memcpy(&callActiveInfo, data, sizeof(TelCallStatusActiveNoti_t));
			call_handle = callActiveInfo.id;
		}
		CALL_ENG_DEBUG(ENG_DEBUG, "IO State: %d", pagent->io_state);
		CALL_ENG_DEBUG(ENG_DEBUG, "Connected Call Handle = %d", call_handle);

		if (call_handle == _vc_core_cm_get_incoming_call_handle(&pagent->call_manager)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Incoming call is being connected...");
		} else if (call_handle == _vc_core_cm_get_outgoing_call_handle(&pagent->call_manager)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call is being connected...");
		} else {
			CALL_ENG_DEBUG(ENG_ERR, "invalid connected event Call Handle = %d", call_handle);

			if ((VC_INVALID_CALL_INDEX != pagent->call_manager.mtcall_index) || (VC_INVALID_CALL_INDEX != pagent->call_manager.setupcall_info.mocall_index)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "incoming/outgoin calls call exits, invalid call handle [PROBLEM]");
				CALL_VC_DUMP_CALLDETAILS(&pagent->call_manager);

				assert(0);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "No pending calls to connect, ignoreing connect event for call handle= %d", call_handle);
				return;
			}
			return;
		}

		/*Handle Connected Call Event */
		_vc_core_tapi_event_handle_call_connect_event(pagent, call_handle);
		CALL_ENG_KPI("TAPI_NOTI_VOICE_CALL_STATUS_ACTIVE done");
	} else if (strcmp(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_DIALING) == 0) {
		/*Dialing IND*/
		call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
		TelCallStatusDialingNoti_t callDialingInfo;
		call_vc_call_objectinfo_t objectInfo;

		CALL_ENG_DEBUG(ENG_DEBUG, "Data Received for DIALING_IND is %p", data);

		if (TRUE == _vc_core_cm_get_outgoing_call_info(&pagent->call_manager, &objectInfo)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "MO call index (%d)",objectInfo.call_id);
		} else {
			CALL_ENG_DEBUG(ENG_ERR, "EXCEPTION:Outgoing call Info Missing..");
		}


		/* tapicallback data =  TelCallStatusHeldNoti_t */
		if (data != NULL) {
			memset(&callDialingInfo, 0, sizeof(TelCallStatusDialingNoti_t));
			memcpy(&callDialingInfo, data, sizeof(TelCallStatusDialingNoti_t));
			call_handle = callDialingInfo.id;
			CALL_ENG_DEBUG(ENG_DEBUG, "Received Call Handle = %d", call_handle);
		}

		if (VC_INOUT_STATE_OUTGOING_WAIT_ORIG != pagent->io_state) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Io State not in WAIT_ORIG, current io state is : %d", pagent->io_state);
			return;
		}

		/* Get the outgoing call handle from CallManger and check */
		if ((VC_TAPI_INVALID_CALLHANDLE == call_handle)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call does not exist or call_handle doesn't match");
			return;
		}

		/* Set the Call Handle to the CallbObject for future reference */
		objectInfo.call_handle = call_handle;
		_vc_core_cm_set_outgoing_call_info(&pagent->call_manager, &objectInfo);

		_vc_core_tapi_event_handle_originated_event(pagent, call_handle);
		CALL_ENG_KPI("TAPI_NOTI_VOICE_CALL_STATUS_DIALING done");
	} else if (strcmp(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_ALERT) == 0) {
		call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
		call_vc_handle mo_call_handle = VC_TAPI_INVALID_CALLHANDLE;
		TelCallStatusAlertNoti_t callAlertInfo;

		CALL_ENG_KPI("TAPI_NOTI_VOICE_CALL_STATUS_ALERT START");
		/*There are possiblities, that TAPI issued the Alert Notification and it is pending in the gmain loop, but meanwhile, the call
		   is released by the user - so ignore the event if it doesn't match with the IN OUT Wait state */
		if (VC_INOUT_STATE_OUTGOING_WAIT_ALERT != pagent->io_state) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Io State not in WAIT_ORIG, current io state is : %d", pagent->io_state);
			return;
		}

		if (data != NULL) {
			memset(&callAlertInfo, 0, sizeof(TelCallStatusAlertNoti_t));
			memcpy(&callAlertInfo, data, sizeof(TelCallStatusAlertNoti_t));
			call_handle = callAlertInfo.id;
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "Alert Call Handle = %d", call_handle);

		mo_call_handle = _vc_core_cm_get_outgoing_call_handle(&pagent->call_manager);
		CALL_ENG_DEBUG(ENG_DEBUG, "MO Call Handle = %d", mo_call_handle);

		/*Get the outgoing call handle from CallManger and check*/
		if ((VC_TAPI_INVALID_CALLHANDLE == call_handle) || (mo_call_handle != call_handle)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing call does not exist or call_handle doesn't match");
			return;
		}

		_vc_core_tapi_event_handle_alert_event(pagent, call_handle);
		CALL_ENG_KPI("TAPI_NOTI_VOICE_CALL_STATUS_ALERT done");
	} else if (strcmp(noti_id, TAPI_NOTI_VOICE_CALL_STATUS_INCOMING) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_VOICE_CALL_STATUS_INCOMING is not used.");
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_ACTIVE) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_ACTIVE");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_ACTIVATE, 0, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_HELD) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_HELD");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_HOLD, 0, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_TRANSFERED) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_TRANSFERRED");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_TRANSFER, 0, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_JOINED) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_JOINED");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_SETUPCONFERENCE, 0, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_WAITING) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_WAITING");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_WAITING, 0, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_FORWARDED) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_FORWARDED");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_FORWARD, VC_FRWD_IND_INCOM_IS_FRWD, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_BARRED_OUTGOING) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_BARRED_OUTGOING");
		pagent->barring_ind_type = VC_BARR_IND_ALL;
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_CUG) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_CUG");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_CUGINFO, 0, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_CALL_NAME_INFORMATION) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_CALL_NAME_INFORMATION");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_CALLINGNAMEINFO, 0, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_DEFLECTED) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_DEFLECTED");
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_FORWARD_UNCONDITIONAL) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_FORWARD_UNCONDITIONAL");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_SSNOTIFY, VC_SSNOTIFY_IND_CFU, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_INFO_FORWARD_CONDITIONAL) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_INFO_FORWARD_CONDITIONAL");
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_IND_SSNOTIFY, VC_SSNOTIFY_IND_ALL_COND_FORWARDING, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_SOUND_WBAMR) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_SOUND_WBAMR");
		TelCallSoundWbamrNoti_t wbamrInfo;
		int wbamr_status = FALSE;

		if (data != NULL) {
			memset(&wbamrInfo, 0, sizeof(TelCallSoundWbamrNoti_t));
			memcpy(&wbamrInfo, data, sizeof(TelCallSoundWbamrNoti_t));
		}

		CALL_ENG_DEBUG(ENG_DEBUG, "WBAMR is %d", wbamrInfo);
		if (wbamrInfo == TAPI_CALL_SOUND_WBAMR_STATUS_ON) {
			wbamr_status = TRUE;
		}

		_vc_core_ca_send_event_to_client(pagent, VC_CALL_NOTI_WBAMR, wbamr_status, 0, NULL);
	} else if (strcmp(noti_id, TAPI_NOTI_CALL_SOUND_NOISE_REDUCTION) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TAPI_NOTI_CALL_SOUND_NOISE_REDUCTION");
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "ERROR!! Noti is not defined");
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "tapi noti(%s) processed done.", noti_id);

	return;
}

static gboolean __call_vc_outgoingcall_endhandle(call_vc_callagent_state_t *pagent, call_vc_handle call_handle, const char *type, TelTapiEndCause_t tapi_cause)
{
	call_vc_call_objectinfo_t objectInfo;
	voice_call_end_cause_type_t endcause_type = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL(pagent != NULL);

	_vc_core_cm_clear_call_object(&objectInfo);
	_vc_core_cm_get_call_object(&pagent->call_manager, call_handle, &objectInfo);

	/*Inform Client App about MO Call Disconnect */
	_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_OUTGOING_ABORTED);
	_vc_core_tapi_event_get_end_cause_type(pagent, type, tapi_cause, &endcause_type);
	_vc_core_ca_send_event_to_client(pagent, VC_CALL_OUTGOING_END, call_handle, (int)endcause_type, NULL);

	/* Response call setup result to SAT if this is SAT call */
	if (VC_CALL_ORIG_TYPE_SAT == objectInfo.call_type) {
		/*Cancelled by user */
		if (VC_CALL_STATE_CANCELLED == objectInfo.state) {
			_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_ME_CLEAR_DOWN_BEFORE_CONN);
		} else {	/* Disconnected by Network */

			call_vc_satsetup_info_t *pcall_vc_satcall_info = NULL;

			pcall_vc_satcall_info = (call_vc_satsetup_info_t *) &(pagent->call_manager.setupcall_info.satcall_setup_info);

			if (FALSE == pcall_vc_satcall_info->redial) {
				/*Send only if SAT redial is not enabled */
				_vc_core_ca_send_sat_response(pagent, SAT_RQST_SETUP_CALL, CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND);
			}
		}

		/*Free SAT Icon data if available */
		if (pagent->call_manager.setupcall_info.satcall_setup_info.psat_rgb_data != NULL) {
			free(pagent->call_manager.setupcall_info.satcall_setup_info.psat_rgb_data);
			pagent->call_manager.setupcall_info.satcall_setup_info.psat_rgb_data = NULL;
		}
	}

	/* SS: 1 send, while outgoing is CONNECTING state... */
	if (CALL_VC_CA_STATE_SS_WAIT_RELEASE_ALL_ACTIVECALL == pagent->callagent_state) {
		gboolean bActiveCall = _vc_core_cm_isexists_active_call(&pagent->call_manager);
		gboolean bHoldCall = _vc_core_cm_isexists_held_call(&pagent->call_manager);

		if (FALSE == bActiveCall) {
			if (TRUE == bHoldCall) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Continue SS Action");

				_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_NONE);

				if (_vc_core_tapi_rqst_retrieve_call(pagent) == TRUE) {
					_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_UNHOLD);
				} else {
					_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
				}
			} else {
				_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
			}
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "active call must not exist!");
			_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
		}
	}

	return TRUE;
}

static gboolean __call_vc_incoming_call_end_idle_cb(gpointer puser_data)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)puser_data;
	call_vc_handle call_handle = -1;

	CALL_ENG_DEBUG(ENG_DEBUG, "Ending the Incoming Call in Idle Callback");
	/*Send the Incoming end indication to the client, only if the io state is in VC_INOUT_STATE_INCOME_END */
	if (VC_INOUT_STATE_INCOME_END == pagent->io_state) {
		call_handle = _vc_core_cm_get_incoming_call_handle(&pagent->call_manager);
		if (-1 != call_handle) {
			__call_vc_incomingcall_endhandle(pagent, call_handle);
		}
	}
	return FALSE;
}

static gboolean __call_vc_incomingcall_endhandle(call_vc_callagent_state_t *pagent, call_vc_handle call_handle)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "");

	CALL_VC_DUMP_CALLDETAILS(&pagent->call_manager);

	if (TRUE == _vc_core_ca_send_event_to_client(pagent, VC_CALL_INCOM_END, call_handle, 0, NULL)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "client call back returned TRUE Removing Inomcing call Object");

		/*Remove Call Object Once the Incoming Call Got Ended*/
		_vc_core_cm_remove_call_object(&pagent->call_manager, call_handle);

		CALL_VC_DUMP_CALLDETAILS(&pagent->call_manager);

		/*Finally Move the IO State None*/
		_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_NONE);
	}
	return TRUE;
}

static gboolean __call_vc_is_callagent_waitstate(call_vc_callagent_state_t *pagent)
{

	switch (pagent->callagent_state) {
	case CALL_VC_CA_STATE_WAIT_SPLIT:
	case CALL_VC_CA_STATE_WAIT_DROP:
	case CALL_VC_CA_STATE_WAIT_SWAP:
	case CALL_VC_CA_STATE_WAIT_HOLD:
	case CALL_VC_CA_STATE_WAIT_UNHOLD:
	case CALL_VC_CA_STATE_WAIT_JOIN:
	case CALL_VC_CA_STATE_WAIT_TRANSFER_CNF:
	case CALL_VC_CA_STATE_WAIT_TRANSFER_CALLEND:
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_ACTIVECALL:
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_HOLDCALL:
	case CALL_VC_CA_STATE_SS_WAIT_RELEASE_ALL_ACTIVECALL:
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS:
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SETUP:
	case CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SWITCH_TO_VIDEO_CALL:
		{
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
}

/**
 * This function checks whether dtmf is possible
 *
 * @return		This function returns TRUE if dtmf is possible or else FALSE
 * @param[in]		pcall_agent			Pointer to the call agent structure
 */
static gboolean __vc_core_is_dtmf_possible(call_vc_callagent_state_t *pcall_agent)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_agent != NULL);
	CALL_ENG_DEBUG(ENG_DEBUG, "");

	if (VC_INOUT_STATE_NONE != pcall_agent->io_state) {
		CALL_ENG_DEBUG(ENG_DEBUG, "__vc_core_is_dtmf_possible, io_state not idle");
		return FALSE;
	}

	if (CALL_VC_CA_STATE_NORMAL != pcall_agent->callagent_state) {
		CALL_ENG_DEBUG(ENG_DEBUG, "__vc_core_is_dtmf_possible, callagent_state not idle");
		return FALSE;
	}

	if (FALSE == _vc_core_cm_isexists_active_call(&pcall_agent->call_manager)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "NO Active Calls available to send DTMF");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "DTMF Possible");
	return TRUE;
}

/**
 * This function checks the voicecall engine's idle status and send VC_ACTION_NO_ACTIVE_TASK to client if engine is idle
 *
 * @return		void
 * @param[in]		pcall_agent	Pointer to the call agent structure
 */
void __vc_core_check_engine_active_task(call_vc_callagent_state_t *pcall_agent)
{
	VOICECALL_RETURN_IF_FAIL(pcall_agent != NULL);
	if (_vc_core_ca_check_end(pcall_agent)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Engine is Idle, Informing the Client");
		_vc_core_ca_send_event_to_client(pcall_agent, VC_ACTION_NO_ACTIVE_TASK, 0, 0, NULL);
	}
}

/**
* This function ends only the connected call corresponding to the given call handle
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle		Call handle of the call to be ended
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also following functions
*				- _vc_core_engine_make_call
*				- _vc_core_engine_end_call
*				- _vc_core_engine_end_call_bycallId
*/
voicecall_error_t _vc_core_engine_end_call_byhandle(voicecall_engine_t *pvoicecall_agent, int call_handle)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(call_handle >= 0, ERROR_VOICECALL_INVALID_ARGUMENTS);

	return (TRUE == _vc_core_tapi_rqst_end_call_by_callhandle(pagent, call_handle)) ? ERROR_VOICECALL_NONE : ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

/**
* This function ends a call corresponding to the given call ID
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_id			call id of the call to be ended
* @remarks		pvoicecall_agent cannot be NULL
*				call_id shall take only values between 1 to 7
* @see			See also following functions
*				- _vc_core_engine_make_call
*				- _vc_core_engine_end_call
*				- _vc_core_engine_end_call_byhandle
*/
voicecall_error_t _vc_core_engine_end_call_bycallId(voicecall_engine_t *pvoicecall_agent, int call_id)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	call_vc_groupstate_t call_group_state = CALL_VC_GROUP_STATE_NONE;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL((call_id >= 1 && call_id <= 7), ERROR_VOICECALL_INVALID_ARGUMENTS);

	call_handle = _vc_core_cm_get_call_handle_ingroup_bycallId(&pagent->call_manager, call_id);
	if (VC_TAPI_INVALID_CALLHANDLE == call_handle) {
		return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
	}

	call_group_state = _vc_core_cm_get_group_state_callid(&pagent->call_manager, call_id);
	if (CALL_VC_GROUP_STATE_ACTIVE == call_group_state) {

		if (-1 == _vc_core_cm_get_active_group_index(&pagent->call_manager)) {
			return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
		}

		if (TRUE == _vc_core_tapi_rqst_end_call_by_callhandle(pagent, call_handle)) {
			_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_DROP);
			return ERROR_VOICECALL_NONE;
		}
	} else if (CALL_VC_GROUP_STATE_HOLD == call_group_state) {
		int held_group_member_num = 0;
		int held_group_index = -1;

		held_group_index = _vc_core_cm_get_held_group_index(&pagent->call_manager);
		if (-1 == held_group_index) {
			return FALSE;
		}

		held_group_member_num = _vc_core_cm_get_member_count_ingroup(&pagent->call_manager, held_group_index);
		if (held_group_member_num > 1) {
			/*Individual calls cannot be ended when the conf call in held state */
			return ERROR_VOICECALL_OPERATION_NOT_ALLOWED;
		}

		if (TRUE == _vc_core_tapi_rqst_end_call_by_callhandle(pagent, call_handle)) {
			_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_DROP);
			return ERROR_VOICECALL_NONE;
		}
	}

	return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
}

/**
* This function ends the call according to the given end call type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	end_call_type		End Call Type
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also following functions
*				- _vc_core_engine_make_call
*				- _vc_core_engine_end_call_byhandle
*				- _vc_core_engine_end_call_bycallId
*				.
*/
voicecall_error_t _vc_core_engine_end_call(voicecall_engine_t *pvoicecall_agent, _vc_core_engine_end_call_type_t end_call_type)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	gboolean bret_val = FALSE;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*If Call Agent is waiting for any of the release event then ignore the end call request */
	if ((CALL_VC_CA_STATE_WAIT_RELEASE_ALL_ACTIVECALL == pagent->callagent_state) || (CALL_VC_CA_STATE_WAIT_RELEASE_ALL_HOLDCALL == pagent->callagent_state) || (CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS == pagent->callagent_state)) {
		return ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "end_call_type = %d", end_call_type);

	switch (end_call_type) {
	case VC_END_OUTGOING_CALL:
		{
			bret_val = _vc_core_tapi_rqst_release_outgoing_call(pagent);
		}
		break;
	case VC_END_INCOMING_CALL:
		{
			bret_val = _vc_core_tapi_rqst_release_incoming_call(pagent);
		}
		break;
	case VC_END_ACTIVE_OR_HELD_CALLS:
		{
			bret_val = _vc_core_tapi_rqst_end_call(pagent);
		}
		break;
	case VC_END_ALL_ACTIVE_CALLS:
		{
			bret_val = _vc_core_ca_end_active_calls(pagent);
		}
		break;
	case VC_END_ALL_HELD_CALLS:
		{
			bret_val = _vc_core_ca_end_held_calls(pagent);
		}
		break;
	case VC_END_ALL_CALLS:
		{
			bret_val = _vc_core_ca_end_all_calls(pagent);
		}
		break;
	default:
		return ERROR_VOICECALL_INVALID_CALL_TYPE;
	}

	return (TRUE == bret_val) ? ERROR_VOICECALL_NONE : ERROR_VOICECALL_INCOMPLETE;
}

/**
* This function does the explicit call transfer
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
*/
voicecall_error_t _vc_core_engine_transfer_calls(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	if (CALL_VC_CA_STATE_WAIT_TRANSFER_CNF == pagent->callagent_state) {
		return ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
	} else if (FALSE == _vc_core_ca_is_transfer_call_possible(pagent)) {
		return ERROR_VOICECALL_TRANSFER_CALL_NOT_POSSIBLE;
	} else {
		if (TRUE == _vc_core_tapi_rqst_transfer_call(pagent)) {
			_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_TRANSFER_CNF);
			return ERROR_VOICECALL_NONE;
		}
	}

	return ERROR_VOICECALL_TRANSFER_FAILED;
}

/**
* This function swaps the active and held calls if any available
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also the following APIs
*				- _vc_core_engine_hold_call
*				- _vc_core_engine_retrieve_call
*
*/
voicecall_error_t _vc_core_engine_swap_calls(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*If call agent is in any of the wait states, then ignore the request */
	if (TRUE == __call_vc_is_callagent_waitstate(pagent)) {
		return ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
	}

	if (TRUE == _vc_core_tapi_rqst_swap_calls(pagent)) {
#ifdef VC_WITHOUT_SWAP_CNF
		_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_SWAP_HOLD_OR_ACTIVATE);
#endif
		return ERROR_VOICECALL_NONE;
	}

	return ERROR_VOICECALL_SWAP_FAILED;
}

/**
* This function puts the active call if any on hold
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	        Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
* @see			voicecall_retreive_call
*/
voicecall_error_t _vc_core_engine_hold_call(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	if (TRUE == __call_vc_is_callagent_waitstate(pagent)) {
		return ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
	}
#ifdef _CPHS_DEFINED_
	if (TRUE == _vc_core_svcall_cphs_csp_get_status(pagent, VC_CPHS_CSP_HOLD)) {
		if (TRUE == _vc_core_tapi_rqst_hold_call(pagent)) {
			_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_HOLD);
			return ERROR_VOICECALL_NONE;
		}
	} else {
		return ERROR_VOICECALL_HOLD_NOT_SUPPORTED;
	}
#else
	if (TRUE == _vc_core_tapi_rqst_hold_call(pagent)) {
		_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_HOLD);
		return ERROR_VOICECALL_NONE;
	}
#endif
	return ERROR_VOICECALL_HOLD_FAILED;
}

/**
* This function retrieves/activates the held call
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
* @see			_vc_core_engine_hold_call
*/
voicecall_error_t _vc_core_engine_retrieve_call(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	if (TRUE == __call_vc_is_callagent_waitstate(pagent)) {
		return ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
	}

	if (TRUE == _vc_core_tapi_rqst_retrieve_call(pagent)) {
		_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_UNHOLD);
		return ERROR_VOICECALL_NONE;
	}
	return ERROR_VOICECALL_RETREIVE_FAILED;
}

/**
* This function sets up a conference beween the currently available active and held calls
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	        Handle to voicecall engine
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also the following APIs
*				- _vc_core_engine_private_call
*				- _vc_core_engine_private_call_by_callid
*
*/
voicecall_error_t _vc_core_engine_setup_conference(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*If call agent is in any of the wait states, then ignore the request */
	if (CALL_VC_CA_STATE_WAIT_JOIN == pagent->callagent_state) {
		return ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
	} else if (FALSE == _vc_core_ca_is_conf_call_possible(pagent)) {
		return ERROR_VOICECALL_CONF_NOT_POSSIBLE;
	} else {
		if (TRUE == _vc_core_tapi_rqst_join_calls(pagent)) {
			_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_WAIT_JOIN);
			return ERROR_VOICECALL_NONE;
		}
	}
	return ERROR_VOICECALL_SETUP_CONF_FAILED;
}

/**
* This function makes a private call to the call member corressponding to the given call id.
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_id			Call ID of the call to be made private
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also the following APIs
*				- _vc_core_engine_setup_conference
*				- _vc_core_engine_private_call
*
*/
voicecall_error_t _vc_core_engine_private_call_by_callid(voicecall_engine_t *pvoicecall_agent, int call_id)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL((call_id >= 1 && call_id <= 7), ERROR_VOICECALL_INVALID_ARGUMENTS);

	if (CALL_VC_CA_STATE_WAIT_SPLIT == pagent->callagent_state) {
		return ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
	} else if (FALSE == _vc_core_ca_is_private_call_possible(pagent)) {
		return ERROR__vc_core_engine_private_call_NOT_POSSIBLE;
	} else {
		call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;

		call_handle = _vc_core_cm_get_call_handle_ingroup_bycallId(&pagent->call_manager, call_id);

		if (VC_TAPI_INVALID_CALLHANDLE != call_handle) {
			return (TRUE == _vc_core_tapi_rqst_private_call(pagent, call_handle)) ? ERROR_VOICECALL_NONE : ERROR__vc_core_engine_private_call_FAILED;
		}
	}

	return ERROR_VOICECALL_INVALID_CALLID;
}

/**
* This function makes a private call to the given call member from the currently available active conference call
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	call_handle		call handle of the call to be made private
* @remarks		pvoicecall_agent cannot be NULL
* @see			See also the following APIs
*				- _vc_core_engine_setup_conference
*				- _vc_core_engine_private_call_by_callid
*
*/
voicecall_error_t _vc_core_engine_private_call(voicecall_engine_t *pvoicecall_agent, int call_handle)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(call_handle >= 0, ERROR_VOICECALL_INVALID_ARGUMENTS);

	if (CALL_VC_CA_STATE_WAIT_SPLIT == pagent->callagent_state) {
		return ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS;
	} else if (FALSE == _vc_core_ca_is_private_call_possible(pagent)) {
		return ERROR__vc_core_engine_private_call_NOT_POSSIBLE;
	}

	return (TRUE == _vc_core_tapi_rqst_private_call(pagent, call_handle)) ? ERROR_VOICECALL_NONE : ERROR_VOICECALL_REQUEST_FAILED;
}

/**
* This function rejects the incoming call if any
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	budub			User Determined User Busy - TRUE, Else - FALSE
* @exception		In case of exceptions return value contains appropriate error code.
* @remarks		pvoicecall_agent cannot be NULL
* @see			_vc_core_engine_answer_call
*/
voicecall_error_t _vc_core_engine_reject_call(voicecall_engine_t *pvoicecall_agent, gboolean budub)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	int error_code = -1;
	gboolean ret = FALSE;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	ret = _vc_core_tapi_rqst_reject_mt_call(pagent, budub, &error_code);

	return (TRUE == ret) ? ERROR_VOICECALL_NONE : error_code;
}

/**
* This function answers a call according to the given answer type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	answer_type		The answer type to be used
* @remarks		pvoicecall_agent and pcall_handle cannot be NULL
* @see			_vc_core_engine_reject_call
*/
voicecall_error_t _vc_core_engine_answer_call(voicecall_engine_t *pvoicecall_agent, voicecall_answer_type_t answer_type)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	int error_code = 0;
	gboolean ret = FALSE;

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
#ifdef RELEASE_ALL_AND_ACCEPT_SUPPORT
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(answer_type, VC_ANSWER_NORMAL, VC_ANSWER_RELEASE_ALL_AND_ACCEPT, ERROR_VOICECALL_INVALID_ARGUMENTS);
#else
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(answer_type, VC_ANSWER_NORMAL, VC_ANSWER_RELEASE_HOLD_AND_ACCEPT, ERROR_VOICECALL_INVALID_ARGUMENTS);
#endif
	ret = _vc_core_tapi_rqst_answer_call(pcall_agent, answer_type, &error_code);

	return (TRUE == ret) ? ERROR_VOICECALL_NONE : error_code;
}

/**
* This function sends the given dtmf digits
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	pdtmf_string		dtmf digits to be sent
* @remarks		pvoicecall_agent and pdtmf_string cannot be NULL
*				pdtmf_string shall only accept strings containing the digit value (0-9,A,B,C,D,*,#)
*
*/
voicecall_error_t _vc_core_engine_send_dtmf(voicecall_engine_t *pvoicecall_agent, char *pdtmf_string)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(pdtmf_string != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	if (FALSE == _vc_core_util_isvalid_dtmf_number(pdtmf_string)) {
		_vc_core_ca_send_event_to_client(pagent, VC_ERROR_OCCURED, ERROR_VOICECALL_INVALID_DTMF_CHAR, -1, NULL);
		return ERROR_VOICECALL_INVALID_DTMF_CHAR;
	}

	return (TRUE == _vc_core_tapi_rqst_start_dtmf(pagent, pdtmf_string)) ? ERROR_VOICECALL_NONE : ERROR_VOICECALL_DTMF_FAILED;
}

/**
* This function sends response to sat based on the given sat response type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	        Handle to voicecall engine
* @param[in]	sat_rqst_resp_type  sat rqst/response type sent by client
* @param[in]	sat_response_type  sat response type to be sent to SAT
*/
voicecall_error_t _vc_core_engine_send_sat_response(voicecall_engine_t *pvoicecall_agent, voicecall_engine_sat_rqst_resp_type sat_rqst_resp_type, call_vc_sat_reponse_type_t sat_response_type)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(sat_rqst_resp_type, SAT_RQST_SETUP_CALL, SAT_RESP_SETUP_CALL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(sat_response_type, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND, CALL_VC_ME_RET_SUCCESS, ERROR_VOICECALL_INVALID_ARGUMENTS);

	return (TRUE == _vc_core_ca_send_sat_response(pagent, sat_rqst_resp_type, sat_response_type)) ? ERROR_VOICECALL_NONE : ERROR_VOICECALL_REQUEST_FAILED;
}

/**
* This function finalizes the voiecall engine and removes all allocated resources
*
* @return		nothing
* @param[in]		pvoicecall_agent	Handle to Voicecall Engine
* @remarks		pvoicecall_agent cannot be NULL
*/
void _vc_core_engine_engine_finish(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_IF_FAIL(pvoicecall_agent != NULL);

	/*Unsubscribe Events */
	CALL_ENG_DEBUG(ENG_DEBUG, "Unsubscribing Events");

	tel_deinit(pcall_agent->tapi_handle);

	VOICECALL_RETURN_IF_FAIL(pcall_agent != NULL);
	_vc_core_ca_finish_agent(pcall_agent);
}

#ifdef _SAT_MENU_
/**
* This function requests SAT Engine to setup SIM services Menu
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @remarks		Voicecall Engine only requests the SAT engine to display the menu.
*/
voicecall_error_t voicecall_request_sat_menu(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	TelSatSetupMenuInfo_t sim_menu;	/*LiMo SAT*/

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	CALL_ENG_DEBUG(ENG_DEBUG, "");
	memset(&sim_menu, 0, sizeof(TelSatSetupMenuInfo_t));
	/*LiMo SAT*/
	if (FALSE == TelTapiSatGetMainMenuList(&sim_menu)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "TelTapiSatGetMainMenuList failed");
		return ERROR_VOICECALL_REQUEST_FAILED;
	}

	return ERROR_VOICECALL_NONE;
}

/**
* This function retreives the SIM Menu Title from the SAT Engine
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[out]	title					contains the sat menu title on sucess
*/
voicecall_error_t voicecall_request_sat_menu_title(voicecall_engine_t *pvoicecall_agent, char *title)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	TelSatMainMenuTitleInfo_t sat_menu_title;	/*LiMo SAT*/

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(title != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	memset(&sat_menu_title, 0, sizeof(TelSatMainMenuTitleInfo_t));
	if (TRUE == TelTapiSatGetMainMenuTitle(&sat_menu_title)) {
		if (TRUE == sat_menu_title.bIsMainMenuPresent) {
			strcpy(title, (char *)sat_menu_title.mainMenuTitle.string);
			return ERROR_VOICECALL_NONE;
		}
	}

	return ERROR_VOICECALL_REQUEST_FAILED;
}
#endif

/**
* This function prepares the engine for the redial call. It preserves the previsouly made call object to used for the next make call
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent	Handle to Voicecall Engine
* @param[in]		call_handle		call handle
* @remarks		If this API is used, _vc_core_engine_prepare_call is not reqired for making the call again. The last prepared call details will
*				be used for the redialling. Application has to just use _vc_core_engine_make_call API to redial the call
*/
voicecall_error_t _vc_core_engine_prepare_redial(voicecall_engine_t *pvoicecall_agent, int call_handle)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_call_objectinfo_t call_object;
	int mo_call_handle = -1;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(call_handle >= 0, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*Check the validity of the call handle */
	mo_call_handle = _vc_core_cm_get_outgoing_call_handle(&pagent->call_manager);
	if ((mo_call_handle == -1) || (mo_call_handle != call_handle)) {
		return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
	}

	if (FALSE == _vc_core_cm_get_call_object(&pagent->call_manager, call_handle, &call_object)) {
		return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
	}

	/*Set the callobject status */
	call_object.state = VC_CALL_STATE_REDIAL;

	/*Reintialize Call ID */
	call_object.call_id = 0;
	_vc_core_cm_set_call_object(&pagent->call_manager, &call_object);

	/*Set Engine IO State */
	_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_OUTGOING_SHOW_REDIALCAUSE);

	/*todo Set SAT Redial Data */

	return ERROR_VOICECALL_NONE;
}

#ifdef _OLD_SAT_
/**
* This function checks whether SAT redial duration is valid
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent	Handle to Voicecall Engine
* @param[out]	bredial_duration	Contains TRUE if SAT redial duration is enabled, FALSE otherwise
* @remarks		pvoicecall_agent and bredial_duration cannot be NULL
*/
voicecall_error_t voicecall_get_sat_redial_duration_status(voicecall_engine_t *pvoicecall_agent, gboolean *bredial_duration)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_satsetup_info_t *pcall_vc_satcall_info = NULL;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bredial_duration != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	pcall_vc_satcall_info = (call_vc_satsetup_info_t *) &(pagent->call_manager.setupcall_info.satcall_setup_info);

	CALL_ENG_DEBUG(ENG_DEBUG, "SAT Redial Duration Status= %d", pcall_vc_satcall_info->bduration);

	*bredial_duration = pcall_vc_satcall_info->bduration;
	return ERROR_VOICECALL_NONE;
}

/**
* This function sets the current duration and retrieves the modified remaining SAT redial duration
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent		Handle to Voicecall Engine
* @param[out]	remaining_duration		remaining sat duration
* @remarks		pvoicecall_agent and remaining_duration cannot be NULL
*/
voicecall_error_t voicecall_get_set_sat_remaining_duration(voicecall_engine_t *pvoicecall_agent, long *remaining_duration)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_satsetup_info_t *pcall_vc_satcall_info = NULL;

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(remaining_duration != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	pcall_vc_satcall_info = (call_vc_satsetup_info_t *) &(pcall_agent->call_manager.setupcall_info.satcall_setup_info);
	*remaining_duration = pcall_vc_satcall_info->remaining_duration;
	CALL_ENG_DEBUG(ENG_DEBUG, "Remaining Duration: %ld", *remaining_duration);

	return ERROR_VOICECALL_NONE;
}
#endif

voicecall_error_t _vc_core_engine_get_sat_dtmf_hidden_mode(voicecall_engine_t *pvoicecall_agent, gboolean *bhidden_mode)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	call_vc_satsetup_info_t *pcall_vc_satcall_info = NULL;

	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_FAIL(bhidden_mode != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	pcall_vc_satcall_info = (call_vc_satsetup_info_t *) &(pagent->call_manager.setupcall_info.satcall_setup_info);

	CALL_ENG_DEBUG(ENG_DEBUG, "SAT Hidden Mode= %d", pcall_vc_satcall_info->satengine_dtmf_data.bIsHiddenMode);

	*bhidden_mode = pcall_vc_satcall_info->satengine_dtmf_data.bIsHiddenMode;
	return ERROR_VOICECALL_NONE;
}

/**
* This function changes the voice audio path
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[in]		audio_path		audio path to be changed
* @remarks		pvoicecall_agent cannot be NULL
*/
voicecall_error_t _vc_core_engine_change_audio_path(voicecall_engine_t *pvoicecall_agent, voicecall_audio_path_t audio_path, gboolean bextra_volume)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	TelCallSoundPathInfo_t tapi_sound_path;
	/*Enum for encapsulating errors from TAPI Lib */
	TapiResult_t tapi_error = TAPI_API_SUCCESS;

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*Voicecall Path Should not be modified if calls are not available */
	if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) <= 0) {
		return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
	}

	switch (audio_path) {
	case VC_AUDIO_PATH_HANDSET:
		{
			tapi_sound_path.path = TAPI_SOUND_PATH_HANDSET;
		}
		break;
	case VC_AUDIO_PATH_HEADSET:
		{
			tapi_sound_path.path = TAPI_SOUND_PATH_HEADSET;
		}
		break;
	case VC_AUDIO_PATH_HANDSFREE:
		{
			tapi_sound_path.path = TAPI_SOUND_PATH_HANDSFREE;
		}
		break;
	case VC_AUDIO_PATH_BLUETOOTH:
		{
			tapi_sound_path.path = TAPI_SOUND_PATH_BLUETOOTH;
		}
		break;
	case VC_AUDIO_PATH_STEREO_BLUETOOTH:
		{
			tapi_sound_path.path = TAPI_SOUND_PATH_STEREO_BLUETOOTH;
		}
		break;
	case VC_AUDIO_PATH_SPK_PHONE:
		{
			tapi_sound_path.path = TAPI_SOUND_PATH_SPK_PHONE;
		}
		break;
	case VC_AUDIO_PATH_HEADSET_3_5PI:
		{
			tapi_sound_path.path = TAPI_SOUND_PATH_HEADSET_3_5PI;
		}
		break;
	default:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Invalid audio path");
			return ERROR_VOICECALL_INVALID_ARGUMENTS;
		}
		break;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "tapi_audio_path: %d(extra voluem: %d)", tapi_sound_path, bextra_volume);
	if ((bextra_volume)
		&& ((tapi_sound_path.path == TAPI_SOUND_PATH_HANDSET)
		|| (tapi_sound_path.path = TAPI_SOUND_PATH_SPK_PHONE))) {
			tapi_sound_path.ex_volume = TAPI_SOUND_EX_VOLUME_ON;
	} else {
		tapi_sound_path.ex_volume = TAPI_SOUND_EX_VOLUME_OFF;
	}

	tapi_error = tel_set_call_sound_path(pcall_agent->tapi_handle, &tapi_sound_path, _vc_core_engine_set_sound_path_resp_cb, NULL);
	if (tapi_error != TAPI_API_SUCCESS) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_set_sound_path error: %d", tapi_error);
		return ERROR_VOICECALL_TAPI_ERROR;
	}

	return ERROR_VOICECALL_NONE;
}

voicecall_error_t _vc_core_engine_set_audio_mute(voicecall_engine_t *pvoicecall_agent, gboolean bmute_audio)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	TapiResult_t error = TAPI_API_SUCCESS;
	TelSoundMuteStatus_t micmute_set = TAPI_SOUND_MUTE_STATUS_OFF; 

	micmute_set = (TRUE == bmute_audio) ? TAPI_SOUND_MUTE_STATUS_ON : TAPI_SOUND_MUTE_STATUS_OFF;

	error = tel_set_call_mute_status(pcall_agent->tapi_handle, micmute_set, _vc_core_engine_set_mute_status_resp_cb, NULL);

	if (error != TAPI_API_SUCCESS) {
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_set_sound_mute_status Error: %d", error);
		return ERROR_VOICECALL_TAPI_ERROR;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "End");
	return ERROR_VOICECALL_NONE;
}

/**
* This function sets the voice call audio volume for the given audio path type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[in]		audio_path_type		audio path for the volume to be set
* @param[in]		vol_level			volume level
* @remarks		pvoicecall_agent cannot be NULL
*/
voicecall_error_t _vc_core_engine_set_audio_volume(voicecall_engine_t *pvoicecall_agent, voicecall_audio_path_t audio_path_type, voicecall_audio_volume_t vol_level)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	/*Enum for encapsulating errors from TAPI Lib */
	TapiResult_t error = TAPI_API_SUCCESS;
	TelCallVolumeInfo_t vol_info;

	CALL_ENG_DEBUG(ENG_DEBUG, "Start! path(%d), volume(%d)", audio_path_type, vol_level);

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(audio_path_type, VC_AUDIO_PATH_HANDSET, VC_AUDIO_PATH_HEADSET_3_5PI, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(vol_level, VC_AUDIO_VOLUME_LEVEL_0, VC_AUDIO_VOLUME_LEVEL_9, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*Ignore the request if calls are not available */
	if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) <= 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "End");
		return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
	}

	vol_info.volume = vol_level;
	vol_info.type = TAPI_SOUND_TYPE_VOICE;
	switch (audio_path_type) {
	case VC_AUDIO_PATH_HEADSET:
		vol_info.device = TAPI_SOUND_DEVICE_HEADSET;
		break;
	case VC_AUDIO_PATH_BLUETOOTH:
	case VC_AUDIO_PATH_STEREO_BLUETOOTH:
		vol_info.device = TAPI_SOUND_DEVICE_BLUETOOTH;
		break;
	case VC_AUDIO_PATH_SPK_PHONE:
		vol_info.device = TAPI_SOUND_DEVICE_SPEAKER_PHONE;
		break;
	default:
		vol_info.device = TAPI_SOUND_DEVICE_RECEIVER;
		break;
	}

	error = tel_set_call_volume_info(pcall_agent->tapi_handle, &vol_info, _vc_core_engine_set_volume_resp_cb, NULL);

	if (error != TAPI_API_SUCCESS) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Tapi API Error: %d", error);
		return ERROR_VOICECALL_TAPI_ERROR;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "End");
	return ERROR_VOICECALL_NONE;
}

/**
* This function retreives the voice call audio volume for the given audio path type
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]		pvoicecall_agent		Handle to Voicecall Engine
* @param[in]		audio_path_type		audio path for the volume to be retreived
* @remarks		pvoicecall_agent cannot be NULL
*				The audio volume level will be send as a response with the below details
*				event	- VC_CALL_GET_VOLUME_RESP
*				param1	- audio_path_type
*				param2   - volume level
*				param3   - NULL
*/
voicecall_error_t _vc_core_engine_get_audio_volume(voicecall_engine_t *pvoicecall_agent, voicecall_audio_path_t audio_path_type)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	TapiResult_t error = TAPI_API_SUCCESS;
	TelSoundDevice_t volume_type = TAPI_SOUND_DEVICE_RECEIVER;

	VOICECALL_RETURN_VALUE_IF_FAIL(pcall_agent != NULL, ERROR_VOICECALL_INVALID_ARGUMENTS);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(audio_path_type, VC_AUDIO_PATH_HANDSET, VC_AUDIO_PATH_HEADSET_3_5PI, ERROR_VOICECALL_INVALID_ARGUMENTS);

	/*Ignore the request if calls are not available */
	if (_vc_core_cm_get_call_member_count(&pcall_agent->call_manager) <= 0) {
		return ERROR_VOICECALL_CALL_INFO_NOT_AVAILABLE;
	}

	switch (audio_path_type) {
	case VC_AUDIO_PATH_HEADSET:
		volume_type = TAPI_SOUND_DEVICE_HEADSET;
		break;
	case VC_AUDIO_PATH_BLUETOOTH:
	case VC_AUDIO_PATH_STEREO_BLUETOOTH:
		volume_type = TAPI_SOUND_DEVICE_BLUETOOTH;
		break;
	case VC_AUDIO_PATH_SPK_PHONE:
		volume_type = TAPI_SOUND_DEVICE_SPEAKER_PHONE;
		break;
	default:
		volume_type = TAPI_SOUND_DEVICE_RECEIVER;
		break;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "volume_type = %d", volume_type);

	pcall_agent->curr_tapi_path = volume_type;
	error = tel_get_call_volume_info(pcall_agent->tapi_handle, volume_type, TAPI_SOUND_TYPE_VOICE, _vc_core_engine_get_volume_resp_cb, NULL);

	if (error != TAPI_API_SUCCESS) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Tapi API Error: %d", error);
		return ERROR_VOICECALL_TAPI_ERROR;
	}

	return ERROR_VOICECALL_NONE;
}

/**
* This function changes the inout state of the engine to the given state
*
* @return		ERROR_VOICECALL_NONE on success or return value contains appropriate error code on failure
* @param[in]	pvoicecall_agent	Handle to voicecall engine
* @param[in]	io_state			Inout state to be set
* @remarks		pvoicecall_agent cannot be NULL
* @see			_vc_core_engine_status_get_engine_iostate
*/
voicecall_error_t _vc_core_engine_change_engine_iostate(voicecall_engine_t *pvoicecall_agent, int io_state)
{
	call_vc_callagent_state_t *pagent = (call_vc_callagent_state_t *)pvoicecall_agent;
	VOICECALL_RETURN_VALUE_IF_FAIL(pagent != NULL, ERROR_VOICECALL_INVALID_CALL_HANDLE);
	VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(io_state, VC_INOUT_STATE_NONE, VC_INOUT_STATE_INCOME_END, ERROR_VOICECALL_INVALID_CALL_HANDLE);

	if (TRUE == _vc_core_ca_change_inout_state(pagent, (voicecall_inout_state_t) io_state)) {
		return ERROR_VOICECALL_NONE;
	}

	return ERROR_VOICECALL_INVALID_ARGUMENTS;
}

voicecall_error_t _vc_core_engine_extract_phone_number(const char *source_tel_number, char *phone_number, const int buf_size)
{
	VOICECALL_RETURN_VALUE_IF_FAIL(source_tel_number != NULL, ERROR_VOICECALL_INVALID_CALL_HANDLE);
	VOICECALL_RETURN_VALUE_IF_FAIL(phone_number != NULL, ERROR_VOICECALL_INVALID_CALL_HANDLE);

	if (FALSE == _vc_core_util_extract_call_number(source_tel_number, phone_number, buf_size)) {
		_vc_core_util_strcpy(phone_number, buf_size, source_tel_number);
	}

	return ERROR_VOICECALL_NONE;
}

voicecall_error_t _vc_core_engine_set_to_default_values(voicecall_engine_t *pvoicecall_agent)
{
	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;

	CALL_ENG_DEBUG(ENG_DEBUG, "Start");
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

	CALL_VC_DUMP_CALLDETAILS(&pcall_agent->call_manager);
	CALL_ENG_DEBUG(ENG_DEBUG, "End");

	return ERROR_VOICECALL_NONE;
}

voicecall_error_t _vc_core_engine_check_incoming_handle(voicecall_engine_t *pvoicecall_agent, int call_id)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "Start Incoming handle : %d", call_id);

	call_vc_callagent_state_t *pcall_agent = (call_vc_callagent_state_t *)pvoicecall_agent;
	TapiResult_t error = TAPI_API_SUCCESS;
	TelCallStatus_t call_status;
	memset(&call_status, 0x0, sizeof(TelCallStatus_t));

	error = tel_get_call_status(pcall_agent->tapi_handle, call_id, &call_status);

	if (error != TAPI_API_SUCCESS) {
		/* If incoming call handle is not valid, terminate current incoming call */
		CALL_ENG_DEBUG(ENG_DEBUG, "tel_get_call_status Error: %d", error);
		CALL_ENG_DEBUG(ENG_DEBUG, "Adding Incoming End Event to Idle Callback");
		_vc_core_ca_change_inout_state(pcall_agent, VC_INOUT_STATE_INCOME_END);

		g_idle_add(__call_vc_incoming_call_end_idle_cb, pcall_agent);
		return ERROR_VOICECALL_TAPI_ERROR;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "End");
	return ERROR_VOICECALL_NONE;
}

void _vc_core_engine_dial_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_dial_call_resp_cb");

	call_vc_call_objectinfo_t objectInfo;
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;

	if (TAPI_CAUSE_SUCCESS != result) {
		CALL_ENG_DEBUG(ENG_DEBUG, "MO Call Dial call Failed with error cause: %d", result);

		_vc_core_cm_clear_call_object(&objectInfo);

		__call_vc_outgoingcall_endhandle(pagent, objectInfo.call_handle, TAPI_NOTI_VOICE_CALL_STATUS_IDLE, TAPI_CC_CAUSE_FACILITY_REJECTED);
		/* Need to make warning popup for abnormal status... */
	}
}

void _vc_core_engine_answer_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_answer_call_resp_cb");

	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;
	if (result != TAPI_CAUSE_SUCCESS) {
		/*If IO State is waiting for Answer Response */
		if ((VC_INOUT_STATE_INCOME_WAIT_CONNECTED == pagent->io_state) || (VC_INOUT_STATE_INCOME_WAIT_HOLD_CONNECTED == pagent->io_state) || (VC_INOUT_STATE_INCOME_WAIT_RELEASE_ACTIVE_CONNECTED == pagent->io_state)) {
			int mt_call_handle = -1;

			mt_call_handle = _vc_core_cm_get_incoming_call_handle(&pagent->call_manager);

			if (mt_call_handle != -1) {
				CALL_ENG_DEBUG(ENG_DEBUG, "mt_call_handle = %d", mt_call_handle);

				/*Send Hold Failed Notification to client UI */
				if (pagent->callagent_state == CALL_VC_CA_STATE_WAIT_HOLD) {
					_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
					_vc_core_ca_send_event_to_client(pagent, VC_ERROR_OCCURED, ERROR_VOICECALL_HOLD_FAILED, 0, NULL);
				}

				/*Send Incoming call MT End Indication to Client UI */
				_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_INCOME_END);
				g_idle_add(__call_vc_incoming_call_end_idle_cb, pagent);
			}
		}
	} else {
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_ANSWER_CNF, 0, 0, NULL);
	}
}

void _vc_core_engine_end_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_end_call_resp_cb..");

	TelCallEndCnf_t callEndInfo ;
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	TelCallEndType_t call_end_type = TAPI_CALL_END;
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;

	memset(&callEndInfo, 0, sizeof(TelCallEndCnf_t));
	memcpy(&callEndInfo, tapi_data, sizeof(TelCallEndCnf_t));

	call_handle = callEndInfo.id;
	call_end_type = callEndInfo.type;

	if (TAPI_CAUSE_SUCCESS == result) {
		/*Ignore this event as endication will be received from TAPI for the call release request */
		CALL_ENG_DEBUG(ENG_DEBUG, "Success response for tel_end_call request");
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "Call Release request failed, proceeding with Call end process");
		call_vc_call_objectinfo_t objectInfo;
		voicecall_call_state_t present_call_state = VC_CALL_STATE_NONE;
		call_vc_handle incoming_call_handle = VC_TAPI_INVALID_CALLHANDLE;
		TelTapiEndCause_t tapi_cause = TAPI_CALL_END_NO_CAUSE;

		CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d,end cause:%d", call_handle, tapi_cause);

		/*the end of incoming call rejected by callagent, because the call had come before the phone is initialized */
		if (call_handle == gphone_rejected_call) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Rejected call..phone not initialized");

			gphone_rejected_call = VC_TAPI_INVALID_CALLHANDLE;

			/*If no more calls available, End the Application */
			__vc_core_check_engine_active_task(pagent);
			return;
		}

		incoming_call_handle = _vc_core_cm_get_incoming_call_handle(&pagent->call_manager);
		present_call_state = _vc_core_cm_get_call_state(&pagent->call_manager, call_handle);

		CALL_ENG_DEBUG(ENG_DEBUG, "New Call Handle = %d, Already registered MT call handle : %d", call_handle, incoming_call_handle);
		switch (present_call_state) {
		case VC_CALL_STATE_NONE:
		case VC_CALL_STATE_ENDED:
		case VC_CALL_STATE_ENDED_FINISH:
			{
				CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d state is %d", call_handle, present_call_state);
				/*If no more calls available, End the Application */
				__vc_core_check_engine_active_task(pagent);
				return;
			}
			break;
		case VC_CALL_STATE_REJECTED:
			{
				/*End of incoming call (not registered as incoming call in CallAgent) rejected by callagent */
				if (incoming_call_handle != call_handle) {
					_vc_core_cm_remove_call_object(&pagent->call_manager, call_handle);
					CALL_ENG_DEBUG(ENG_DEBUG, "end of call rejected by callagent");

					/*If no more calls available, End the Application */
					__vc_core_check_engine_active_task(pagent);
					return;
				}
			}
			break;
		default:
			break;
		}

		/*End of the call rejected by user or by callagent (when hold is failed) */
		if ((VC_INOUT_STATE_INCOME_WAIT_RELEASE == pagent->io_state) && (incoming_call_handle == call_handle)) {
			_vc_core_cm_remove_call_object(&pagent->call_manager, call_handle);

			/*Change the In Out state to None*/
			_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_NONE);

			/*Notify Client about rejected Event*/
			_vc_core_ca_send_event_to_client(pagent, VC_CALL_REJECTED_END, call_handle, 0, NULL);

			return;
		}

		/*End of Incoming Call */
		if (incoming_call_handle == call_handle) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Adding Incoming End Event to Idle Callback");
			_vc_core_ca_change_inout_state(pagent, VC_INOUT_STATE_INCOME_END);
			/*Make sure that the End Indication is processed always after the Incoming Indication , as both are
			   processed in Idle Add Callbacks */
			g_idle_add(__call_vc_incoming_call_end_idle_cb, pagent);
			return;
		}

		/*End of Outgoing Call */
		if (_vc_core_cm_get_outgoing_call_handle(&pagent->call_manager) == call_handle) {
			__call_vc_outgoingcall_endhandle(pagent, call_handle, TAPI_NOTI_VOICE_CALL_STATUS_IDLE, tapi_cause);
			return;
		}

		/*End of Normal Connected Call */
		_vc_core_tapi_event_handle_call_end_event(pagent, "", call_handle, tapi_cause);

		CALL_VC_DUMP_CALLDETAILS(&pagent->call_manager);
		_vc_core_cm_clear_call_object(&objectInfo);
		if (FALSE == _vc_core_cm_get_call_object(&pagent->call_manager, call_handle, &objectInfo)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Call Already Cleared for Call Handle = %d", call_handle);

			/* jspark
			 * Because of _vc_core_tapi_rqst_answer_call( .., VC_ANSWER_HOLD_ACTIVE_AND_ACCEPT,.. ) inside _vc_core_tapi_event_handle_call_end_event(),
			 * pagent->call_manager is cleared. so, we didn't send VC_CALL_NORMAL_END to call-ui.
			 * so we should send this event to call-ui.
			 */
			{
				voice_call_end_cause_type_t end_cause_type;
				_vc_core_tapi_event_get_end_cause_type(pagent, "", tapi_cause, &end_cause_type);
				_vc_core_ca_send_event_to_client(pagent, VC_CALL_NORMAL_END, call_handle, end_cause_type, NULL);
			}
		} else {
			_vc_core_ca_send_event_to_client(pagent, VC_CALL_NORMAL_END, objectInfo.call_handle, objectInfo.end_cause_type, NULL);
		}
	}
	return;
}

void _vc_core_engine_hold_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_hold_call_resp_cb");
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;

	if (TAPI_CAUSE_SUCCESS == result) {
		memcpy(&call_handle, tapi_data, sizeof(call_vc_handle));
	} else {
		_vc_core_cm_get_first_active_call_handle(&pagent->call_manager, &call_handle);
	}

	if (_vc_core_tapi_event_handle_call_held_event(pagent, call_handle, result) == FALSE) {
		_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
	} else {
		/*
		   Be carefull in clearing the end call member, because _vc_core_engine_status_is_any_call_ending
		   function depends on the end call object status. If it is cleared often, the check by
		   _vc_core_engine_status_is_any_call_ending becomes invalid
		 */
		_vc_core_cm_clear_endcall_member(&pagent->call_manager);
	}
}

void _vc_core_engine_active_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_active_call_resp_cb");
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;
	TelCallActiveCnf_t callActiveInfo;

	if (TAPI_CAUSE_SUCCESS == result) {
		memset(&callActiveInfo, 0, sizeof(TelCallActiveCnf_t));
		memcpy(&callActiveInfo, tapi_data, sizeof(TelCallActiveCnf_t));
		call_handle = callActiveInfo.id;
	} else {
		_vc_core_cm_get_first_held_call_handle(&pagent->call_manager, &call_handle);
	}

	if (_vc_core_tapi_event_handle_call_retrieve_event(pagent, call_handle, result) == FALSE) {
		_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
	} else {
		/*
		   Be carefull in clearing the end call member, because _vc_core_engine_status_is_any_call_ending
		   function depends on the end call object status. If it is cleared often, the check by
		   _vc_core_engine_status_is_any_call_ending becomes invalid
		 */
		_vc_core_cm_clear_endcall_member(&pagent->call_manager);
	}
}

void _vc_core_engine_swap_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;
	TelCallSwapCnf_t callSwapInfo;

	CALL_ENG_DEBUG(ENG_DEBUG, "result:%d, handle:%d", result);

	if (TAPI_CAUSE_SUCCESS == result) {
		memset(&callSwapInfo, 0, sizeof(TelCallSwapCnf_t));
		memcpy(&callSwapInfo, tapi_data, sizeof(TelCallSwapCnf_t));
		call_handle = callSwapInfo.id;

		_vc_core_cm_swap_group_state(&pagent->call_manager);
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_SS_SWAP, call_handle, 0, NULL);
	} else {
		/*Reset the Call Agent State*/
		_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
		_vc_core_ca_send_event_to_client(pagent, VC_ERROR_OCCURED, ERROR_VOICECALL_SWAP_FAILED, 0, NULL);
	}
}

void _vc_core_engine_join_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;
	TelCallJoinCnf_t callJoinInfo;

	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_join_call_resp_cb...");

	if (TAPI_CAUSE_SUCCESS == result) {
		memset(&callJoinInfo, 0, sizeof(TelCallJoinCnf_t));
		memcpy(&callJoinInfo, tapi_data, sizeof(TelCallJoinCnf_t));
		call_handle = callJoinInfo.id;
	} else {
		call_handle = 0;
	}

	_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
	_vc_core_tapi_event_handle_call_join_event(pagent, call_handle, result);
}

void _vc_core_engine_split_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	call_vc_handle call_handle = VC_TAPI_INVALID_CALLHANDLE;
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;
	TelCallSplitCnf_t callSplitInfo;

	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_split_call_resp_cb");

	if (TAPI_CAUSE_SUCCESS == result) {
		memset(&callSplitInfo, 0, sizeof(TelCallSplitCnf_t));
		memcpy(&callSplitInfo, tapi_data, sizeof(TelCallSplitCnf_t));
		call_handle = callSplitInfo.id;
	}

	_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
	_vc_core_tapi_event_handle_call_split_event(pagent, call_handle, result);
}

void _vc_core_engine_transfer_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;

	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_transfer_call_resp_cb");

	_vc_core_ca_change_agent_state(pagent, CALL_VC_CA_STATE_NORMAL);
	_vc_core_tapi_event_handle_call_transfer_event(pagent, result);
}

void _vc_core_engine_dtmf_call_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;

	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_dtmf_call_resp_cb");

	if (TAPI_CAUSE_SUCCESS != result) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Tapi Error Code %d", result);
		/*Forward the events to client */
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_DTMF_ACK, FALSE, 0, NULL);
	} else {
		/*Forward the events to client */
		_vc_core_ca_send_event_to_client(pagent, VC_CALL_DTMF_ACK, TRUE, 0, NULL);
	}
}

void _vc_core_engine_set_volume_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_set_volume_resp_cb : %d", result);
}

void _vc_core_engine_get_volume_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	int tapi_sound_path = 0;
	int volume_level = 0;
	TelCallGetVolumeInfoResp_t snd_resp_data;
	call_vc_callagent_state_t *pagent = gpcall_agent_for_callback;

	memset(&snd_resp_data, 0, sizeof(TelCallGetVolumeInfoResp_t));
	memcpy(&snd_resp_data, tapi_data, sizeof(TelCallGetVolumeInfoResp_t));

	int i = 0;
	tapi_sound_path = pagent->curr_tapi_path;
	for (i = 0; i < snd_resp_data.record_num; i++) {
		if (tapi_sound_path == snd_resp_data.record[i].device) {
			volume_level = snd_resp_data.record[i].volume;
			break;
		}
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Changed Vol Type = %d, Vol Level = %d", tapi_sound_path, volume_level);

	_vc_core_ca_send_event_to_client(pagent, VC_CALL_GET_VOLUME_RESP, tapi_sound_path, volume_level, NULL);
}

void _vc_core_engine_set_sound_path_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_set_sound_path_resp_cb : %d", result);
}

void _vc_core_engine_set_mute_status_resp_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_set_mute_status_resp_cb : %d", result);
}

void _vc_core_engine_get_aoc_info_cb(TapiHandle *handle, int result, void *tapi_data, void *user_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_get_aoc_info_cb : %d", result);
}