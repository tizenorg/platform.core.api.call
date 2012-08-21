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
#include "vcui-document.h"
#include "vcui-view-popup.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-single-call.h"
#include "vcui-view-common.h"
#define VCONFKEY_BT_HEADSET_NAME "memory/bluetooth/sco_headset_name"

#define TICKER_TIMEOUT	2

#define	POPUP_LIST_HD_W 610
#define	POPUP_LIST_ITEM_HD_H 113

typedef struct {
	Evas_Object *glist;
	Evas_Object *btn;

	Elm_Genlist_Item_Class *itc_option_list;
	char *text_buffer[3];
}_second_mtcall_popup_data_t;

static _second_mtcall_popup_data_t *g_popup_data;

static void __vcui_view_popup_response_cb_vol_voice(void *data, Evas_Object *obj, void *event_info);
static void __vcui_view_popup_response_cb_vol_ringtone(void *data, Evas_Object *obj, void *event_info);
static void __vcui_view_popup_response_cb_vol_bt(void *data, Evas_Object *obj, void *event_info);

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

		elm_win_rotation_with_resize_set(eo, ad->rotate_angle);
		evas_object_show(eo);
	}

	return eo;
}

void _vcui_view_popup_load(char *popup_msg, double time_out, int bterminate)
{
	CALL_UI_DEBUG("msg:[%s], bterminate:[%d]", popup_msg, bterminate);
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	char *markup_text = NULL;

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

	markup_text = evas_textblock_text_utf8_to_markup(NULL, popup_msg);
	if (markup_text) {
		evas_object_size_hint_weight_set(ad->popup_eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_text_set(ad->popup_eo, markup_text);
		elm_popup_timeout_set(ad->popup_eo, time_out);
		if (bterminate == 0) {
			evas_object_smart_callback_add(ad->popup_eo, "timeout", __vcui_view_popup_response_cb, ad);
		} else {
			evas_object_smart_callback_add(ad->popup_eo, "timeout", __vcui_view_popup_response_cb_terminate, ad);
		}
		evas_object_show(ad->popup_eo);
		free(markup_text);
		markup_text = NULL;
	} else {
		evas_object_del(ad->popup_eo);
		ad->popup_eo = NULL;
		__vcui_view_popup_win_del(ad);
		return;
	}
}

void _vcui_view_popup_replace(char *popup_msg, double time_out, int bterminate)
{
	CALL_UI_DEBUG("msg:[%s], bterminate:[%d]", popup_msg, bterminate);
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	char *markup_text = NULL;

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

		markup_text = evas_textblock_text_utf8_to_markup(NULL, popup_msg);
		if (markup_text) {
			elm_object_text_set(ad->popup_eo, markup_text);
			elm_popup_timeout_set(ad->popup_eo, time_out);
			if (bterminate == 0) {
				evas_object_smart_callback_add(ad->popup_eo, "timeout", __vcui_view_popup_response_cb, ad);
			} else {
				evas_object_smart_callback_add(ad->popup_eo, "timeout", __vcui_view_popup_response_cb_terminate, ad);
			}
			evas_object_show(ad->popup_eo);
			free(markup_text);
			markup_text = NULL;
		} else {
			evas_object_del(ad->popup_eo);
			ad->popup_eo = NULL;
			__vcui_view_popup_win_del(ad);
			return;
		}
	}
}

void _vcui_view_popup_load_reject_call(char *name, char *number, int end_app)
{
	CALL_UI_DEBUG("..");
	char msg[DEF_BUF_LEN_LONG] = { 0, };

	if (strlen(name) == 0) {
		snprintf(msg, sizeof(msg), "%s\n%s", number, _("IDS_CALL_POP_REJECTED"));
	} else if (strlen(number) == 0) {
		snprintf(msg, sizeof(msg), "%s\n%s", name, _("IDS_CALL_POP_REJECTED"));
	} else {
		snprintf(msg, sizeof(msg), "%s\n%s\n%s", name, number, _("IDS_CALL_POP_REJECTED"));
	}

	_vcui_view_popup_load(msg, POPUP_TIMEOUT_LONG, end_app);

}

void _vcui_view_popup_load_endcall_time(call_data_t *cd)
{
	unsigned long sec = 0;
	unsigned long min = 0;
	unsigned long hr = 0;
	unsigned long call_duration_in_sec = _vcui_get_diff_now(_vcui_doc_get_call_start_time(cd));
	char buf[DEF_BUF_LEN] = { 0, };
	char msg[DEF_BUF_LEN_LONG] = { 0, };

	sec = call_duration_in_sec % 60;
	min = (call_duration_in_sec / 60) % 60;
	hr = call_duration_in_sec / 3600;
	snprintf(buf, sizeof(buf), "%c%c:%c%c:%c%c", (int)((hr / 10) + '0'), (int)((hr % 10) + '0'), (int)((min / 10) + '0'), (int)((min % 10) + '0'), (int)((sec / 10) + '0'), (int)((sec % 10) + '0'));

	snprintf(msg, sizeof(msg), "%s\n%s\n%s", _vcui_doc_get_call_display_name(cd), _vcui_doc_get_call_number(cd), buf);

	_vcui_view_popup_load(msg, POPUP_TIMEOUT_SHORT, EINA_FALSE);
}

void _vcui_view_popup_load_redial(void)
{
	_vcui_view_popup_load(_("IDS_CALL_POP_REDIALLING"), POPUP_TIMEOUT_NORMAL, EINA_FALSE);
}

static Eina_Bool __vcui_view_popup_timer_cb_vol_ringtone(void *data)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	__vcui_view_popup_response_cb_vol_ringtone(ad, ad->vol_ringtone_popup_eo, NULL);

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

	icon = elm_image_add(ad->popup_vol_ly);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, 0, 0);
	elm_object_part_content_set(ad->popup_vol_ly, "volume_icon", icon);
	if (volume_level == 0)
		elm_image_file_set(icon, VOLUME_MUTE_ICON, NULL);
	else
		elm_image_file_set(icon, VOLUME_ICON, NULL);
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

	vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_RINGTONE, ad->ringtone_val);

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

	if (ad->vol_ringtone_popup_del_timer) {
		ecore_timer_del(ad->vol_ringtone_popup_del_timer);
		ad->vol_ringtone_popup_del_timer = NULL;
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
		evas_object_smart_callback_add(popup, "block,clicked", __vcui_view_popup_response_cb_vol_ringtone, ad);

		ad->vol_ringtone_slider_eo = slider;
		evas_object_show(popup);
	}

}

static Eina_Bool __vcui_view_popup_timer_cb_vol_voice(void *data)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	__vcui_view_popup_response_cb_vol_voice(ad, ad->vol_voice_popup_eo, NULL);

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

	vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_VOICE, ad->voice_vol_val);

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

	if (ad->vol_voice_popup_del_timer) {
		ecore_timer_del(ad->vol_voice_popup_del_timer);
		ad->vol_voice_popup_del_timer = NULL;
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

	__vcui_view_popup_response_cb_vol_bt(ad, ad->vol_bt_popup_eo, NULL);

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

	vcall_engine_set_volume_level(VCALL_ENGINE_VOL_TYPE_HEADSET, ad->bt_vol_val);

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

	if (ad->vol_bt_popup_del_timer) {
		ecore_timer_del(ad->vol_bt_popup_del_timer);
		ad->vol_bt_popup_del_timer = NULL;
	}

	_vcui_cache_flush();
	__vcui_view_popup_win_del(ad);
	return;
}

void _vcui_view_popup_vol_bt(int vol_level)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	Evas_Object *popup = NULL;
	Evas_Object *slider = NULL;

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
		evas_object_smart_callback_add(popup, "block,clicked", __vcui_view_popup_response_cb_vol_bt, ad);

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
	CALL_UI_DEBUG("popup_progress_set_label ...");

	label = (Evas_Object *)evas_object_data_get(ad->popup_progress_eo, "progress_label");
	elm_object_text_set(label, status_string);
}

static void __vcui_view_popup_progressbar_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	CALL_UI_DEBUG("__vcui_view_popup_progressbar_response_cb callback=%d", (int)event_info);

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

	snprintf(msg, DEF_BUF_LEN_LONG, "%s\n%s", status_string, dtmf_num);

	_vcui_view_popup_load_progress(msg);
}

static void __vcui_view_popup_snd_path_bt_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_HEADSET);

	vcall_engine_change_sound_path(VCALL_ENGINE_AUDIO_HEADSET);

	ad->speaker_status = EINA_FALSE;
	ad->headset_status = EINA_TRUE;
}

static void __vcui_view_popup_snd_path_earjack_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_RECEIVER_EARJACK);

	vcall_engine_change_sound_path(VCALL_ENGINE_AUDIO_RECEIVER_EARJACK);

	ad->speaker_status = EINA_FALSE;
	ad->headset_status = EINA_FALSE;
}

static void __vcui_view_popup_snd_path_speaker_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	elm_radio_value_set(ad->ctxpopup_radio_group_eo, VCUI_SND_PATH_SPEAKER);

	vcall_engine_change_sound_path(VCALL_ENGINE_AUDIO_SPEAKER);

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

static void __vcui_view_popup_move_snd_path(Evas_Object *ctxpopup, Evas_Object *layout, Evas_Coord touch_x, Evas_Coord touch_y)
{
	evas_object_move(ctxpopup, touch_x, touch_y);
}

void _vcui_view_popup_load_snd_path(void *data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
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

	ctxpopup = elm_ctxpopup_add(vd->layout);

	evas_object_smart_callback_add(ctxpopup, "dismissed", __vcui_view_popup_hide_snd_path_cb, ad);

	__vcui_view_popup_move_snd_path(ctxpopup, vd->layout, ad->touch_x, ad->touch_y);

	/* bt headset */
	group = radio = elm_radio_add(ctxpopup);
	elm_radio_state_value_set(radio, VCUI_SND_PATH_HEADSET);
	evas_object_data_set(radio, "idx", (void *)(VCUI_SND_PATH_HEADSET));
	evas_object_show(radio);
	elm_ctxpopup_item_append(ctxpopup, bt_name, radio, __vcui_view_popup_snd_path_bt_cb, ad);

	/* earjack or receiver */
	radio = elm_radio_add(ctxpopup);
	elm_radio_state_value_set(radio, VCUI_SND_PATH_RECEIVER_EARJACK);
	elm_radio_group_add(radio, group);
	evas_object_data_set(radio, "idx", (void *)(VCUI_SND_PATH_RECEIVER_EARJACK));
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
	vcall_engine_get_sound_path(&snd_path);
	switch (snd_path) {
	case VCALL_ENGINE_AUDIO_SPEAKER:
		{
			elm_radio_value_set(group, VCUI_SND_PATH_SPEAKER);
		}
		break;
	case VCALL_ENGINE_AUDIO_HEADSET:
		{
			elm_radio_value_set(group, VCUI_SND_PATH_HEADSET);
		}
		break;
	case VCALL_ENGINE_AUDIO_RECEIVER_EARJACK:
		{
			elm_radio_value_set(group, VCUI_SND_PATH_RECEIVER_EARJACK);
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
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
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

	CALL_UI_DEBUG("load_ticker_noti_at_bottom enter");

	ticker_noti = elm_tickernoti_add(NULL);
	elm_object_style_set(ticker_noti, "info");
	elm_object_text_set(ticker_noti, ticker_msg);
	elm_tickernoti_orient_set(ticker_noti, ELM_TICKERNOTI_ORIENT_BOTTOM);
	elm_tickernoti_rotation_set(ticker_noti, ad->rotate_angle);
	if (ad->ticker_tm) {
		ecore_timer_del(ad->ticker_tm);
		ad->ticker_tm = NULL;
	}
	ad->ticker_tm = ecore_timer_add(TICKER_TIMEOUT, __vcui_view_ticker_response_cb, ad);
	evas_object_show(ticker_noti);

	CALL_UI_DEBUG("load_ticker_noti_at_bottom over");
}
#endif

static Eina_Bool __vcui_view_send_dtmf_idler_cb(void *data)
{
	CALL_UI_DEBUG("..");
	gboolean bsuccess = TRUE;

	if (strncmp(elm_object_text_get((Evas_Object *)data), "Send", 4) == 0) {
		bsuccess = TRUE;
	} else if (strncmp(elm_object_text_get((Evas_Object *)data), "Cancel", 6) == 0) {
		bsuccess = FALSE;
	}
	CALL_UI_DEBUG("bsuccess %d", bsuccess);
	vcall_engine_send_dtmf(bsuccess);

	return EINA_FALSE;
}

static void __vcui_view_dtmf_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	ecore_idler_add((Ecore_Task_Cb) __vcui_view_send_dtmf_idler_cb, obj);
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

void _vcui_view_load_send_dtmf_popup_with_buttons(char *status_string, char *dtmf_num)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	char popup_msg[DEF_BUF_LEN_LONG] = { 0, };
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	snprintf(popup_msg, DEF_BUF_LEN_LONG, "%s\n%s", status_string, dtmf_num);
	CALL_UI_DEBUG("msg:[%s]", popup_msg);

	_vcui_view_popup_unload_progress(ad);
	_vcui_view_popup_unload(ad->popup_eo);

	ad->popup_mw = __vcui_view_popup_create_win(ad, "base");
	ad->popup_eo = elm_popup_add(ad->popup_mw);
	evas_object_size_hint_weight_set(ad->popup_eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(ad->popup_eo, popup_msg);
	btn1 = elm_button_add(ad->popup_eo);
	elm_object_text_set(btn1, "Send");
	elm_object_part_content_set(ad->popup_eo, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", __vcui_view_dtmf_popup_response_cb, ad);
	btn2 = elm_button_add(ad->popup_eo);
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(ad->popup_eo, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", __vcui_view_dtmf_popup_response_cb, ad);
	evas_object_show(ad->popup_eo);
}

void _vcui_view_popup_second_mtcall_unload(void *data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	_second_mtcall_popup_data_t *p_popup_data = (_second_mtcall_popup_data_t *) data;
	int iter = 0;

	if (p_popup_data != NULL) {

		while (iter < 3) {
			if (p_popup_data->text_buffer[iter] != NULL) {
				CALL_UI_DEBUG("Free the string %d buffer", iter);
				free(p_popup_data->text_buffer[iter]);
				p_popup_data->text_buffer[iter] = NULL;
			}
			iter++;
		}

		if (p_popup_data->itc_option_list != NULL) {
			elm_genlist_item_class_free(p_popup_data->itc_option_list);
			p_popup_data->itc_option_list = NULL;
		}

		free(p_popup_data);
		p_popup_data = NULL;

		_vcui_view_set_second_mtcall_popup_data(NULL);
	}

	_vcui_view_popup_unload(ad->popup_eo);
}

static char *__second_mtcall_option_list_gl_label_get_option(void *data, Evas_Object *obj, const char *part)
{
	VCUI_RETURN_NULL_IF_FAIL(part != NULL);
	CALL_UI_DEBUG("..");
	_second_mtcall_popup_data_t *p_popup_data = (_second_mtcall_popup_data_t *) _vcui_view_get_second_mtcall_popup_data();
	char *list_text = NULL;

	list_text = p_popup_data->text_buffer[(int) data];
	CALL_UI_DEBUG("list_text: %s", list_text);

	if (strcmp(part, "elm.text") == 0) {
		CALL_UI_DEBUG("..");
		if (list_text != NULL) {
			return strdup(list_text);
		} else {
			return NULL;
		}
	}

	return NULL;
}

static void __second_mtcall_option_list_gl_sel_option(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	int index = -1;
	int unhold_call_count = 0;
	int hold_call_count = 0;

	if (item != NULL) {

		index = (int) elm_object_item_data_get(item);
		CALL_UI_DEBUG("index: %d", index);

		unhold_call_count = _vcui_doc_get_unhold_call_data_count();
		hold_call_count = _vcui_doc_get_hold_call_data_count();

		if ((unhold_call_count >= 1) && (hold_call_count == 0)) {
			CALL_UI_DEBUG("1 active call OR 1 active conference call");
			if (index == 0) {
				vcall_engine_answer_call_by_type(VCALL_ENGINE_ANSWER_HOLD_ACTIVE_AND_ACCEPT);
			} else if (index == 1) {
				vcall_engine_answer_call_by_type(VCALL_ENGINE_ANSWER_RELEASE_ACTIVE_AND_ACCEPT);
			} else {
				CALL_UI_DEBUG("Wrong index.. Should never get here");
			}
		} else if ((unhold_call_count == 1 && hold_call_count == 1) || (unhold_call_count > 1 && hold_call_count == 1) || \
				(unhold_call_count == 1 && hold_call_count > 1)) {
			CALL_UI_DEBUG("1 active call + 1 held call OR 1 active conf call + 1 held call OR 1 active call + 1 held conf call");
			if (index == 0) {
				vcall_engine_answer_call_by_type(VCALL_ENGINE_ANSWER_RELEASE_ACTIVE_AND_ACCEPT);
			} else if (index == 1) {
				vcall_engine_answer_call_by_type(VCALL_ENGINE_ANSWER_RELEASE_HOLD_AND_ACCEPT);
			} else if (index == 2) {
				vcall_engine_answer_call_by_type(VCALL_ENGINE_ANSWER_RELEASE_ALL_AND_ACCEPT);
			} else {
				CALL_UI_DEBUG("Wrong index.. Should never get here");
			}
		}
	}
}

static void __second_mtcall_cancel_btn_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");

	_vcui_view_popup_second_mtcall_unload(data);

	_vcui_view_change(VIEW_INCOMING_LOCK_VIEW, -1, NULL, NULL);

	return;
}

void _vcui_view_popup_second_mtcall_load(char *title_text, int unhold_call_count, int hold_call_count)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	_second_mtcall_popup_data_t *popup_data = NULL;
	Evas_Object *box = NULL;

	call_data_t *hold_call_data = NULL;
	call_data_t *unhold_call_data = NULL;
	char *hold_call_number = NULL;
	char *hold_call_name = NULL;
	char *unhold_call_number = NULL;
	char *unhold_call_name = NULL;
	char *temp_str = NULL;

	int iter = 0;

	hold_call_data = _vcui_doc_get_call_data_by_call_status(CALL_HOLD);
	unhold_call_data = _vcui_doc_get_call_data_by_call_status(CALL_UNHOLD);

	if (unhold_call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return;
	}

	_vcui_view_popup_second_mtcall_unload(NULL);

	ad->popup_eo = elm_popup_add(ad->win_main);
	evas_object_size_hint_weight_set(ad->popup_eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_style_set(ad->popup_eo, "min_menustyle");
	elm_object_part_text_set(ad->popup_eo, "title,text", title_text);

	popup_data = (_second_mtcall_popup_data_t *) calloc(1, sizeof(_second_mtcall_popup_data_t));
	if (popup_data == NULL) {
		CALL_UI_DEBUG("memory allocation failed for popup data.. return");
		return;
	}
	_vcui_view_set_second_mtcall_popup_data(popup_data);

	popup_data->itc_option_list = elm_genlist_item_class_new();

	popup_data->itc_option_list->item_style = "1text";
	popup_data->itc_option_list->func.text_get = __second_mtcall_option_list_gl_label_get_option;
	popup_data->itc_option_list->func.content_get = NULL;
	popup_data->itc_option_list->func.state_get = NULL;
	popup_data->itc_option_list->func.del = NULL;

	box = elm_box_add(ad->popup_eo);

	popup_data->glist = elm_genlist_add(box);
	evas_object_size_hint_weight_set(popup_data->glist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup_data->glist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	for (iter = 0; iter < 3; iter++) {
		popup_data->text_buffer[iter] = (char *) calloc(1, DEF_BUF_LEN);
		memset(popup_data->text_buffer[iter], 0x00, DEF_BUF_LEN);
	}

	CALL_UI_DEBUG("..");
	if (hold_call_data != NULL) {
		hold_call_number = _vcui_doc_get_call_number(hold_call_data);
		hold_call_name = _vcui_doc_get_call_display_name(hold_call_data);
		if (strlen(hold_call_name) == 0)
			hold_call_name = hold_call_number;
	}

	if (unhold_call_data != NULL) {
		unhold_call_number = _vcui_doc_get_call_number(unhold_call_data);
		unhold_call_name = _vcui_doc_get_call_display_name(unhold_call_data);
		if (strlen(unhold_call_name) == 0)
			unhold_call_name = unhold_call_number;
	}

	if ((unhold_call_count == 1) && (hold_call_count == 0)) {
		CALL_UI_DEBUG("1 active call");

		iter = 0;
		temp_str = _("IDS_CALL_BODY_PUTTING_PS_ON_HOLD");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, unhold_call_name);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

		iter++;
		temp_str = _("IDS_CALL_BODY_ENDING_CALL_WITH_PS");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, unhold_call_name);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

	} else if ((unhold_call_count > 1) && (hold_call_count == 0)) {
		CALL_UI_DEBUG("1 active conference call");

		iter = 0;
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, "%s (%d)", _("IDS_CALL_BODY_HOLD_ACTIVE_CALL_ABB"), unhold_call_count);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

		iter++;
		temp_str = _("IDS_CALL_BODY_END_ACTIVE_CALLS_HPD");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, unhold_call_count);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

	} else if ((unhold_call_count == 1) && (hold_call_count == 1)) {
		CALL_UI_DEBUG("1 active call + 1 held call");

		iter = 0;
		temp_str = _("IDS_CALL_BODY_ENDING_CALL_WITH_PS");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, unhold_call_name);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

		iter++;
		temp_str = _("IDS_CALL_BODY_ENDING_CALL_WITH_PS");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, hold_call_name);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

		iter++;
		strncpy(popup_data->text_buffer[iter], _("IDS_CALL_OPT_END_ALL_CALLS"), DEF_BUF_LEN - 1);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

	} else if ((unhold_call_count > 1) && (hold_call_count == 1)) {
		CALL_UI_DEBUG("1 active conf call + 1 held call");

		iter = 0;
		temp_str = _("IDS_CALL_BODY_END_ACTIVE_CALLS_HPD");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, unhold_call_count);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

		iter++;
		temp_str = _("IDS_CALL_BODY_ENDING_CALL_WITH_PS");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, hold_call_name);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

		iter++;
		strncpy(popup_data->text_buffer[iter], _("IDS_CALL_OPT_END_ALL_CALLS"), DEF_BUF_LEN - 1);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

	} else if ((unhold_call_count == 1) && (hold_call_count > 1)) {
		CALL_UI_DEBUG("1 active call + 1 held conf call");

		iter = 0;
		temp_str = _("IDS_CALL_BODY_ENDING_CALL_WITH_PS");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, unhold_call_name);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

		iter++;
		temp_str = _("IDS_CALL_BODY_END_HELD_CALLS_HPD");
		snprintf(popup_data->text_buffer[iter], DEF_BUF_LEN, temp_str, hold_call_count);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);

		iter++;
		strncpy(popup_data->text_buffer[iter], _("IDS_CALL_OPT_END_ALL_CALLS"), DEF_BUF_LEN - 1);
		elm_genlist_item_append(popup_data->glist, popup_data->itc_option_list, (void *)iter, NULL, ELM_GENLIST_ITEM_NONE, __second_mtcall_option_list_gl_sel_option, popup_data);
	}

	popup_data->btn = elm_button_add(ad->popup_eo);
	elm_object_text_set(popup_data->btn,  dgettext("sys_string", "IDS_COM_POP_CANCEL"));
	elm_object_part_content_set(ad->popup_eo, "button1", popup_data->btn);
	evas_object_smart_callback_add(popup_data->btn, "clicked", __second_mtcall_cancel_btn_response_cb, popup_data);

	evas_object_size_hint_min_set(box, POPUP_LIST_HD_W * ad->scale_factor, ((POPUP_LIST_ITEM_HD_H * (iter + 1))) * ad->scale_factor);
	evas_object_show(popup_data->glist);
	elm_box_pack_end(box, popup_data->glist);
	elm_object_content_set(ad->popup_eo, box);
	evas_object_show(ad->popup_eo);

}

void _vcui_view_set_second_mtcall_popup_data(void *p_popup_data)
{
	g_popup_data = (_second_mtcall_popup_data_t *) p_popup_data;
}

void *_vcui_view_get_second_mtcall_popup_data(void)
{
	return ((void *) g_popup_data);
}

static void __vcui_view_popup_hide_more_option(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->ctxpopup_eo) {
		evas_object_del(ad->ctxpopup_eo);
		ad->ctxpopup_eo = NULL;
	}
}

static void __vcui_view_popup_move_more_option(Evas_Object *ctxpopup, Evas_Object *layout, Evas_Coord touch_x, Evas_Coord touch_y)
{
	evas_object_move(ctxpopup, touch_x, touch_y);
}

static void __vcui_view_popup_manage_conf_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	if (ad->ctxpopup_eo) {
		evas_object_del(ad->ctxpopup_eo);
		ad->ctxpopup_eo = NULL;
	}

	_vcui_view_change(VIEW_INCALL_MULTICALL_LIST_VIEW, 0, 0, 0);
}

void _vcui_view_popup_load_more_option(void *data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	Evas_Object *ctxpopup = NULL;
	int hold_calls = _vcui_doc_get_hold_call_data_count();
	int active_calls = _vcui_doc_get_unhold_call_data_count();
	Evas_Object* icon = NULL;

	if (ad->ctxpopup_eo) {
		evas_object_del(ad->ctxpopup_eo);
		ad->ctxpopup_eo = NULL;
	}

	ctxpopup = elm_ctxpopup_add(vd->layout);
	elm_object_style_set(ctxpopup, "more_ctxpopup");

	evas_object_smart_callback_add(ctxpopup, "dismissed", __vcui_view_popup_hide_more_option, ad);

	if ((active_calls > 1)
		|| ((active_calls == 0) && (hold_calls > 1))) {
		CALL_UI_DEBUG("Manage conference (active:%d, hold:%d)", active_calls, hold_calls);
		icon = elm_image_add(ctxpopup);
		elm_image_file_set(icon, MORE_MANAGE_CONF_ICON, NULL);
		elm_ctxpopup_item_append(ctxpopup, _("IDS_CALL_HEADER_MANAGE_CONFERENCE_CALL"), icon, __vcui_view_popup_manage_conf_cb, ad);
	}

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_LEFT,
												ELM_CTXPOPUP_DIRECTION_RIGHT, ELM_CTXPOPUP_DIRECTION_UP);
	__vcui_view_popup_move_more_option(ctxpopup, vd->layout, MORE_MENU_CTXPOPUP_X, MORE_MENU_CTXPOPUP_Y);
	evas_object_show(ctxpopup);
	ad->ctxpopup_eo = ctxpopup;
}

