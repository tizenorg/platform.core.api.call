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


#include "vcui-application.h"
#include "vcui-engine-interface.h"
#include "vcui-view-dialing.h"

/* For Debug Information, Call Event name string constant */
static char *gszcall_callback_msg[VC_ENGINE_MSG_MAX_TO_UI] = {
	"VC_ENGINE_MSG_INCOM_TO_UI",
	"VC_ENGINE_MSG_OUTGOING_TO_UI",
	"VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI",
	"VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI_TEST",
	"VC_ENGINE_MSG_OUTGOING_ALERT_TO_UI",
	"VC_ENGINE_MSG_CONNECTED_TO_UI",
	"VC_ENGINE_MSG_NORMAL_END_TO_UI",
	"VC_ENGINE_MSG_INCOM_END_TO_UI",
	"VC_ENGINE_MSG_REJECTED_END_TO_UI",
	"VC_ENGINE_MSG_OUTGOING_END_TO_UI",
	"VC_ENGINE_MSG_OUTGOING_END_SIGNAL_PLAY_TO_UI",
	"VC_ENGINE_MSG_OUTGOING_ABORTED_TO_UI",
	"VC_ENGINE_MSG_DTMF_ACK_TO_UI",

	"VC_ENGINE_MSG_SS_HELD_TO_UI",
	"VC_ENGINE_MSG_SS_RETREIVED_TO_UI",
	"VC_ENGINE_MSG_SS_SWAP_TO_UI",
	"VC_ENGINE_MSG_SS_SETUP_CONF_TO_UI",
	"VC_ENGINE_MSG_SS_SPLIT_CONF_TO_UI",
	"VC_ENGINE_MSG_SS_TRANSFERRED_TO_UI",
	"VC_ENGINE_MSG_SS_CONNECT_LINE_IND_TO_UI",

	"VC_ENGINE_MSG_IND_FORWARD_TO_UI",
	"VC_ENGINE_MSG_IND_ACTIVATE_TO_UI",
	"VC_ENGINE_MSG_IND_HOLD_TO_UI",
	"VC_ENGINE_MSG_IND_TRANSFER_TO_UI",
	"VC_ENGINE_MSG_IND_SETUPCONFERENCE_TO_UI",
	"VC_ENGINE_MSG_IND_BARRING_TO_UI",
	"VC_ENGINE_MSG_IND_WAITING_TO_UI",
	"VC_ENGINE_MSG_IND_CUGINFO_TO_UI",
	"VC_ENGINE_MSG_IND_SSNOTIFY_TO_UI",
	"VC_ENGINE_MSG_IND_CALLINGNAMEINFO_TO_UI",
	"VC_ENGINE_MSG_IND_REDIRECT_CNF_TO_UI",
	"VC_ENGINE_MSG_IND_ACTIVATECCBS_CNF_TO_UI",
	"VC_ENGINE_MSG_IND_ACTIVATECCBS_USERINFO_TO_UI",
	"VC_ENGINE_MSG_IND_AOC_TO_UI",

	"VC_ENGINE_MSG_ERROR_OCCURED_TO_UI",

	"VC_ENGINE_MSG_ACTION_INCOM_FORCE_TO_UI",
	"VC_ENGINE_MSG_ACTION_SAT_REQUEST_TO_UI",
	"VC_ENGINE_MSG_ACTION_SAT_RESPONSE_TO_UI",
	"VC_ENGINE_MSG_ACTION_CALL_END_HELD_RETREIVED_TO_UI",
	"VC_ENGINE_MSG_ACTION_NO_ACTIVE_TASK_TO_UI",

	"VC_ENGINE_MSG_GET_VOLUME_RESP_TO_UI",
	"VC_ENGINE_MSG_SET_VOLUME_FROM_BT_TO_UI",
	"VC_ENGINE_MSG_HEADSET_STATUS_TO_UI",
	"VC_ENGINE_MSG_EARJACK_STATUS_TO_UI",

	"VC_ENGINE_MSG_ACCEPT_CHOICE_BOX_TO_UI",
	"VC_ENGINE_MSG_MESSAGE_BOX_TO_UI",

	"VC_ENGINE_MSG_REDIAL_TO_UI",
	"VC_ENGINE_MSG_STOPPED_RECORDING_TO_UI",
	"VC_ENGINE_MSG_CREATE_NEWVOICEFILE_TO_UI",
};

static char *gszcall_error_msg[IDS_CALL_MAX] = {
	"IDS_CALL_POP_CALL_IS_DIVERTED",
	"IDS_CALL_POP_CALLFAILED",
	"IDS_CALL_POP_CALLING_EMERG_ONLY",
	"IDS_CALL_POP_CALLNOTCALLOWED",
	"IDS_CALL_POP_CAUSE_WRONG_NUMBER",
	"IDS_CALL_POP_CHANGEOFFLINEMODETOCALL",
	"IDS_CALL_POP_DTMFSENDING_FAIL",
	"IDS_CALL_POP_FDNCALLONLY",
	"IDS_CALL_POP_HOLD_FAILED",
	"IDS_CALL_POP_HOLD_NOT_SUPPORTED",
	"IDS_CALL_POP_INCOMPLETE",
	"IDS_CALL_POP_JOIN_FAILED",
	"IDS_CALL_POP_JOIN_NOT_SUPPORTED",
	"IDS_CALL_POP_OPERATION_REFUSED",
	"IDS_CALL_POP_PHONE_NOT_INITIALISED",
	"IDS_CALL_POP_REJECTED",
	"IDS_CALL_POP_SENDING",
	"IDS_CALL_POP_SOS_CALL_ONLY_IN_NO_SIM_MODE",
	"IDS_CALL_POP_SPLIT_FAILED",
	"IDS_CALL_POP_SPLIT_NOT_SUPPORTED",
	"IDS_CALL_POP_SWAP_FAILED",
	"IDS_CALL_POP_SWAP_NOT_SUPPORTED",
	"IDS_CALL_POP_TRANSFER_FAILED",
	"IDS_CALL_POP_TRANSFER_NOT_SUPPORTED",
	"IDS_CALL_POP_UNABLE_TO_RETRIEVE",
	"IDS_CALL_POP_UNAVAILABLE",
	"IDS_CALL_POP_UNHOLD_NOT_SUPPORTED",
	"IDS_CALL_POP_VOICE_CALL_IS_NOT_ALLOWED_DURING_VIDEO_CALL",
	"IDS_CALL_POP_WAITING_ACTIVE",
	"IDS_CALL_BODY_CALLENDED",
	"Invalid DTMF",
	"Sent"
};

void _vcui_engine_init(vcui_app_call_data_t *ad)
{
	CALL_UI_DEBUG("..");
	vcall_engine_init((vcall_engine_app_cb) _vcui_engine_callback, ad);
	CALL_UI_DEBUG("End..");
}

void _vcui_engine_answer_call(void)
{
	int ret = VCALL_ENGINE_API_SUCCESS;

	CALL_UI_DEBUG("..");

	ret = vcall_engine_answer_call();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_answer_call_by_type(int ans_type)
{
	CALL_UI_DEBUG("..");

	vcall_engine_answer_call_by_type(ans_type);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_cancel_call(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_cancel_call();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_reject_call(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_reject_call();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_set_volume_level(vcui_vol_type_t vol_type, int level)
{
	int vol = 0;
	vcall_engine_vol_type_t engine_vol_type = VCALL_ENGINE_VOL_TYPE_RINGTONE;
	CALL_UI_DEBUG("..");

	if (vol_type == VCUI_VOL_RING) {
		engine_vol_type = VCALL_ENGINE_VOL_TYPE_RINGTONE;
	} else if (vol_type == VCUI_VOL_VOICE) {
		engine_vol_type = VCALL_ENGINE_VOL_TYPE_VOICE;
	} else if (vol_type == VCUI_VOL_HEADSET) {
		engine_vol_type = VCALL_ENGINE_VOL_TYPE_HEADSET;
	}

	vol = vcall_engine_set_volume_level(engine_vol_type, level);
	CALL_UI_DEBUG("End..");
}

int _vcui_engine_get_volume_level(vcui_vol_type_t vol_type)
{
	int vol = 0;
	vcall_engine_vol_type_t engine_vol_type = VCALL_ENGINE_VOL_TYPE_RINGTONE;
	CALL_UI_DEBUG("vol_type(%d)", vol_type);

	if (vol_type == VCUI_VOL_RING) {
		engine_vol_type = VCALL_ENGINE_VOL_TYPE_RINGTONE;
	} else if (vol_type == VCUI_VOL_VOICE) {
		engine_vol_type = VCALL_ENGINE_VOL_TYPE_VOICE;
	} else if (vol_type == VCUI_VOL_HEADSET) {
		engine_vol_type = VCALL_ENGINE_VOL_TYPE_HEADSET;
	}

	vol = vcall_engine_get_volume_level(engine_vol_type);
	CALL_UI_DEBUG("End..");

	return vol;
}

void _vcui_engine_change_sound_path(vcui_audio_type_t sound_path)
{
	int ret = 0;
	vcall_engine_audio_type_t rqst_snd_path = VCALL_ENGINE_AUDIO_NONE;
	CALL_UI_DEBUG("sound_path(%d)", sound_path);

	if (sound_path == VCUI_AUDIO_SPEAKER) {
		rqst_snd_path = VCALL_ENGINE_AUDIO_SPEAKER;
	} else if (sound_path == VCUI_AUDIO_HEADSET) {
		rqst_snd_path = VCALL_ENGINE_AUDIO_HEADSET;
	} else if (sound_path == VCUI_AUDIO_EARJACK) {
		rqst_snd_path = VCALL_ENGINE_AUDIO_EARJACK;
	}

	vcall_engine_change_sound_path(rqst_snd_path);
	CALL_UI_DEBUG("End..");
}

vcui_audio_type_t _vcui_engine_get_sound_path(void)
{
	int ret = 0;
	int snd_path = VCALL_ENGINE_AUDIO_NONE;

	ret = vcall_engine_get_sound_path(&snd_path);

	if (snd_path == VCALL_ENGINE_AUDIO_SPEAKER) {
		return VCUI_AUDIO_SPEAKER;
	} else if (snd_path == VCALL_ENGINE_AUDIO_HEADSET) {
		return VCUI_AUDIO_HEADSET;
	} else if (snd_path == VCALL_ENGINE_AUDIO_EARJACK) {
		return VCUI_AUDIO_EARJACK;
	} else if (snd_path == VCALL_ENGINE_AUDIO_RECEIVER) {
		return VCUI_AUDIO_RECEIVER;
	} else {
		return VCALL_ENGINE_AUDIO_NONE;
	}
}

void _vcui_engine_stop_alert(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_stop_alert();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_end_call(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_release_call();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_end_call_by_handle(int handle)
{
	CALL_UI_DEBUG("handle(%d)",handle);

	vcall_engine_release_call_by_handle(handle);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_end_all_call(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_CALLS);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_end_active_calls(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_ACTIVE_CALLS);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_end_held_calls(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_HELD_CALLS);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_hold_unhold_swap_call(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_process_hold_call();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_join_call(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_join_call();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_split_call(int call_handle)
{
	CALL_UI_DEBUG("..");

	vcall_engine_split_call(call_handle);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_transfer_call(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_transfer_call();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_speaker_on_off(int bLoundSpeaker)
{
	CALL_UI_DEBUG("..");

	vcall_engine_process_loudspeaker(bLoundSpeaker);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_mute_on_off(int bMute)
{
	CALL_UI_DEBUG("..");

	vcall_engine_process_voice_mute(bMute);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_interface_process_auto_redial(int bRedial)
{
	CALL_UI_DEBUG("..");

	vcall_engine_process_auto_redial(bRedial);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_interface_process_mute_alert(void)
{
	CALL_UI_DEBUG("..");

	vcall_engine_mute_alert();

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_interface_send_dtmf_number(char data)
{
	CALL_UI_DEBUG("..");
	char dtmf_number[2];

	dtmf_number[0] = data;
	dtmf_number[1] = '\0';

	vcall_engine_send_dtmf_number(dtmf_number);

	CALL_UI_DEBUG("End..");
}

void _vcui_engine_interface_process_mo_call(vcui_call_type_t call_type, vcui_call_mo_data_t *data)
{
	CALL_UI_DEBUG(".");

	CALL_UI_KPI("vcall_engine_process_normal_call start");
	vcui_call_mo_data_t *mocall = (vcui_call_mo_data_t *) data;
	CALL_UI_DEBUG("number is : [%s] ", mocall->call_number);

	if (call_type == VCUI_CALL_TYPE_MO)
		vcall_engine_process_normal_call(mocall->call_number, mocall->ct_index, EINA_FALSE);
	else if (call_type == VCUI_CALL_TYPE_DOWNLOAD_CALL)
		vcall_engine_process_normal_call(mocall->call_number, mocall->ct_index, EINA_TRUE);

	CALL_UI_KPI("vcall_engine_process_normal_call done");

}

void _vcui_engine_interface_process_mt_call(vcui_call_type_t call_type, vcui_call_mt_data_t *data)
{
	CALL_UI_KPI("vcall_engine_process_incoming_call start");
	vcui_call_mt_data_t *mtcall = (vcui_call_mt_data_t *) data;
	vcall_engine_incoming_info_t mtcall_info;
	CALL_UI_DEBUG("number is : [%s] ", mtcall->call_num);

	mtcall_info.call_handle = mtcall->call_handle;
	mtcall_info.call_type = mtcall->call_type;
	mtcall_info.cli_presentation_indicator = mtcall->cli_presentation_indicator;
	_vc_core_util_strcpy(mtcall_info.call_num, sizeof(mtcall_info.call_num), mtcall->call_num);
	mtcall_info.calling_name_mode = mtcall->calling_name_mode;
	_vc_core_util_strcpy(mtcall_info.calling_name, sizeof(mtcall_info.calling_name), mtcall->calling_name);
	_vc_core_util_strcpy(mtcall_info.redirected_number, sizeof(mtcall_info.redirected_number), mtcall->redirected_number);
	_vc_core_util_strcpy(mtcall_info.redirected_sub_address, sizeof(mtcall_info.redirected_sub_address), mtcall->redirected_sub_address);
	mtcall_info.cli_cause = mtcall->cli_cause;
	mtcall_info.bfwded = mtcall->bfwded;
	mtcall_info.active_line = mtcall->active_line;

	vcall_engine_process_incoming_call(&mtcall_info);
	CALL_UI_KPI("vcall_engine_process_incoming_call done");
}

void _vcui_engine_interface_process_ecc_call(vcui_call_type_t call_type, vcui_call_ecc_data_t *data)
{
	vcui_call_ecc_data_t *emercall = (vcui_call_ecc_data_t *) data;

	CALL_UI_DEBUG("number is : [%s] ", emercall->call_number);
	if (call_type == VCUI_CALL_TYPE_ECC)
		vcall_engine_process_emergency_call(emercall->call_number);
	else if (call_type == VCUI_CALL_TYPE_ECC_TEST)
		vcall_engine_process_emergency_call_test(emercall->call_number);

}

void _vcui_engine_interface_process_sat_call(vcui_call_type_t call_type, vcui_call_sat_data_t *data)
{
	vcui_call_sat_data_t *satcall = (vcui_call_sat_data_t *) data;
	vcall_engine_sat_setup_call_info_t sat_setup_call_info;
	CALL_UI_DEBUG("..");

	memset(&sat_setup_call_info, 0, sizeof(sat_setup_call_info));
	sat_setup_call_info.command_id = satcall->command_id;
	sat_setup_call_info.command_qualifier = satcall->command_qualifier;
	sat_setup_call_info.duration = satcall->duration;
	memcpy(sat_setup_call_info.disp_text, satcall->disp_text, sizeof(sat_setup_call_info.disp_text));
	memcpy(sat_setup_call_info.call_num, satcall->call_num, VC_PHONE_NUMBER_LENGTH_MAX);

	vcall_engine_process_sat_setup_call(&sat_setup_call_info);
}

void _vcui_engine_callback(int event, void *pdata, void *puser_data)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) puser_data;
	vc_engine_msg_type *msg = (vc_engine_msg_type *)pdata;

	if ((ad == NULL) || (msg == NULL)) {
		CALL_UI_DEBUG("ERROR.NULL pointer");
		return;
	}
	CALL_UI_DEBUG("@@@ event:[%s], view_top:[%d], count:[%d]  @@@ \n", gszcall_callback_msg[event], ad->view_top, _vcui_doc_get_count());

	switch (event) {
	case VC_ENGINE_MSG_INCOM_TO_UI:
		{
			_voicecall_dvc_control_lcd_state(VC_LCD_ON_LOCK);

			CALL_UI_DEBUG("num:[%s], name:[%s]", msg->incoming.call_num, msg->incoming.call_name);

			call_data_t *call_data = malloc(sizeof(call_data_t));
			if (call_data == NULL)
				return;
			memset(call_data, 0, sizeof(call_data_t));

			call_data->call_handle = msg->incoming.call_handle;
			call_data->contact_id = msg->incoming.contact_index;
			call_data->contact_phone_type = msg->incoming.phone_type;
			vcall_engine_util_strcpy(call_data->call_num, sizeof(call_data->call_num), msg->incoming.call_num);
			if (msg->incoming.brestricted == EINA_TRUE) {
				if (msg->incoming.bpayphone == EINA_TRUE) {
					vcall_engine_util_strcpy(call_data->call_display, VC_DISPLAY_NAME_LENGTH_MAX, _("Payphone"));
				} else {
					vcall_engine_util_strcpy(call_data->call_display, VC_DISPLAY_NAME_LENGTH_MAX, dgettext("sys_string", "IDS_COM_BODY_UNKNOWN"));
				}
			} else {
				vcall_engine_util_strcpy(call_data->call_display, VC_DISPLAY_NAME_LENGTH_MAX, msg->incoming.call_name);
			}

			if (strcmp((char *)msg->incoming.call_file_path, "default") == 0)
				vcall_engine_util_strcpy(call_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
			else if (strlen((char *)msg->incoming.call_file_path) == 0)
				vcall_engine_util_strcpy(call_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
			else
				vcall_engine_util_strcpy(call_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, msg->incoming.call_file_path);

			if (strcmp((char *)msg->incoming.call_full_file_path, "default") == 0)
				vcall_engine_util_strcpy(call_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
			else if (strlen((char *)msg->incoming.call_full_file_path) == 0)
				vcall_engine_util_strcpy(call_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
			else
				vcall_engine_util_strcpy(call_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, msg->incoming.call_full_file_path);

			call_data->caller_status = NO_STATUS;
			call_data->mo_mt_status = CALL_INCOMING;

			_vcui_doc_set_mt_recent(call_data);
			_vcui_doc_add_call_data(call_data);

			if (_vcui_is_idle_lock() == CALL_LOCK) {
				_vcui_view_change(VIEW_INCOMING_LOCK_VIEW, 0, NULL, NULL);
			} else {
				_vcui_view_change(VIEW_INCOMING_VIEW, 0, NULL, NULL);
			}
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_TO_UI:
		{
			call_data_t *call_data = malloc(sizeof(call_data_t));
			if (call_data == NULL)
				return;
			memset(call_data, 0, sizeof(call_data_t));

			call_data->call_handle = NO_HANDLE;
			call_data->contact_id = msg->outgoing.contact_index;
			call_data->contact_phone_type = msg->outgoing.phone_type;
			vcall_engine_util_strcpy(call_data->call_num, VC_PHONE_NUMBER_LENGTH_MAX, msg->outgoing.call_num);
			vcall_engine_util_strcpy(call_data->call_display, VC_DISPLAY_NAME_LENGTH_MAX, msg->outgoing.call_name);

			if (strcmp((char *)msg->outgoing.call_file_path, "default") == 0)
				vcall_engine_util_strcpy(call_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
			else if (strlen((char *)msg->outgoing.call_file_path) == 0)
				vcall_engine_util_strcpy(call_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
			else
				vcall_engine_util_strcpy(call_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, msg->outgoing.call_file_path);

			if (strcmp((char *)msg->outgoing.call_full_file_path, "default") == 0)
				vcall_engine_util_strcpy(call_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
			else if (strlen((char *)msg->outgoing.call_full_file_path) == 0)
				vcall_engine_util_strcpy(call_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
			else
				vcall_engine_util_strcpy(call_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, msg->outgoing.call_full_file_path);

			call_data->caller_status = NO_STATUS;
			call_data->mo_mt_status = CALL_OUTGOING;
			call_data->start_time = time(&(call_data->start_time));

			_vcui_doc_set_mo_recent(call_data);

		}
		break;

	case VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI:
		{
			if (_voicecall_dvc_get_proximity_sensor_state() != VCALL_SENSOR_NEAR)
				_voicecall_dvc_control_lcd_state(VC_LCD_ON_LOCK);

			vc_engine_outgoing_orig_type outgoing_orig = msg->outgoing_orig;

			call_data_t *call_data = _vcui_doc_get_recent_mo();
			call_data->call_handle = outgoing_orig.call_handle;

			if (outgoing_orig.bemergency == EINA_TRUE) {
				CALL_UI_DEBUG("it is emergency call");
				char *em_name = _("IDS_CALL_POP_EMERGENCY_CALL");

				memset(call_data->call_display, 0, sizeof(call_data->call_display));
				memset(call_data->call_file_path, 0, sizeof(call_data->call_file_path));
				memset(call_data->call_full_file_path, 0, sizeof(call_data->call_full_file_path));

				vcall_engine_util_strcpy(call_data->call_display, VC_DISPLAY_NAME_LENGTH_MAX, em_name);
				vcall_engine_util_strcpy(call_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
				vcall_engine_util_strcpy(call_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
				call_data->contact_phone_type = -1;
			}

			_vcui_doc_add_call_data(call_data);
			_vcui_view_change(VIEW_DIALLING_VIEW, 0, NULL, NULL);
  		}
		break;

	case VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI_TEST:
		{
			if (_voicecall_dvc_get_proximity_sensor_state() != VCALL_SENSOR_NEAR)
				_voicecall_dvc_control_lcd_state(VC_LCD_ON_LOCK);

			vc_engine_outgoing_orig_type outgoing_orig = msg->outgoing_orig;

			call_data_t *call_data = _vcui_doc_get_recent_mo();
			call_data->call_handle = 1;

			if (outgoing_orig.bemergency == EINA_TRUE) {
				CALL_UI_DEBUG("it is emergency call");
				char *em_name = _("IDS_CALL_POP_EMERGENCY_CALL");

				memset(call_data->call_display, 0, sizeof(call_data->call_display));
				memset(call_data->call_file_path, 0, sizeof(call_data->call_file_path));
				memset(call_data->call_full_file_path, 0, sizeof(call_data->call_full_file_path));

				vcall_engine_util_strcpy(call_data->call_display, VC_DISPLAY_NAME_LENGTH_MAX, em_name);
				vcall_engine_util_strcpy(call_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
				vcall_engine_util_strcpy(call_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, NOIMG_ICON);
				call_data->contact_phone_type = -1;
			}

			_vcui_doc_add_call_data(call_data);
			_vcui_view_change(VIEW_DIALLING_VIEW, 0, NULL, NULL);
  		}
		break;

	case VC_ENGINE_MSG_OUTGOING_ALERT_TO_UI:
		{
			call_data_t *call_data = _vcui_doc_get_recent_mo();

			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}

			if (_vcui_doc_is_call_data(call_data) == EINA_FALSE) {
				CALL_UI_DEBUG("Error. check outgoing_orig msg.");
				elm_exit();
				return;
			}

			_vcui_view_dialing_draw_txt_connecting(ad->view_st[VIEW_DIALLING_VIEW]);
 		}
		break;

	case VC_ENGINE_MSG_CONNECTED_TO_UI:
		{
			_voicecall_dvc_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_connected_type connected = msg->connected;
			call_data_t *call_data = _vcui_doc_get_call_handle(connected.call_handle);

			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}

			call_data->caller_status = CALL_UNHOLD;
			call_data->call_handle = connected.call_handle;
			call_data->start_time = time(&(call_data->start_time));

			/* When new call connected, if it's multiparty call, always show split1 first. */
			ad->bswapped = EINA_FALSE;

			_vcui_view_auto_change();
 		}
		break;

	case VC_ENGINE_MSG_NORMAL_END_TO_UI:
		{
			vc_engine_normal_end_type normal_end = msg->normal_end;

			CALL_UI_DEBUG("end_cause_type:[%d]", normal_end.end_cause_type);

			call_data_t *call_data = _vcui_doc_get_call_handle(normal_end.call_handle);
			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}
			if (call_data->mo_mt_status == CALL_INCOMING)
				_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_INCOMMING, call_data, EINA_FALSE);
			else if (call_data->mo_mt_status == CALL_OUTGOING)
				_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_OUTGOING, call_data, EINA_FALSE);

			time_t start_time;
			memcpy(&start_time, &(call_data->start_time), sizeof(call_data->start_time));

			if (call_data->bno_end_show == EINA_TRUE) {
				_vcui_doc_remove_call_data(call_data);
				_vcui_view_common_call_terminate_or_view_change();
			} else {
				vcui_app_call_data_t *ad = _vcui_get_app_data();
				if (_vcui_doc_get_count() == 1 && ad->view_top == VIEW_INCALL_ONECALL_VIEW) {
					ad->call_end_type = CALL_END_TYPE_SINGLE_CALL;
				}
				CALL_UI_DEBUG("ad->call_end_type[%d]", ad->call_end_type);
				if (ad->call_end_type == CALL_END_TYPE_SINGLE_CALL || ad->call_end_type == CALL_END_TYPE_CONF_CALL) {
					CALL_UI_DEBUG("Show end screen - %d", ad->call_end_type);
					_vcui_view_change(VIEW_ENDCALL_VIEW, -1, call_data, NULL);
					ad->call_end_type = CALL_END_TYPE_NONE;
					CALL_UI_DEBUG("Blink show: end call time");
					_vcui_view_common_call_end_show(start_time, normal_end.end_cause_type);
					_vcui_doc_remove_call_data(call_data);
				}
				else {
					_vcui_doc_remove_call_data(call_data);
					_vcui_view_common_call_terminate_or_view_change();
				}
			}
 		}
		break;

	case VC_ENGINE_MSG_INCOM_END_TO_UI:
		{
			_voicecall_dvc_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_common_with_handle_type incom_end = msg->incom_end;

			call_data_t *call_data = _vcui_doc_get_call_handle(incom_end.call_handle);
			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}

			_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN, call_data, EINA_FALSE);
			_vcui_doc_remove_call_data(call_data);

			_vcui_view_common_call_terminate_or_view_change();

		}
		break;

	case VC_ENGINE_MSG_REJECTED_END_TO_UI:
		{
			_voicecall_dvc_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_common_with_handle_type rejected_end = msg->rejected_end;

			call_data_t *call_data = _vcui_doc_get_call_handle(rejected_end.call_handle);

			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();

				return;
			}

			_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_REJECT, call_data, EINA_FALSE);
			_vcui_doc_remove_call_data(call_data);
			_vcui_view_common_call_terminate_or_view_change();
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_END_TO_UI:
		{
			_voicecall_dvc_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_outgoing_end_type outgoing_end = msg->outgoing_end;
			call_data_t *call_data = _vcui_doc_get_call_handle(outgoing_end.call_handle);

			if (call_data == NULL) {
				CALL_UI_DEBUG("It is the case which call orig is not received.");
				char data[VC_DATA_LENGTH_MAX] = { 0, };
				_vcui_view_popup_load(_vcui_get_endcause_string(outgoing_end.end_cause_type, data), POPUP_TIMEOUT_LONG, EINA_TRUE);
			} else {
				_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_OUTGOING, call_data, EINA_TRUE);
				if (outgoing_end.bauto_redial == EINA_TRUE) {
					CALL_UI_DEBUG("bauto_redial is EINA_TRUE");
					_vcui_doc_remove_call_data_only_list(call_data);
				} else {
					CALL_UI_DEBUG("show the call end screen");
					vcui_app_call_data_t *ad = _vcui_get_app_data();
					if (_vcui_doc_get_count() == 1 && ad->view_top == VIEW_DIALLING_VIEW) {
						ad->call_end_type = CALL_END_TYPE_SINGLE_CALL;
						_vcui_view_change(VIEW_ENDCALL_VIEW, -1, call_data, NULL);
					}
					_vcui_doc_remove_call_data(call_data);
				}
				_vcui_view_common_call_end_show_dialing(outgoing_end.end_cause_type, outgoing_end.bauto_redial);
			}

		}
		break;

	case VC_ENGINE_MSG_OUTGOING_END_SIGNAL_PLAY_TO_UI:
		{
			vc_engine_outgoing_end_signal_play_type outgoing_end_signal_play = msg->outgoing_end_signal_play;
			call_data_t *call_data = _vcui_doc_get_call_handle(outgoing_end_signal_play.call_handle);

			if (call_data != NULL) {
				_vcui_view_dialing_draw_txt_ended(ad->view_st[ad->view_top], outgoing_end_signal_play.end_cause_type);
			} else {
				CALL_UI_DEBUG("Check it whether call data exists. handle:[%d]", outgoing_end_signal_play.call_handle);
			}
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_ABORTED_TO_UI:
		{
			_voicecall_dvc_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_common_with_handle_type outgoing_aborted = msg->outgoing_aborted;
			call_data_t *call_data = _vcui_doc_get_call_handle(outgoing_aborted.call_handle);

			if (call_data == NULL)
				call_data = _vcui_doc_get_recent_mo();

			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}

			_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_OUTGOING, call_data, EINA_TRUE);

			_vcui_doc_remove_call_data(call_data);

			_vcui_view_common_call_terminate_or_view_change();

		}
		break;

	case VC_ENGINE_MSG_DTMF_ACK_TO_UI:
		{
#ifdef	PDIAL_SEND_DTMF

			vc_engine_dtmf_ack_type dtmf_ack = msg->dtmf_progress;

			if (EINA_FALSE == dtmf_ack.bstatus) {
				_vcui_view_popup_unload_progress(ad);
				if ((dtmf_ack.string_id != -1) && (dtmf_ack.string_id != IDS_CALL_POP_DTMF_SENT)) {
					CALL_UI_DEBUG("load popup window... Start");
					_vcui_view_popup_load(_(gszcall_error_msg[dtmf_ack.string_id]), POPUP_TIMEOUT_LONG, EINA_FALSE);
				}
			} else {
				CALL_UI_DEBUG("display_string:[%s], string_id:[%d]", dtmf_ack.display_string, dtmf_ack.string_id);
				_vcui_view_popup_load_sending_dtmf(_(gszcall_error_msg[dtmf_ack.string_id]), dtmf_ack.display_string);
			}
#endif
		}
		break;

	case VC_ENGINE_MSG_SS_HELD_TO_UI:
		{
			_vcui_doc_set_hold_all();
			_vcui_view_update();
 		}
		break;

	case VC_ENGINE_MSG_SS_RETREIVED_TO_UI:
		{
			_vcui_doc_set_unhold_all();
			_vcui_view_update();
 		}
		break;

	case VC_ENGINE_MSG_SS_SWAP_TO_UI:
		{
			_vcui_view_popup_unload(ad->popup_eo);

			if (ad->bholdisleft == EINA_TRUE) {
				ad->bswapped = EINA_FALSE;
			} else {
				ad->bswapped = EINA_TRUE;
			}

			_vcui_doc_set_swap_all();
			_vcui_view_auto_change();
 		}
		break;

	case VC_ENGINE_MSG_SS_SETUP_CONF_TO_UI:
		{
			_vcui_view_popup_unload(ad->popup_eo);

			_vcui_doc_set_unhold_all();
			_vcui_view_auto_change();
 		}
		break;

	case VC_ENGINE_MSG_SS_SPLIT_CONF_TO_UI:
		{
			vc_engine_common_with_handle_type ss_split_conf = msg->ss_split_conf;

			CALL_UI_DEBUG("[UI]The handle is %d ", ss_split_conf.call_handle);

			call_data_t *call_data = _vcui_doc_get_call_handle(ss_split_conf.call_handle);
			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			} else {
				_vcui_doc_set_hold_all();
				call_data->caller_status = CALL_UNHOLD;

				_vcui_view_auto_change();
 			}
		}
		break;

	case VC_ENGINE_MSG_SS_TRANSFERRED_TO_UI:
		break;

	case VC_ENGINE_MSG_SS_CONNECT_LINE_IND_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_FORWARD_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_ACTIVATE_TO_UI:
		{
			_vcui_view_popup_load(_("IDS_CALL_POP_UNHELD"), POPUP_TIMEOUT_SHORT, EINA_FALSE);
		}
		break;

	case VC_ENGINE_MSG_IND_HOLD_TO_UI:
		{
			_vcui_view_popup_load(_("IDS_CALL_POP_HELD"), POPUP_TIMEOUT_SHORT, EINA_FALSE);
		}
		break;

	case VC_ENGINE_MSG_IND_TRANSFER_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_SETUPCONFERENCE_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_BARRING_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_WAITING_TO_UI:
		{
			_vcui_view_popup_load(_("IDS_CALL_POP_WAITING_ACTIVE"), POPUP_TIMEOUT_SHORT, EINA_FALSE);
		}
		break;

	case VC_ENGINE_MSG_IND_CUGINFO_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_SSNOTIFY_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_CALLINGNAMEINFO_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_REDIRECT_CNF_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_ACTIVATECCBS_CNF_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_ACTIVATECCBS_USERINFO_TO_UI:
		break;

	case VC_ENGINE_MSG_GET_VOLUME_RESP_TO_UI:
		{
			vc_engine_vol_resp_type vol_resp = msg->vol_resp;
			_vcui_response_volume(vol_resp.vol_alert_type, vol_resp.vol_level);
		}
		break;

	case VC_ENGINE_MSG_SET_VOLUME_FROM_BT_TO_UI:
		{
			vc_engine_vol_set_from_bt_type vol_set_from_bt = msg->vol_set_from_bt;
			ad->bt_vol_val = vol_set_from_bt.vol_level;
			_vcui_view_popup_vol_bt(ad->bt_vol_val);
		}
		break;

	case VC_ENGINE_MSG_ACTION_NO_ACTIVE_TASK_TO_UI:
		{

		}
		break;

	case VC_ENGINE_MSG_ACTION_CALL_END_HELD_RETREIVED_TO_UI:
		{

		}
		break;

	case VC_ENGINE_MSG_ACTION_SAT_RESPONSE_TO_UI:
		{

		}
		break;

	case VC_ENGINE_MSG_ACTION_SAT_REQUEST_TO_UI:
		{

		}
		break;

	case VC_ENGINE_MSG_ERROR_OCCURED_TO_UI:
		break;

	case VC_ENGINE_MSG_IND_AOC_TO_UI:
		break;

	case VC_ENGINE_MSG_ACCEPT_CHOICE_BOX_TO_UI:
		{
			CALL_UI_DEBUG("not supported");
		}
		break;

	case VC_ENGINE_MSG_HEADSET_STATUS_TO_UI:
		{
			vc_engine_headset_status_type headset_status = msg->headset_status;
			ad->headset_status = headset_status.bstatus;
			if (ad->headset_status == EINA_TRUE) {
				ad->speaker_status = EINA_FALSE;
			}

			CALL_UI_DEBUG("Headset Status = %d", ad->headset_status);
			CALL_UI_DEBUG("ad->view_top:[%d]", ad->view_top);

			if ((ad->view_top == VIEW_INCALL_ONECALL_VIEW) || (ad->view_top == VIEW_INCALL_MULTICALL_SPLIT_VIEW)
			    || (ad->view_top == VIEW_INCALL_MULTICALL_CONF_VIEW) || (ad->view_top == VIEW_DIALLING_VIEW)) {
				if (ad->view_st[ad->view_top] != NULL) {
					ad->view_st[ad->view_top]->onUpdate(ad->view_st[ad->view_top], NULL, NULL);
				}
			}

		}
		break;

	case VC_ENGINE_MSG_EARJACK_STATUS_TO_UI:
		{
			vc_engine_earjack_status_type earjack_status = msg->earjack_status;

			CALL_UI_DEBUG("earjack Status = %d", earjack_status.bstatus);
			CALL_UI_DEBUG("ad->view_top:[%d]", ad->view_top);

			if(earjack_status.bstatus == EINA_TRUE) {
				if (ad->ctxpopup_radio_group_eo != NULL )
					elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_EARJACK);
#ifdef _NEW_SND_
#else
				_vcui_engine_change_sound_path(VCUI_AUDIO_EARJACK);
#endif
				
				ad->speaker_status = EINA_FALSE;
				ad->headset_status = EINA_FALSE;
				
				if ((ad->view_top == VIEW_INCALL_ONECALL_VIEW) || (ad->view_top == VIEW_INCALL_MULTICALL_SPLIT_VIEW)
					|| (ad->view_top == VIEW_INCALL_MULTICALL_CONF_VIEW) || (ad->view_top == VIEW_DIALLING_VIEW)) {
					if (ad->view_st[ad->view_top] != NULL) {
						ad->view_st[ad->view_top]->onUpdate(ad->view_st[ad->view_top], NULL, NULL);
					}
				}
			} else {
				if (_vcui_is_headset_conected() == EINA_TRUE) {
					if (ad->ctxpopup_radio_group_eo != NULL )
						elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_HEADSET);
					
#ifdef _NEW_SND_
#else
					_vcui_engine_change_sound_path(VCUI_AUDIO_HEADSET);
#endif

					ad->speaker_status = EINA_FALSE;
					ad->headset_status = EINA_TRUE;

					if ((ad->view_top == VIEW_INCALL_ONECALL_VIEW) || (ad->view_top == VIEW_INCALL_MULTICALL_SPLIT_VIEW)
						|| (ad->view_top == VIEW_INCALL_MULTICALL_CONF_VIEW) || (ad->view_top == VIEW_DIALLING_VIEW)) {
						if (ad->view_st[ad->view_top] != NULL) {
							ad->view_st[ad->view_top]->onUpdate(ad->view_st[ad->view_top], NULL, NULL);
						}
					}
				} else {
					if (ad->ctxpopup_radio_group_eo != NULL )
						elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_EARJACK);
					
#ifdef _NEW_SND_
#else
					_vcui_engine_change_sound_path(VCUI_AUDIO_EARJACK);
#endif

					ad->speaker_status = EINA_FALSE;
					ad->headset_status = EINA_FALSE;

					if ((ad->view_top == VIEW_INCALL_ONECALL_VIEW) || (ad->view_top == VIEW_INCALL_MULTICALL_SPLIT_VIEW)
						|| (ad->view_top == VIEW_INCALL_MULTICALL_CONF_VIEW) || (ad->view_top == VIEW_DIALLING_VIEW)) {
						if (ad->view_st[ad->view_top] != NULL) {
							ad->view_st[ad->view_top]->onUpdate(ad->view_st[ad->view_top], NULL, NULL);
						}
					}
				}
			}
		}
		break;

	case VC_ENGINE_MSG_ACTION_INCOM_FORCE_TO_UI:
		{
			call_data_t *recent_mo = _vcui_doc_get_recent_mo();
			if (recent_mo != NULL && recent_mo->call_handle == NO_HANDLE) {
				_vcui_doc_remove_call_data_only_list(recent_mo);
				_vcui_doc_set_mo_recent(NULL);
			}

		}
		break;

	case VC_ENGINE_MSG_MESSAGE_BOX_TO_UI:
		{
			_voicecall_dvc_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_msg_box_type msg_box = msg->msg_box;
			_vcui_view_popup_load(_(gszcall_error_msg[msg_box.string_id]), POPUP_TIMEOUT_LONG, EINA_TRUE);
		}
		break;

	case VC_ENGINE_MSG_REDIAL_TO_UI:
		{
			CALL_UI_DEBUG("not used");
		}
		break;

	case VC_ENGINE_MSG_CREATE_NEWVOICEFILE_TO_UI:
		{
			CALL_UI_DEBUG("not used");
		}

	default:
		break;
	}

	_vcui_doc_all_print(gszcall_callback_msg[event]);

	CALL_UI_DEBUG("End..");

}
