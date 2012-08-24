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
#include "vcui-app-window.h"
#include "vcui-app-data.h"
#include "vcui-document.h"

#include "vcui-view-dialing.h"
#include "vcui-view-incoming-lock.h"
#include "vcui-view-keypad.h"
#include "vcui-view-single-call.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-callend.h"
#include "vcui-view-quickpanel.h"
#include "vcui-view-popup.h"
#include <glib.h>
#include <glib-2.0/glib.h>
#include <dbus/dbus-glib.h>
#include <notification.h>
#ifdef SIGNAL_HANDLER
#include <signal.h>
#endif
#include "appsvc.h"
#include "voice-call-engine.h"

static vcui_app_call_data_t global_ad;

#ifdef SIGNAL_HANDLER
#define VCUI_SIG_NUM 12
static struct sigaction vcui_app_sigact;
static struct sigaction vcui_app_sigoldact[VCUI_SIG_NUM];
static int vcui_app_sig_to_handle[] = { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGQUIT, SIGSEGV, SIGSYS, SIGTRAP, SIGXCPU, SIGXFSZ, SIGTERM, SIGPIPE };
#endif

static gboolean g_avoid_multi_setup = EINA_FALSE;
static Eina_Bool __vcui_avoid_multi_setup_timer_cb(void *data);

#define CALL_PKG_NAME "org.tizen.call"
#define CISS_AUL_CMD "org.tizen.ciss"
#define CISS_MODE_OPT "REQ"
#define __VCUI_NOTIFICATION_CALL_GROUP_ID 1001

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

static int __vcui_launch_ciss(const char *number);
static void __vcui_cache_flush_set(Evas *e);
static void __vcui_init_view_register_function(vcui_app_call_data_t *app_data, vcui_app_call_view_id_t view_id, voice_call_view_data_t *(*view_new) ());
static int __vcui_app_create(void *data);
static int __vcui_app_pause(void *data);
static int __vcui_app_reset(bundle *kb, void *data);
static int __vcui_app_resume(void *data);
static int __vcui_app_terminate(void *data);
static void __vcui_fade_out_cb_routine(void);
static int __vcui_lang_changed_cb(void *data);
static int __vcui_low_mem_cb(void *data);
static int __vcui_low_bat_cb(void *data);
static int __vcui_rotate_cb(enum appcore_rm m, void *data);
static void __vcui_init(vcui_app_call_data_t *ad);
static void __vcui_app_callback(int event, void *pdata, void *puser_data);

#ifdef SIGNAL_HANDLER
/**
* This function serves as the signal handler function for the SIGSEGV Signal
*
* @return		nothing
* @param[in]		signal_no		Signal Number
* @param[in]		signal_info	Information associated with the generated signal
* @param[in]		signal_context	Signal Context Info
*/
static void __vcui_exit_handler(void)
{
	printf("\n __vcui_app_exit_handler\n");
	/*syslog (LOG_INFO, "[VOICE-CALL] __vcui_app_exit_handler\n"); */
}

/**
* This function serves as the signal handler function for the SIGSEGV Signal
*
* @return		nothing
* @param[in]		signal_no		Signal Number
* @param[in]		signal_info	Information associated with the generated signal
* @param[in]		signal_context	Signal Context Info
*/
static void __vcui_sigsegv_handler(int signal_no)
{
	int i = 0;

	CALL_UI_DEBUG("SISEGV Received, Signal Number: :%d \n", signal_no);

	vcall_engine_force_reset();
	__vcui_fade_out_cb_routine();

	for (i = 0; i < VCUI_SIG_NUM; i++) {
		sigaction(vcui_app_sig_to_handle[i], &(vcui_app_sigoldact[i]), NULL);
	}

	raise(signal_no);	/*raise signal intentionally (pass the same signal) */
}

/**
* This function registers a user space signal handler for the signal SIGSEGV (Signal #11)
*
* @return		nothing
*/
static void __vcui_register_sigsegv_handler()
{
	CALL_UI_DEBUG("..");

	int i = 0;
	vcui_app_sigact.sa_flags = SA_NOCLDSTOP;
	vcui_app_sigact.sa_handler = (void *)__vcui_sigsegv_handler;
	sigemptyset(&vcui_app_sigact.sa_mask);

	for (; i < VCUI_SIG_NUM; i++) {
		sigaction(vcui_app_sig_to_handle[i], &vcui_app_sigact, &(vcui_app_sigoldact[i]));
	}

}
#endif

static int __vcui_launch_ciss(const char *number)
{
	bundle *kb;

	CALL_UI_DEBUG("number(%s)");
	kb = bundle_create();
	bundle_add(kb, "CISS_LAUNCHING_MODE", CISS_MODE_OPT);
	bundle_add(kb, "CISS_REQ_STRING", number);
	aul_launch_app(CISS_AUL_CMD, kb);
	bundle_free(kb);
	return VC_NO_ERROR;
}

static int __vcui_app_create(void *data)
{
	CALL_UI_DEBUG("__vcui_app_create()..");
	CALL_UI_KPI("__vcui_app_create start");
	vcui_app_call_data_t *ad = data;

	elm_theme_extension_add(NULL, CALL_THEME);

	CALL_UI_KPI("_vcui_app_win_create_main start");
	ad->win_main = (Evas_Object *)_vcui_app_win_create_main(ad, PACKAGE);
	CALL_UI_KPI("_vcui_app_win_create_main done");
	if (ad->win_main == NULL) {
		CALL_UI_DEBUG("ERROR");
		return VC_ERROR;
	}

	UG_INIT_EFL(ad->win_main, UG_OPT_INDICATOR_ENABLE);

	__vcui_hide_main_ui_set_flag();

	ad->evas = evas_object_evas_get(ad->win_main);
	ad->scale_factor = elm_config_scale_get();

	__vcui_cache_flush_set(ad->evas);

	ecore_init();
	ecore_x_init(NULL);
	_vcui_app_win_key_grab(ad);

	_vcui_view_common_timer_text_init();

	_vcui_app_win_set_noti_type(EINA_TRUE);

	/* add system event callback */
	appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, __vcui_lang_changed_cb, ad);
	appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, __vcui_low_mem_cb, NULL);
	appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, __vcui_low_bat_cb, NULL);

	CALL_UI_KPI("appcore_set_rotation_cb start");
	appcore_set_rotation_cb(__vcui_rotate_cb, ad);
	CALL_UI_KPI("appcore_set_rotation_cb done");

#ifdef SIGNAL_HANDLER
	__vcui_register_sigsegv_handler();
	atexit(__vcui_exit_handler);
#endif

	/*
	** Requested by Inpyo Kang
	** This is temp fix for email UG bs proglem.. must be removed later.
	*/
	elm_config_preferred_engine_set("opengl_x11");

	CALL_UI_KPI("__vcui_app_create done");
	return VC_NO_ERROR;
}

static int __vcui_app_pause(void *data)
{
	CALL_UI_DEBUG("__vcui_app_pause()..");

	return VC_NO_ERROR;
}

static int __vcui_app_reset(bundle *kb, void *data)
{
/*
#aul_test launch org.tizen.call launch-type "MO" number "01030011234"
#aul_test launch org.tizen.call launch-type "MT" number "01030011234" handle "1" clicause "1" fwded "1" activeline "1"
#aul_test open_content "01030011234"  ( /opt/share/miregex/phonenum.uri)
*/
	CALL_UI_DEBUG("__vcui_app_reset()..");
	CALL_UI_KPI("__vcui_app_reset start");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	const char *launch_type = NULL;
	const char *mime_type = NULL;
	const char *tmp = NULL;
	const char *uri_bundle = NULL;
	char *telnum = NULL;

#ifdef _RESET_OPEN_APP_
	{
		launch_type = bundle_get_val(kb, "__AUL_CMD__");
		if (launch_type != NULL) {
			CALL_UI_DEBUG("launch type: [%s]", launch_type);
			if (!strncmp(launch_type, "OPEN_APP", 8)) {
				elm_win_raise(ad->win_main);
				return;
			}
		}
	}
#endif

	/* mime content based AUL */
	mime_type = bundle_get_val(kb, AUL_K_MIME_TYPE);
	if (mime_type != NULL) {
		CALL_UI_DEBUG("mime_type: [%s]", mime_type);
		if (strncmp(mime_type, "phonenum.uri", 12) == 0 || strncmp(mime_type, "tel.uri", 7) == 0) {
			tmp = bundle_get_val(kb, AUL_K_MIME_CONTENT);
			if (tmp == NULL) {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			CALL_UI_DEBUG("AUL_K_MIME_CONTENT: [%s]", tmp);
			if (strncmp(tmp, "tel:", 4) == 0) {
				telnum = (char *)tmp + 4;
			} else {
				telnum = (char *)tmp;
			}

			CALL_UI_DEBUG("number: [%s]", telnum);

			vcui_call_mo_data_t call_data;
			memset(&call_data, 0, sizeof(call_data));
			snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", telnum);
			vcall_engine_process_normal_call(call_data.call_number, call_data.ct_index, EINA_FALSE);

			return VC_NO_ERROR;
		} else {
			CALL_UI_DEBUG("wrong mime type!!");
			elm_exit();
			return VC_NO_ERROR;
		}
	}

	uri_bundle = (const char *)appsvc_get_uri(kb);
	if (uri_bundle != NULL) {
		CALL_UI_DEBUG("tmp: [%s]", uri_bundle);
		if (strncmp(uri_bundle, "tel:", 4) == 0) {
			telnum = (char *)uri_bundle + 4;
			CALL_UI_DEBUG("number: [%s]", telnum);

			tmp = (char *)appsvc_get_data(kb, "calltype");

			if (tmp) {
				CALL_UI_DEBUG("calltype: [%s]", tmp);
				if (!strncmp(tmp, "EMERGENCY", 9)) {
					vcui_call_ecc_data_t call_data;
					memset(&call_data, 0, sizeof(call_data));
					snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", telnum);
					vcall_engine_process_emergency_call(call_data.call_number);
				} else {
					CALL_UI_DEBUG("wrong calltype!");
					elm_exit();
					return VC_ERROR;
				}
			} else {
				vcui_call_mo_data_t call_data;
				memset(&call_data, 0, sizeof(call_data));
				snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", telnum);

				tmp = (char *)appsvc_get_data(kb, "ctindex");
				if (tmp) {
					CALL_UI_DEBUG("ctindex: [%s]", tmp);
					call_data.ct_index = atoi(tmp);
				} else {
					CALL_UI_DEBUG("bundle val is NULL");
					call_data.ct_index = -1;
				}

				if ((vcall_engine_check_incall_ss_string(call_data.call_number) == EINA_TRUE) && (_vcui_doc_get_all_call_data_count() >= 1)) {
					vcall_engine_process_incall_ss(call_data.call_number);
					_vcui_view_auto_change();
				} else if (vcall_engine_check_ss_string(call_data.call_number) == EINA_TRUE) {
					__vcui_launch_ciss(call_data.call_number);
					if (_vcui_doc_get_all_call_data_count() == 0)
						elm_exit();
					return VC_NO_ERROR;
				} else {
					vcall_engine_process_normal_call(call_data.call_number, call_data.ct_index, EINA_FALSE);
				}
			}

			return VC_NO_ERROR;
		} else {
			CALL_UI_DEBUG("wrong type!");
			elm_exit();
			return VC_NO_ERROR;
		}
	}

	/* AUL */
	launch_type = bundle_get_val(kb, "launch-type");
	if (launch_type != NULL) {
		CALL_UI_DEBUG("launch type: [%s]", launch_type);
		if (!strncmp(launch_type, "MO", 2)) {

			if (g_avoid_multi_setup == EINA_TRUE) {
				CALL_UI_DEBUG("Avoid multi touch setup");
				return VC_NO_ERROR;
			}

			ecore_timer_add(3.5, __vcui_avoid_multi_setup_timer_cb, ad);
			g_avoid_multi_setup = EINA_TRUE;

			vcui_call_mo_data_t call_data;
			memset(&call_data, 0, sizeof(call_data));

			tmp = bundle_get_val(kb, "number");
			if (tmp) {
				CALL_UI_DEBUG("number: [%s]", tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}
			snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", tmp);

			tmp = bundle_get_val(kb, "ctindex");
			if (tmp) {
				CALL_UI_DEBUG("ctindex: [%s]", tmp);
				call_data.ct_index = atoi(tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				call_data.ct_index = -1;
			}

			if ((vcall_engine_check_incall_ss_string(call_data.call_number) == EINA_TRUE) && (_vcui_doc_get_all_call_data_count() >= 1)) {
				vcall_engine_process_incall_ss(call_data.call_number);
				_vcui_view_auto_change();
			} else if (vcall_engine_check_ss_string(call_data.call_number) == EINA_TRUE) {
				__vcui_launch_ciss(call_data.call_number);
				if (_vcui_doc_get_all_call_data_count() == 0)
					elm_exit();
				return VC_NO_ERROR;
			} else {
				vcall_engine_process_normal_call(call_data.call_number, call_data.ct_index, EINA_FALSE);
			}
		} else if (!strncmp(launch_type, "MT", 2)) {
			vcui_call_mt_data_t call_data;
			memset(&call_data, 0, sizeof(call_data));

			tmp = bundle_get_val(kb, "handle");
			if (tmp) {
				CALL_UI_DEBUG("handle: [%s]", tmp);
				call_data.call_handle = atoi(tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			tmp = bundle_get_val(kb, "calltype");
			if (tmp) {
				CALL_UI_DEBUG("calltype: [%s]", tmp);
				call_data.call_type = atoi(tmp);
			} else {
				CALL_UI_DEBUG("calltype is NULL but NOT mendatory");
				call_data.call_type = 0;
			}

			tmp = bundle_get_val(kb, "cliindicator");
			if (tmp) {
				CALL_UI_DEBUG("cliindicator: [%s]", tmp);
				call_data.cli_presentation_indicator = atoi(tmp);
			} else {
				CALL_UI_DEBUG("cliindicator is NULL but NOT mendatory");
				call_data.cli_presentation_indicator = 0;
			}

			tmp = bundle_get_val(kb, "number");
			if (tmp) {
				CALL_UI_DEBUG("number: [%s]", tmp);
				vcall_engine_util_strcpy(call_data.call_num, VC_PHONE_NUMBER_LENGTH_MAX, tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				/*return VC_NO_ERROR; *//*check clir mt call. */
			}

			tmp = bundle_get_val(kb, "name_mode");
			if (tmp) {
				CALL_UI_DEBUG("name_mode: [%s]", tmp);
				call_data.calling_name_mode = atoi(tmp);
			} else {
				CALL_UI_DEBUG("name_mode is NULL but NOT mendatory");
				call_data.calling_name_mode = -1;
			}

			tmp = bundle_get_val(kb, "name");
			if (tmp) {
				CALL_UI_DEBUG("name: [%s]", tmp);
				vcall_engine_util_strcpy(call_data.calling_name, VC_PHONE_NAME_LENGTH_MAX, tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL but NOT mendatory");
			}

			tmp = bundle_get_val(kb, "rdnumber");
			if (tmp) {
				CALL_UI_DEBUG("rdnumber: [%s]", tmp);
				vcall_engine_util_strcpy(call_data.redirected_number, VC_PHONE_NUMBER_LENGTH_MAX, tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL but NOT mendatory");
			}

			tmp = bundle_get_val(kb, "rdsubaddress");
			if (tmp) {
				CALL_UI_DEBUG("rdnumber: [%s]", tmp);
				vcall_engine_util_strcpy(call_data.redirected_sub_address, VC_PHONE_SUBADDRESS_LENGTH_MAX, tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL but NOT mendatory");
			}

			tmp = bundle_get_val(kb, "clicause");
			if (tmp) {
				CALL_UI_DEBUG("clicause: [%s]", tmp);
				call_data.cli_cause = atoi(tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			tmp = bundle_get_val(kb, "fwded");
			if (tmp) {
				CALL_UI_DEBUG("fwded: [%s]", tmp);
				call_data.bfwded = atoi(tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			tmp = bundle_get_val(kb, "activeline");
			if (tmp) {
				CALL_UI_DEBUG("activeline: [%s]", tmp);
				call_data.active_line = atoi(tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			vcall_engine_incoming_info_t mtcall_info;

			mtcall_info.call_handle = call_data.call_handle;
			mtcall_info.call_type = call_data.call_type;
			mtcall_info.cli_presentation_indicator = call_data.cli_presentation_indicator;
			vcall_engine_util_strcpy(mtcall_info.call_num, sizeof(mtcall_info.call_num), call_data.call_num);
			mtcall_info.calling_name_mode = call_data.calling_name_mode;
			vcall_engine_util_strcpy(mtcall_info.calling_name, sizeof(mtcall_info.calling_name), call_data.calling_name);
			vcall_engine_util_strcpy(mtcall_info.redirected_number, sizeof(mtcall_info.redirected_number), call_data.redirected_number);
			vcall_engine_util_strcpy(mtcall_info.redirected_sub_address, sizeof(mtcall_info.redirected_sub_address), call_data.redirected_sub_address);
			mtcall_info.cli_cause = call_data.cli_cause;
			mtcall_info.bfwded = call_data.bfwded;
			mtcall_info.active_line = call_data.active_line;

			vcall_engine_process_incoming_call(&mtcall_info);
		} else if (!strncmp(launch_type, "EMERGENCY", 9)) {
			vcui_call_ecc_data_t call_data;
			memset(&call_data, 0, sizeof(call_data));

			tmp = bundle_get_val(kb, "number");
			if (tmp) {
				CALL_UI_DEBUG("number: [%s]", tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
			}
			snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", tmp);
			vcall_engine_process_emergency_call(call_data.call_number);
		} else if (!strncmp(launch_type, "SATSETUPCALL", 12)) {
			vcui_call_sat_data_t sat_setup_call_info;
			memset(&sat_setup_call_info, 0, sizeof(sat_setup_call_info));

			tmp = bundle_get_val(kb, "cmd_id");
			if (tmp) {
				CALL_UI_DEBUG("cmd_id: [%s]", tmp);
				sat_setup_call_info.command_id = atoi(tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			tmp = bundle_get_val(kb, "cmd_qual");
			if (tmp) {
				CALL_UI_DEBUG("cmd_qual: [%s]", tmp);
				sat_setup_call_info.command_qualifier = atoi(tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			tmp = bundle_get_val(kb, "disp_text");
			if (tmp) {
				CALL_UI_DEBUG("disp_text: [%s]", tmp);
				vcall_engine_util_strcpy(sat_setup_call_info.disp_text, sizeof(sat_setup_call_info.disp_text), tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				/*elm_exit();
				   return VC_NO_ERROR; */
			}

			tmp = bundle_get_val(kb, "call_num");
			if (tmp) {
				CALL_UI_DEBUG("call_num: [%s]", tmp);
				vcall_engine_util_strcpy(sat_setup_call_info.call_num, sizeof(sat_setup_call_info.call_num), tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			tmp = bundle_get_val(kb, "dur");
			if (tmp) {
				CALL_UI_DEBUG("dur: [%s]", tmp);
				sat_setup_call_info.duration = atoi(tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			vcall_engine_sat_setup_call_info_t sat_setup_data;
			memset(&sat_setup_data, 0, sizeof(sat_setup_data));
			sat_setup_data.command_id = sat_setup_call_info.command_id;
			sat_setup_data.command_qualifier = sat_setup_call_info.command_qualifier;
			sat_setup_data.duration = sat_setup_call_info.duration;
			memcpy(sat_setup_data.disp_text, sat_setup_call_info.disp_text, sizeof(sat_setup_data.disp_text));
			memcpy(sat_setup_data.call_num, sat_setup_call_info.call_num, VC_PHONE_NUMBER_LENGTH_MAX);

			vcall_engine_process_sat_setup_call(&sat_setup_data);
		} else if (!strncmp(launch_type, "ECCTEST", 7)) {
			vcui_call_ecc_data_t call_data;
			memset(&call_data, 0, sizeof(call_data));
			vcall_engine_process_emergency_call_test(call_data.call_number);
		} else if (!strncmp(launch_type, "DOWNLOADCALL", 12)) {
			vcui_call_mo_data_t call_data;
			memset(&call_data, 0, sizeof(call_data));

			tmp = bundle_get_val(kb, "number");
			if (tmp) {
				CALL_UI_DEBUG("number: [%s]", tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
			}

			if (tmp != NULL) {
				snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", tmp);
			} else {
				snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", "116");
			}
			vcall_engine_util_strcpy(call_data.call_number, sizeof(call_data.call_number), tmp);
			vcall_engine_process_normal_call(call_data.call_number, call_data.ct_index, EINA_TRUE);
		} else if (!strncmp(launch_type, "CALL_COMMAND", 12)) {
			vcui_call_mo_data_t call_data;
			memset(&call_data, 0, sizeof(call_data));

			tmp = bundle_get_val(kb, "value");
			if (tmp) {
				CALL_UI_DEBUG("number: [%s]", tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
			}

			if (!strncmp(tmp, "REJECT_MT", 9)) {
				vcall_engine_reject_call();
			}	/* If need to add more command, add here. */
		} else {
			CALL_UI_DEBUG("unknown launch type");
		}
		CALL_UI_KPI("__vcui_app_reset done");
		return VC_NO_ERROR;
	}

	CALL_UI_DEBUG("bundle data is wrong!!");
	elm_exit();
	return VC_NO_ERROR;
}

static int __vcui_app_resume(void *data)
{
	CALL_UI_DEBUG("__vcui_app_resume()..");

	return VC_NO_ERROR;
}

static int __vcui_app_terminate(void *data)
{
	CALL_UI_DEBUG("__vcui_app_terminate()..");

	if (_vcui_doc_get_all_call_data_count() >= 1) {
		CALL_UI_DEBUG("WARNING!! call exists. abnormal terminate!!");
		vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_CALLS);
		vcall_engine_set_to_default();
	}

	__vcui_fade_out_cb_routine();
	return VC_NO_ERROR;
}

static void __vcui_cache_flush_set(Evas *e)
{
#ifdef _CACHE_FLUSH_
	evas_image_cache_set(e, 4096 * 1024);
	evas_font_cache_set(e, 512 * 1024);

	edje_file_cache_set(0);
	edje_collection_cache_set(0);
#endif
}

static void __vcui_init_view_register_function(vcui_app_call_data_t *app_data, vcui_app_call_view_id_t view_id, voice_call_view_data_t *(*view_new) ())
{
	app_data->func_new[view_id] = view_new;
}

void _vcui_response_volume(int vol_alert_type, int vol_level)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (vol_alert_type == VCUI_VOL_VOICE) {
		ad->voice_vol_val = vol_level;
		_vcui_set_volume(ad->vol_key_status);
	} else if (vol_alert_type == VCUI_VOL_HEADSET) {
		ad->bt_vol_val = vol_level;
		_vcui_set_volume(ad->vol_key_status);
	} else {
		CALL_UI_DEBUG("ERROR");
	}
}

void _vcui_set_volume(int key_status)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (_vcui_doc_get_all_call_data_count() == 0) {
		CALL_UI_DEBUG("ignore it");
		return;
	}

	if (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW) {
		int err_code = 0;
		int settings_sound_status = EINA_FALSE;

		err_code = vconf_get_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &settings_sound_status);
		if (settings_sound_status == EINA_FALSE) {
			CALL_UI_DEBUG("ringtone vol is ignored in sound off status.");
			return;
		}

		if (ad->bmute_ringtone == EINA_TRUE) {
			CALL_UI_DEBUG("during mute ringtone, vol will not changed");
			return;
		}

		if (ad->ringtone_val < RINGTONE_MIN) {
			int vol_level = vcall_engine_get_volume_level(VCALL_ENGINE_VOL_TYPE_RINGTONE);

			if ((vol_level < RINGTONE_MIN) || (vol_level > RINGTONE_MAX)) {
				CALL_UI_DEBUG("ERROR : ringtone vol:[%d]", vol_level);
				return;
			}
			ad->ringtone_val = vol_level;
		}

		CALL_UI_DEBUG("RINGTONE : Set Volume");
		if (key_status == VAL_VOL_DOWN && ad->ringtone_val > RINGTONE_MIN) {
			ad->ringtone_val--;
			vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_RINGTONE, ad->ringtone_val);
		} else if (key_status == VAL_VOL_UP && ad->ringtone_val < RINGTONE_MAX) {
			ad->ringtone_val++;
			vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_RINGTONE, ad->ringtone_val);
		}
		_vcui_view_popup_vol_ringtone(ad->ringtone_val);
	} else if (ad->headset_status == EINA_TRUE) {
		if (ad->bt_vol_val < BT_VOL_MIN) {
			CALL_UI_DEBUG("BT VOL : Get Volume");
			ad->vol_key_status = key_status;
			ad->bt_vol_val = vcall_engine_get_volume_level(VCALL_ENGINE_VOL_TYPE_HEADSET);
			return;
		}

		CALL_UI_DEBUG("BT VOL : Set Volume");
		if (key_status == VAL_VOL_DOWN && ad->bt_vol_val > BT_VOL_MIN) {	/*Key Down */
			ad->bt_vol_val--;
			vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_HEADSET, ad->bt_vol_val);
		} else if (key_status == VAL_VOL_UP && ad->bt_vol_val < BT_VOL_MAX) {	/*Key Up */
			ad->bt_vol_val++;
			vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_HEADSET, ad->bt_vol_val);
		}
		_vcui_view_popup_vol_bt(ad->bt_vol_val);
	} else {
		CALL_UI_DEBUG("TAPI VOL : Get Volume");
		ad->vol_key_status = key_status;
		ad->voice_vol_val = vcall_engine_get_volume_level(VCALL_ENGINE_VOL_TYPE_VOICE);

		CALL_UI_DEBUG("TAPI VOL : Set Volume");
		if (key_status == VAL_VOL_DOWN && ad->voice_vol_val > VOICE_VOL_MIN) {	/*Key Down */
			ad->voice_vol_val--;
			vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_VOICE, ad->voice_vol_val);
		} else if (key_status == VAL_VOL_UP && ad->voice_vol_val < VOICE_VOL_MAX) {	/*Key Up */
			ad->voice_vol_val++;
			vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_VOICE, ad->voice_vol_val);
		}
		_vcui_view_popup_vol_voice(ad->voice_vol_val);
	}

}

static void __vcui_fade_out_cb_routine()
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (ad->child_is == 1) {
		/*system("killall dialer"); */
	}
	if (ad->disp && ad->win) {
		utilx_ungrab_key(ad->disp, ad->win, KEY_VOLUMEUP);
		utilx_ungrab_key(ad->disp, ad->win, KEY_VOLUMEDOWN);
		utilx_ungrab_key(ad->disp, ad->win, KEY_SELECT);
	}

	if (ad->focus_in) {
		ecore_event_handler_del(ad->focus_in);
		ad->focus_in = NULL;
	}
	if (ad->focus_out) {
		ecore_event_handler_del(ad->focus_out);
		ad->focus_out = NULL;
	}

	_vcui_doc_remove_all_call_data();
	vcall_engine_device_control_lcd_state(VC_LCD_ON_UNLOCK);
	_vcui_view_common_timer_destroy();
}

static int __vcui_lang_changed_cb(void *data)
{
	CALL_UI_DEBUG("..");

	_vcui_view_auto_change();
	_vcui_view_quickpanel_change();

	return VC_NO_ERROR;
}

static int __vcui_low_mem_cb(void *data)
{
	CALL_UI_DEBUG("..");

	return VC_NO_ERROR;
}

static int __vcui_low_bat_cb(void *data)
{
	CALL_UI_DEBUG("..");

	return VC_NO_ERROR;
}

static int __vcui_get_rotate_angle(enum appcore_rm rotate_mode)
{
	CALL_UI_DEBUG("..");
	int rotate_angle;
	if (APPCORE_RM_UNKNOWN == rotate_mode) {
		appcore_get_rotation_state(&rotate_mode);
	}

	switch (rotate_mode) {
	case APPCORE_RM_PORTRAIT_NORMAL:
					/**< Portrait mode */
		CALL_UI_DEBUG("rotate mode is APPCORE_RM_PORTRAIT_NORMAL");
		rotate_angle = 0;
		break;
	case APPCORE_RM_PORTRAIT_REVERSE:
					 /**< Portrait upside down mode */
		CALL_UI_DEBUG("rotate mode is APPCORE_RM_PORTRAIT_REVERSE");
		rotate_angle = 180;
		break;
	case APPCORE_RM_LANDSCAPE_NORMAL:
					 /**< Left handed landscape mode */
		CALL_UI_DEBUG("rotate mode is APPCORE_RM_LANDSCAPE_NORMAL");
		rotate_angle = 270;
		break;
	case APPCORE_RM_LANDSCAPE_REVERSE:
					  /**< Right handed landscape mode */
		CALL_UI_DEBUG("rotate mode is APPCORE_RM_LANDSCAPE_REVERSE");
		rotate_angle = 90;
		break;
	default:
		rotate_angle = -1;
		break;
	}

	return rotate_angle;
}

static void __vcui_ug_handle_rotate_event(const int rotate_angle, void *data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	switch (rotate_angle) {
	case 0:
	       /**< Portrait mode */
		CALL_UI_DEBUG("rotate mode is APPCORE_RM_PORTRAIT_NORMAL");
		ug_send_event(UG_EVENT_ROTATE_PORTRAIT);
		break;
	case 180:
		 /**< Portrait upside down mode */
		CALL_UI_DEBUG("rotate mode is APPCORE_RM_PORTRAIT_REVERSE");
		ug_send_event(UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN);
		break;
	case 270:
		 /**< Left handed landscape mode */
		CALL_UI_DEBUG("rotate mode is APPCORE_RM_LANDSCAPE_NORMAL");
		ug_send_event(UG_EVENT_ROTATE_LANDSCAPE);
		break;
	case 90:
		/**< Right handed landscape mode */
		CALL_UI_DEBUG("rotate mode is APPCORE_RM_LANDSCAPE_REVERSE");
		ug_send_event(UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN);
		break;
	default:
		break;
	}
	if ((rotate_angle >= 0)
		&& (ad->contact_ug == NULL)) {
		elm_win_rotation_with_resize_set(ad->win_main, rotate_angle);
	}
}

#ifdef	__LANDSCAPE_MODE_
static Eina_Bool __vcui_handle_rotate_evnt_idle_cb(void *data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad == NULL)
		return EINA_FALSE;

	CALL_UI_DEBUG("Rotate on ad->view_top[%d]", ad->view_top);
	_vcui_view_change(ad->view_top, 0, NULL, NULL);
	return ECORE_CALLBACK_CANCEL;
}

static void __vcui_handle_rotate_event(const int rotate_angle, void *data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad == NULL) {
		CALL_UI_DEBUG("ad is NULL");
		return;
	}

	if (0 == rotate_angle || 180 == rotate_angle || 270 == rotate_angle || 90 == rotate_angle) {
		ad->rotate_angle = rotate_angle;
		ecore_idler_add(__vcui_handle_rotate_evnt_idle_cb, ad);
	}
	return;
}
#endif

static int __vcui_rotate_cb(enum appcore_rm rotate_mode, void *data)
{
	CALL_UI_DEBUG("__vcui_rotate_cb()..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	int rotate_angle;
	if (ad == NULL) {
		CALL_UI_DEBUG("ad is NULL");
		return VC_ERROR;
	}

	rotate_angle = __vcui_get_rotate_angle(rotate_mode);
	CALL_UI_DEBUG("rotate_angle [%d]", rotate_angle);
	if (ad->contact_ug)
	{
		__vcui_ug_handle_rotate_event(rotate_angle, ad);
	}
#ifdef	__LANDSCAPE_MODE_
	else {
		/* Implement the landscape implementation here */
		__vcui_handle_rotate_event(rotate_angle, ad);
	}
#endif
	return VC_NO_ERROR;
}

static void __vcui_init(vcui_app_call_data_t *ad)
{
	CALL_UI_KPI("g_type_init start");
	g_type_init();
	CALL_UI_KPI("g_type_init done");
	_vcui_doc_data_init();
	_vcui_view_common_init();
	vcall_engine_init((vcall_engine_app_cb) __vcui_app_callback, ad);

	CALL_UI_KPI("__vcui_init_view_register_function for all views start");
	__vcui_init_view_register_function(ad, VIEW_DIALLING_VIEW, _vcui_view_dialing_new);
	__vcui_init_view_register_function(ad, VIEW_INCOMING_LOCK_VIEW, _vcui_view_incoming_lock_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_ONECALL_VIEW, _vc_ui_view_single_call_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_MULTICALL_SPLIT_VIEW, _vcui_view_multi_call_split_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_MULTICALL_CONF_VIEW, _vcui_view_multi_call_conf_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_MULTICALL_LIST_VIEW, _vcui_view_multi_call_list_new);
	__vcui_init_view_register_function(ad, VIEW_ENDCALL_VIEW, _vcui_view_callend_new);
	__vcui_init_view_register_function(ad, VIEW_QUICKPANEL_VIEW, _vcui_view_qp_new);
	CALL_UI_KPI("__vcui_init_view_register_function for all views done");

	ad->view_top = -1;
	ad->view_before_top = -1;
	ad->headset_status = (int)_vcui_is_headset_conected();
	ad->speaker_status = EINA_FALSE;
	ad->mute_status = EINA_FALSE;
	ad->child_is = -1;
	ad->show_flag = WIN_HIDE;
	ad->ringtone_val = -1;
	ad->voice_vol_val = -1;
	ad->bt_vol_val = -1;
	ad->call_end_type = CALL_END_TYPE_NONE;
	ad->wbamr_status= EINA_FALSE;
}

char *_vcui_get_endcause_string(int end_cause, char *data)
{
	char *string_id = NULL;
	CALL_UI_DEBUG("end type : %d", end_cause);
	switch (end_cause) {
	case VC_ENGINE_ENDCAUSE_USER_UNAVAILABLE:
		string_id = _("IDS_CALL_BODY_CALLED_PARTY_UNAVAILABLE");
		break;
	case VC_ENGINE_ENDCAUSE_UNASSIGNED_NUMBER:
		string_id = _("IDS_CALL_BODY_NUMBER_DOES_NOT_EXIST");
		break;
	case VC_ENGINE_ENDCAUSE_USER_DOESNOT_RESPOND:
		string_id = _("IDS_CALL_BODY_NO_ANSWER");
		break;
	case VC_ENGINE_ENDCAUSE_CALL_DISCONNECTED:
		string_id = _("IDS_CALL_BODY_DISCONNECTED");
		break;
	case VC_ENGINE_ENDCAUSE_CALL_ENDED:
		string_id = _("IDS_CALL_BODY_CALLENDED");
		break;
	case VC_ENGINE_ENDCAUSE_CALL_SERVICE_NOT_ALLOWED:
		string_id = _("IDS_CALL_POP_SERVICE_NOT_ALLOWED");
		break;
	case VC_ENGINE_ENDCAUSE_CALL_BARRED:
		string_id = _("IDS_CALL_POP_CALL_BARRED");
		break;
	case VC_ENGINE_ENDCAUSE_NO_SERVICE:
		string_id = _("IDS_CALL_POP_NOSERVICE");
		break;
	case VC_ENGINE_ENDCAUSE_NW_BUSY:
		string_id = _("IDS_CALL_POP_NETWORKBUSY");
		break;
	case VC_ENGINE_ENDCAUSE_NW_FAILED:
		string_id = _("IDS_CALL_POP_NETWORK_UNAVAILABLE");
		break;
	case VC_ENGINE_ENDCAUSE_SERVICE_TEMP_UNAVAILABLE:
		string_id = _("IDS_CALL_BODY_SERVICE_UNAVAILABLE");
		break;
	case VC_ENGINE_ENDCAUSE_NO_ANSWER:
		string_id = _("IDS_CALL_BODY_NO_ANSWER");
		break;
	case VC_ENGINE_ENDCAUSE_NO_CREDIT:
		string_id = _("IDS_CALL_POP_NOCREDITLEFT");
		break;
	case VC_ENGINE_ENDCAUSE_REJECTED:
		string_id = _("IDS_CALL_BODY_CALL_REJECTED");
		break;
	case VC_ENGINE_ENDCAUSE_USER_BUSY:
		string_id = _("IDS_CALL_POP_USER_BUSY");
		break;
	case VC_ENGINE_ENDCAUSE_WRONG_GROUP:
		string_id = _("IDS_CALL_POP_WRONG_GROUP");
		break;
	case VC_ENGINE_ENDCAUSE_INVALID_NUMBER_FORMAT:
		string_id = _("IDS_CALL_POP_CAUSE_WRONG_NUMBER");
		break;
	case VC_ENGINE_ENDCAUSE_CALL_NOT_ALLOWED:
		string_id = _("IDS_CALL_POP_CALLNOTCALLOWED");
		break;
	case VC_ENGINE_ENDCAUSE_TAPI_ERROR:
		string_id = _("IDS_CALL_POP_AEESYS_SYSTEMFAILUREERROR");
		break;
	case VC_ENGINE_ENDCAUSE_CALL_FAILED:
		string_id = _("IDS_CALL_POP_CALLFAILED");
		break;
	case VC_ENGINE_ENDCAUSE_NUMBER_CHANGED:
		string_id = _("IDS_CALL_BODY_NUMBER_CHANGED");
		break;
	case VC_ENGINE_ENDCAUSE_IMEI_REJECTED:
		string_id = _("IDS_CALL_POP_VERIFY_SIM_OR_INSERT_VALID_SIM");
		break;
	case VC_ENGINE_ENDCAUSE_NO_USER_RESPONDING:	/**< User not responding */
	case VC_ENGINE_ENDCAUSE_USER_ALERTING_NO_ANSWER:	/**< User Alerting No Answer */
	default:
		string_id = _("IDS_CALL_BODY_CALLENDED");
		break;
	}
	vcall_engine_util_strcpy(data, VC_DATA_LENGTH_MAX, string_id);
	return data;
}

void _vcui_cache_flush()
{
#ifdef _CACHE_FLUSH_
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	evas_font_cache_flush(ad->evas);
	evas_image_cache_flush(ad->evas);

	edje_file_cache_flush();
	edje_collection_cache_flush();

	evas_render_idle_flush(ad->evas);
#endif
}

int _vcui_is_idle_lock()
{
	int lock_state;
	int ret = vconf_get_int(VCONFKEY_IDLE_LOCK_STATE, &(lock_state));
	if (ret == -1) {
		CALL_UI_DEBUG("Cannot get vconf key");
	}

	if (lock_state == VCONFKEY_IDLE_LOCK)
		return CALL_LOCK;
	else
		return CALL_UNLOCK;
}

unsigned long _vcui_get_diff_now(time_t start_time)
{
	time_t curr_time;
	unsigned long call_duration_in_sec = 0;
	curr_time = time(&curr_time);
	call_duration_in_sec = curr_time - start_time;
	return call_duration_in_sec;
}

gboolean _vcui_is_gcf_mode(void)
{
	gboolean bgcf_status = EINA_FALSE;
	int ret = -1;

	ret = vconf_get_int(VCONFKEY_ADMIN_GCF_TEST, &bgcf_status);
	if (0 == ret) {
		CALL_UI_DEBUG("bgcf_status = [%d]\n", bgcf_status);
	} else {
		CALL_UI_DEBUG("vconf_get_int failed..[%d]\n", ret);
	}

	return bgcf_status;
}

gboolean _vcui_is_headset_conected(void)
{
	int bt_connected = VCONFKEY_BT_DEVICE_NONE;
	Eina_Bool ret = EINA_FALSE;

	ret = vconf_get_int(VCONFKEY_BT_DEVICE, &bt_connected);
	if (0 == ret) {
		CALL_UI_DEBUG("bt_connected = [0x%x] ", bt_connected);
	} else {
		CALL_UI_DEBUG("vconf_get_int failed..[%d]", ret);
	}

	return (VCONFKEY_BT_DEVICE_HEADSET_CONNECTED == (bt_connected & VCONFKEY_BT_DEVICE_HEADSET_CONNECTED)) ? EINA_TRUE : EINA_FALSE;
}

gboolean _vcui_is_headset_switch_on(void)
{
	int bt_status = VCONFKEY_BT_STATUS_OFF;
	Eina_Bool ret = EINA_FALSE;

	ret = vconf_get_int(VCONFKEY_BT_STATUS, &bt_status);
	if (0 == ret) {
		CALL_UI_DEBUG("bt_status = [0x%x] ", bt_status);
	} else {
		CALL_UI_DEBUG("vconf_get_int failed..[%d]", ret);
	}

	return (VCONFKEY_BT_STATUS_ON == (bt_status & VCONFKEY_BT_STATUS_ON)) ? EINA_TRUE : EINA_FALSE;
}

gboolean _vcui_is_answering_mode_on(void)
{
	gboolean bAnswerMode = EINA_FALSE;
	Eina_Bool ret = EINA_FALSE;

	ret = vconf_get_bool(VCONFKEY_CISSAPPL_ANSWERING_KEY_BOOL, &bAnswerMode);
	if (0 == ret) {
		CALL_UI_DEBUG("bAnswerMode = [%d] \n", bAnswerMode);
	} else {
		CALL_UI_DEBUG("vconf_get_int failed..[%d]\n", ret);
	}

	return bAnswerMode;
}

gboolean _vcui_is_powerkey_mode_on(void)
{
	gboolean bPowerkeyMode = EINA_FALSE;
	Eina_Bool ret = EINA_FALSE;

	ret = vconf_get_bool(VCONFKEY_CISSAPPL_POWER_KEY_ENDS_CALL_BOOL, &bPowerkeyMode);
	if (0 == ret) {
		CALL_UI_DEBUG("bPowerkeyMode = [%d] \n", bPowerkeyMode);
	} else {
		CALL_UI_DEBUG("vconf_get_int failed..[%d]\n", ret);
	}

	return bPowerkeyMode;
}

gboolean _vcui_is_phonelock_status()
{
	gboolean b_phonelock = EINA_FALSE;
	if (!vconf_get_bool(VCONFKEY_SETAPPL_STATE_POWER_ON_LOCK_BOOL, &b_phonelock)) {
		CALL_UI_DEBUG("b_phonelock:[%d]", b_phonelock);
		return b_phonelock;
	} else {
		CALL_UI_DEBUG("get VCONFKEY_SETAPPL_STATE_POWER_ON_LOCK_BOOL failed..");
		return EINA_FALSE;
	}
}

void _vcui_add_calllog(int type, call_data_t *data, int boutgoing_end)
{
	CALL_UI_DEBUG("type = [0x%x] ", type);
	CTSvalue *plog;
	time_t current_time;
	int missed_cnt = 0;

	if (data == NULL)
		return;
	contacts_svc_connect();

	current_time = time(NULL);

	plog = contacts_svc_value_new(CTS_VALUE_PHONELOG);
	contacts_svc_value_set_str(plog, CTS_PLOG_VAL_NUMBER_STR, _vcui_doc_get_call_number(data));
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT, (int)current_time);
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT, type);
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_RELATED_ID_INT, _vcui_doc_get_contact_index(data));

	if ((type == CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN) || (type == CTS_PLOG_TYPE_VOICE_REJECT) || (type == CTS_PLOG_TYPE_VOICE_BLOCKED) || (boutgoing_end == EINA_TRUE)) {
		contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, 0);
	} else {
		contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, _vcui_get_diff_now(_vcui_doc_get_call_start_time(data)));
	}
	contacts_svc_insert_phonelog(plog);

	contacts_svc_value_free(plog);
	missed_cnt = contacts_svc_count(CTS_GET_UNSEEN_MISSED_CALL);
	contacts_svc_disconnect();

	/* vconf set & quickpanel noti (for missed call) */
	if (type == CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN) {
		char szname[255] = { 0, };
		char str_buf[1024] = { 0, };
		int noti_flags = 0;
		notification_h noti = NULL;
		notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

		if (strlen(_vcui_doc_get_call_display_name(data)) == 0) {
			snprintf(szname, sizeof(szname), "%s", _vcui_doc_get_call_number(data));
		} else {
			snprintf(szname, sizeof(szname), "%s", _vcui_doc_get_call_display_name(data));
		}
		CALL_UI_DEBUG("szname:[%s]", szname);

		if (missed_cnt == 1) {
			strncpy(str_buf, _("IDS_CALL_POP_CALLMISSED"), sizeof(str_buf) - 1);
		} else {
			char *temp = _("IDS_CALL_HEADER_PD_MISSED_CALLS");
			snprintf(str_buf, sizeof(str_buf), temp, missed_cnt);
		}
		CALL_UI_DEBUG("Notification string :[%s]", str_buf);

		noti = notification_new(NOTIFICATION_TYPE_NOTI, __VCUI_NOTIFICATION_CALL_GROUP_ID, NOTIFICATION_PRIV_ID_NONE);
		if (noti == NULL) {
			CALL_UI_DEBUG("Fail to notification_new");
			return;
		}

		noti_err = notification_set_pkgname(noti, CALL_PKG_NAME);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to noification_set_pkgname %d", noti_err);
		}

		noti_err = notification_set_application(noti, DIALER_PKG);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_application : %d", noti_err);
		}

		bundle *args = bundle_create();

		bundle_add(args, "logs", "missed_call");
		noti_err = notification_set_args(noti, args, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_args : %d", noti_err);
		}
		bundle_free(args);

		noti_err = notification_set_time(noti, current_time);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_icon : %d", noti_err);
		}

		/* Set notification single title*/
		noti_err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, szname, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_text(single-title) : %d", noti_err);
		}

		/* Set noitification single content*/
		noti_err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, str_buf, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_text(single-content) : %d", noti_err);
		}

		/* Set notification group title*/
		noti_err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_GROUP_TITLE, str_buf, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_text(group-title) : %d", noti_err);
		}

		noti_flags = NOTIFICATION_PROP_DISABLE_TICKERNOTI | NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE;
		noti_err = notification_set_property(noti, noti_flags);
		if(noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_property : %d", noti_err);
		}

		char *file_path = _vcui_doc_get_caller_id_file_path(data);
		CALL_UI_DEBUG("data->call_file_path(%s)", file_path);
		if ((strcmp(file_path, NOIMG_ICON) == 0)
			|| (missed_cnt > 1)) {
			noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_THUMBNAIL, NOTIFY_MISSED_CALL_ICON);
		} else {
			noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_THUMBNAIL, file_path);
		}
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_icon : %d", noti_err);
		}
		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NOTIFY_SUB_MISSED_CALL_ICON);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_icon : %d", noti_err);
		}

		noti_err = notification_insert(noti, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_text_domain");
		}

		noti_err = notification_free(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_text_domain");
		}
	}
}

void _vcui_raise_main_win()
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = _vcui_get_app_data();
	if (ad == NULL) {
		CALL_UI_DEBUG("App data is NULL");
		return;
	}
	if (ad->win_main == NULL) {
		CALL_UI_DEBUG("Main Window is NULL");
		return;
	}

	elm_win_activate(ad->win_main);
	_vcui_show_main_ui_set_flag();
	/*view_refresh_now(); */

}

int _vcui_check_valid_eo(Evas_Object *eo, char *v_name)
{
	/*CALL_UI_DEBUG("eo addr:[%p], v_name:[%s]", eo, v_name); */
	const char *obj_name = evas_object_name_get(eo);
	if (obj_name == NULL) {
		CALL_UI_DEBUG("obj_name is NULL!!. eo addr:[%p], v_name:[%s]", eo, v_name);
		return VC_ERROR;
	}
	if (strncmp(obj_name, v_name, strlen(obj_name)) == 0) {
		return VC_NO_ERROR;
	} else {
		CALL_UI_DEBUG("different name !! ");
		return VC_ERROR;
	}
}

vcui_app_call_data_t *_vcui_get_app_data()
{
	return &global_ad;
}

void _vcui_show_main_ui_set_flag()
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	if (ad->show_flag == WIN_HIDE) {
		CALL_UI_DEBUG("show_flag : WIN_SHOW");
		evas_object_show(ad->win_main);
		ad->show_flag = WIN_SHOW;
	}
}

void __vcui_hide_main_ui_set_flag()
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	CALL_UI_DEBUG("show_flag: WIN_HIDE");
	evas_object_hide(ad->win_main);
	ad->show_flag = WIN_HIDE;
}

Evas_Object *_vcui_load_edj(Evas_Object *parent, const char *file, const char *group)
{
	Evas_Object *eo;
	int r;

	eo = elm_layout_add(parent);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			evas_object_del(eo);
			CALL_UI_DEBUG("ERROR!!");
			return NULL;
		}

		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(parent, eo);
	}

	return eo;
}

int main(int argc, char *argv[])
{
	CALL_UI_DEBUG("voice call ui main()..");

	struct appcore_ops ops = {
		.create = __vcui_app_create,
		.terminate = __vcui_app_terminate,
		.pause = __vcui_app_pause,
		.resume = __vcui_app_resume,
		.reset = __vcui_app_reset,
	};

	memset(&global_ad, 0, sizeof(vcui_app_call_data_t));

	ops.data = &global_ad;

	CALL_UI_KPI("__vcui_init start");
	__vcui_init(&global_ad);
	CALL_UI_KPI("__vcui_init done");

	appcore_set_i18n(PACKAGE, LOCALEDIR);

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}

static Eina_Bool __vcui_avoid_multi_setup_timer_cb(void *data)
{
	CALL_UI_DEBUG("..");

	g_avoid_multi_setup = EINA_FALSE;

	return ECORE_CALLBACK_CANCEL;
}

static void __vcui_app_callback(int event, void *pdata, void *puser_data)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)puser_data;
	vc_engine_msg_type *msg = (vc_engine_msg_type *)pdata;

	if ((ad == NULL) || (msg == NULL)) {
		CALL_UI_DEBUG("ERROR.NULL pointer");
		return;
	}
	CALL_UI_DEBUG("@@@ event:[%s], view_top:[%d], count:[%d]  @@@ \n", gszcall_callback_msg[event], ad->view_top, _vcui_doc_get_all_call_data_count());

	switch (event) {
	case VC_ENGINE_MSG_INCOM_TO_UI:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_LOCK);

			CALL_UI_DEBUG("num:[%s], name:[%s]", msg->incoming.call_num, msg->incoming.call_name);

			call_data_t *call_data = NULL;
			call_data = _vcui_doc_allocate_call_data_memory();
			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}
			_vcui_doc_set_call_handle(call_data, msg->incoming.call_handle);
			_vcui_doc_set_contact_index(call_data, msg->incoming.contact_index);
			_vcui_doc_set_contact_phone_type(call_data, msg->incoming.phone_type);
			_vcui_doc_set_birthday_remaining_days(call_data, msg->incoming.bday_remaining_days);
			_vcui_doc_set_call_number(call_data, msg->incoming.call_num);

			if (msg->incoming.brestricted == EINA_TRUE) {
				if (msg->incoming.bpayphone == EINA_TRUE) {
					_vcui_doc_set_call_display_name(call_data, _("Payphone"));
				} else {
					_vcui_doc_set_call_display_name(call_data, dgettext("sys_string", "IDS_COM_BODY_UNKNOWN"));
				}
			} else {
				_vcui_doc_set_call_display_name(call_data, msg->incoming.call_name);
			}

			if ((strcmp((char *)msg->incoming.call_file_path, "default") == 0) || (strlen((char *)msg->incoming.call_file_path) == 0)) {
				_vcui_doc_set_caller_id_file_path(call_data, NOIMG_ICON);
			} else {
				_vcui_doc_set_caller_id_file_path(call_data, msg->incoming.call_file_path);
			}

			if ((strcmp((char *)msg->incoming.call_full_file_path, "default") == 0) || (strlen((char *)msg->incoming.call_full_file_path) == 0)) {
				_vcui_doc_set_caller_id_full_file_path(call_data, NOIMG_ICON);
			} else {
				_vcui_doc_set_caller_id_full_file_path(call_data, msg->incoming.call_full_file_path);
			}
			_vcui_doc_set_call_status(call_data, NO_STATUS);
			_vcui_doc_set_call_type(call_data, CALL_INCOMING);
			_vcui_doc_set_auto_reject_status(call_data, msg->incoming.brejected);

			_vcui_doc_set_recent_mt_call_data(call_data);
			_vcui_doc_add_call_data(call_data);

			{
				/*Auto Reject Check */
				if (EINA_TRUE == msg->incoming.brejected) {
					CALL_UI_DEBUG("the call rejected and show reject popup");
					vcall_engine_reject_call();
					_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_BLOCKED, call_data, EINA_FALSE);

					if (_vcui_doc_get_all_call_data_count() > 1) {
						_vcui_view_popup_load_reject_call(_vcui_doc_get_call_display_name(call_data), _vcui_doc_get_call_number(call_data), EINA_FALSE);
					} else {
						_vcui_view_popup_load_reject_call(_vcui_doc_get_call_display_name(call_data), _vcui_doc_get_call_number(call_data), EINA_TRUE);
					}
				} else {
					_vcui_view_change(VIEW_INCOMING_LOCK_VIEW, 0, NULL, NULL);
				}
			}
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_TO_UI:
		{
			call_data_t *call_data = NULL;
			call_data = _vcui_doc_allocate_call_data_memory();
			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}

			_vcui_doc_set_call_handle(call_data, NO_HANDLE);
			_vcui_doc_set_contact_index(call_data, msg->outgoing.contact_index);
			_vcui_doc_set_contact_phone_type(call_data, msg->outgoing.phone_type);
			_vcui_doc_set_birthday_remaining_days(call_data, msg->outgoing.bday_remaining_days);
			_vcui_doc_set_call_number(call_data, msg->outgoing.call_num);
			_vcui_doc_set_call_display_name(call_data, msg->outgoing.call_name);

			if ((strcmp((char *)msg->outgoing.call_file_path, "default") == 0) || (strlen((char *)msg->outgoing.call_file_path) == 0)) {
				_vcui_doc_set_caller_id_file_path(call_data, NOIMG_ICON);
			} else {
				_vcui_doc_set_caller_id_file_path(call_data, msg->outgoing.call_file_path);
			}

			if ((strcmp((char *)msg->outgoing.call_full_file_path, "default") == 0) || (strlen((char *)msg->outgoing.call_full_file_path) == 0)) {
				_vcui_doc_set_caller_id_full_file_path(call_data, NOIMG_ICON);
			} else {
				_vcui_doc_set_caller_id_full_file_path(call_data, msg->outgoing.call_full_file_path);
			}
			_vcui_doc_set_call_status(call_data, NO_STATUS);
			_vcui_doc_set_call_type(call_data, CALL_OUTGOING);
			_vcui_doc_set_call_start_time(call_data);

			_vcui_doc_set_recent_mo_call_data(call_data);
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_LOCK);

			vc_engine_outgoing_orig_type outgoing_orig = msg->outgoing_orig;

			call_data_t *call_data = _vcui_doc_get_recent_mo_call_data();
			_vcui_doc_set_call_handle(call_data, outgoing_orig.call_handle);

			if (outgoing_orig.bemergency == EINA_TRUE) {
				CALL_UI_DEBUG("it is emergency call");
				char *em_name = _("IDS_CALL_POP_EMERGENCY_CALL");
				_vcui_doc_set_call_display_name(call_data, em_name);
				_vcui_doc_set_caller_id_file_path(call_data, NOIMG_ICON);
				_vcui_doc_set_caller_id_full_file_path(call_data, NOIMG_ICON);
				_vcui_doc_set_contact_phone_type(call_data, -1);
				_vcui_doc_set_birthday_remaining_days(call_data, -1);
			}

			_vcui_doc_add_call_data(call_data);
			_vcui_view_change(VIEW_DIALLING_VIEW, 0, NULL, NULL);

			/*Update the quick-panel window */
			_vcui_view_quickpanel_change();
			_vcui_view_qp_update_text_status(ad->view_st[VIEW_QUICKPANEL_VIEW], _("IDS_CALL_POP_CALLING"));
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI_TEST:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_LOCK);

			vc_engine_outgoing_orig_type outgoing_orig = msg->outgoing_orig;

			call_data_t *call_data = _vcui_doc_get_recent_mo_call_data();
			_vcui_doc_set_call_handle(call_data, 1);

			if (outgoing_orig.bemergency == EINA_TRUE) {
				CALL_UI_DEBUG("it is emergency call");
				char *em_name = _("IDS_CALL_POP_EMERGENCY_CALL");
				_vcui_doc_set_call_display_name(call_data, em_name);
				_vcui_doc_set_caller_id_file_path(call_data, NOIMG_ICON);
				_vcui_doc_set_caller_id_full_file_path(call_data, NOIMG_ICON);
				_vcui_doc_set_contact_phone_type(call_data, -1);
				_vcui_doc_set_birthday_remaining_days(call_data, -1);
			}

			_vcui_doc_add_call_data(call_data);
			_vcui_view_change(VIEW_DIALLING_VIEW, 0, NULL, NULL);

			/*Update the quick-panel window */
			_vcui_view_quickpanel_change();
			_vcui_view_qp_update_text_status(ad->view_st[VIEW_QUICKPANEL_VIEW], _("IDS_CALL_POP_CALLING"));
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_ALERT_TO_UI:
		{
			call_data_t *call_data = _vcui_doc_get_recent_mo_call_data();
			if (_vcui_doc_is_valid_call_data(call_data) == EINA_FALSE) {
				CALL_UI_DEBUG("Error. check outgoing_orig msg.");
				elm_exit();
				return;
			}

			_vcui_view_qp_update_text_status(ad->view_st[VIEW_QUICKPANEL_VIEW], _("IDS_CALL_POP_CALLING"));
		}
		break;

	case VC_ENGINE_MSG_CONNECTED_TO_UI:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_connected_type connected = msg->connected;
			call_data_t *call_data = _vcui_doc_get_call_data_by_handle(connected.call_handle);
			if (_vcui_doc_is_valid_call_data(call_data) == EINA_FALSE) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}
			_vcui_doc_set_call_status(call_data, CALL_UNHOLD);
			_vcui_doc_set_call_handle(call_data, connected.call_handle);
			_vcui_doc_set_call_start_time(call_data);
			/* When new call connected, if it's multiparty call, always show split1 first. */
			ad->bswapped = EINA_FALSE;

			_vcui_view_auto_change();

			/*Update the quick-panel window */
			_vcui_view_quickpanel_change();
		}
		break;

	case VC_ENGINE_MSG_NORMAL_END_TO_UI:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_LOCK);

			vc_engine_normal_end_type normal_end = msg->normal_end;

			CALL_UI_DEBUG("end_cause_type:[%d]", normal_end.end_cause_type);

			vcui_app_call_data_t *ad = _vcui_get_app_data();
			time_t start_time;
			int	call_type = -1;

			call_data_t *call_data = _vcui_doc_get_call_data_by_handle(normal_end.call_handle);
			if (_vcui_doc_is_valid_call_data(call_data) == EINA_FALSE) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}
			call_type = _vcui_doc_get_call_type(call_data);
			CALL_UI_DEBUG("call_type: %d", call_type);
			if (call_type == CALL_OUTGOING) {
				_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_OUTGOING, call_data, EINA_FALSE);
			} else if (call_type == CALL_INCOMING) {
				_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_INCOMMING, call_data, EINA_FALSE);
			}
			start_time = _vcui_doc_get_call_start_time(call_data);


			if (_vcui_doc_get_all_call_data_count() == 1 && ad->view_top == VIEW_INCALL_ONECALL_VIEW) {
				ad->call_end_type = CALL_END_TYPE_SINGLE_CALL;
			}
			CALL_UI_DEBUG("ad->call_end_type[%d]", ad->call_end_type);

			if (ad->call_end_type == CALL_END_TYPE_SINGLE_CALL) {
				CALL_UI_DEBUG("Show end screen - %d", ad->call_end_type);
				_vcui_view_change(VIEW_ENDCALL_VIEW, -1, call_data, NULL);
				ad->call_end_type = CALL_END_TYPE_NONE;
				CALL_UI_DEBUG("Blink show: end call time");
				_vcui_view_common_call_end_show(start_time, normal_end.end_cause_type);
				_vcui_doc_remove_call_data(call_data);
			} else if (ad->call_end_type == CALL_END_TYPE_CONF_CALL) {
				if (_vcui_doc_get_all_call_data_count() == 1) {
					ad->call_end_type = CALL_END_TYPE_NONE;
				} else if (ad->view_top != VIEW_ENDCALL_VIEW) {
					_vcui_view_change(VIEW_ENDCALL_VIEW, -1, call_data, NULL);
					_vcui_view_common_call_end_show(start_time, normal_end.end_cause_type);
				}
				_vcui_doc_remove_call_data(call_data);
			} else {
				_vcui_doc_remove_call_data(call_data);
				_vcui_view_common_call_terminate_or_view_change();
			}
			/*Update the quick-panel window */
			_vcui_view_quickpanel_change();
		}
		break;

	case VC_ENGINE_MSG_INCOM_END_TO_UI:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_common_with_handle_type incom_end = msg->incom_end;

			call_data_t *call_data = _vcui_doc_get_call_data_by_handle(incom_end.call_handle);
			if (_vcui_doc_is_valid_call_data(call_data) == EINA_FALSE) {
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
			vcall_engine_device_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_common_with_handle_type rejected_end = msg->rejected_end;

			call_data_t *pcall_data = _vcui_doc_get_call_data_by_handle(rejected_end.call_handle);
			if (_vcui_doc_is_valid_call_data(pcall_data) == EINA_FALSE) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			}

			if (_vcui_doc_get_auto_reject_status(pcall_data) == EINA_TRUE) {
				CALL_UI_DEBUG("auto rejected.");
				_vcui_doc_remove_call_data(pcall_data);
			} else {
				_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_REJECT, pcall_data, EINA_FALSE);
				_vcui_doc_remove_call_data(pcall_data);
				_vcui_view_common_call_terminate_or_view_change();
			}
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_END_TO_UI:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_outgoing_end_type outgoing_end = msg->outgoing_end;
			call_data_t *call_data = _vcui_doc_get_call_data_by_handle(outgoing_end.call_handle);
			if (_vcui_doc_is_valid_call_data(call_data) == EINA_FALSE) {
				CALL_UI_DEBUG("It is the case which call orig is not received.");
				char data[VC_DATA_LENGTH_MAX] = { 0, };
				_vcui_view_popup_load(_vcui_get_endcause_string(outgoing_end.end_cause_type, data), POPUP_TIMEOUT_LONG, EINA_TRUE);
			} else {
				_vcui_add_calllog(CTS_PLOG_TYPE_VOICE_OUTGOING, call_data, EINA_TRUE);
				if (outgoing_end.bauto_redial == EINA_TRUE) {
					CALL_UI_DEBUG("bauto_redial is EINA_TRUE");
					_vcui_doc_remove_call_data_from_list(call_data);
				} else {
					CALL_UI_DEBUG("show the call end screen");
					vcui_app_call_data_t *ad = _vcui_get_app_data();
					if (_vcui_doc_get_all_call_data_count() == 1 && ad->view_top == VIEW_DIALLING_VIEW) {
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
			call_data_t *call_data = _vcui_doc_get_call_data_by_handle(outgoing_end_signal_play.call_handle);
			if (call_data != NULL) {
				_vcui_view_dialing_draw_txt_ended(ad->view_st[ad->view_top], outgoing_end_signal_play.end_cause_type);
			} else {
				CALL_UI_DEBUG("Check it whether call data exists. handle:[%d]", outgoing_end_signal_play.call_handle);
			}
		}
		break;

	case VC_ENGINE_MSG_OUTGOING_ABORTED_TO_UI:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_common_with_handle_type outgoing_aborted = msg->outgoing_aborted;
			call_data_t *call_data = _vcui_doc_get_call_data_by_handle(outgoing_aborted.call_handle);
			if (call_data == NULL)
				call_data = _vcui_doc_get_recent_mo_call_data();

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

			vc_engine_dtmf_ack_type dtmf_ack = msg->dtmf_progress;

			if (EINA_FALSE == dtmf_ack.bstatus) {
				_vcui_view_popup_unload_progress(ad);
				if ((dtmf_ack.string_id != -1) && (dtmf_ack.string_id != IDS_CALL_POP_DTMF_SENT)) {
					CALL_UI_DEBUG("load popup window... Start");
					_vcui_view_popup_load(_(gszcall_error_msg[dtmf_ack.string_id]), POPUP_TIMEOUT_LONG, EINA_FALSE);
				}
			} else {
				CALL_UI_DEBUG("display_string:[%s], string_id:[%d]", dtmf_ack.display_string, dtmf_ack.string_id);
				if (dtmf_ack.string_id == IDS_CALL_POP_UNAVAILABLE) {	/*check for the ID when string is added */
					_vcui_view_load_send_dtmf_popup_with_buttons(_("Send DTMF tones?"), dtmf_ack.display_string);	/*add the string */
				} else if (dtmf_ack.string_id == IDS_CALL_POP_SENDING) {
					_vcui_view_popup_load_sending_dtmf(_(gszcall_error_msg[dtmf_ack.string_id]), dtmf_ack.display_string);
				}
			}
		}
		break;

	case VC_ENGINE_MSG_SS_HELD_TO_UI:
		{
			_vcui_doc_set_all_call_data_to_hold_status();
			_vcui_view_update();
			/*Update the quick-panel window */
			_vcui_view_quickpanel_change();

		}
		break;

	case VC_ENGINE_MSG_SS_RETREIVED_TO_UI:
		{
			_vcui_doc_set_all_call_data_to_unhold_status();
			_vcui_view_update();
			/*Update the quick-panel window */
			_vcui_view_quickpanel_change();

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

			_vcui_doc_swap_all_call_data_status();
			_vcui_view_auto_change();
			/*Update the quick-panel window */
			_vcui_view_quickpanel_change();
		}
		break;

	case VC_ENGINE_MSG_SS_SETUP_CONF_TO_UI:
		{
			_vcui_view_popup_unload(ad->popup_eo);

			_vcui_doc_set_all_call_data_to_unhold_status();
			_vcui_view_auto_change();
			/*Update the quick-panel window */
			_vcui_view_quickpanel_change();
		}
		break;

	case VC_ENGINE_MSG_SS_SPLIT_CONF_TO_UI:
		{
			vc_engine_common_with_handle_type ss_split_conf = msg->ss_split_conf;

			CALL_UI_DEBUG("[UI]The handle is %d ", ss_split_conf.call_handle);

			call_data_t *call_data = _vcui_doc_get_call_data_by_handle(ss_split_conf.call_handle);
			if (call_data == NULL) {
				CALL_UI_DEBUG("Error");
				elm_exit();
				return;
			} else {
				_vcui_doc_set_all_call_data_to_hold_status();
				_vcui_doc_set_call_status(call_data, CALL_UNHOLD);

				_vcui_view_auto_change();
				/*Update the quick-panel window */
				_vcui_view_quickpanel_change();
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

			if (earjack_status.bstatus == EINA_TRUE) {
				if (ad->ctxpopup_radio_group_eo != NULL)
					elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_RECEIVER_EARJACK);

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
					if (ad->ctxpopup_radio_group_eo != NULL)
						elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_HEADSET);

					ad->speaker_status = EINA_FALSE;
					ad->headset_status = EINA_TRUE;

					if ((ad->view_top == VIEW_INCALL_ONECALL_VIEW) || (ad->view_top == VIEW_INCALL_MULTICALL_SPLIT_VIEW)
					    || (ad->view_top == VIEW_INCALL_MULTICALL_CONF_VIEW) || (ad->view_top == VIEW_DIALLING_VIEW)) {
						if (ad->view_st[ad->view_top] != NULL) {
							ad->view_st[ad->view_top]->onUpdate(ad->view_st[ad->view_top], NULL, NULL);
						}
					}
				} else {
					if (ad->ctxpopup_radio_group_eo != NULL)
						elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_RECEIVER_EARJACK);

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
			call_data_t *call_data = _vcui_doc_get_recent_mo_call_data();
			if (call_data != NULL && _vcui_doc_get_call_handle(call_data) == NO_HANDLE) {
				_vcui_doc_remove_call_data_from_list(call_data);
				_vcui_doc_set_recent_mo_call_data(NULL);
			}
		}
		break;

	case VC_ENGINE_MSG_MESSAGE_BOX_TO_UI:
		{
			vcall_engine_device_control_lcd_state(VC_LCD_ON_UNLOCK);

			vc_engine_msg_box_type msg_box = msg->msg_box;
			_vcui_view_popup_load(_(gszcall_error_msg[msg_box.string_id]), POPUP_TIMEOUT_LONG, EINA_TRUE);
		}
		break;

	case VC_ENGINE_MSG_REDIAL_TO_UI:
		{
			CALL_UI_DEBUG("not used");
		}
		break;

	case VC_ENGINE_MSG_NOTI_WBAMR_TO_UI:
		{
			vc_engine_wbamr_status_type wbamr_status = msg->wbamr_status;
			ad->wbamr_status = wbamr_status.bstatus;
		}
		break;
	default:
		break;
	}

	_vcui_doc_print_all_call_data(gszcall_callback_msg[event]);

	CALL_UI_DEBUG("End..");
}
