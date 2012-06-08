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


#ifndef _VCUI_APP_DATA_H_
#define _VCUI_APP_DATA_H_

typedef enum {
	CALL_LOCK = 1,
	CALL_UNLOCK
} vcui_app_lock_type_t;

typedef enum {
	CALL_OUTGOING = 1,
	CALL_INCOMING
} vcui_app_call_dirction_t;

typedef enum {
	CALL_HOLD = 1,
	CALL_UNHOLD
} vcui_app_call_status_t;

#define TIME_END_START 1
#define TIME_END_NO      0
#define TIME_END_MAX_SHOW 8

#define MY_HANDLE 252
#define ALL_HANDLE 253
#define NO_HANDLE 254
#define NO_STATUS 254

#define WIN_HIDE 0
#define WIN_SHOW 1

typedef enum {
	VIEW_DIALLING_VIEW = 0,
	VIEW_INCOMING_VIEW,
	VIEW_INCOMING_LOCK_VIEW,
	VIEW_INCALL_ONECALL_VIEW,
	VIEW_INCALL_MULTICALL_SPLIT_VIEW,
	VIEW_INCALL_MULTICALL_CONF_VIEW,
	VIEW_INCALL_MULTICALL_LIST_VIEW,
	VIEW_INCALL_KEYPAD_VIEW,
	VIEW_ENDCALL_VIEW,
 	VIEW_MAX
} vcui_app_call_view_id_t;

typedef struct _appdata {
	Evas *evas;
	Evas_Object *win_main;

	int root_w;	/**<Width of a root window */
	int root_h;	/**<Height of a root window */

	Evas_Coord touch_x;
	Evas_Coord touch_y;

	double scale_factor;

	Evas_Object *actionslider;

	Ecore_X_Window win;	/* key grab */
	Ecore_X_Display *disp;	/* key grab */

	Evas_Object *bg;

	int full_image_type;
	Evas_Object *full_image_eo;

	int headset_status;
	int speaker_status;
	int mute_status;

	int brecord_voice;
	int show_flag;
	int ball_view_hide;

	struct _view_data *(*func_new[VIEW_MAX]) ();
	struct _view_data *view_st[VIEW_MAX];
	vcui_app_call_view_id_t view_top;
	vcui_app_call_view_id_t view_before_top;
	vcui_app_call_view_id_t view_before_reject_view;

	Ecore_Timer *popup_delay;

	Evas_Object *popup_mw;
	Evas_Object *popup_eo;
	Evas_Object *popup_vol_ly;
	Evas_Object *popup_progress_eo;
	Evas_Object *ctxpopup_eo;
	Evas_Object *ctxpopup_radio_group_eo;

	int child_is;

	/* for multi split view */
	int bswapped;
	int bholdisleft;

	/* for one-call view */
	int beffect_needed;

	/* Volume Control */
	int bmute_ringtone;
	int vol_key_status;
	int vol_longpress_cnt;

	int ringtone_val;
	int voice_vol_val;
	int bt_vol_val;

	Ecore_Timer *ringtone_longpress_mute_timer;
	Ecore_Timer *volup_key_longpress_timer;
	Ecore_Timer *voldown_key_longpress_timer;

	Evas_Object *vol_ringtone_slider_eo;
	Evas_Object *vol_ringtone_popup_eo;
	Ecore_Timer *vol_ringtone_popup_del_timer;

	Evas_Object *vol_voice_slider_eo;
	Evas_Object *vol_voice_popup_eo;
	Ecore_Timer *vol_voice_popup_del_timer;

	Evas_Object *vol_bt_slider_eo;
	Evas_Object *vol_bt_popup_eo;
	Ecore_Timer *vol_bt_popup_del_timer;

	struct ui_gadget *contact_ug;
	Evas_Object *ly;

	struct vcui_ugs_array ugs_array_data;

	int call_end_type;

	Ecore_Event_Handler *downkey_handler;
	Ecore_Event_Handler *upkey_handler;
	Ecore_Event_Handler *mouse_evnt_handler;

	Ecore_Timer *ticker_tm;
} vcui_app_call_data_t;

typedef struct _view_data {
	vcui_app_call_view_id_t type;
	vcui_app_call_data_t *app_data;

	int (*onCreate) (struct _view_data *view_data, int param1, void *param2, void *param3);
	int (*onUpdate) (struct _view_data *view_data, void *update_data1, void *update_data2);
	int (*onDestroy) (struct _view_data *view_data);
	int (*onShow) (struct _view_data *view_data);
	int (*onHide) (struct _view_data *view_data);

	Evas_Object *layout;
	void *priv;
} voice_call_view_data_t;

#endif
