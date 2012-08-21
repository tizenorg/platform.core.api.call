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
#include "vcui-view-common.h"
#include "vcui-view-layout-hd.h"

#include "vcui-view-dialing.h"
#include "vcui-view-incoming-lock.h"
#include "vcui-view-single-call.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-keypad.h"
#include "vcui-view-callend.h"
#include "vcui-view-quickpanel.h"

static vcui_view_common_t gincall_common_data;

void _vcui_view_common_init()
{
	memset(&gincall_common_data, 0, sizeof(vcui_view_common_t));
}

static vcui_view_common_t *__vcui_view_common_get_common_data()
{
	return &gincall_common_data;
}

static Eina_Bool __vcui_view_common_timer_cb(void *data)
{
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();
	char dur[TIME_BUF_LEN];
	static int end_count = 0;

	if (common_data->timer_flag == 1) {
		if (common_data->time_end_flag == TIME_END_START) {
			end_count = 0;
			return 1;
		} else if (common_data->time_end_flag == TIME_END_NO) {
			end_count++;
			if (end_count == 1) {
				return 1;
			}
		}
	}
	common_data->current_call_time = time(NULL);

	if (common_data->timer_flag == 1) {
		if (common_data->current_call_time > common_data->start_call_time) {
			time_t call_time = common_data->current_call_time - common_data->start_call_time;
			struct tm loctime;

			gmtime_r((const time_t *)&call_time, &loctime);
			snprintf(dur, TIME_BUF_LEN, "%02d:%02d:%02d", loctime.tm_hour, loctime.tm_min, loctime.tm_sec);

			_vcui_view_common_set_text_time(dur);
		}
	}

	return 1;
}

static Eina_Bool __vcui_view_common_timer_end_cb(void *data)
{
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();
	int end_type = common_data->end_type;
	char dur[TIME_BUF_LEN];

	if ((common_data->timer_flag == 1) && (common_data->time_end_flag == TIME_END_START)) {
		snprintf(dur, TIME_BUF_LEN, "%02d:%02d:%02d", common_data->hour, common_data->min, common_data->sec);
		if (common_data->time_count == 0) {
			_vcui_view_common_set_text_time(dur);
		} else if (common_data->time_count == 1) {
			_vcui_view_common_set_text_time(_(" "));
		} else if (common_data->time_count == 2) {
			_vcui_view_common_set_text_time(dur);
		} else if (common_data->time_count == 3) {
			_vcui_view_common_set_text_time(_(" "));
		} else if (common_data->time_count == 4) {
			char data_txt[VC_DATA_LENGTH_MAX] = { 0, };
			_vcui_view_common_set_text_time(_vcui_get_endcause_string(end_type, data_txt));
		}

		common_data->time_count++;
		if (common_data->time_count == TIME_END_MAX_SHOW) {
			common_data->time_end_flag = TIME_END_NO;
			common_data->time_count = 0;

			if (common_data->tm_end) {
				ecore_timer_del(common_data->tm_end);
				common_data->tm_end = NULL;
			}

			_vcui_view_common_call_terminate_or_view_change();
		}
	}
	return 1;
}

static Eina_Bool __vcui_view_common_timer_end_dialing_cb(void *data)
{
	CALL_UI_DEBUG("..");
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();

	if (common_data->tm_end_dialing) {
		ecore_timer_del(common_data->tm_end_dialing);
		common_data->tm_end_dialing = NULL;
	}

	if (common_data->bredial == EINA_TRUE) {
		vcall_engine_process_auto_redial(EINA_TRUE);
		_vcui_view_popup_load_redial();
	} else {
		_vcui_view_common_call_terminate_or_view_change();
	}

	return 1;
}

static Eina_Bool __vcui_view_common_timer_end_force_cb(void *data)
{
	CALL_UI_DEBUG("..");
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();

	if (common_data->tm_end_force) {
		ecore_timer_del(common_data->tm_end_force);
		common_data->tm_end_force = NULL;
	}

	_vcui_view_common_call_terminate_or_view_change();

	return 1;
}

void _vcui_view_common_set_text_time(char *time_dur)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *data = ad->view_st[ad->view_top];

	if (data != NULL) {
		if (data->type == VIEW_INCALL_ONECALL_VIEW) {
			_vc_ui_view_single_call_set_call_timer(data, time_dur);
		} else if (data->type == VIEW_INCALL_MULTICALL_SPLIT_VIEW) {
			_vcui_view_multi_call_split_set_call_timer(data, time_dur);
		} else if (data->type == VIEW_INCALL_MULTICALL_CONF_VIEW) {
			_vcui_view_multi_call_conf_set_call_timer(data, time_dur);
		} else if (data->type == VIEW_INCALL_MULTICALL_LIST_VIEW) {
			_vcui_view_multi_call_list_set_call_timer(data, time_dur);
		} else if (data->type == VIEW_ENDCALL_VIEW) {
			_vc_ui_view_callend_set_call_timer(data, time_dur);
		} else {
			/*to do nothing in case of other view. */
		}
	}

	if (ad->win_quickpanel && ad->quickpanel_layout) {
		_vc_ui_view_qp_set_call_timer(ad->quickpanel_layout, time_dur);
	}
}

void _vcui_view_common_set_each_time(time_t starttime)
{
	time_t curr_time;
	unsigned long call_duration_in_sec = 0;
	unsigned long sec = 0;
	unsigned long min = 0;
	unsigned long hr = 0;

	curr_time = time(&curr_time);
	call_duration_in_sec = curr_time - starttime;
	sec = call_duration_in_sec % 60;
	min = (call_duration_in_sec / 60) % 60;
	hr = call_duration_in_sec / 3600;

	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();

	/*set the start time for every call */
	common_data->start_call_time = starttime;
	CALL_UI_DEBUG(" common_data->start_call_time %lu", (unsigned long)common_data->start_call_time);

	CALL_UI_DEBUG(" _vcui_view_common_set_each_time curr_time %d starttime %d", (int)curr_time, (int)starttime);

	common_data->sec = sec;
	common_data->min = min;
	common_data->hour = hr;

	char dur[TIME_BUF_LEN];
	snprintf(dur, TIME_BUF_LEN, "%02d:%02d:%02d", common_data->hour, common_data->min, common_data->sec);
	if (common_data->timer_flag == 1)
		_vcui_view_common_set_text_time(dur);

	CALL_UI_DEBUG(" complete input time");

}

void _vcui_view_common_timer_text_init()
{
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();

	if (!common_data->tm) {
		if (common_data->timer_flag == 0) {
			common_data->sec = 0;
			common_data->min = 0;
			common_data->hour = 0;
			common_data->timer_flag = 1;
		}
		common_data->tm = ecore_timer_add(TIMER_TIMEOUT_1_SEC, __vcui_view_common_timer_cb, NULL);
	}
}

void _vcui_view_common_timer_destroy()
{
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();
	if (common_data->tm) {
		ecore_timer_del(common_data->tm);
		common_data->tm = NULL;
	}
}

void _vcui_view_common_timer_end_destroy()
{
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();
	if (common_data->tm_end) {
		ecore_timer_del(common_data->tm_end);
		common_data->tm_end = NULL;
	}
}

void _vcui_view_common_timer_end_dialing_destroy()
{
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();
	if (common_data->tm_end_dialing) {
		ecore_timer_del(common_data->tm_end_dialing);
		common_data->tm_end_dialing = NULL;
	}
}

void _vcui_view_common_timer_redial_reset()
{
	CALL_UI_DEBUG("..");
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();

	_vcui_view_common_timer_end_destroy();
	_vcui_view_common_timer_end_dialing_destroy();
	common_data->time_end_flag = TIME_END_NO;
}

void _vcui_view_common_call_end_show_dialing(int end_type, int bredial)
{
	CALL_UI_DEBUG("end_type:[%d]", end_type);
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();
	char data[VC_DATA_LENGTH_MAX] = { 0, };

	if (ad->call_end_type == CALL_END_TYPE_NONE) {
		_vcui_view_dialing_draw_txt_ended(ad->view_st[ad->view_top], end_type);
	} else if (ad->call_end_type == CALL_END_TYPE_SINGLE_CALL) {
		common_data->time_end_flag = TIME_END_START;	/*to stop timer from updating the call end screen */
		_vcui_view_common_set_text_time(_vcui_get_endcause_string(end_type, data));
	}

	if (common_data->tm_end_dialing) {
		ecore_timer_del(common_data->tm_end_dialing);
		common_data->tm_end_dialing = NULL;
	}
	common_data->bredial = bredial;
	common_data->tm_end_dialing = ecore_timer_add(TIMER_TIMEOUT_2_SEC, __vcui_view_common_timer_end_dialing_cb, NULL);

}

void _vcui_view_common_call_end_show(time_t start_time, int end_type)
{
	CALL_UI_DEBUG("end_type:[%d]", end_type);
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();

	if (common_data->tm_end) {
		ecore_timer_del(common_data->tm_end);
		common_data->tm_end = NULL;
	}
	common_data->time_end_flag = TIME_END_START;
	_vcui_view_common_set_each_time(start_time);

	common_data->end_type = end_type;
	common_data->tm_end = ecore_timer_add(TIMER_TIMEOUT_0_3_SEC, __vcui_view_common_timer_end_cb, NULL);
}

void _vcui_view_common_call_end_timer_reset(void)
{
	CALL_UI_DEBUG("..");
	vcui_view_common_t *common_data = __vcui_view_common_get_common_data();
	if (common_data->tm_end) {
		ecore_timer_del(common_data->tm_end);
		common_data->tm_end = NULL;
	}

	if (common_data->tm_end_dialing) {
		ecore_timer_del(common_data->tm_end_dialing);
		common_data->tm_end_dialing = NULL;
	}

	common_data->tm_end_force = ecore_timer_add(TIMER_TIMEOUT_2_SEC, __vcui_view_common_timer_end_force_cb, NULL);
}

int _vcui_view_common_call_terminate_or_view_change(void)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (_vcui_doc_get_all_call_data_count() == 0) {
		if (ad->contact_ug == NULL) {
			CALL_UI_DEBUG("EXIT - contact ug is closed");
			elm_exit();
		} else {
			CALL_UI_DEBUG("show contact ug");
			CALL_UI_DEBUG("hide & destory [%d]", ad->view_top);
			if (ad->view_top != -1) {
				ad->view_st[ad->view_top]->onHide(ad->view_st[ad->view_top]);
				ad->view_st[ad->view_top]->onDestroy(ad->view_st[ad->view_top]);
				ad->view_top = -1;
			}
			evas_object_show((Evas_Object *)ug_get_layout(ad->contact_ug));
		}
	} else {
		_vcui_view_auto_change();
	}
	return VC_NO_ERROR;
}

void _vcui_view_common_show_noid_image(Evas_Object *layout)
{
    CALL_UI_DEBUG("..");
    vcui_app_call_data_t *ad = _vcui_get_app_data();
    Evas_Object *noid_icon = NULL;

    noid_icon = edje_object_part_swallow_get(_EDJ(layout), "swl-cid-noid");
    if (noid_icon) {
		edje_object_part_unswallow(_EDJ(layout), noid_icon);
		evas_object_del(noid_icon);
    }

    noid_icon = elm_image_add(ad->win_main);
    elm_object_part_content_set(layout, "swl-cid-noid", noid_icon);
    elm_image_file_set(noid_icon, MO_NOCALLER_ID_ICON, NULL);

    edje_object_signal_emit(_EDJ(layout), "show-noid-image", "show-noid");
}

void _vcui_view_common_update_mute_btn()
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	vcui_app_call_view_id_t view_id = ad->view_top;
	voice_call_view_data_t *vd = ad->view_st[view_id];
	VCUI_RETURN_IF_FAIL(vd);

	int valid = 0;

	if (vd->layout) {
		if (view_id == VIEW_DIALLING_VIEW) {
			valid = _vc_ui_view_dialing_check_valid_eo(vd);
		} else if (view_id == VIEW_INCALL_ONECALL_VIEW) {
			valid = _vc_ui_view_single_call_check_valid_eo(vd);
		} else if (view_id == VIEW_INCALL_MULTICALL_SPLIT_VIEW) {
			valid = _vcui_view_multi_call_split_check_valid_eo(vd);
		} else if (view_id == VIEW_INCALL_MULTICALL_CONF_VIEW) {
			valid = _vcui_view_multi_call_conf_check_valid_eo(vd);
		} else if (view_id == VIEW_INCALL_MULTICALL_LIST_VIEW) {
			valid = _vcui_view_multi_call_list_check_valid_eo(vd);
		} else {
			CALL_UI_DEBUG("[============ BAD INPUT!!!! Check Input Layout!!!!! %d============]", view_id);
		}
		if (valid == -1) {
			CALL_UI_DEBUG("[========== WARNING!! Invalid Evas Object  ==========]");
			return;
		}
	}

	_vcui_create_bottom_middle_button(vd);
}

