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

#include "vcui-view-dialing.h"
#include "vcui-view-incoming.h"
#include "vcui-view-single-call.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-keypad.h"
#include "vcui-view-callend.h"

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
	//voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	//vcui_app_call_data_t *ad = vd->app_data;
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
			char data[VC_DATA_LENGTH_MAX] = { 0, };
			_vcui_view_common_set_text_time(_vcui_get_endcause_string(end_type, data));
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
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (common_data->tm_end_dialing) {
		ecore_timer_del(common_data->tm_end_dialing);
		common_data->tm_end_dialing = NULL;
	}

	if (common_data->bredial == EINA_TRUE) {
		/*_vcui_view_all_hide();*/
		_vcui_engine_interface_process_auto_redial(EINA_TRUE);
		_vcui_view_popup_load_redial();
		_vcui_view_dialing_draw_txt_dialing(ad->view_st[VIEW_DIALLING_VIEW]);
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
	int valid = 0;

	if (data != NULL) {
		if (data->type == VIEW_INCALL_ONECALL_VIEW) {
			if (data->priv) {
				incall_one_view_priv_t *priv = data->priv;
				if (priv->contents) {
					/* ============ Check valid Evas Object ============= */
					valid = _vcui_check_valid_eo(priv->contents, "ONEVIEW");
					if (valid == -1) {
						CALL_UI_DEBUG("[========== ONEVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
						return;
					}
					/* ============ Check valid Evas Object ============= */
					edje_object_part_text_set(_EDJ(priv->contents), "txt_timer", _(time_dur));	//TODO
				} else {
					CALL_UI_DEBUG("ERR : Null Evas_Object priv->contents");
				}

			}
		} else if (data->type == VIEW_INCALL_MULTICALL_SPLIT_VIEW) {
			if (data->priv) {
				incall_multi_view_split_priv_t *priv = data->priv;
				/* ============ Check valid Evas Object ============= */
				valid = _vcui_check_valid_eo(priv->contents, "MULTIVIEWSPLIT");
				if (valid == -1) {
					CALL_UI_DEBUG("[========== MULTIVIEWSPLIT : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
					return;
				}
				/* ============ Check valid Evas Object ============= */
				edje_object_part_text_set(_EDJ(priv->contents), "txt_timer", _(time_dur));	//TODO
			} else {
				CALL_UI_DEBUG("ERR : Null Evas_Object data->layout");
			}
		} else if (data->type == VIEW_INCALL_MULTICALL_CONF_VIEW) {
			if (data->priv) {
				vcui_view_multi_call_conf_priv_t *priv = data->priv;
				/* ============ Check valid Evas Object ============= */
				valid = _vcui_check_valid_eo(priv->contents, "MULTIVIEWCONF");
				if (valid == -1) {
					CALL_UI_DEBUG("[========== MULTIVIEWCONF : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
					return;
				}
				/* ============ Check valid Evas Object ============= */
				edje_object_part_text_set(_EDJ(priv->contents), "txt_timer", _(time_dur));	//TODO
			} else {
				CALL_UI_DEBUG("ERR : Null Evas_Object data->layout");
			}
		} else if (data->type == VIEW_INCALL_MULTICALL_LIST_VIEW) {
			if (data->priv) {
				vcui_view_multi_call_list_priv_t *priv = data->priv;
				/* ============ Check valid Evas Object ============= */
				valid = _vcui_check_valid_eo(priv->contents, "MULTIVIEWLIST");
				if (valid == -1) {
					CALL_UI_DEBUG("[========== MULTIVIEWLIST : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
					return;
				}
				/* ============ Check valid Evas Object ============= */
				edje_object_part_text_set(_EDJ(priv->contents), "txt_timer", _(time_dur));	//TODO
			} else {
				CALL_UI_DEBUG("ERR : Null Evas_Object data->layout");
			}
		} else if (data->type == VIEW_INCALL_KEYPAD_VIEW) {
			if (data->priv) {
				vcui_view_keypad_priv_t *priv = data->priv;
				/* ============ Check valid Evas Object ============= */
				valid = _vcui_check_valid_eo(priv->contents, "KEYPADVIEW");
				if (valid == -1) {
					CALL_UI_DEBUG("[========== KEYPADVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
					return;
				}
				/* ============ Check valid Evas Object ============= */
				edje_object_part_text_set(_EDJ(priv->contents), "txt_timer", _(time_dur));	//TODO
			} else {
				CALL_UI_DEBUG("ERR : Null Evas_Object data->layout");
			}
		} else if (data->type == VIEW_ENDCALL_VIEW) {
			if (data->priv) {
				endcall_view_priv_t *priv = data->priv;
				/* ============ Check valid Evas Object ============= */
				valid = _vcui_check_valid_eo(priv->contents, "ENDCALLVIEW");
				if (valid == -1) {
					CALL_UI_DEBUG("[========== KEYPADVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
					return;
				}
				/* ============ Check valid Evas Object ============= */
				edje_object_part_text_set(_EDJ(priv->contents), "txt_timer", _(time_dur));	//TODO
			} else {
				CALL_UI_DEBUG("ERR : Null Evas_Object data->layout");
			}
		} else {
			/*to do nothing in case of other view.*/
		}
	}
 }

void _vcui_view_common_set_each_time(time_t starttime)
{
	time_t curr_time;
	//char call_duration[9];
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

	// set the start time for every call
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
		common_data->time_end_flag = TIME_END_START;	// to stop timer from updating the call end screen
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

	if (_vcui_doc_get_count() == 0) {
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
