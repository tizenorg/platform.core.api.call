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
#include "vcui-view-popup.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-single-call.h"
#include "vcui-view-common.h"
 #define VCONFKEY_BT_HEADSET_NAME "memory/bluetooth/sco_headset_name"

#define TICKER_TIMEOUT	2
static void __vcui_view_popup_response_cb_vol_voice(void *data, Evas_Object *obj, void *event_info);

static void __vcui_view_popup_win_del(void *data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	CALL_UI_DEBUG("delete the popup main_win - if created...");
	if (ad->popup_mw != NULL) {
		CALL_UI_DEBUG("main_win is NOT null");
		evas_object_del(ad->popup_mw);
		ad->popup_mw = NULL;
	}
}

void _vcui_view_popup_unload(Evas_Object *popup_eo)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (popup_eo) {
		evas_object_hide(popup_eo);
		evas_object_del(popup_eo);
		popup_eo = NULL;
	}

	__vcui_view_popup_win_del(ad);
	_vcui_cache_flush();

	return;
}

static void __vcui_view_popup_response_cb_terminate(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->popup_eo) {
		CALL_UI_DEBUG("popup eo delete.");
		evas_object_hide(ad->popup_eo);
		evas_object_del(ad->popup_eo);
		ad->popup_eo = NULL;
	}
	_vcui_cache_flush();
	__vcui_view_popup_win_del(ad);

	_vcui_view_common_call_terminate_or_view_change();

	return;
}

static void __vcui_view_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->popup_eo) {
		CALL_UI_DEBUG("popup eo delete.");
		evas_object_hide(ad->popup_eo);
		evas_object_del(ad->popup_eo);
		ad->popup_eo = NULL;
	}
	_vcui_cache_flush();
	__vcui_view_popup_win_del(ad);
	return;
}

/* create a separate main window for the popups with a high priority   */
/* this window will always be displayed on top, even on a lock screen  */
static Evas_Object *__vcui_view_popup_create_win(vcui_app_call_data_t *ad, const char *name)
{
	CALL_UI_DEBUG("..");
	Ecore_X_Window xwin;
	Evas_Object *eo;
	int w, h;

	CALL_UI_DEBUG("create the popup main_win...");
	eo = elm_win_add(NULL, name, ELM_WIN_DIALOG_BASIC);
	if (eo) {
		elm_win_alpha_set(eo, EINA_TRUE);
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		elm_win_raise(eo);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
		evas_object_resize(eo, w, h);

		/* Set the popup window type as LEVEL-HIGH so it is always displayed on top */
		CALL_UI_DEBUG("...Set HIGH priority...");
		xwin = elm_win_xwindow_get(eo);
		ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);
		utilx_set_system_notification_level(ecore_x_display_get(), xwin, UTILX_NOTIFICATION_LEVEL_HIGH);

		evas_object_show(eo);
	}

	return eo;
}

void _vcui_view_popup_load(char *popup_msg, double time_out, int bterminate)
{
	CALL_UI_DEBUG("msg:[%s], bterminate:[%d]", popup_msg, bterminate);
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (popup_msg == NULL) {
		return;
	}

	_vcui_view_popup_unload(ad->popup_eo);

#ifdef	CALL_MW
	if (ad->bwin_top) {
		CALL_UI_DEBUG("parent window TOP");
		ad->popup_eo = elm_popup_add(ad->win_main);
		_vcui_show_main_ui_set_flag();
	} else
#endif
	{
		ad->popup_mw = __vcui_view_popup_create_win(ad, "base");
		ad->popup_eo = elm_popup_add(ad->popup_mw);
	}
	evas_object_size_hint_weight_set(ad->popup_eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(ad->popup_eo, popup_msg);
	elm_popup_timeout_set(ad->popup_eo, time_out);
	if (bterminate == 0) {
		evas_object_smart_callback_add(ad->popup_eo, "timeout", __vcui_view_popup_response_cb, ad);
	} else {
		evas_object_smart_callback_add(ad->popup_eo, "timeout", __vcui_view_popup_response_cb_terminate, ad);
	}
	evas_object_show(ad->popup_eo);
}

void _vcui_view_popup_replace(char *popup_msg, double time_out, int bterminate)
{
	CALL_UI_DEBUG("msg:[%s], bterminate:[%d]", popup_msg, bterminate);
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (ad->popup_eo == NULL) {
		_vcui_view_popup_load(popup_msg, time_out, bterminate);
	} else {
		if (popup_msg == NULL) {
			return;
		}

		if (ad->view_top != -1) {
#ifdef	CALL_MW
			_vcui_raise_main_win();
#else
			if (ad->popup_mw != NULL)
				elm_win_activate(ad->popup_mw);
#endif
		}

		elm_object_text_set(ad->popup_eo, popup_msg);
		elm_popup_timeout_set(ad->popup_eo, time_out);
		if (bterminate == 0) {
			evas_object_smart_callback_add(ad->popup_eo, "timeout", __vcui_view_popup_response_cb, ad);
		} else {
			evas_object_smart_callback_add(ad->popup_eo, "timeout", __vcui_view_popup_response_cb_terminate, ad);
		}
		evas_object_show(ad->popup_eo);
	}
}

void _vcui_view_popup_load_reject_call(char *name, char *number, int end_app)
{
	CALL_UI_DEBUG("..");
	char msg[DEF_BUF_LEN_LONG] = { 0, };

	if (strlen(name) == 0) {
		snprintf(msg, sizeof(msg), "%s<br>%s", number, _("IDS_CALL_POP_REJECTED"));
	} else if (strlen(number) == 0) {
		snprintf(msg, sizeof(msg), "%s<br>%s", name, _("IDS_CALL_POP_REJECTED"));
	} else {
		snprintf(msg, sizeof(msg), "%s<br>%s<br>%s", name, number, _("IDS_CALL_POP_REJECTED"));
	}

	_vcui_view_popup_load(msg, POPUP_TIMEOUT_LONG, end_app);

}

void _vcui_view_popup_load_endcall_time(call_data_t *cd)
{
	unsigned long sec = 0;
	unsigned long min = 0;
	unsigned long hr = 0;
	unsigned long call_duration_in_sec = _vcui_get_diff_now(cd->start_time);
	char buf[DEF_BUF_LEN] = { 0, };
	char msg[DEF_BUF_LEN_LONG] = { 0, };

	sec = call_duration_in_sec % 60;
	min = (call_duration_in_sec / 60) % 60;
	hr = call_duration_in_sec / 3600;
	snprintf(buf, sizeof(buf), "%c%c:%c%c:%c%c", (int)((hr / 10) + '0'), (int)((hr % 10) + '0'), (int)((min / 10) + '0'), (int)((min % 10) + '0'), (int)((sec / 10) + '0'), (int)((sec % 10) + '0'));

	snprintf(msg, sizeof(msg), "%s<br>%s<br>%s", cd->call_display, cd->call_num, buf);

	_vcui_view_popup_load(msg, POPUP_TIMEOUT_SHORT, EINA_FALSE);
}

void _vcui_view_popup_load_redial(void)
{
	_vcui_view_popup_load(_("IDS_CALL_POP_REDIALLING"), POPUP_TIMEOUT_NORMAL, EINA_FALSE);
}

static Eina_Bool __vcui_view_popup_timer_cb_vol_ringtone(void *data)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	ad->vol_ringtone_popup_del_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __vcui_view_popup_vol_update_icon(int volume_level)
{
	CALL_UI_DEBUG("...");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	Evas_Object *icon = NULL;

	icon = edje_object_part_swallow_get(_EDJ(ad->popup_vol_ly), "volume_icon");
	if (icon) {
		edje_object_part_unswallow(_EDJ(ad->popup_vol_ly), icon);
		evas_object_del(icon);
	}

	icon = elm_icon_add(ad->popup_vol_ly);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_resizable_set(icon, 0, 0);
	elm_object_part_content_set(ad->popup_vol_ly, "volume_icon", icon);
	if (volume_level == 0)
		elm_icon_file_set(icon, VOLUME_MUTE_ICON, NULL);
	else
		elm_icon_file_set(icon, VOLUME_ICON, NULL);
}

static void __vcui_view_popup_vol_ringtone_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	double value;

	value = elm_slider_value_get(obj);
	if (value > RINGTONE_MAX)
		value = RINGTONE_MAX;
	else if (value < RINGTONE_MIN)
		value = RINGTONE_MIN;

	ad->ringtone_val = (int)value;

	_vcui_engine_set_volume_level(VCUI_VOL_RING, ad->ringtone_val);

	elm_slider_value_set(obj, ad->ringtone_val);
	if (ad->vol_ringtone_popup_del_timer) {
		ecore_timer_del(ad->vol_ringtone_popup_del_timer);
		ad->vol_ringtone_popup_del_timer = NULL;
	}

	CALL_UI_DEBUG("ad->ringtone_val %d...", ad->ringtone_val);
	__vcui_view_popup_vol_update_icon(ad->ringtone_val);
	ad->vol_ringtone_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_ringtone, ad);
}

static void __vcui_view_popup_response_cb_vol_ringtone(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->vol_ringtone_popup_eo) {
		CALL_UI_DEBUG("vol_ringtone_popup_eo delete.");
		evas_object_del(ad->vol_ringtone_popup_eo);
		ad->vol_ringtone_popup_eo = NULL;
	}

	ad->vol_longpress_cnt = 0;

	if (ad->volup_key_longpress_timer) {
		ecore_timer_del(ad->volup_key_longpress_timer);
		ad->volup_key_longpress_timer = NULL;
	}

	if (ad->voldown_key_longpress_timer) {
		ecore_timer_del(ad->voldown_key_longpress_timer);
		ad->voldown_key_longpress_timer = NULL;
	}

	_vcui_cache_flush();
	__vcui_view_popup_win_del(ad);
	return;
}

void _vcui_view_popup_vol_ringtone(int vol_level)
{
	CALL_UI_DEBUG("vol_level(%d)", vol_level);
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	Evas_Object *popup = NULL;
	Evas_Object *slider = NULL;
	Evas_Object *icon = NULL;

	if (ad->vol_ringtone_popup_eo) {
		__vcui_view_popup_vol_update_icon(vol_level);
		elm_slider_value_set(ad->vol_ringtone_slider_eo, vol_level);
		if (ad->vol_ringtone_popup_del_timer) {
			ecore_timer_del(ad->vol_ringtone_popup_del_timer);
			ad->vol_ringtone_popup_del_timer = NULL;
		}
		ad->vol_ringtone_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_ringtone, ad);
	} else {
#ifdef	CALL_MW
		popup = elm_popup_add(ad->win_main);
#else
		ad->popup_mw = __vcui_view_popup_create_win(ad, "base");
		ad->vol_ringtone_popup_eo = popup = elm_popup_add(ad->popup_mw);
#endif
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_text_set(popup, "title,text", dgettext("sys_string", "IDS_COM_BODY_RINGTONE"));

		ad->popup_vol_ly = _vcui_load_edj(popup, EDJ_NAME, "volume");
		if (ad->popup_vol_ly == NULL) {
			if (ad->vol_ringtone_popup_eo) {
				CALL_UI_DEBUG("vol_ringtone_popup_eo delete.");
				evas_object_del(ad->vol_ringtone_popup_eo);
				ad->vol_ringtone_popup_eo = NULL;
			}
			__vcui_view_popup_win_del(ad);
			return;
		}
		elm_object_content_set(popup, ad->popup_vol_ly);
		__vcui_view_popup_vol_update_icon(vol_level);

		slider = elm_slider_add(popup);
		elm_slider_horizontal_set(slider, EINA_TRUE);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%.0f");
		elm_object_part_content_set(ad->popup_vol_ly, "volume_slider", slider);
		elm_slider_min_max_set(slider, 0, 15);
		elm_slider_value_set(slider, vol_level);
		evas_object_smart_callback_add(slider, "changed", __vcui_view_popup_vol_ringtone_changed_cb, ad);
		ad->vol_ringtone_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_ringtone, ad);
		evas_object_smart_callback_add(popup, "timeout", __vcui_view_popup_response_cb_vol_ringtone, ad);

		ad->vol_ringtone_slider_eo = slider;
		evas_object_show(popup);
	}

}

static Eina_Bool __vcui_view_popup_timer_cb_vol_voice(void *data)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	__vcui_view_popup_response_cb_vol_voice(ad, ad->vol_voice_popup_eo, NULL);
	ad->vol_voice_popup_del_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __vcui_view_popup_vol_voice_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	double value;

	value = elm_slider_value_get(obj);
	if (value > VOICE_VOL_MAX)
		value = VOICE_VOL_MAX;
	else if (value < VOICE_VOL_MIN)
		value = VOICE_VOL_MIN;
	ad->voice_vol_val = (int)value;

	_vcui_engine_set_volume_level(VCUI_VOL_VOICE, ad->voice_vol_val);

	elm_slider_value_set(obj, ad->voice_vol_val);
	if (ad->vol_voice_popup_del_timer) {
		ecore_timer_del(ad->vol_voice_popup_del_timer);
		ad->vol_voice_popup_del_timer = NULL;
	}
	ad->vol_voice_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_voice, ad);
}

static void __vcui_view_popup_response_cb_vol_voice(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->vol_voice_popup_eo) {
		CALL_UI_DEBUG("vol_voice_popup_eo delete.");
		evas_object_del(ad->vol_voice_popup_eo);
		ad->vol_voice_popup_eo = NULL;
	}

	ad->vol_longpress_cnt = 0;

	if (ad->volup_key_longpress_timer) {
		ecore_timer_del(ad->volup_key_longpress_timer);
		ad->volup_key_longpress_timer = NULL;
	}

	if (ad->voldown_key_longpress_timer) {
		ecore_timer_del(ad->voldown_key_longpress_timer);
		ad->voldown_key_longpress_timer = NULL;
	}

	_vcui_cache_flush();
	__vcui_view_popup_win_del(ad);
	return;
}

void _vcui_view_popup_vol_voice(int vol_level)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	Evas_Object *popup = NULL;
	Evas_Object *slider = NULL;
	Evas_Object *icon = NULL;

	if (ad->vol_voice_popup_eo) {
		elm_slider_value_set(ad->vol_voice_slider_eo, vol_level);
		if (ad->vol_voice_popup_del_timer) {
			ecore_timer_del(ad->vol_voice_popup_del_timer);
			ad->vol_voice_popup_del_timer = NULL;
		}
		ad->vol_voice_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_voice, ad);
	} else {
#ifdef	CALL_MW
		popup = elm_popup_add(ad->win_main);
#else
		ad->popup_mw = __vcui_view_popup_create_win(ad, "base");
		popup = elm_popup_add(ad->popup_mw);
#endif
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_text_set(popup, "title,text", _("IDS_CALL_BODY_CALL"));

		ad->popup_vol_ly = _vcui_load_edj(popup, EDJ_NAME, "volume");
		if (ad->popup_vol_ly == NULL) {
			if (popup) {
				CALL_UI_DEBUG("popup delete.");
				evas_object_del(popup);
				popup = NULL;
			}
			__vcui_view_popup_win_del(ad);
			return;
		}
		elm_object_content_set(popup, ad->popup_vol_ly);
		__vcui_view_popup_vol_update_icon(vol_level);

		slider = elm_slider_add(ad->popup_vol_ly);
		elm_slider_horizontal_set(slider, EINA_TRUE);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%.0f");
		elm_object_part_content_set(ad->popup_vol_ly, "volume_slider", slider);
		elm_slider_min_max_set(slider, 0, 7);
		elm_slider_value_set(slider, vol_level);
		evas_object_smart_callback_add(slider, "changed", __vcui_view_popup_vol_voice_changed_cb, ad);
		ad->vol_voice_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_voice, ad);
		evas_object_smart_callback_add(popup, "block,clicked", __vcui_view_popup_response_cb_vol_voice, ad);

		ad->vol_voice_slider_eo = slider;
		ad->vol_voice_popup_eo = popup;
		evas_object_show(popup);
	}
}

static Eina_Bool __vcui_view_popup_timer_cb_vol_bt(void *data)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	ad->vol_bt_popup_del_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void __vcui_view_popup_vol_bt_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	double value;

	value = elm_slider_value_get(obj);
	if (value > BT_VOL_MAX)
		value = BT_VOL_MAX;
	else if (value < BT_VOL_MIN)
		value = BT_VOL_MIN;

	ad->bt_vol_val = (int)value;

	_vcui_engine_set_volume_level(VCUI_VOL_HEADSET, ad->bt_vol_val);

	elm_slider_value_set(obj, ad->bt_vol_val);
	if (ad->vol_bt_popup_del_timer) {
		ecore_timer_del(ad->vol_bt_popup_del_timer);
		ad->vol_bt_popup_del_timer = NULL;
	}
	ad->vol_bt_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_bt, ad);
}

static void __vcui_view_popup_response_cb_vol_bt(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->vol_bt_popup_eo) {
		CALL_UI_DEBUG("vol_bt_popup_eo delete.");
		evas_object_del(ad->vol_bt_popup_eo);
		ad->vol_bt_popup_eo = NULL;
		__vcui_view_popup_win_del(ad);
	}

	ad->vol_longpress_cnt = 0;

	if (ad->volup_key_longpress_timer) {
		ecore_timer_del(ad->volup_key_longpress_timer);
		ad->volup_key_longpress_timer = NULL;
	}

	if (ad->voldown_key_longpress_timer) {
		ecore_timer_del(ad->voldown_key_longpress_timer);
		ad->voldown_key_longpress_timer = NULL;
	}

	_vcui_cache_flush();
	return;
}

void _vcui_view_popup_vol_bt(int vol_level)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	Evas_Object *popup = NULL;
	Evas_Object *slider = NULL;
	Evas_Object *icon = NULL;

	if (ad->vol_bt_popup_eo) {
		elm_slider_value_set(ad->vol_bt_slider_eo, vol_level);
		if (ad->vol_bt_popup_del_timer) {
			ecore_timer_del(ad->vol_bt_popup_del_timer);
			ad->vol_bt_popup_del_timer = NULL;
		}
		ad->vol_bt_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_bt, ad);
	} else {
#ifdef	CALL_MW
		popup = elm_popup_add(ad->win_main);
#else
		ad->popup_mw = __vcui_view_popup_create_win(ad, "base");
		popup = elm_popup_add(ad->popup_mw);
#endif
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_text_set(popup, "title,text", _("IDS_CALL_BUTTON_HEADSET"));

		ad->popup_vol_ly = _vcui_load_edj(popup, EDJ_NAME, "volume");
		if (ad->popup_vol_ly == NULL) {
			if (popup) {
				CALL_UI_DEBUG("popup delete.");
				evas_object_del(popup);
				popup = NULL;
			}
			__vcui_view_popup_win_del(ad);
			return;
		}
		elm_object_content_set(popup, ad->popup_vol_ly);
		__vcui_view_popup_vol_update_icon(vol_level);

		slider = elm_slider_add(popup);
		elm_slider_horizontal_set(slider, EINA_TRUE);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%.0f");
		elm_object_part_content_set(ad->popup_vol_ly, "volume_slider", slider);
		elm_slider_min_max_set(slider, 0, 15);
		elm_slider_value_set(slider, vol_level);
		evas_object_smart_callback_add(slider, "changed", __vcui_view_popup_vol_bt_changed_cb, ad);
		ad->vol_bt_popup_del_timer = ecore_timer_add(POPUP_TIMEOUT_SHORT, __vcui_view_popup_timer_cb_vol_bt, ad);
		evas_object_smart_callback_add(popup, "timeout", __vcui_view_popup_response_cb_vol_bt, ad);

		ad->vol_bt_slider_eo = slider;
		ad->vol_bt_popup_eo = popup;
		evas_object_show(popup);
	}

}

static Eina_Bool __vcui_view_popup_response_cb_delay(void *data)
{
	CALL_UI_DEBUG("..");

	_vcui_view_popup_load((char *)data, POPUP_TIMEOUT_SHORT, EINA_FALSE);

	vcui_app_call_data_t *ad = _vcui_get_app_data();
	ecore_timer_del(ad->popup_delay);
	ad->popup_delay = NULL;

	CALL_UI_DEBUG("delay popup work");
	return EINA_FALSE;
}

void _vcui_view_popup_load_with_delay(char *popup_msg, double delay_time)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	ad->popup_delay = ecore_timer_add(delay_time, __vcui_view_popup_response_cb_delay, popup_msg);
	CALL_UI_DEBUG("delay popup");
}

void _vcui_view_popup_unload_progress(vcui_app_call_data_t *ad)
{
	CALL_UI_DEBUG("..");

	if (ad->popup_progress_eo) {
		evas_object_hide(ad->popup_progress_eo);
		evas_object_del(ad->popup_progress_eo);
		ad->popup_progress_eo = NULL;
	}
	__vcui_view_popup_win_del(ad);

	_vcui_cache_flush();

	return;
}

static void __vcui_view_popup_progressbar_set_label(char *status_string)
{
	CALL_UI_DEBUG("..");
	Evas_Object *label;
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	CALL_UI_DEBUG("popup_progress_set_label ...\n");

	label = (Evas_Object *)evas_object_data_get(ad->popup_progress_eo, "progress_label");
	elm_object_text_set(label, status_string);
}

static void __vcui_view_popup_progressbar_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	CALL_UI_DEBUG("__vcui_view_popup_progressbar_response_cb callback=%d\n", (int)event_info);

	if (ad->popup_progress_eo) {
		evas_object_del(ad->popup_progress_eo);
		ad->popup_progress_eo = NULL;
	}

	evas_object_del(obj);
	__vcui_view_popup_win_del(ad);
}

void _vcui_view_popup_load_progress(char *display_string)
{
	CALL_UI_DEBUG("..");
	Evas_Object *progressbar_eo;
	Evas_Object *box, *label;
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	if (NULL != ad->popup_progress_eo) {
		__vcui_view_popup_progressbar_set_label(display_string);
		return;
	}
#ifdef	CALL_MW
	ad->popup_progress_eo = elm_popup_add(ad->win_main);
#else
	ad->popup_mw = __vcui_view_popup_create_win(ad, "base");
	ad->popup_progress_eo = elm_popup_add(ad->popup_mw);
#endif

	label = elm_label_add(ad->popup_progress_eo);
	elm_object_text_set(label, display_string);
	evas_object_show(label);
	box = elm_box_add(ad->popup_progress_eo);
	progressbar_eo = elm_progressbar_add(ad->popup_progress_eo);
	elm_object_style_set(progressbar_eo, "list_progress");
	elm_progressbar_pulse(progressbar_eo, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar_eo, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar_eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar_eo, EINA_TRUE);
	evas_object_show(progressbar_eo);
	elm_box_pack_end(box, label);
	elm_box_pack_end(box, progressbar_eo);
	evas_object_show(box);
	elm_object_content_set(ad->popup_progress_eo, box);
	evas_object_smart_callback_add(ad->popup_progress_eo, "timeout", __vcui_view_popup_progressbar_response_cb, NULL);
	evas_object_show(ad->popup_progress_eo);

	evas_object_data_set(ad->popup_progress_eo, "progress_label", (void *)label);
}

void _vcui_view_popup_load_sending_dtmf(char *status_string, char *dtmf_num)
{
	CALL_UI_DEBUG("..");
	char msg[DEF_BUF_LEN_LONG] = { 0, };

	snprintf(msg, DEF_BUF_LEN_LONG, "%s<br>%s", status_string, dtmf_num);

	_vcui_view_popup_load_progress(msg);
}

static void __vcui_view_popup_snd_path_bt_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_HEADSET);

	_vcui_engine_change_sound_path(VCUI_AUDIO_HEADSET);

	ad->speaker_status = EINA_FALSE;
	ad->headset_status = EINA_TRUE;
}

static void __vcui_view_popup_snd_path_earjack_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_EARJACK);

	_vcui_engine_change_sound_path(VCUI_AUDIO_EARJACK);

	ad->speaker_status = EINA_FALSE;
	ad->headset_status = EINA_FALSE;
}

static void __vcui_view_popup_snd_path_speaker_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_SPEAKER);

	_vcui_engine_change_sound_path(VCUI_AUDIO_SPEAKER);

	ad->speaker_status = EINA_TRUE;
	ad->headset_status = EINA_FALSE;
}

static void __vcui_view_popup_hide_snd_path_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->ctxpopup_eo) {
		evas_object_del(ad->ctxpopup_eo);
		ad->ctxpopup_eo = NULL;
	}
}

static void __vcui_view_popup_move_snd_path(Evas_Object *ctxpopup, Evas_Object *win, Evas_Coord touch_x, Evas_Coord touch_y)
{
	evas_object_move(ctxpopup, touch_x, touch_y);
}

void _vcui_view_popup_load_snd_path()
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	int bearjack_status = EINA_FALSE;

	Evas_Object *ctxpopup = NULL;
	Evas_Object *radio = NULL;
	Evas_Object *group = NULL;

	int status = -1;
	char *bt_name = NULL;
	int snd_path;

	/* get bt name */
	bt_name = vconf_get_str(VCONFKEY_BT_HEADSET_NAME);
	if (!bt_name) {
		bt_name = _("IDS_CALL_BUTTON_HEADSET");
	}

	/* check earjack status */
	if (!vconf_get_int(VCONFKEY_SYSMAN_EARJACK, &status)) {
		if (status != VCONFKEY_SYSMAN_EARJACK_REMOVED) {
			bearjack_status = EINA_TRUE;
		}
	}

	if (ad->ctxpopup_eo) {
		evas_object_del(ad->ctxpopup_eo);
		ad->ctxpopup_eo = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->win_main);

	evas_object_smart_callback_add(ctxpopup, "dismissed", __vcui_view_popup_hide_snd_path_cb, ad);

	__vcui_view_popup_move_snd_path(ctxpopup, ad->win_main, ad->touch_x, ad->touch_y);

	/* bt headset */
	group = radio = elm_radio_add(ctxpopup);
	elm_radio_state_value_set(radio, VCUI_SND_PATH_HEADSET);
	evas_object_data_set(radio, "idx", (void *)(VCUI_SND_PATH_HEADSET));
	evas_object_show(radio);
	elm_ctxpopup_item_append(ctxpopup, bt_name, radio, __vcui_view_popup_snd_path_bt_cb, ad);

	/* earjack or receiver */
	radio = elm_radio_add(ctxpopup);
	elm_radio_state_value_set(radio, VCUI_SND_PATH_EARJACK);
	elm_radio_group_add(radio, group);
	evas_object_data_set(radio, "idx", (void *)(VCUI_SND_PATH_EARJACK));
	evas_object_show(radio);
	if (bearjack_status) {
		elm_ctxpopup_item_append(ctxpopup, _("IDS_CALL_BUTTON_HEADPHONES"), radio, __vcui_view_popup_snd_path_earjack_cb, ad);
	} else {
		elm_ctxpopup_item_append(ctxpopup, _("IDS_CALL_BUTTON_RECEIVER"), radio, __vcui_view_popup_snd_path_earjack_cb, ad);
	}

	/* speaker */
	radio = elm_radio_add(ctxpopup);
	elm_radio_state_value_set(radio, VCUI_SND_PATH_SPEAKER);
	elm_radio_group_add(radio, group);
	evas_object_data_set(radio, "idx", (void *)(VCUI_SND_PATH_SPEAKER));
	evas_object_show(radio);
	elm_ctxpopup_item_append(ctxpopup, _("IDS_CALL_BUTTON_SPEAKER"), radio, __vcui_view_popup_snd_path_speaker_cb, ad);
	/* current sound path set */
	snd_path = _vcui_engine_get_sound_path();
	switch (snd_path) {
	case VCUI_AUDIO_SPEAKER:
		{
			elm_radio_value_set(group, VCUI_SND_PATH_SPEAKER);
		}
		break;
	case VCUI_AUDIO_HEADSET:
		{
			elm_radio_value_set(group, VCUI_SND_PATH_HEADSET);
		}
		break;
	case VCUI_AUDIO_EARJACK:
	case VCUI_AUDIO_RECEIVER:
		{
			elm_radio_value_set(group, VCUI_SND_PATH_EARJACK);
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong type:[%d]", snd_path);
		return;
		break;
	}

	evas_object_show(ctxpopup);

	ad->ctxpopup_eo = ctxpopup;
	ad->ctxpopup_radio_group_eo = group;
}

#if 0
static Eina_Bool __vcui_view_ticker_response_cb(void *data)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;
	CALL_UI_DEBUG("..");

	if (ad->ticker_tm) {
		ecore_timer_del(ad->ticker_tm);
		ad->ticker_tm = NULL;
	}

	/* to be implemented */

	return ECORE_CALLBACK_CANCEL;
}

void _vcui_view_create_ticker_noti(char *ticker_msg)
{
	Evas_Object *ticker_noti = NULL;
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	CALL_UI_DEBUG("load_ticker_noti_at_bottom enter \n");

	ticker_noti = elm_tickernoti_add (NULL);
	elm_object_style_set(ticker_noti, "info");
	elm_object_text_set(ticker_noti, ticker_msg);
	elm_tickernoti_orient_set(ticker_noti, ELM_TICKERNOTI_ORIENT_BOTTOM);
	if (ad->ticker_tm) {
		ecore_timer_del(ad->ticker_tm);
		ad->ticker_tm = NULL;
	}
	ad->ticker_tm = ecore_timer_add(TICKER_TIMEOUT, __vcui_view_ticker_response_cb, ad);
	evas_object_show(ticker_noti);

	CALL_UI_DEBUG("load_ticker_noti_at_bottom over \n");
}
#endif
