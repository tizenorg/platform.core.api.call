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


#ifndef __VCUI_MAIN_H_
#define __VCUI_MAIN_H_

#include <contacts-svc.h>
 
#include <dlog.h>

#include <glib.h>

#ifndef Eina_Bool
#include <stdbool.h>
#endif

#include <Evas.h>
#include <Edje.h>
#include <Eina.h>
#include <stdio.h>
#include <string.h>
#include <Elementary.h>
#include <Ecore_X.h>
#include <Ecore_X_Atoms.h>
#include <utilX.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include <aul.h>
#include "appcore-efl.h"

#include "vcui-doc-launch.h"
#include "voice-call-engine-msg.h"

#include "vcui-app-data.h"
#include "vcui-document.h"
#include "vcui-view-choice.h"
#include "vcui-engine-interface.h"
#include "vcui-view-common.h"
#include "vcui-view-elements.h"
#include "vcui-view-popup.h"

#include <vconf.h>

#include "ui-gadget.h"		/*for UG_INIT_EFL() & ug usage*/

#include "libintl.h"

#define DIALER_PKG		"org.tizen.phone"
#define CONTACTS_PKG	"org.tizen.contacts"

#if !defined(PACKAGE)
#  define PACKAGE "voice-call-ui"
#endif

#if !defined(LOCALEDIR)
#  define LOCALEDIR "/opt/apps/org.tizen.call/res/locale"
#endif

#if !defined(EDJDIR)
#  define EDJDIR "/opt/apps/org.tizen.call/res/edje"
#endif

#if !defined(IMGDIR)
#  define IMGDIR "/opt/apps/org.tizen.call/res/images"
#endif

#if !defined(MEDIADIR)
#  define MEDIADIR "/opt/apps/org.tizen.call/res/media"
#endif

#if !defined(ICONDIR)
#  define ICONDIR "/opt/apps/org.tizen.call/res/icons/default/small"
#endif

#define EDJ_NAME EDJDIR"/voice-call-ui.edj"
#define CALL_THEME EDJDIR"/call_theme.edj"

#define GRP_MTVIEW "mt-view"
#define GRP_MOVIEW "mo-view"
#define GRP_INCALL "incall"
#define GRP_MTLOCK "mtlock"
#define GRP_KEYPAD "keypad"
#define GRP_MULTICALL_SPLIT "multicall-split"
#define GRP_MULTICALL_SPLIT2 "multicall-split2"
#define GRP_MULTICALL_CONF "multicall-conf"
#define GRP_MULTICALL "multicall-list"
#define GRP_UG_EFFECT "ug_effect"
#define GRP_END_SINGLECALL "end-singlecall"
#define GRP_END_CONFCALL "end-confcall"
 
#define GRP_LOCK_ACCEPT "lock_accept"
#define GRP_LOCK_REJECT "lock_reject"

#define	QP_NOIMG_ICON	IMGDIR"/vc_qp_caller_ID.png"
#define	QP_CONF_ICON	IMGDIR"/vc_qp_caller_ID_group.png"
#define NOIMG_ICON IMGDIR"/vc_normal_caller_ID.png"
#define CONF_ICON IMGDIR"/vc_normal_caller_ID_group.png"
#define PRIVATE_ICON IMGDIR"/vc_conference_private.png"
#define HOLD_ICON IMGDIR"/vc_icon_hold.png"
#define UNHOLD_ICON IMGDIR"/vc_icon_unhold.png"
#define KEYPAD_ICON IMGDIR"/vc_keypad_icon.png"
#define CONTACT_ICON IMGDIR"/vc_contact_icon.png"
#define ADDCALL_ICON IMGDIR"/vc_add_icon.png"
#define JOIN_ICON IMGDIR"/vc_join_icon.png"
#define SPEAKER_ICON IMGDIR"/vc_speaker_icon.png"
#define MUTE_ICON IMGDIR"/vc_mute_icon.png"
#define PLAY_ICON IMGDIR"/vc_icon_play.png"
#define PAUSE_ICON IMGDIR"/vc_icon_pause.png"
#define MORE_ICON IMGDIR"/vc_btn_more.png"
#define CONF_CALL_END_ICON IMGDIR"/vc_icon_conf_call_end.png"
#define CALLING_NAME_BG_IMAGE IMGDIR"/vc_calling_name_BG_image.png"

#define HOLD_DISABLED_ICON IMGDIR"/vc_icon_hold_dim.png"
#define UNHOLD_DISABLED_ICON IMGDIR"/vc_icon_unhold_dim.png"
#define KEYPAD_DISABLED_ICON IMGDIR"/vc_keypad_icon_dim.png"
#define CONTACT_DISABLED_ICON IMGDIR"/vc_contact_icon_dim.png"
#define ADDCALL_DISABLED_ICON IMGDIR"/vc_add_icon_dim.png"
#define JOIN_DISABLED_ICON IMGDIR"/vc_join_icon_dim.png"
#define SPEAKER_DISABLED_ICON IMGDIR"/vc_speaker_icon_dim.png"
#define MUTE_DISABLED_ICON IMGDIR"/vc_mute_icon_dim.png"
#define	CONF_LIST_HOLD_ICON IMGDIR"/vc_icon_conf_list_hold.png"
#define	CONF_LIST_UNHOLD_ICON IMGDIR"/vc_icon_conf_list_unhold.png"

#define VOLUME_ICON IMGDIR"/vc_volume_icon.png"
#define VOLUME_MUTE_ICON IMGDIR"/vc_volume_mute_icon.png"

#define NVAI_CONTROL_OTHER_ICON IMGDIR"/vc_reject_with_msg_header_icon_others.png"

#define MINI_CONTROLLER_WIDTH (480)
#define MINI_CONTROLLER_HEIGHT (42)

#define DEF_BUF_LEN (128)
#define DEF_BUF_LEN_LONG (256)

#define VAL_VOL_UP (1)
#define VAL_VOL_DOWN (0)

#define RINGTONE_MIN (0)
#define RINGTONE_MAX (15)
#define VOICE_VOL_MIN (1)
#define VOICE_VOL_MAX (7)	/* It must change to 6 later.. */
#define BT_VOL_MIN (1)
#define BT_VOL_MAX (15)

#define RINGTONE_LONGPRESS_MUTE_TIMEOUT			0.7
#define VOLUME_KEY_LONG_PRESS_TIMEOUT			0.1

#define POPUP_TIMEOUT_SHORT		(2.0)
#define POPUP_TIMEOUT_NORMAL		(3.0)
#define POPUP_TIMEOUT_LONG		(5.0)
#define POPUP_TIMEOUT_VERY_LONG	(10.0)

#define TIMER_TIMEOUT_0_1_SEC	(0.1)
#define TIMER_TIMEOUT_0_3_SEC	(0.3)
#define TIMER_TIMEOUT_0_5_SEC	(0.5)
#define TIMER_TIMEOUT_1_SEC	(1.0)
#define TIMER_TIMEOUT_2_SEC	(2.0)
#define TIMER_TIMEOUT_4_SEC	(4.0)

#define BLUR_VALUE				30
#define IMG_TYPE_FULL			1
#define IMG_TYPE_WALLPAPER		2
#define IMG_TYPE_BLUR			3

#define BG_DEFAULT_PATH		"/opt/media/Images and videos/Wallpapers/Home_default.png"

#ifndef EINA_TRUE
#define EINA_TRUE	1
#endif

#ifndef EINA_FALSE
#define EINA_FALSE	0
#endif

#ifdef CALL_DEBUG_ON_DLOG
#define CALL_UI_DEBUG(frmt, args...)  do { LOG(LOG_DEBUG, TAG_CALL, "[vcui] [%s:%d] "frmt"\n",  __func__, __LINE__, ##args); } while (0)
#define	CALL_UI_KPI(frmt, args...)  do { LOG(LOG_DEBUG, TAG_CALL_LAUNCH, "[VC_KPI] [%s:%d] "frmt"\n",  __func__, __LINE__, ##args); } while (0)
#else
#define CALL_UI_DEBUG(args...)
#endif

#ifndef retv_if
#define retv_if(expr, val) do { \
		if (expr) { \
			CALL_UI_DEBUG("(%s) -> %s() return", #expr, __FUNCTION__); \
			return (val); \
		} \
	} while (0)
#endif

#ifndef retvm_if
#define retvm_if(expr, val, fmt, arg...) do { \
		if (expr) { \
			CALL_UI_DEBUG(fmt, ##arg); \
			CALL_UI_DEBUG("(%s) -> %s() return", #expr, __FUNCTION__); \
			return (val); \
		} \
	} while (0)
#endif

#ifndef VCUI_RETURN_IF_FAIL
#define VCUI_RETURN_IF_FAIL(check_condition)	\
			if (!(check_condition)) return;
#endif

#ifndef VCUI_RETURN_FALSE_IF_FAIL
#define VCUI_RETURN_FALSE_IF_FAIL(check_condition)	\
			if (!(check_condition)) return EINA_FALSE;
#endif

#ifndef VCUI_RETURN_VALUE_IF_FAIL
#define VCUI_RETURN_VALUE_IF_FAIL(check_condition, value)	\
			if (!(check_condition)) return value;
#endif

#ifndef VCUI_RETURN_NULL_IF_FAIL
#define VCUI_RETURN_NULL_IF_FAIL(check_condition)	\
			if (!(check_condition)) return NULL;
#endif

#ifndef VCUI_RETURN_ZERO_IF_FAIL
#define VCUI_RETURN_ZERO_IF_FAIL(check_condition)	\
			if (!(check_condition)) return 0;
#endif

#ifndef VCUI_RETURN_INVALID_IF_FAIL
#define VCUI_RETURN_INVALID_IF_FAIL(check_condition)	\
			if (!(check_condition)) return -1;
#endif

#ifndef VCUI_RETURN_VALUE_IF_NOT_IN_RANGE
#define VCUI_RETURN_VALUE_IF_NOT_IN_RANGE(value, min_value, max_value, ret_val)	\
			if ((value < min_value) || (value > max_value)) return ret_val;
#endif

#ifndef _EDJ
#define _EDJ(obj) elm_layout_edje_get(obj)
#endif

#define	_EVAS_OBJ_DEL(obj)	if (obj) \
	{ \
		evas_object_del(obj); \
		obj = NULL; \
	}

struct text_part {
	char *part;
	char *msgid;
};

typedef enum _voice_call_bg_type_t {
	BG_HIDE,
	BG_SHOW
} voice_call_bg_type_t;

typedef enum {
	CALL_END_TYPE_NONE = -1,
	CALL_END_TYPE_SINGLE_CALL,
	CALL_END_TYPE_CONF_CALL,
} voice_call_end_type_t;

/************************************************************************************/
void _create_main_ui_set_flag();
void _create_main_ui_real();
void _vcui_show_main_ui_set_flag();
void __vcui_hide_main_ui_set_flag();

void _vcui_determine_background_show_hide();

void _vcui_cache_flush();
vcui_app_call_data_t *_vcui_get_app_data();
int _vcui_is_idle_lock();

gboolean _vcui_is_gcf_mode(void);
gboolean _vcui_is_headset_conected(void);
gboolean _vcui_is_headset_switch_on(void);
gboolean _vcui_is_answering_mode_on(void);
gboolean _vcui_is_powerkey_mode_on(void);
gboolean _vcui_is_phonelock_status(void);

void _vcui_add_calllog(int type, call_data_t *data, int boutgoing_end);

void _vcui_response_volume(int vol_alert_type, int vol_level);
void _vcui_set_volume(int key_status);

void _vcui_raise_main_win();
int _vcui_check_valid_eo(Evas_Object *eo, char *v_name);

unsigned long _vcui_get_diff_now(time_t start_time);

char *_vcui_get_endcause_string(int end_cause, char *data);

Evas_Object *_vcui_load_edj(Evas_Object *parent, const char *file, const char *group);

#endif				/* __VCUI_MAIN_H_ */
