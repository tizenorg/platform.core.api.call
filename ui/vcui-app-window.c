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


#include "vcui-app-window.h"

static void __vcui_app_win_focus_in_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_app_win_focus_out_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_app_win_main_win_del_cb(void *data, Evas_Object *obj, void *event);
static Eina_Bool __vcui_app_win_hard_key_down_cb(void *data, int type, void *event);
static Eina_Bool __vcui_app_win_hard_key_up_cb(void *data, int type, void *event);
static Eina_Bool __vcui_app_win_mouse_down_cb(void *data, int type, void *event);
static Eina_Bool __vcui_app_win_longpress_mute_timer_cb(void *data);
static Eina_Bool __vcui_app_win_volup_key_longpress_timer_cb(void *data);
static Eina_Bool __vcui_app_win_voldown_key_longpress_timer_cb(void *data);

Evas_Object *_vcui_app_win_create_main(vcui_app_call_data_t *ad, const char *name)
{
	Evas_Object *eo;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", __vcui_app_win_main_win_del_cb, NULL);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &ad->root_w, &ad->root_h);
		evas_object_resize(eo, ad->root_w, ad->root_h);
		elm_win_indicator_mode_set(eo, ELM_WIN_INDICATOR_SHOW);
		evas_object_smart_callback_add(eo, "focus-in", __vcui_app_win_focus_in_cb, ad);
		evas_object_smart_callback_add(eo, "focus-out", __vcui_app_win_focus_out_cb, ad);
	}

	return eo;
}

static void __vcui_app_win_focus_in_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
}

static void __vcui_app_win_focus_out_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
}

static void __vcui_app_win_main_win_del_cb(void *data, Evas_Object *obj, void *event)
{
	CALL_UI_DEBUG("..");
	elm_exit();
}

static Eina_Bool __vcui_app_win_hard_key_down_cb(void *data, int type, void *event)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;
	Ecore_Event_Key *ev = event;

	retvm_if(ev == NULL, 0, "ERROR!!! ========= Event is NULL!!!");

	if (ad->view_top == -1) {
		CALL_UI_DEBUG("ad->view_top is -1.");
#ifndef END_KEY_PROCESSING
		if ((ad->contact_ug != NULL) && (!strcmp(ev->keyname, KEY_END))) {
			CALL_UI_DEBUG("send end key to contact ug.");
			ug_send_key_event(UG_KEY_EVENT_END);
		}
#endif
		return EINA_FALSE;
	}

	if (!strcmp(ev->keyname, KEY_VOLUMEUP)) {
		CALL_UI_DEBUG("[KEY]KEY_VOLUMEUP pressed");
		if ((ad->view_st[ad->view_top]->type == VIEW_INCOMING_VIEW) || (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW)) {
			_vcui_engine_interface_process_mute_alert();
			ad->bmute_ringtone = EINA_TRUE;
		} else {
			ad->vol_longpress_cnt = 0;
			if (_voicecall_dvc_get_proximity_sensor_state() != VCALL_SENSOR_NEAR)
				_voicecall_dvc_control_lcd_state(VC_LCD_ON);
			_vcui_set_volume(VAL_VOL_UP);
			ad->volup_key_longpress_timer = ecore_timer_add(VOLUME_KEY_LONG_PRESS_TIMEOUT, __vcui_app_win_volup_key_longpress_timer_cb, ad);
		}
	} else if (!strcmp(ev->keyname, KEY_VOLUMEDOWN)) {
		CALL_UI_DEBUG("[KEY]KEY_VOLUMEDOWN pressed");
		if ((ad->view_st[ad->view_top]->type == VIEW_INCOMING_VIEW) || (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW)) {
			_vcui_engine_interface_process_mute_alert();
			ad->bmute_ringtone = EINA_TRUE;
		} else {
			ad->vol_longpress_cnt = 0;
			if (_voicecall_dvc_get_proximity_sensor_state() != VCALL_SENSOR_NEAR)
				_voicecall_dvc_control_lcd_state(VC_LCD_ON);
			_vcui_set_volume(VAL_VOL_DOWN);
			ad->voldown_key_longpress_timer = ecore_timer_add(VOLUME_KEY_LONG_PRESS_TIMEOUT, __vcui_app_win_voldown_key_longpress_timer_cb, ad);
		}
	}

	CALL_UI_DEBUG("End..");
	return EINA_FALSE;
}

static Eina_Bool __vcui_app_win_hard_key_up_cb(void *data, int type, void *event)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;
	Ecore_Event_Key *ev = event;

	retvm_if(ev == NULL, 0, "ERROR!!! ========= Event is NULL!!!");

	if (0 == strcmp(ev->keyname, KEY_VOLUMEDOWN)) {
		if (ad->ringtone_longpress_mute_timer) {
			ecore_timer_del(ad->ringtone_longpress_mute_timer);
			ad->ringtone_longpress_mute_timer = NULL;
		}
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

	return EINA_FALSE;
}

static Eina_Bool __vcui_app_win_mouse_down_cb(void *data, int type, void *event)
{
	/*CALL_UI_DEBUG("..");*/

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;
	Ecore_Event_Mouse_Button *ev = event;

	ad->touch_x = ev->x;
	ad->touch_y = ev->y;
	return EINA_FALSE;
}

void _vcui_app_win_key_grab(vcui_app_call_data_t *ad)
{
	int result = 0;
	CALL_UI_KPI("_vcui_app_win_key_grab start");

	/* Key Grab */
	ad->disp = ecore_x_display_get();
	ad->win = elm_win_xwindow_get(ad->win_main);

	result = utilx_grab_key(ad->disp, ad->win, KEY_VOLUMEUP, EXCLUSIVE_GRAB);
	if (result)
		CALL_UI_DEBUG("KEY_VOLUMEUP key grab failed");

	result = utilx_grab_key(ad->disp, ad->win, KEY_VOLUMEDOWN, EXCLUSIVE_GRAB);
	if (result)
		CALL_UI_DEBUG("KEY_VOLUMEDOWN key grab failed");

	if (ad->downkey_handler == NULL)
		ad->downkey_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, __vcui_app_win_hard_key_down_cb, ad);
	if (ad->upkey_handler == NULL)
		ad->upkey_handler = ecore_event_handler_add(ECORE_EVENT_KEY_UP, __vcui_app_win_hard_key_up_cb, ad);	/* (for long press)*/
	if (ad->mouse_evnt_handler == NULL)
		ad->mouse_evnt_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, __vcui_app_win_mouse_down_cb, ad);	/*for ctxpopup*/

	CALL_UI_KPI("_vcui_app_win_key_grab done");
}

static Eina_Bool __vcui_app_win_longpress_mute_timer_cb(void *data)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;
	call_data_t *call_data = _vcui_doc_get_recent_mt();

	if (call_data == NULL)
		return ECORE_CALLBACK_CANCEL;

	if (ad->ringtone_longpress_mute_timer) {
		ecore_timer_del(ad->ringtone_longpress_mute_timer);
		ad->ringtone_longpress_mute_timer = NULL;
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

	_vcui_view_popup_unload(ad->vol_ringtone_popup_eo);

	_vcui_engine_interface_process_mute_alert();
	ad->bmute_ringtone = EINA_TRUE;

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __vcui_app_win_volup_key_longpress_timer_cb(void *data)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;

	if ((ad->view_st[ad->view_top]->type == VIEW_INCOMING_VIEW || ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW)) {
		if((ad->ringtone_val == RINGTONE_MAX) || (ad->bmute_ringtone == EINA_TRUE)){
			if (ad->volup_key_longpress_timer) {
				ecore_timer_del(ad->volup_key_longpress_timer);
				ad->volup_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	} else if (ad->headset_status == EINA_TRUE) {
		if(ad->bt_vol_val== BT_VOL_MAX) {
			if (ad->volup_key_longpress_timer) {
				ecore_timer_del(ad->volup_key_longpress_timer);
				ad->volup_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	} else {
		if(ad->voice_vol_val == VOICE_VOL_MAX) {
			if (ad->volup_key_longpress_timer) {
				ecore_timer_del(ad->volup_key_longpress_timer);
				ad->volup_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	}

	ad->vol_longpress_cnt++;

	if ((ad->vol_longpress_cnt % 3) == 0) {
		_vcui_set_volume(VAL_VOL_UP);
	}

	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool __vcui_app_win_voldown_key_longpress_timer_cb(void *data)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *) data;

	if ((ad->view_st[ad->view_top]->type == VIEW_INCOMING_VIEW || ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW)) {
		if((ad->ringtone_val == RINGTONE_MAX) || (ad->bmute_ringtone == EINA_TRUE)) {
			if (ad->voldown_key_longpress_timer) {
				ecore_timer_del(ad->voldown_key_longpress_timer);
				ad->voldown_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	} else if (ad->headset_status == EINA_TRUE) {
		if(ad->bt_vol_val == BT_VOL_MIN) {
			if (ad->voldown_key_longpress_timer) {
				ecore_timer_del(ad->voldown_key_longpress_timer);
				ad->voldown_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	} else {
		if(ad->voice_vol_val == VOICE_VOL_MIN) {
			if (ad->voldown_key_longpress_timer) {
				ecore_timer_del(ad->voldown_key_longpress_timer);
				ad->voldown_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	}

	ad->vol_longpress_cnt++;

	if((ad->vol_longpress_cnt % 3) == 0) {
		_vcui_set_volume(VAL_VOL_DOWN);
	}

	return ECORE_CALLBACK_RENEW;
}

void _vcui_app_win_set_noti_type(int bwin_noti)
{
	Ecore_X_Window xwin;
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	/* Get x-window */
	xwin = elm_win_xwindow_get(ad->win_main);

	if (bwin_noti == EINA_FALSE) {
		CALL_UI_DEBUG("window type: NORMAL");
		/* Set Normal window */
		ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NORMAL);
	} else {
		CALL_UI_DEBUG("window type: NOTI-HIGH");
		/* Set Notification window */
		ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);
		/* Set Notification's priority to LEVEL_HIGH */
		utilx_set_system_notification_level(ecore_x_display_get(), xwin, UTILX_NOTIFICATION_LEVEL_HIGH);
	}
	return;
}

