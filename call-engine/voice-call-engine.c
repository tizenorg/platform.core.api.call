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


#include "voice-call-engine.h"
#include "vc-core-util.h"
#include "voice-call-core.h"
#include "vc-core-engine-types.h"
#include "voice-call-service.h"
#include "voice-call-dbus.h"

typedef struct {
	void *puser_data;
	vcall_engine_app_cb cb_func;
} app_cb_t;

call_vc_core_state_t *global_pcall_core = NULL;

static app_cb_t *app_client_data = NULL;

/**
 * This function send events to client.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
gboolean vcall_engine_send_event_to_client(int event, void *pdata)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");
	if (app_client_data->cb_func != NULL) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Sending Event to APP Client\n");
		app_client_data->cb_func(event, pdata, app_client_data->puser_data);
	}
	CALL_ENG_DEBUG(ENG_ERR, "End..\n");
	return TRUE;
}

/**
 * This function initialize voice call engine.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_init(vcall_engine_app_cb pcb_func, void *puser_data)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");

	/* thread system initialization */
	if (!g_thread_supported()) {
		CALL_ENG_KPI("g_thread_init start");
		g_thread_init(NULL);
		CALL_ENG_KPI("g_thread_init done");
	}

	global_pcall_core = (call_vc_core_state_t *)calloc(1, sizeof(call_vc_core_state_t));
	if (global_pcall_core == NULL) {
		CALL_ENG_DEBUG(ENG_ERR, "Memory Allocation Failed\n");
		return VCALL_ENGINE_API_FAILED;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "global_pcall_core alloctated memory:[%d],global_pcall_core(0x%x) \n", sizeof(call_vc_core_state_t), global_pcall_core);

	if (FALSE == voicecall_core_init(global_pcall_core)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "voicecall_core_init() failed");
		return VCALL_ENGINE_API_FAILED;
	}

	app_client_data = (app_cb_t *) calloc(1, sizeof(app_cb_t));
	if (app_client_data == NULL) {
		CALL_ENG_DEBUG(ENG_ERR, "Memory Allocation Failed\n");
		return VCALL_ENGINE_API_FAILED;
	}
	app_client_data->cb_func = pcb_func;
	app_client_data->puser_data = puser_data;

	CALL_ENG_DEBUG(ENG_DEBUG, "Init dbus connection\n");
	vc_engine_dbus_receiver_setup();
	return VCALL_ENGINE_API_SUCCESS;

}

/**
 * This function processes mo nomal call.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_process_normal_call(char *number, int ct_index, gboolean b_download_call)
{
	CALL_ENG_DEBUG(ENG_DEBUG, " number is (%s)\n", number);
	voicecall_engine_t *pcall_engine = global_pcall_core->pcall_engine;
	vc_engine_outgoing_type event_data;
	char number_after_removal[VC_PHONE_NUMBER_LENGTH_MAX] = {"\0",};
	int io_state;

	if (number == NULL || pcall_engine == NULL)
		return VCALL_ENGINE_API_FAILED;

	_vc_core_engine_status_set_download_call(pcall_engine, b_download_call);

	/* check the call-engine state before proceeding with call processing */
	_vc_core_engine_status_get_engine_iostate(pcall_engine, &io_state);
	if (io_state != VC_INOUT_STATE_NONE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "IO State [%d] not NONE, cannot proceed with the call \n", io_state);
		voicecall_core_set_status(global_pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
		vc_engine_msg_box_type mbox_event_data;

		memset(&mbox_event_data, 0, sizeof(mbox_event_data));
		mbox_event_data.string_id = IDS_CALL_POP_CALLNOTCALLOWED;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&mbox_event_data);
		return VCALL_ENGINE_API_FAILED;
	}

	memset(&(global_pcall_core->call_setup_info), 0, sizeof(global_pcall_core->call_setup_info));
	global_pcall_core->call_setup_info.call_setup_by = VC_CALL_SETUP_BY_NORMAL;
	global_pcall_core->call_setup_info.call_type = VC_CALL_ORIG_TYPE_NORMAL;

	memset(&event_data, 0, sizeof(event_data));

	_vc_core_util_remove_invalid_chars(number, number_after_removal);
	snprintf(global_pcall_core->call_setup_info.source_tel_number, VC_PHONE_NUMBER_LENGTH_MAX, number_after_removal);
	voicecall_core_extract_phone_number(number_after_removal, event_data.call_num, VC_PHONE_NUMBER_LENGTH_MAX);

	if (strlen(event_data.call_num) > VC_PHONE_NUMBER_LENGTH_MAX) {
		CALL_ENG_DEBUG(ENG_ERR, " WARNING!! number is larger than max num length!! \n");
		memcpy(global_pcall_core->call_setup_info.tel_number, event_data.call_num, VC_PHONE_NUMBER_LENGTH_MAX);
	} else {
		memcpy(global_pcall_core->call_setup_info.tel_number, event_data.call_num, strlen(event_data.call_num));
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "number:[%s]", number);
	CALL_ENG_DEBUG(ENG_DEBUG, "number_after_removal:[%s]", number_after_removal);
	CALL_ENG_DEBUG(ENG_DEBUG, "global_pcall_core->call_setup_info.source_tel_number:[%s]", global_pcall_core->call_setup_info.source_tel_number);
	CALL_ENG_DEBUG(ENG_DEBUG, "global_pcall_core->call_setup_info.tel_number:[%s]", global_pcall_core->call_setup_info.tel_number);
	CALL_ENG_DEBUG(ENG_DEBUG, "event_data.call_num:[%s]", event_data.call_num);

	vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_TO_UI, (void *)&event_data);

	if (voicecall_core_setup_call(global_pcall_core, FALSE)) {
		CALL_ENG_DEBUG(ENG_DEBUG, " success!! \n");
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, " failed!! \n");
	}
	return VCALL_ENGINE_API_SUCCESS;

}

/**
 * This function processes mo emergency call.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_process_emergency_call(char *number)
{
	CALL_ENG_DEBUG(ENG_DEBUG, " number is : [%s] \n", number);
	voicecall_engine_t *pcall_engine = global_pcall_core->pcall_engine;
	gboolean bDefaultNumber = FALSE;
	vc_engine_outgoing_type event_data;
	int io_state;

	/* check the call-engine state before proceeding with emergency call processing */
	_vc_core_engine_status_get_engine_iostate(pcall_engine, &io_state);
	if (io_state != VC_INOUT_STATE_NONE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "IO State [%d] not NONE, cannot proceed with the call \n", io_state);
		voicecall_core_set_status(global_pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
		vc_engine_msg_box_type mbox_event_data;

		memset(&mbox_event_data, 0, sizeof(mbox_event_data));
		mbox_event_data.string_id = IDS_CALL_POP_CALLNOTCALLOWED;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&mbox_event_data);
		return VCALL_ENGINE_API_FAILED;
	}

	memset(&event_data, 0, sizeof(event_data));

	memset(&(global_pcall_core->call_setup_info), 0, sizeof(global_pcall_core->call_setup_info));

	bDefaultNumber = strlen(number) ? FALSE : TRUE;
	if (bDefaultNumber == TRUE) {
		global_pcall_core->call_setup_info.call_setup_by = VC_CALL_SETUP_BY_NORMAL;
		global_pcall_core->call_setup_info.call_type = VC_CALL_ORIG_TYPE_EMERGENCY;
		if (strlen("911") < sizeof(event_data.call_num))
			_vc_core_util_strcpy(event_data.call_num, VC_PHONE_NUMBER_LENGTH_MAX, "911");

		memcpy(global_pcall_core->call_setup_info.tel_number, "911", strlen("911"));
	} else {
		global_pcall_core->call_setup_info.call_setup_by = VC_CALL_SETUP_BY_NORMAL;
		global_pcall_core->call_setup_info.call_type = VC_CALL_ORIG_TYPE_PINLOCK;
		voicecall_core_extract_phone_number(number, event_data.call_num, VC_PHONE_NUMBER_LENGTH_MAX);
		if (strlen(number) > VC_PHONE_NUMBER_LENGTH_MAX) {
			CALL_ENG_DEBUG(ENG_ERR, " WARNING!! number is larger than max num length!! \n");
			memcpy(global_pcall_core->call_setup_info.source_tel_number, number, VC_PHONE_NUMBER_LENGTH_MAX);
			memcpy(global_pcall_core->call_setup_info.tel_number, number, VC_PHONE_NUMBER_LENGTH_MAX);
		} else {
			memcpy(global_pcall_core->call_setup_info.source_tel_number, number, VC_PHONE_NUMBER_LENGTH_MAX);
			memcpy(global_pcall_core->call_setup_info.tel_number, number, strlen(number));
		}
	}

	event_data.contact_index = -1;
	event_data.phone_type = -1;

	vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_TO_UI, (void *)&event_data);

	if (voicecall_core_setup_call(global_pcall_core, TRUE)) {
		CALL_ENG_DEBUG(ENG_DEBUG, " success!! \n");
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, " failed!! \n");
	}
	return VCALL_ENGINE_API_SUCCESS;
}

int vcall_engine_process_emergency_call_test(char *number)
{
	CALL_ENG_DEBUG(ENG_DEBUG, " number is : [%s] \n", number);
	voicecall_engine_t *pcall_engine = global_pcall_core->pcall_engine;
	gboolean bDefaultNumber = FALSE;
	vc_engine_outgoing_type event_data;
	int io_state;

	/* check the call-engine state before proceeding with emergency call processing */
	_vc_core_engine_status_get_engine_iostate(pcall_engine, &io_state);
	if (io_state != VC_INOUT_STATE_NONE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "IO State [%d] not NONE, cannot proceed with the call \n", io_state);
		voicecall_core_set_status(global_pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
		vc_engine_msg_box_type mbox_event_data;

		memset(&mbox_event_data, 0, sizeof(mbox_event_data));
		mbox_event_data.string_id = IDS_CALL_POP_CALLNOTCALLOWED;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&mbox_event_data);
		return VCALL_ENGINE_API_FAILED;
	}

	memset(&event_data, 0, sizeof(event_data));

	memset(&(global_pcall_core->call_setup_info), 0, sizeof(global_pcall_core->call_setup_info));

	bDefaultNumber = strlen(number) ? FALSE : TRUE;
	if (bDefaultNumber == TRUE) {
		global_pcall_core->call_setup_info.call_setup_by = VC_CALL_SETUP_BY_NORMAL;
		global_pcall_core->call_setup_info.call_type = VC_CALL_ORIG_TYPE_EMERGENCY;
		if (strlen("911") < sizeof(event_data.call_num))
			_vc_core_util_strcpy(event_data.call_num, VC_PHONE_NUMBER_LENGTH_MAX, "911");

		memcpy(global_pcall_core->call_setup_info.tel_number, "911", strlen("911"));
	} else {
		global_pcall_core->call_setup_info.call_setup_by = VC_CALL_SETUP_BY_NORMAL;
		global_pcall_core->call_setup_info.call_type = VC_CALL_ORIG_TYPE_PINLOCK;
		voicecall_core_extract_phone_number(number, event_data.call_num, VC_PHONE_NUMBER_LENGTH_MAX);
		if (strlen(number) > VC_PHONE_NUMBER_LENGTH_MAX) {
			CALL_ENG_DEBUG(ENG_ERR, " WARNING!! number is larger than max num length!! \n");
			memcpy(global_pcall_core->call_setup_info.source_tel_number, number, VC_PHONE_NUMBER_LENGTH_MAX);
			memcpy(global_pcall_core->call_setup_info.tel_number, number, VC_PHONE_NUMBER_LENGTH_MAX);
		} else {
			memcpy(global_pcall_core->call_setup_info.source_tel_number, number, VC_PHONE_NUMBER_LENGTH_MAX);
			memcpy(global_pcall_core->call_setup_info.tel_number, number, strlen(number));
		}
	}

	event_data.contact_index = -1;
	event_data.phone_type = -1;

	vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_TO_UI, (void *)&event_data);

	{
		int call_handle = 1;
		vc_engine_outgoing_orig_type orig_event_data;
		call_vc_call_objectinfo_t callobject_info;

		memset(&orig_event_data, 0, sizeof(orig_event_data));
		orig_event_data.call_handle = call_handle;
		orig_event_data.bemergency = TRUE;

		CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d, bemergency:[%d] \n", orig_event_data.call_handle, orig_event_data.bemergency);
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI_TEST, (void *)&orig_event_data);
	}

	return VCALL_ENGINE_API_SUCCESS;
}

/**
 * This function processes sat setup call.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_process_sat_setup_call(vcall_engine_sat_setup_call_info_t *sat_setup_call_info)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	voicecall_core_process_sat_setup_call(sat_setup_call_info);
	return VCALL_ENGINE_API_SUCCESS;
}

/**
 * This function processes incoming call.
 *
 * @return	int	API Result Code.
 * @param[in]		
*/
int vcall_engine_process_incoming_call(vcall_engine_incoming_info_t *incoming_call_info)
{
	call_vc_core_incoming_info_t tmp_incom_info = { 0, };

	CALL_ENG_DEBUG(ENG_WARN, "..\n");

	tmp_incom_info.call_handle = incoming_call_info->call_handle;
	tmp_incom_info.call_type = incoming_call_info->call_type;
	tmp_incom_info.cli_presentation_indicator = incoming_call_info->cli_presentation_indicator;
	_vc_core_util_strcpy(tmp_incom_info.call_num, sizeof(tmp_incom_info.call_num), incoming_call_info->call_num);
	tmp_incom_info.calling_name_mode = incoming_call_info->calling_name_mode;
	_vc_core_util_strcpy(tmp_incom_info.calling_name, sizeof(tmp_incom_info.calling_name), incoming_call_info->calling_name);
	_vc_core_util_strcpy(tmp_incom_info.redirected_number, sizeof(tmp_incom_info.redirected_number), incoming_call_info->redirected_number);
	_vc_core_util_strcpy(tmp_incom_info.redirected_sub_address, sizeof(tmp_incom_info.redirected_sub_address), incoming_call_info->redirected_sub_address);
	tmp_incom_info.cli_cause = incoming_call_info->cli_cause;
	tmp_incom_info.bfwded = incoming_call_info->bfwded;
	tmp_incom_info.active_line = incoming_call_info->active_line;

	voicecall_core_process_incoming_call(&tmp_incom_info);
	return VCALL_ENGINE_API_SUCCESS;
}

/**
 * This function answers an incoming call
 *
 * @return	int	API Result Code.
 * @param[in]		none
*/
int vcall_engine_answer_call(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = VCALL_ENGINE_API_FAILED;

	ret = voicecall_core_answer_call(global_pcall_core, FALSE);
	CALL_ENG_DEBUG(ENG_DEBUG, " ret:[%d] \n", ret);
	return ret;
}

/**
 * This function answers an incoming call  according to the given type
 *
 * @return	int	API Result Code.
 * @param[in]		answer_type		answer type
 */
int vcall_engine_answer_call_by_type(vcall_engine_answer_type_t answer_type)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	CALL_ENG_DEBUG(ENG_DEBUG, "answer_type:[%d]\n", answer_type);

	if (TRUE == voicecall_core_answer_call_bytype(global_pcall_core, answer_type)) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}

}

/**
 * This function cancel outgoing call
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_cancel_call(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");

	if (TRUE == voicecall_core_cancel_call(global_pcall_core)) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function reject incoming call
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_reject_call(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");

	if (TRUE == voicecall_core_reject_mt(global_pcall_core, TRUE)) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function release a active or held call
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_release_call(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");

	if (TRUE == voicecall_core_end_call(global_pcall_core)) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function release a call by handle
 *
 * @return	int	API Result Code.
 * @param[in]	int	call_handle
 */
int vcall_engine_release_call_by_handle(int call_handle)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle:[%d]\n", call_handle);

	if (TRUE == voicecall_core_end_call_by_handle(global_pcall_core, call_handle)) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function release calls by type
 *
 * @return	int	API Result Code.
 * @param[in]		release_type release_type
 */
int vcall_engine_release_call_by_type(vcall_engine_release_type_t release_type)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, " release_type : [%d]\n", release_type);
	switch (release_type) {
	case VCALL_ENGINE_RELEASE_ALL_ACTIVE_CALLS:
		{
			ret = voicecall_core_end_all_active_calls(global_pcall_core);
		}
		break;

	case VCALL_ENGINE_RELEASE_ALL_HELD_CALLS:
		{
			ret = voicecall_core_end_all_held_calls(global_pcall_core);
		}
		break;

	case VCALL_ENGINE_RELEASE_ALL_CALLS:
		{
			ret = voicecall_core_end_all_calls(global_pcall_core);
		}
		break;

	default:
		CALL_ENG_DEBUG(ENG_DEBUG, " Unknown release_type : [%d]\n", release_type);
		break;

	}

	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function processes hold/retrive/swap calls.
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_process_hold_call(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_core_process_hold_call(global_pcall_core);
	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function processes in call SS code..
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
void vcall_engine_process_incall_ss(char *number)
{
	CALL_ENG_DEBUG(ENG_WARN, "number(%s)\n");

	voicecall_core_process_incall_ss(global_pcall_core, number);
}

/**
 * This function sets up a conference calls
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_join_call(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_core_setup_conference(global_pcall_core);
	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function make a private call in conference calls.
 *
 * @return	int	API Result Code.
 * @param[in]	int	call_handle
 */
int vcall_engine_split_call(int call_handle)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_core_make_private_call(global_pcall_core, call_handle);
	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function transfers the call from active call to the held call
 *
 * @return	int	API Result Code.
 * @param[in]	int	call_handle
 */
int vcall_engine_transfer_call(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_core_transfer_calls(global_pcall_core);
	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function processed loud speaker.
 *
 * @return	int	API Result Code.
 * @param[in]	int	bstatus
 */
int vcall_engine_process_loudspeaker(int bstatus)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, " bstatus : [%d]\n", bstatus);
	if (bstatus) {
		ret = voicecall_service_loudspeaker_on(global_pcall_core);
	} else {
		ret = voicecall_service_loudspeaker_off(global_pcall_core);
	}

	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function processed voice mute status.
 *
 * @return	int	API Result Code.
 * @param[in]	int	bstatus
 */
int vcall_engine_process_voice_mute(int bstatus)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, " bstatus : [%d]\n", bstatus);
	if (bstatus) {
		ret = voicecall_service_mute_status_on(global_pcall_core);
	} else {
		ret = voicecall_service_mute_status_off(global_pcall_core);
	}

	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function gets the volume level
 *
 * @return	int	API Result Code.
 * @param[in]		vcall_engine_vol_type_t vol_type
 * @param[in]		voicecall_snd_volume_alert_type_t vol_alert_type
 */
int vcall_engine_get_volume_level(vcall_engine_vol_type_t vol_type)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");

	CALL_ENG_DEBUG(ENG_DEBUG, " vol_type : [%d]\n", vol_type);

	return voicecall_snd_get_volume(global_pcall_core->papp_snd, vol_type);

}

/**
 * This function sets the volume level
 *
 * @return	int	API Result Code.
 * @param[in]		vcall_engine_vol_type_t vol_type
 * @param[in]		int vol_level
 */
int vcall_engine_set_volume_level(vcall_engine_vol_type_t vol_type, int vol_level)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, " vol_type : [%d], vol_level:[%d]\n", vol_type, vol_level);
	ret = voicecall_service_set_volume(global_pcall_core, vol_type, vol_level);
	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function stop alert
 *
 * @return	int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_stop_alert(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_core_stop_alert(global_pcall_core);
	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function mute alert.
 *
 * @return int	API Result Code.
 * @param[in]		none
 */
int vcall_engine_mute_alert(void)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_snd_mute_alert(global_pcall_core->papp_snd);
	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function process auto redial.
 *
 * @return	int	API Result Code.
 * @param[in]		int bstatus
 */
int vcall_engine_process_auto_redial(int bstatus)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, " bstatus : [%d]\n", bstatus);
	if (bstatus) {
		ret = voicecall_core_start_redial(global_pcall_core, TRUE);
	} else {
		ret = voicecall_core_stop_redial(global_pcall_core);
	}
	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function send the DTMF number
 *
 * @return	int	API Result Code.
 * @param[in]		char* dtmf_number
 */
int vcall_engine_send_dtmf_number(char *dtmf_number)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_core_send_dtmf(global_pcall_core, dtmf_number);

	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function processed sound path
 *
 * @return	int	API Result Code.
 * @param[in]	int	sound_path
 */
int vcall_engine_change_sound_path(vcall_engine_audio_type_t sound_path)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_core_change_sound_path(global_pcall_core, sound_path);

	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function get sound path
 *
 * @return	int	API Result Code.
 * @param[out]	int	sound_path
 */
int vcall_engine_get_sound_path(int *sound_path)
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;
	int snd_path = 0;

	ret = voicecall_core_get_sound_path(global_pcall_core, &snd_path);

	if (snd_path == VOICE_CALL_AUDIO_SPEAKER) {
		*sound_path = VCALL_ENGINE_AUDIO_SPEAKER;
	} else if (snd_path == VOICE_CALL_AUDIO_RECEIVER) {
		*sound_path = VCALL_ENGINE_AUDIO_RECEIVER;
	} else if (snd_path == VOICE_CALL_AUDIO_HEADSET) {
		*sound_path = VCALL_ENGINE_AUDIO_HEADSET;
	} else if (snd_path == VOICE_CALL_AUDIO_EARJACK) {
		*sound_path = VCALL_ENGINE_AUDIO_EARJACK;
	} else {
		*sound_path = VCALL_ENGINE_AUDIO_RECEIVER;
	}

	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

/**
 * This function set call engine to default
 *
 * @return	int	API Result Code.
 * @param[in]		
 */
int vcall_engine_set_to_default()
{
	CALL_ENG_DEBUG(ENG_WARN, "..\n");
	int ret = FALSE;

	ret = voicecall_core_set_to_default(global_pcall_core);

	if (ret == TRUE) {
		return VCALL_ENGINE_API_SUCCESS;
	} else {
		return VCALL_ENGINE_API_FAILED;
	}
}

gpointer vcall_engine_get_core_state(void)
{
	return global_pcall_core;
}

/**
 * This function is interface to call-utility to perform string copy
 *
 * @return	instance of the core state
 * @param[out]	pbuffer		Target Buffer
 * @param[in]	buf_count	Size of Target Buffer
 * @param[in]	pstring		Source String
 */
gboolean vcall_engine_util_strcpy(char *pbuffer, int buf_count, const char *pstring)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	return _vc_core_util_strcpy(pbuffer, buf_count, pstring);
}

/**
 * This function returns the number of groups
 *
 * @param[in]		pcount		count of the groups
 */
gboolean vcall_engine_get_group_count(int *pcount)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	return _vc_core_engine_group_get_enabled_group_count(global_pcall_core->pcall_engine, pcount);
}

/**
 * This function is interface to call-utility to perform string copy
 *
 * @return	instance of the core state
 * @param[out]	pbuffer		Target Buffer
 * @param[in]	time		time
 */
char *vcall_engine_util_get_date_time(time_t time)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	return _vc_core_util_get_date_time(time);
}

/**
 * This function is force reset all engine status.
 *
 * @return	void
 * @param[in] void
 */
void vcall_engine_force_reset(void)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	voicecall_snd_unregister_cm(global_pcall_core->papp_snd);

	_vc_core_util_set_call_status(VCONFKEY_CALL_OFF);

	voicecall_core_end_all_calls(global_pcall_core);
}
