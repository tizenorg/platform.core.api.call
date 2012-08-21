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

#include <ui-gadget.h>
#include "voice-call-engine.h"
#include "vcui-doc-launch.h"

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

typedef enum {
	VIEW_DIALLING_VIEW = 0,
	VIEW_INCOMING_LOCK_VIEW,
	VIEW_INCALL_ONECALL_VIEW,
	VIEW_INCALL_MULTICALL_SPLIT_VIEW,
	VIEW_INCALL_MULTICALL_CONF_VIEW,
	VIEW_INCALL_MULTICALL_LIST_VIEW,
	VIEW_ENDCALL_VIEW,
	VIEW_QUICKPANEL_VIEW,
	VIEW_MAX
} vcui_app_call_view_id_t;

typedef enum _vcui_snd_path_type_t {
	VCUI_SND_PATH_NONE,
	VCUI_SND_PATH_HEADSET,
	VCUI_SND_PATH_RECEIVER_EARJACK,
	VCUI_SND_PATH_SPEAKER,
	VCUI_SND_PATH_MAX,
} vcui_snd_path_type_t;

typedef enum _vcui_vol_type_t {
	VCUI_VOL_RING,
	VCUI_VOL_VOICE,
	VCUI_VOL_HEADSET,
	VCUI_VOL_MAX
} vcui_vol_type_t;

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
	int wbamr_status;

	int show_flag;
	int ball_view_hide;

	struct _view_data *(*func_new[VIEW_MAX]) ();
	struct _view_data *view_st[VIEW_MAX];
	vcui_app_call_view_id_t view_top;
	vcui_app_call_view_id_t view_before_top;

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

	/* for keypad and UG*/
	int beffect_show;
	int badd_call_clicked;
	int bcontact_clicked;

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

	ui_gadget_h contact_ug;
	Evas_Object *ly;

	struct vcui_ugs_array ugs_array_data;

	int call_end_type;

	Evas_Object *win_quickpanel;
	Evas_Object *quickpanel_layout;
	char *quickpanel_text;

	Ecore_Event_Handler *downkey_handler;
	Ecore_Event_Handler *upkey_handler;
	Ecore_Event_Handler *mouse_evnt_handler;
	Ecore_Event_Handler *focus_in;
	Ecore_Event_Handler *focus_out;

	int rotate_angle;
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

typedef struct _vcui_call_mo_data_t {
	char call_number[VC_PHONE_NUMBER_LENGTH_MAX];
	int ct_index;
} vcui_call_mo_data_t;

typedef struct _vcui_call_mt_data_t {
	int call_handle;
	int call_type;
	int cli_presentation_indicator;
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];
	int calling_name_mode;
	char calling_name[VC_PHONE_NAME_LENGTH_MAX];
	char redirected_number[VC_PHONE_NUMBER_LENGTH_MAX];
	char redirected_sub_address[VC_PHONE_SUBADDRESS_LENGTH_MAX];
	int cli_cause;
	int bfwded;
	int active_line;
} vcui_call_mt_data_t;

typedef struct _vcui_call_ecc_data_t {
	char call_number[VC_PHONE_NUMBER_LENGTH_MAX];
} vcui_call_ecc_data_t;

typedef struct _vcui_call_sat_data_t {
	int command_id;								/**<Proactive Command Number sent by USIM*/
	int command_qualifier;						/**<call type*/
	char disp_text[500 + 1];					/**<character data*/
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];	/**<call number*/
	unsigned int duration;						/**<maximum repeat duration*/
} vcui_call_sat_data_t;

#endif
