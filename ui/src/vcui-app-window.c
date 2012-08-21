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
#include "vcui-view-quickpanel.h"

static Eina_Bool __vcui_app_win_focus_in_cb(void *data, int type, void *event);
static Eina_Bool __vcui_app_win_focus_out_cb(void *data, int type, void *event);
static void __vcui_app_win_main_win_del_cb(void *data, Evas_Object *obj, void *event);
static Eina_Bool __vcui_app_win_hard_key_down_cb(void *data, int type, void *event);
static Eina_Bool __vcui_app_win_hard_key_up_cb(void *data, int type, void *event);
static Eina_Bool __vcui_app_win_mouse_down_cb(void *data, int type, void *event);
/*static Eina_Bool __vcui_app_win_longpress_mute_timer_cb(void *data);*/
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
	}

	return eo;
}

static Eina_Bool __vcui_app_win_focus_in_cb(void *data, int type, void *event)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	Ecore_X_Event_Window_Focus_Out *ev = (Ecore_X_Event_Window_Focus_Out *)event;

	if (ad == NULL) {
		CALL_UI_DEBUG("ad == NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->win_main!= NULL && ev->win == elm_win_xwindow_get(ad->win_main)) {
		CALL_UI_DEBUG("Uninstall quick-panel view");
		_vcui_view_qp_uninstall_window(ad->view_st[VIEW_QUICKPANEL_VIEW]);
	}

	CALL_UI_DEBUG("__win_focus_in_cb leave");

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool __vcui_app_win_focus_out_cb(void *data, int type, void *event)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	Ecore_X_Event_Window_Focus_Out *ev = (Ecore_X_Event_Window_Focus_Out *)event;

	if (ad == NULL) {
		CALL_UI_DEBUG("ad == NULL");
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->win_main != NULL && ev->win == elm_win_xwindow_get(ad->win_main)) {
		CALL_UI_DEBUG("install quick-panel view");
		_vcui_view_qp_install_window(ad->view_st[VIEW_QUICKPANEL_VIEW]);
	}

	CALL_UI_DEBUG("__win_focus_out_cb leave");

	return ECORE_CALLBACK_PASS_ON;
}

static void __vcui_app_win_main_win_del_cb(void *data, Evas_Object *obj, void *event)
{
	CALL_UI_DEBUG("..");
	elm_exit();
}

static Eina_Bool __vcui_app_win_hard_key_down_cb(void *data, int type, void *event)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	Ecore_Event_Key *ev = event;
	gboolean banswering_enabled = EINA_FALSE;
	gboolean bpowerkey_enabled = EINA_FALSE;

	retvm_if(ev == NULL, 0, "ERROR!!! ========= Event is NULL!!!");

	if (ad->view_top == -1) {
		CALL_UI_DEBUG("ad->view_top is -1.");
#ifdef SEND_END_TO_UG
		if ((ad->contact_ug != NULL) && (!strcmp(ev->keyname, KEY_END))) {
			CALL_UI_DEBUG("send end key to contact ug.");
			ug_send_key_event(UG_KEY_EVENT_END);
		}
#endif
		return EINA_FALSE;
	}

	if (!strcmp(ev->keyname, KEY_POWER)) {
		bpowerkey_enabled = _vcui_is_powerkey_mode_on();
		CALL_UI_DEBUG("[KEY]KEY_POWER pressed, bpowerkey_enabled(%d)", bpowerkey_enabled);
		if (bpowerkey_enabled == EINA_TRUE) {
			CALL_UI_DEBUG("Lock the LCD state to ON...");
			vcall_engine_device_control_lcd_state(VC_LCD_ON_LOCK);

			if (ad->view_st[ad->view_top]->type == VIEW_DIALLING_VIEW) {
				if (ad->ball_view_hide) {
					CALL_UI_DEBUG("VCUI_RQST_REDIAL_STOP !!");
					vcall_engine_process_auto_redial(EINA_FALSE);
					elm_exit();
				} else {
					vcall_engine_cancel_call();
				}
			} else if (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW) {
				vcall_engine_reject_call();
			} else if ((ad->view_st[ad->view_top]->type >= VIEW_INCALL_ONECALL_VIEW)
				   && (ad->view_st[ad->view_top]->type <= VIEW_INCALL_MULTICALL_LIST_VIEW)) {
				vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_CALLS);
			} else {
				CALL_UI_DEBUG("nothing...");
			}
		}
	} else if (!strcmp(ev->keyname, KEY_VOLUMEUP)) {
		CALL_UI_DEBUG("[KEY]KEY_VOLUMEUP pressed");
		if (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW) {
			vcall_engine_mute_alert();
			ad->bmute_ringtone = EINA_TRUE;
		} else {
			ad->vol_longpress_cnt = 0;
			vcall_engine_device_control_lcd_state(VC_LCD_ON);
			{
				_vcui_set_volume(VAL_VOL_UP);
				ad->volup_key_longpress_timer = ecore_timer_add(VOLUME_KEY_LONG_PRESS_TIMEOUT, __vcui_app_win_volup_key_longpress_timer_cb, ad);
			}
		}
	} else if (!strcmp(ev->keyname, KEY_VOLUMEDOWN)) {
		CALL_UI_DEBUG("[KEY]KEY_VOLUMEDOWN pressed");
		if (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW) {
			vcall_engine_mute_alert();
			ad->bmute_ringtone = EINA_TRUE;
		} else {
			ad->vol_longpress_cnt = 0;
			vcall_engine_device_control_lcd_state(VC_LCD_ON);
			{
				_vcui_set_volume(VAL_VOL_DOWN);
				ad->voldown_key_longpress_timer = ecore_timer_add(VOLUME_KEY_LONG_PRESS_TIMEOUT, __vcui_app_win_voldown_key_longpress_timer_cb, ad);
			}
		}
	} else if (!strcmp(ev->keyname, KEY_SELECT)) {
		banswering_enabled = _vcui_is_answering_mode_on();
		CALL_UI_DEBUG("[KEY]KEY_SELECT pressed");
		if (banswering_enabled == EINA_TRUE) {
			if (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW) {
				CALL_UI_DEBUG("Answering mode on and Home key pressed on MT screen");

				if (_vcui_is_phonelock_status() == EINA_FALSE)
					vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_UNLOCK);
				vcall_engine_answer_call();
			}
		}
	}

	CALL_UI_DEBUG("End..");
	return EINA_FALSE;
}

static Eina_Bool __vcui_app_win_hard_key_up_cb(void *data, int type, void *event)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
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
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
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
		ad->upkey_handler = ecore_event_handler_add(ECORE_EVENT_KEY_UP, __vcui_app_win_hard_key_up_cb, ad);
	if (ad->mouse_evnt_handler == NULL)
		ad->mouse_evnt_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, __vcui_app_win_mouse_down_cb, ad);	/*for ctxpopup */
	if (ad->focus_in == NULL)
		ad->focus_in = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, __vcui_app_win_focus_in_cb, ad);
	if (ad->focus_out == NULL)
		ad->focus_out = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, __vcui_app_win_focus_out_cb, ad);

	CALL_UI_KPI("_vcui_app_win_key_grab done");
}

#if 0
static Eina_Bool __vcui_app_win_longpress_mute_timer_cb(void *data)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	call_data_t *call_data = _vcui_doc_get_recent_mt_call_data();

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

	vcall_engine_mute_alert();
	ad->bmute_ringtone = EINA_TRUE;

	return ECORE_CALLBACK_CANCEL;
}
#endif

static Eina_Bool __vcui_app_win_volup_key_longpress_timer_cb(void *data)
{
	CALL_UI_DEBUG("..");

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW) {
		if ((ad->ringtone_val == RINGTONE_MAX) || (ad->bmute_ringtone == EINA_TRUE)) {
			if (ad->volup_key_longpress_timer) {
				ecore_timer_del(ad->volup_key_longpress_timer);
				ad->volup_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	} else if (ad->headset_status == EINA_TRUE) {
		if (ad->bt_vol_val == BT_VOL_MAX) {
			if (ad->volup_key_longpress_timer) {
				ecore_timer_del(ad->volup_key_longpress_timer);
				ad->volup_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	} else {
		if (ad->voice_vol_val == VOICE_VOL_MAX) {
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

	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;

	if (ad->view_st[ad->view_top]->type == VIEW_INCOMING_LOCK_VIEW) {
		if ((ad->ringtone_val == RINGTONE_MAX) || (ad->bmute_ringtone == EINA_TRUE)) {
			if (ad->voldown_key_longpress_timer) {
				ecore_timer_del(ad->voldown_key_longpress_timer);
				ad->voldown_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	} else if (ad->headset_status == EINA_TRUE) {
		if (ad->bt_vol_val == BT_VOL_MIN) {
			if (ad->voldown_key_longpress_timer) {
				ecore_timer_del(ad->voldown_key_longpress_timer);
				ad->voldown_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	} else {
		if (ad->voice_vol_val == VOICE_VOL_MIN) {
			if (ad->voldown_key_longpress_timer) {
				ecore_timer_del(ad->voldown_key_longpress_timer);
				ad->voldown_key_longpress_timer = NULL;
			}
			return ECORE_CALLBACK_CANCEL;
		}
	}

	ad->vol_longpress_cnt++;

	if ((ad->vol_longpress_cnt % 3) == 0) {
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
