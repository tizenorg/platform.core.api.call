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
#include "vcui-view-incoming.h"
#include "vcui-view-incoming-lock.h"
#include "vcui-view-keypad.h"
#include "vcui-view-single-call.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-callend.h"
 #include "vcui-view-popup.h"
#include <glib.h>
#include <glib-2.0/glib.h>
#include <dbus/dbus-glib.h>
#include <notification.h>
#ifdef SIGNAL_HANDLER
#include <signal.h>
#endif

static vcui_app_call_data_t global_ad;

#ifdef SIGNAL_HANDLER
#define VCUI_SIG_NUM 12
static struct sigaction vcui_app_sigact;
static struct sigaction vcui_app_sigoldact[VCUI_SIG_NUM];
static int vcui_app_sig_to_handle[] = {SIGABRT,SIGBUS,SIGFPE,SIGILL,SIGQUIT,SIGSEGV,SIGSYS,SIGTRAP,SIGXCPU,SIGXFSZ,SIGTERM,SIGPIPE};
#endif

static gboolean g_avoid_multi_setup = EINA_FALSE;
static Eina_Bool __vcui_avoid_multi_setup_timer_cb(void *data);

#define CISS_AUL_CMD "org.tizen.ciss"
#define CISS_MODE_OPT "REQ"
#define __VCUI_NOTIFICATION_CALL_GROUP_ID 1001

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
static void __vcui_init(vcui_app_call_data_t *ad);

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
	/*syslog (LOG_INFO, "[VOICE-CALL] __vcui_app_exit_handler\n");*/
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
	int i=0;

	CALL_UI_DEBUG("SISEGV Received, Signal Number: :%d \n", signal_no);

	vcall_engine_force_reset();
	__vcui_fade_out_cb_routine();

	for (i=0; i < VCUI_SIG_NUM; i++)
	{
		sigaction(vcui_app_sig_to_handle[i], &(vcui_app_sigoldact[i]), NULL);
	}

	raise(signal_no); /*raise signal intentionally (pass the same signal)*/
}

/**
* This function registers a user space signal handler for the signal SIGSEGV (Signal #11)
*
* @return		nothing
*/
static void __vcui_register_sigsegv_handler()
{
	CALL_UI_DEBUG("..");

	int i =0;
	vcui_app_sigact.sa_flags = SA_NOCLDSTOP;
	vcui_app_sigact.sa_handler =  (void *)__vcui_sigsegv_handler;
	sigemptyset(&vcui_app_sigact.sa_mask);


	for (;i < VCUI_SIG_NUM; i++)
	{
		sigaction (vcui_app_sig_to_handle[i], &vcui_app_sigact, &(vcui_app_sigoldact[i]));
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

#ifdef SIGNAL_HANDLER
		__vcui_register_sigsegv_handler();
		atexit(__vcui_exit_handler);
#endif

	CALL_UI_KPI("__vcui_app_create done");
	return VC_NO_ERROR;
}

static int __vcui_app_pause(void *data)
{
	CALL_UI_DEBUG("__vcui_app_pause()..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;
	return VC_NO_ERROR;
}

static int __vcui_app_reset(bundle *kb, void *data)
{
	CALL_UI_DEBUG("__vcui_app_reset()..");
	CALL_UI_KPI("__vcui_app_reset start");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;

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
		if (strncmp(mime_type, "phonenum.uri",12) == 0 || strncmp(mime_type, "tel.uri",7) == 0) {
			tmp = bundle_get_val(kb, AUL_K_MIME_CONTENT);
			if (tmp == NULL) {
				CALL_UI_DEBUG("bundle val is NULL");
				elm_exit();
				return VC_NO_ERROR;
			}

			if (strncmp(tmp, "tel:", 4) == 0) {
				telnum = (char *)tmp + 4;
			}

			CALL_UI_DEBUG("number: [%s]", telnum);

			vcui_call_type_t call_type;
			vcui_call_mo_data_t call_data;

			memset(&call_data, 0, sizeof(call_data));
			call_type = VCUI_CALL_TYPE_MO;

			snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", telnum);

			_vcui_engine_interface_process_mo_call(call_type, &call_data);

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

			vcui_call_type_t call_type;
			tmp = (char *)appsvc_get_data(kb, "calltype");
			
			if (tmp) {
				CALL_UI_DEBUG("calltype: [%s]", tmp);
				if (!strncmp(tmp, "EMERGENCY", 9)) {
					vcui_call_ecc_data_t call_data;

					memset(&call_data, 0, sizeof(call_data));
					call_type = VCUI_CALL_TYPE_ECC;

					snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", telnum);

					_vcui_engine_interface_process_ecc_call(call_type, &call_data);
				} else {
					CALL_UI_DEBUG("wrong calltype!");
					elm_exit();
					return VC_ERROR;
				}
			} else {
				vcui_call_mo_data_t call_data;

				memset(&call_data, 0, sizeof(call_data));

				snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", telnum);

				call_type = VCUI_CALL_TYPE_MO;

				tmp = (char *)appsvc_get_data(kb, "ctindex");
				if (tmp) {
					CALL_UI_DEBUG("ctindex: [%s]", tmp);
					call_data.ct_index = atoi(tmp);
				} else {
					CALL_UI_DEBUG("bundle val is NULL");
					call_data.ct_index = -1;
				}

				if ((_vc_core_util_check_incall_ss_string(call_data.call_number) == EINA_TRUE) && (_vcui_doc_get_count() >= 1)) {
					vcall_engine_process_incall_ss(call_data.call_number);
					_vcui_view_auto_change();
				} else if (_vc_core_util_check_ss_string(call_data.call_number) == EINA_TRUE) {
					__vcui_launch_ciss(call_data.call_number);
					if (_vcui_doc_get_count() == 0)
						elm_exit();
					return VC_NO_ERROR;
				} else {
					_vcui_engine_interface_process_mo_call(call_type, &call_data);
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
			
			vcui_call_type_t call_type;
			vcui_call_mo_data_t call_data;

			memset(&call_data, 0, sizeof(call_data));
			call_type = VCUI_CALL_TYPE_MO;

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

			if ((_vc_core_util_check_incall_ss_string(call_data.call_number) == EINA_TRUE) && (_vcui_doc_get_count() >= 1)) {
				vcall_engine_process_incall_ss(call_data.call_number);
				_vcui_view_auto_change();
			} else if (_vc_core_util_check_ss_string(call_data.call_number) == EINA_TRUE) {
				__vcui_launch_ciss(call_data.call_number);
				if(_vcui_doc_get_count() == 0)
					elm_exit();
				return VC_NO_ERROR;
			} else {
				_vcui_engine_interface_process_mo_call(call_type, &call_data);
			}
		} else if (!strncmp(launch_type, "MT", 2)) {

			vcui_call_type_t call_type;
			vcui_call_mt_data_t call_data;

			memset(&call_data, 0, sizeof(call_data));
			call_type = VCUI_CALL_TYPE_MT;

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
				/*return VC_NO_ERROR;*/	/*check clir mt call.*/
			}

			tmp = bundle_get_val(kb, "name_mode");
			if (tmp) {
				CALL_UI_DEBUG("name_mode: [%s]", tmp);
				call_data.calling_name_mode= atoi(tmp);
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
			_vcui_engine_interface_process_mt_call(call_type, &call_data);
			} else if (!strncmp(launch_type, "EMERGENCY", 9)) {

			vcui_call_type_t call_type;
			vcui_call_ecc_data_t call_data;

			memset(&call_data, 0, sizeof(call_data));
			call_type = VCUI_CALL_TYPE_ECC;

			tmp = bundle_get_val(kb, "number");
			if (tmp) {
				CALL_UI_DEBUG("number: [%s]", tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
			}
			snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", tmp);

			_vcui_engine_interface_process_ecc_call(call_type, &call_data);

		} else if (!strncmp(launch_type, "SATSETUPCALL", 12)) {
			vcui_call_type_t call_type;
			vcui_call_sat_data_t sat_setup_call_info;

			memset(&sat_setup_call_info, 0, sizeof(sat_setup_call_info));
			call_type = VCUI_CALL_TYPE_SAT;

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
				return VC_NO_ERROR;*/
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
			_vcui_engine_interface_process_sat_call(call_type, &sat_setup_call_info);
			} else if (!strncmp(launch_type, "ECCTEST", 7)) {

			vcui_call_type_t call_type;
			vcui_call_ecc_data_t call_data;

			memset(&call_data, 0, sizeof(call_data));
			call_type = VCUI_CALL_TYPE_ECC_TEST;

			_vcui_engine_interface_process_ecc_call(call_type, &call_data);

		} else if (!strncmp(launch_type, "DOWNLOADCALL", 12)) {
			vcui_call_type_t call_type;
			vcui_call_mo_data_t call_data;

			memset(&call_data, 0, sizeof(call_data));
			call_type = VCUI_CALL_TYPE_DOWNLOAD_CALL;

			tmp = bundle_get_val(kb, "number");
			if (tmp) {
				CALL_UI_DEBUG("number: [%s]", tmp);
			} else {
				CALL_UI_DEBUG("bundle val is NULL");
			}

			if(tmp != NULL) {
				snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", tmp);
			} else {
				snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", "116");
			}

			vcall_engine_util_strcpy(call_data.call_number, sizeof(call_data.call_number), tmp);

			_vcui_engine_interface_process_mo_call(call_type, &call_data);
		} else {	/*if ear jack is needed, add it*/
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

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;
	return VC_NO_ERROR;
}

static int __vcui_app_terminate(void *data)
{
	CALL_UI_DEBUG("__vcui_app_terminate()..");

	if (_vcui_doc_get_count() >= 1) {
		CALL_UI_DEBUG("WARNING!! call exists. abnormal terminate!!");
		_vcui_engine_end_all_call();
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

	if (_vcui_doc_get_count() == 0) {
		CALL_UI_DEBUG("ignore it");
		return;
	}

	if ((ad->view_st[ad->view_top]->type == VIEW_INCOMING_VIEW || ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW)) {
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
			int vol_level = _vcui_engine_get_volume_level(VCUI_VOL_RING);
			if ((vol_level < RINGTONE_MIN) || (vol_level > RINGTONE_MAX)) {
				CALL_UI_DEBUG("ERROR : ringtone vol:[%d]", vol_level);
				return;
			}
			ad->ringtone_val = vol_level;
		}

		CALL_UI_DEBUG("RINGTONE : Set Volume");
		if (key_status == VAL_VOL_DOWN && ad->ringtone_val > RINGTONE_MIN) {
			ad->ringtone_val--;
			_vcui_engine_set_volume_level(VCUI_VOL_RING, ad->ringtone_val);
		} else if (key_status == VAL_VOL_UP && ad->ringtone_val < RINGTONE_MAX) {
			ad->ringtone_val++;
			_vcui_engine_set_volume_level(VCUI_VOL_RING, ad->ringtone_val);
		}
		_vcui_view_popup_vol_ringtone(ad->ringtone_val);
	} else if (ad->headset_status == EINA_TRUE) {
		if (ad->bt_vol_val < BT_VOL_MIN) {
			CALL_UI_DEBUG("BT VOL : Get Volume");
			ad->vol_key_status = key_status;
			_vcui_engine_get_volume_level(VCUI_VOL_HEADSET);
			return;
		}

		CALL_UI_DEBUG("BT VOL : Set Volume");
		if (key_status == VAL_VOL_DOWN && ad->bt_vol_val > BT_VOL_MIN) {	/*Key Down*/
			ad->bt_vol_val--;
			_vcui_engine_set_volume_level(VCUI_VOL_HEADSET, ad->bt_vol_val);
		} else if (key_status == VAL_VOL_UP && ad->bt_vol_val < BT_VOL_MAX) {	/*Key Up*/
			ad->bt_vol_val++;
			_vcui_engine_set_volume_level(VCUI_VOL_HEADSET, ad->bt_vol_val);
		}
		_vcui_view_popup_vol_bt(ad->bt_vol_val);
	} else {
		CALL_UI_DEBUG("TAPI VOL : Get Volume");
		ad->vol_key_status = key_status;
		ad->voice_vol_val = _vcui_engine_get_volume_level(VCUI_VOL_VOICE);

		CALL_UI_DEBUG("TAPI VOL : Set Volume");
		if (key_status == VAL_VOL_DOWN && ad->voice_vol_val > VOICE_VOL_MIN) {	/*Key Down*/
			ad->voice_vol_val--;
			_vcui_engine_set_volume_level(VCUI_VOL_VOICE, ad->voice_vol_val);
		} else if (key_status == VAL_VOL_UP && ad->voice_vol_val < VOICE_VOL_MAX) {	/*Key Up*/
			ad->voice_vol_val++;
			_vcui_engine_set_volume_level(VCUI_VOL_VOICE, ad->voice_vol_val);
		}
		_vcui_view_popup_vol_voice(ad->voice_vol_val);
	}

}

static void __vcui_fade_out_cb_routine()
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (ad->child_is == 1) {
		/*system("killall dialer");*/
	}
	if (ad->disp && ad->win) {
		utilx_ungrab_key(ad->disp, ad->win, KEY_VOLUMEUP);
		utilx_ungrab_key(ad->disp, ad->win, KEY_VOLUMEDOWN);
	}
	_vcui_doc_remove_all_data();
	_voicecall_dvc_proximity_sensor_deinit();
	_voicecall_dvc_control_lcd_state(VC_LCD_ON_UNLOCK);
	_vcui_view_common_timer_destroy();
}

static int __vcui_lang_changed_cb(void *data)
{
	CALL_UI_DEBUG("..");

	_vcui_view_auto_change();
 
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

static void __vcui_init(vcui_app_call_data_t *ad)
{
	CALL_UI_KPI("g_type_init start");
	g_type_init();
	CALL_UI_KPI("g_type_init done");
	_vcui_doc_recent_init();
	_vcui_doc_caller_list_init();
	_vcui_view_common_init();
	_vcui_engine_init(ad);

	CALL_UI_KPI("__vcui_init_view_register_function for all views start");
	__vcui_init_view_register_function(ad, VIEW_DIALLING_VIEW, _vcui_view_dialing_new);
	__vcui_init_view_register_function(ad, VIEW_INCOMING_VIEW, _vcui_view_incoming_new);
	__vcui_init_view_register_function(ad, VIEW_INCOMING_LOCK_VIEW, _vcui_view_incoming_lock_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_ONECALL_VIEW, _vc_ui_view_single_call_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_MULTICALL_SPLIT_VIEW, _vcui_view_multi_call_split_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_MULTICALL_CONF_VIEW, _vcui_view_multi_call_conf_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_MULTICALL_LIST_VIEW, _vcui_view_multi_call_list_new);
	__vcui_init_view_register_function(ad, VIEW_INCALL_KEYPAD_VIEW, _vcui_view_keypad_new);
	__vcui_init_view_register_function(ad, VIEW_ENDCALL_VIEW, _vcui_view_callend_new);
 	CALL_UI_KPI("__vcui_init_view_register_function for all views done");

	ad->view_top = -1;
	ad->view_before_top = -1;
	ad->view_before_reject_view = -1;
	ad->headset_status = (int)_vcui_is_headset_conected();
	ad->speaker_status = EINA_FALSE;
	ad->mute_status = EINA_FALSE;
	ad->child_is = -1;
	ad->show_flag = WIN_HIDE;
	ad->brecord_voice = 0;
	ad->ringtone_val = -1;
	ad->voice_vol_val = -1;
	ad->bt_vol_val = -1;
	ad->call_end_type = CALL_END_TYPE_NONE;
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

	if (data == NULL)
		return;
	contacts_svc_connect();

	current_time = time(NULL);

	plog = contacts_svc_value_new(CTS_VALUE_PHONELOG);
	contacts_svc_value_set_str(plog, CTS_PLOG_VAL_NUMBER_STR, data->call_num);
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT, (int)current_time);
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT, type);
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_RELATED_ID_INT, data->contact_id);

	if ((type == CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN) || (type == CTS_PLOG_TYPE_VOICE_REJECT) ||
		(type == CTS_PLOG_TYPE_VOICE_BLOCKED) || (boutgoing_end == EINA_TRUE)) {
		contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, 0);
	} else {
		contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, _vcui_get_diff_now(data->start_time));
	}
	contacts_svc_insert_phonelog(plog);

	contacts_svc_value_free(plog);
	contacts_svc_disconnect();

	/* vconf set & quickpanel noti (for missed call) */
	if (type == CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN) {
		char szname[255] = { 0, };
		int ret;
		notification_h noti = NULL;
		notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

		if (strlen((char *)data->call_display) == 0) {
			snprintf(szname, sizeof(szname), "%s", data->call_num);
		} else {
			snprintf(szname, sizeof(szname), "%s", data->call_display);
		}
		CALL_UI_DEBUG("szname:[%s]", szname);

		noti = notification_new(NOTIFICATION_TYPE_NOTI, __VCUI_NOTIFICATION_CALL_GROUP_ID, NOTIFICATION_PRIV_ID_NONE);
		if(noti == NULL) {
			CALL_UI_DEBUG("Fail to notification_new\n");
			return;
		}

		noti_err = notification_set_application(noti, DIALER_PKG);
		if(noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_application : %d\n", noti_err);
		}

		bundle *args  = bundle_create();

		bundle_add(args, "logs", "missed_call");
		noti_err = notification_set_args(noti, args, NULL);
		if(noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_args : %d\n", noti_err);
		}
		bundle_free(args);

		noti_err = notification_set_time(noti, current_time);
		if(noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_icon : %d\n", noti_err);
		}

		noti_err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, szname, szname, NOTIFICATION_VARIABLE_TYPE_NONE);
		if(noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_title : %d\n", noti_err);
		}

		CALL_UI_DEBUG("data->call_file_path(%s)",data->call_file_path);
		if (strlen(data->call_file_path) > 0) {
			noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_THUMBNAIL, data->call_file_path);
			if(noti_err != NOTIFICATION_ERROR_NONE) {
				CALL_UI_DEBUG("Fail to notification_set_title : %d\n", noti_err);
			}
		}
		
		noti_err = notification_insert(noti, NULL);
		if(noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_text_domain\n");
		}

		noti_err = notification_free(noti);
		if(noti_err != NOTIFICATION_ERROR_NONE) {
			CALL_UI_DEBUG("Fail to notification_set_text_domain\n");
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
	/*view_refresh_now();*/

}

int _vcui_check_valid_eo(Evas_Object *eo, char *v_name)
{
	/*CALL_UI_DEBUG("eo addr:[%p], v_name:[%s]", eo, v_name);*/
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

