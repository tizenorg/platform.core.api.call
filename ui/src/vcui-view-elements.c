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
#include "vcui-view-elements.h"
#include "vcui-view-dialing.h"
#include "vcui-view-single-call.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-keypad.h"
#include "vcui-view-incoming-lock.h"
#include "vcui-view-callend.h"
#include "vcui-view-quickpanel.h"
#include "vcui-view-popup.h"

static void __vcui_hold_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_unhold_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_join_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_keypad_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_contacts_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_addcall_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_sound_path_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_spk_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_spk_press_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_mute_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_bigend_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_videocall_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_voicecall_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_msg_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_hold_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcall_engine_process_hold_call();
}

static void __vcui_unhold_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	vcall_engine_process_hold_call();
}

void _vcui_swap_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	ad->bholdisleft = ad->bswapped;
	_vcui_view_popup_load(_("IDS_CALL_POP_SWAPPED"), POPUP_TIMEOUT_LONG, EINA_FALSE);
	vcall_engine_process_hold_call();
}

static void __vcui_join_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_create_top_left_button_disabled(data);
	vcall_engine_join_call();
}

void _vcui_conf_img_cb(void *data, Evas_Object *obj, void *event_info)
{				/* for multicall list view */
	CALL_UI_DEBUG("..");
	_vcui_view_change(VIEW_INCALL_MULTICALL_LIST_VIEW, 0, 0, 0);
}

static void __vcui_keypad_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *view_ly = NULL;

	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			view_ly = _vc_ui_view_dialing_get_layout(vd);
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			view_ly = _vc_ui_view_single_call_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			view_ly = _vcui_view_multi_call_split_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			view_ly = _vcui_view_multi_call_conf_get_main_layout(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("code should never reach here");
		return;
	}

	if (_vcui_keypad_get_show_status() == EINA_FALSE) {	/*show keypad region*/
		CALL_UI_DEBUG("..");

		/*Hide caller info area*/
		edje_object_signal_emit(_EDJ(view_ly), "HIDE", "CALL_AREA");
		edje_object_signal_emit(_EDJ(view_ly), "HIDE_NOISE_REDUCTIION", "NOISE_REDUCTIION");
		edje_object_signal_emit(_EDJ(view_ly), "HIDE_SOUND_EQ", "SOUND_EQ");

		/*Show keypad layout - ON view*/
		edje_object_signal_emit(_EDJ(view_ly), "SHOW", "KEYPAD_BTN");

		/*Actual show with animation*/
		_vcui_keypad_show_layout(vd);

		elm_object_text_set(obj, dgettext("sys_string", "IDS_COM_SK_HIDE"));

		_vcui_keypad_set_show_status(EINA_TRUE);
	} else {
		CALL_UI_DEBUG("..");

		/*Hide animation on keypad*/
		_vcui_keypad_show_hide_effect(vd, view_ly);

		/*Set keypad text*/
		elm_object_text_set(obj, _("IDS_CALL_SK3_KEYPAD"));

		/*Set keypad status flag*/
		_vcui_keypad_set_show_status(EINA_FALSE);
	}
}

static void __vcui_contacts_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	if (ad->beffect_show == EINA_TRUE) {
		ad->bcontact_clicked = EINA_TRUE;
	} else {
		_vcui_doc_launch_contact_list_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data));
	}
}

static void __vcui_addcall_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	if (ad->beffect_show == EINA_TRUE) {
		ad->badd_call_clicked = EINA_TRUE;
	} else {
		_vcui_doc_launch_phoneui_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data));
	}
}

static void __vcui_sound_path_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_view_popup_load_snd_path(data);
}

/* #define RCS_TEST */

static void __vcui_spk_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	vcall_engine_process_loudspeaker(EINA_TRUE);

	ad->speaker_status = EINA_TRUE;
	_vcui_create_bottom_left_button(vd);

}

static void __vcui_spk_press_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	vcall_engine_process_loudspeaker(EINA_FALSE);

	ad->speaker_status = EINA_FALSE;
	_vcui_create_bottom_left_button(vd);

}

static void __vcui_mute_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	vcall_engine_process_voice_mute(EINA_TRUE);

	ad->mute_status = EINA_TRUE;
	if (vd->type == VIEW_QUICKPANEL_VIEW) {
		_vcui_view_common_update_mute_btn();
	} else {
		_vcui_create_bottom_middle_button(vd);
	}
	_vcui_create_quickpanel_mute_button(vd);

}

static void __vcui_mute_press_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	vcall_engine_process_voice_mute(EINA_FALSE);

	ad->mute_status = EINA_FALSE;

	if (vd->type == VIEW_QUICKPANEL_VIEW) {
		_vcui_view_common_update_mute_btn();
	} else {
		_vcui_create_bottom_middle_button(vd);
	}
	_vcui_create_quickpanel_mute_button(vd);

}

static void __vcui_bigend_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			vcall_engine_cancel_call();
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			vcall_engine_release_call();
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_ACTIVE_CALLS);
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			int call_status = -1;
			call_status = _vcui_view_multi_call_conf_get_call_status(vd);
			if (call_status == CALL_HOLD)
				vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_HELD_CALLS);
			else if (call_status == CALL_UNHOLD)
				vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_ACTIVE_CALLS);
			ad->call_end_type = CALL_END_TYPE_CONF_CALL;
		}
		break;
	case VIEW_INCALL_MULTICALL_LIST_VIEW:
		{
			int call_status = -1;
			call_status = _vcui_view_multi_call_list_get_call_status(vd);
			if (call_status == CALL_HOLD)
				vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_HELD_CALLS);
			else if (call_status == CALL_UNHOLD)
				vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_ACTIVE_CALLS);
			int grp_count = 0;
			vcall_engine_get_group_count(&grp_count);
			CALL_UI_DEBUG("No. of groups - %d", grp_count);
			if (grp_count == 1)
				ad->call_end_type = CALL_END_TYPE_CONF_CALL;
			else
				ad->call_end_type = CALL_END_TYPE_NONE;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return;
		break;

	}
}

Evas_Object *__vcui_create_button_style(void *data, Evas_Object **p_button, char *part_name)
{
	Evas_Object *layout;
	Evas_Object *sw;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			layout = _vc_ui_view_dialing_get_button_layout(vd);
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			layout = _vc_ui_view_single_call_get_button_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			layout = _vcui_view_multi_call_split_get_button_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			layout = _vcui_view_multi_call_conf_get_button_layout(vd);
		}
		break;
	case VIEW_ENDCALL_VIEW:
		{
			layout = _vc_ui_view_callend_get_button_layout(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), part_name);
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	*p_button = elm_button_add(layout);
	elm_object_part_content_set(layout, part_name, *p_button);

	return layout;
}

/* Add-Call/Join button ENABLED*/
Evas_Object *_vcui_create_top_left_button(void *data)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	Evas_Object *btn;
	Evas_Object *layout;
	Eina_Bool bjoin = EINA_FALSE;

	if (vd->type == VIEW_INCALL_MULTICALL_SPLIT_VIEW)
		bjoin = EINA_TRUE;

	layout = __vcui_create_button_style(data, &btn, "top_left_button");
	{
		if (bjoin) {
			CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_join");
			elm_object_style_set(btn, "style_call_sixbtn_join");
			CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_join");
			elm_object_text_set(btn, _("IDS_CALL_BUTTON_JOIN"));
			evas_object_smart_callback_add(btn, "clicked", __vcui_join_btn_cb, vd);
		} else {
			CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_add");
			elm_object_style_set(btn, "style_call_sixbtn_add");
			CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_add");
			elm_object_text_set(btn, _("IDS_CALL_BUTTON_ADD_CALL"));
			evas_object_smart_callback_add(btn, "clicked", __vcui_addcall_btn_cb, data);
		}
	}
	return btn;
}

/* Add-Call/Join Button DISABLED */
Evas_Object *_vcui_create_top_left_button_disabled(void *data)
{
	CALL_UI_KPI("_vcui_create_top_left_button_disabled start");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *btn;
	Evas_Object *layout;
	Eina_Bool bjoin = EINA_FALSE;

	if (vd->type == VIEW_INCALL_MULTICALL_SPLIT_VIEW)
		bjoin = EINA_TRUE;

	layout = __vcui_create_button_style(data, &btn, "top_left_button");
	{
		if (bjoin) {
			CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_disabled_join");
			elm_object_style_set(btn, "style_call_sixbtn_disabled_join");
			CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_disabled_join");
			elm_object_text_set(btn, _("IDS_CALL_BUTTON_JOIN"));
		} else {
			CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_disabled_add");
			elm_object_style_set(btn, "style_call_sixbtn_disabled_add");
			CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_disabled_add");
			elm_object_text_set(btn, _("IDS_CALL_BUTTON_ADD_CALL"));
		}
	}

	CALL_UI_KPI("_vcui_create_top_left_button_disabled done");

	return btn;
}

/* Keypad Button ENABLED */
Evas_Object *_vcui_create_top_middle_button(void *data)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *view_ly = NULL;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	layout = __vcui_create_button_style(data, &btn, "top_middle_button");
	CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_keypad");
	elm_object_style_set(btn, "style_call_sixbtn_keypad");
	CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_keypad");

	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			view_ly = _vc_ui_view_dialing_get_layout(vd);
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			view_ly = _vc_ui_view_single_call_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			view_ly = _vcui_view_multi_call_split_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			view_ly = _vcui_view_multi_call_conf_get_main_layout(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;
	}

	/*Create keypad layout*/
	_vcui_keypad_create_layout(vd, view_ly);
	if (_vcui_keypad_get_show_status() == EINA_FALSE) {
		elm_object_text_set(btn, _("IDS_CALL_SK3_KEYPAD"));
	} else {
		elm_object_text_set(btn, dgettext("sys_string", "IDS_COM_SK_HIDE"));
	}
	evas_object_smart_callback_add(btn, "clicked", __vcui_keypad_btn_cb, data);

	return btn;
}

/* Keypad Button DISABLED */
Evas_Object *_vcui_create_top_middle_button_disabled(void *data)
{
	CALL_UI_KPI("_vcui_create_top_middle_button_disabled start");
	Evas_Object *btn;
	Evas_Object *layout;

	layout = __vcui_create_button_style(data, &btn, "top_middle_button");
	CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_disabled_keypad");
	elm_object_style_set(btn, "style_call_sixbtn_disabled_keypad");
	CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_disabled_keypad");
	elm_object_text_set(btn, _("IDS_CALL_SK3_KEYPAD"));
	CALL_UI_KPI("_vcui_create_top_middle_button_disabled done");

	return btn;
}

/* End Call Button ENABLED */
Evas_Object *_vcui_create_top_right_button(void *data)
{
	Evas_Object *btn;
	Evas_Object *layout;

	layout = __vcui_create_button_style(data, &btn, "top_right_button");
	elm_object_style_set(btn, "style_call_sixbtn_end");
	elm_object_text_set(btn, _("IDS_CALL_SK3_END_CALL"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_bigend_btn_cb, data);

	return btn;
}

/* End Call Button DISABLED */
Evas_Object *_vcui_create_top_right_button_disabled(void *data)
{
	Evas_Object *btn;
	Evas_Object *layout;

	layout = __vcui_create_button_style(data, &btn, "top_right_button");
	elm_object_style_set(btn, "style_call_sixbtn_disabled_end");
	elm_object_text_set(btn, _("IDS_CALL_SK3_END_CALL"));

	return btn;
}

/* Speaker Button ENABLED */
Evas_Object *_vcui_create_bottom_left_button(void *data)
{
	CALL_UI_KPI("_vcui_create_bottom_left_button start");
	Evas_Object *btn;
	Evas_Object *layout;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	layout = __vcui_create_button_style(data, &btn, "bottom_left_button");
	if (EINA_TRUE == _vcui_is_headset_conected()) {
		elm_object_style_set(btn, "style_call_sixbtn_speaker");
		elm_object_text_set(btn, _("IDS_CALL_BUTTON_SOUND_ABB"));
		evas_object_smart_callback_add(btn, "clicked", __vcui_sound_path_btn_cb, vd);
	} else {
		elm_object_text_set(btn, _("IDS_CALL_BUTTON_SPEAKER"));
		if (ad->speaker_status == EINA_FALSE) {
			elm_object_style_set(btn, "style_call_sixbtn_speaker");
			evas_object_smart_callback_add(btn, "clicked", __vcui_spk_btn_cb, vd);
		} else {
			elm_object_style_set(btn, "style_call_sixbtn_speaker_on");
			evas_object_smart_callback_add(btn, "clicked", __vcui_spk_press_btn_cb, vd);
		}
	}
	CALL_UI_KPI("_vcui_create_bottom_left_button done");

	return btn;
}

/* Speaker Button DISABLED */
Evas_Object *_vcui_create_bottom_left_button_disabled(void *data)
{
	CALL_UI_KPI("_vcui_create_bottom_left_button_disabled start");
	Evas_Object *btn;
	Evas_Object *layout;

	layout = __vcui_create_button_style(data, &btn, "bottom_left_button");
	CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_disabled_speaker");
	elm_object_style_set(btn, "style_call_sixbtn_disabled_speaker");
	CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_disabled_speaker");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_SPEAKER"));
	CALL_UI_KPI("_vcui_create_bottom_left_button_disabled done");

	return btn;
}

/* Mute Button ENABLED */
Evas_Object *_vcui_create_bottom_middle_button(void *data)
{
	Evas_Object *btn;
	Evas_Object *layout;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	layout = __vcui_create_button_style(data, &btn, "bottom_middle_button");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_MUTE_ABB"));
	if (ad->mute_status == EINA_FALSE) {
		elm_object_style_set(btn, "style_call_sixbtn_mute");
		evas_object_smart_callback_add(btn, "clicked", __vcui_mute_btn_cb, vd);
	} else {
		elm_object_style_set(btn, "style_call_sixbtn_mute_on");
		evas_object_smart_callback_add(btn, "clicked", __vcui_mute_press_btn_cb, vd);
	}

	return btn;
}

/* Mute Button DISABLED */
Evas_Object *_vcui_create_bottom_middle_button_disabled(void *data)
{
	CALL_UI_KPI("_vcui_create_bottom_middle_button_disabled start");
	Evas_Object *btn;
	Evas_Object *layout;

	layout = __vcui_create_button_style(data, &btn, "bottom_middle_button");
	CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_disabled_mute");
	elm_object_style_set(btn, "style_call_sixbtn_disabled_mute");
	CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_disabled_mute");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_MUTE_ABB"));
	CALL_UI_KPI("_vcui_create_bottom_middle_button_disabled done");

	return btn;
}

/* Share/Contacts Button ENABLED */
Evas_Object *_vcui_create_bottom_right_button(void *data)
{
	Evas_Object *btn;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	__vcui_create_button_style(data, &btn, "bottom_right_button");

	{
		CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_contacts");
		elm_object_style_set(btn, "style_call_sixbtn_contacts");
		CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_contacts");
		elm_object_text_set(btn, _("IDS_CALL_BUTTON_CONTACTS"));
		evas_object_smart_callback_add(btn, "clicked", __vcui_contacts_btn_cb, vd);
	}

	return btn;
}

/* Share/Contacts Button DISABLED */
Evas_Object *_vcui_create_bottom_right_button_disabled(void *data)
{
	CALL_UI_KPI("_vcui_create_bottom_right_button_disabled start");
	Evas_Object *btn;

	__vcui_create_button_style(data, &btn, "bottom_right_button");
	CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_disabled");
	CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_disabled");

	{
		CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_disabled_contacts");
		elm_object_style_set(btn, "style_call_sixbtn_disabled_contacts");
		CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_disabled_contacts");
		elm_object_text_set(btn, _("IDS_CALL_BUTTON_CONTACTS"));
	}

	CALL_UI_KPI("_vcui_create_bottom_right_button_disabled done");

	return btn;
}

Evas_Object *_vcui_create_hold_swap_button(void *data)
{
	Evas_Object *btn;
	Evas_Object *ic;
	Evas_Object *layout;
	Evas_Object *sw;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	int call_status = 0;
	Eina_Bool bswap_button = EINA_FALSE;
	Eina_Bool bhold_button = EINA_FALSE;

	switch (vd->type) {
	case VIEW_INCALL_ONECALL_VIEW:
		{
			layout = _vc_ui_view_single_call_get_layout(vd);
			call_status = _vc_ui_view_single_call_get_call_status(vd);
			bhold_button = EINA_TRUE;
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			layout = _vcui_view_multi_call_conf_get_main_layout(vd);
			call_status = _vcui_view_multi_call_conf_get_call_status(vd);
			bhold_button = EINA_TRUE;
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			layout = _vcui_view_multi_call_split_get_layout(vd);
			bswap_button = EINA_TRUE;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;
	}
	CALL_UI_DEBUG("vd type:[%d], call_status:[%d]", vd->type, call_status);

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_hold_swap");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_hold_swap", btn);
	ic = elm_image_add(layout);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_resizable_set(ic, 1, 1);
	elm_object_content_set(btn, ic);

	if (bhold_button) {
		elm_object_style_set(btn, "style_normal_holdbtn");
		if (call_status == CALL_UNHOLD) {
			elm_image_file_set(ic, HOLD_ICON, NULL);
			edje_object_part_text_set(_EDJ(layout), "txt_hold_swap", _("IDS_CALL_BUTTON_HOLD"));
			evas_object_smart_callback_add(btn, "clicked", __vcui_hold_btn_cb, vd);
		} else {		/* CALL_HOLD */
			elm_image_file_set(ic, UNHOLD_ICON, NULL);
			edje_object_part_text_set(_EDJ(layout), "txt_hold_swap", _("IDS_CALL_BUTTON_UNHOLD"));
			evas_object_smart_callback_add(btn, "clicked", __vcui_unhold_btn_cb, vd);
		}
	} else if (bswap_button) {
		elm_object_style_set(btn, "style_normal_swapbtn");
		elm_image_file_set(ic, SWAP_ICON, NULL);
		edje_object_part_text_set(_EDJ(layout), "txt_hold_swap", _("Swap"));
		evas_object_smart_callback_add(btn, "clicked", _vcui_swap_btn_cb, vd);
	}

	return layout;
}

Evas_Object *_vcui_create_button_bigend(void *data)
{
	CALL_UI_KPI("_vcui_create_button_bigend start");
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			layout = _vc_ui_view_dialing_get_layout(vd);
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			layout = _vc_ui_view_single_call_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			layout = _vcui_view_multi_call_split_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			layout = _vcui_view_multi_call_conf_get_main_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_LIST_VIEW:
		{
			layout = _vcui_view_multi_call_list_get_main_layout(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_bigend");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_bigend", btn);
	CALL_UI_KPI("elm_object_style_set start :: style_call_text_only_red");
	elm_object_style_set(btn, "style_call_text_only_red");
	CALL_UI_KPI("elm_object_style_set done :: style_call_text_only_red");
	elm_object_text_set(btn, _("IDS_CALL_OPT_END_ALL_CALLS"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_bigend_btn_cb, vd);
	CALL_UI_KPI("_vcui_create_button_bigend done");
	return layout;
}

Evas_Object *_vcui_create_button_bigend_disabled(void *data)
{
	CALL_UI_KPI("_vcui_create_button_bigend_disabled start");
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			layout = _vc_ui_view_dialing_get_layout(vd);
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			layout = _vc_ui_view_single_call_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			layout = _vcui_view_multi_call_split_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			layout = _vcui_view_multi_call_conf_get_main_layout(vd);
		}
		break;
	case VIEW_ENDCALL_VIEW:
		{
			layout = _vc_ui_view_callend_get_layout(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_bigend");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_bigend", btn);
	CALL_UI_KPI("elm_object_style_set start :: style_call_text_only_red_disabled");
	elm_object_style_set(btn, "style_call_text_only_red_disabled");
	CALL_UI_KPI("elm_object_style_set done :: style_call_text_only_red_disabled");
	elm_object_text_set(btn, _("IDS_CALL_SK3_END_CALL"));
	CALL_UI_KPI("_vcui_create_button_bigend_disabled done");
	return layout;
}

Evas_Object *_vcui_create_conf_list_button_hold(void *data)
{
	Evas_Object *btn;
	Evas_Object *ic;
	Evas_Object *layout;
	Evas_Object *sw;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	int call_status = 0;
	switch (vd->type) {
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			layout = _vcui_view_multi_call_conf_get_main_layout(vd);
			call_status = _vcui_doc_get_group_call_status();
		}
		break;
	case VIEW_INCALL_MULTICALL_LIST_VIEW:
		{
			layout = _vcui_view_multi_call_list_get_main_layout(vd);
			call_status = _vcui_doc_get_group_call_status();
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}
	CALL_UI_DEBUG("vd type:[%d], call_status:[%d]", vd->type, call_status);

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_hold");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	CALL_UI_KPI("elm_object_style_set start :: style_list_holdbtn");
	elm_object_style_set(btn, "style_list_holdbtn");
	CALL_UI_KPI("elm_object_style_set done :: style_list_holdbtn");
	elm_object_part_content_set(layout, "btn_hold", btn);
	ic = elm_image_add(layout);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_resizable_set(ic, 1, 1);
	elm_object_content_set(btn, ic);

	if (call_status == CALL_UNHOLD) {
		elm_image_file_set(ic, HOLD_ICON, NULL);
		elm_object_text_set(btn, _("IDS_CALL_BUTTON_HOLD"));

		evas_object_smart_callback_add(btn, "clicked", __vcui_hold_btn_cb, vd);
	} else {		/* CALL_HOLD */
		elm_image_file_set(ic, UNHOLD_ICON, NULL);
		elm_object_text_set(btn, _("IDS_CALL_BUTTON_UNHOLD"));

		evas_object_smart_callback_add(btn, "clicked", __vcui_unhold_btn_cb, vd);
	}

	return layout;
}

Evas_Object *_vcui_show_wallpaper_image(Evas_Object *contents)
{
	Evas_Object *d_image = NULL;
	return d_image;
}

void _vcui_delete_contact_image(Evas_Object *contents)
{
	Evas_Object *sw;

	sw = edje_object_part_swallow_get(_EDJ(contents), "swl_cid");
	if (sw) {
		edje_object_part_unswallow(_EDJ(contents), sw);
		evas_object_del(sw);
	}
}

Evas_Object *_vcui_show_contact_image(Evas_Object *contents, Evas_Object *win_main, char *path)
{
	Evas_Object *sw;
	Evas_Object *ic;

	sw = edje_object_part_swallow_get(_EDJ(contents), "swl_cid");
	if (sw) {
		edje_object_part_unswallow(_EDJ(contents), sw);
		evas_object_del(sw);
	}

	ic = elm_image_add(win_main);
	elm_object_part_content_set(contents, "swl_cid", ic);
	elm_image_file_set(ic, path, NULL);

	return ic;
}

Evas_Object *_vcui_show_default_image(Evas_Object *contents, Evas_Object *win_main, char *path)
{
	Evas_Object *sw;
	Evas_Object *ic;

	sw = edje_object_part_swallow_get(_EDJ(contents), "swl_cid");
	if (sw) {
		edje_object_part_unswallow(_EDJ(contents), sw);
		evas_object_del(sw);
	}

	ic = elm_image_add(win_main);
	elm_object_part_content_set(contents, "swl_cid", ic);
	elm_image_file_set(ic, path, NULL);

	return ic;
}

static void __vcui_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("__vcui_more_btn_cb..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	_vcui_view_popup_load_more_option(data);
}

static void __vcui_videocall_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("__vcui_videocall_btn_cb..");
	char *tel_num = (char *)data;

	bundle *kb;
	kb = bundle_create();
	bundle_add(kb, "KEY_CALL_TYPE", "MO");
	bundle_add(kb, "number", tel_num);
	aul_launch_app("org.tizen.vtmain", kb);
	bundle_free(kb);

	free(tel_num);
	tel_num = NULL;

	_vcui_view_common_call_end_timer_reset();
	evas_object_smart_callback_del(obj, "clicked", __vcui_videocall_btn_cb);
}

static void __vcui_voicecall_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("__vcui_voicecall_btn_cb..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	char *tel_num = (char *)data;
	vcui_call_mo_data_t call_data;

	_vcui_view_common_timer_redial_reset();
	ad->speaker_status = EINA_FALSE;
	ad->mute_status = EINA_FALSE;
	ad->wbamr_status = EINA_FALSE;

	memset(&call_data, 0, sizeof(call_data));

	snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", tel_num);
	call_data.ct_index = -1;

	vcall_engine_process_normal_call(call_data.call_number, call_data.ct_index, EINA_FALSE);

	free(tel_num);
	tel_num = NULL;

	evas_object_smart_callback_del(obj, "clicked", __vcui_voicecall_btn_cb);
}

static void __vcui_msg_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("__vcui_msg_btn_cb..");
	char *tel_num = (char *)data;

	_vcui_doc_launch_msg_composer(NULL, tel_num);

	free(tel_num);
	tel_num = NULL;

	_vcui_view_common_call_end_timer_reset();
	evas_object_smart_callback_del(obj, "clicked", __vcui_msg_btn_cb);
}

static void __vcui_add_to_contacts_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = ad->view_st[VIEW_ENDCALL_VIEW];
	char *tel_num = (char *)data;

	_vcui_doc_launch_add_to_contacts_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data), tel_num);

	free(tel_num);
	tel_num = NULL;

	evas_object_smart_callback_del(obj, "clicked", __vcui_add_to_contacts_btn_cb);
}

static void __vcui_view_contact_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = ad->view_st[VIEW_ENDCALL_VIEW];

	_vcui_doc_launch_view_contact_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data), data);

	evas_object_smart_callback_del(obj, "clicked", __vcui_view_contact_btn_cb);
}

Evas_Object *_vcui_create_videocall_button(void *data, char *number)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;
	voice_call_view_data_t *vd = NULL;
	char *tel_number = NULL;

	VCUI_RETURN_NULL_IF_FAIL((vd = (voice_call_view_data_t *)data) != NULL);
	layout = _vc_ui_view_callend_get_layout(vd);

	tel_number = strdup(number);

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_videocall");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_videocall", btn);
	elm_object_style_set(btn, "style_call_end_video_call_button");

	elm_object_text_set(btn, dgettext("sys_string", "IDS_COM_BODY_VIDEO_CALL"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_videocall_btn_cb, tel_number);

	return layout;
}

Evas_Object *_vcui_create_voicecall_button(void *data, char *number)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;
	voice_call_view_data_t *vd = NULL;
	char *tel_number = NULL;

	VCUI_RETURN_NULL_IF_FAIL((vd = (voice_call_view_data_t *)data) != NULL);
	layout = _vc_ui_view_callend_get_layout(vd);

	tel_number = strdup(number);

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_voicecall");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_voicecall", btn);
	elm_object_style_set(btn, "style_call_end_voice_call_button");

	elm_object_text_set(btn, dgettext("sys_string", "IDS_COM_BODY_VOICE_CALL"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_voicecall_btn_cb, tel_number);

	return layout;
}

Evas_Object *_vcui_create_message_button(void *data, char *number)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;
	voice_call_view_data_t *vd = NULL;
	char *tel_number = NULL;

	VCUI_RETURN_NULL_IF_FAIL((vd = (voice_call_view_data_t *)data) != NULL);
	layout = _vc_ui_view_callend_get_layout(vd);

	tel_number = strdup(number);

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_message");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_message", btn);
	elm_object_style_set(btn, "style_call_end_message_button");

	elm_object_text_set(btn, dgettext("sys_string", "IDS_COM_BODY_MESSAGE"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_msg_btn_cb, tel_number);

	return layout;
}

Evas_Object *_vcui_create_add_to_contacts_button(void *data, char *number)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;
	char *tel_number = strdup(number);

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	switch (vd->type) {
	case VIEW_ENDCALL_VIEW:
		{
			layout = _vc_ui_view_callend_get_layout(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		if (tel_number != NULL) {
			free(tel_number);
			tel_number = NULL;
		}
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_contacts");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_contacts", btn);
	elm_object_style_set(btn, "style_call_text_only_grey");
	elm_object_text_set(btn, dgettext("sys_string", "IDS_COM_OPT_ADD_TO_CONTACTS"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_add_to_contacts_btn_cb, tel_number);

	return layout;
}

Evas_Object *_vcui_create_view_contact_button(void *data, int ct_id)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	switch (vd->type) {
	case VIEW_ENDCALL_VIEW:
		{
			layout = _vc_ui_view_callend_get_layout(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_contacts");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_contacts", btn);
	elm_object_style_set(btn, "style_call_text_only_grey");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_VIEW_CONTACT"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_view_contact_btn_cb, (void *)ct_id);

	return layout;
}

static void __qp_end_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	int grp_count = 0;

	vcall_engine_get_group_count(&grp_count);
	CALL_UI_DEBUG("No. of groups - %d", grp_count);

	if (grp_count > 1) {
		CALL_UI_DEBUG("multi-party call");
		vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_ACTIVE_CALLS);
	} else if (grp_count == 1) {
		CALL_UI_DEBUG("single-party call");
		int all_calls = 0, call_status = 0;
		all_calls = _vcui_doc_get_all_call_data_count();
		call_status = _vcui_doc_get_group_call_status();
		CALL_UI_DEBUG("all_calls[%d], call_status[%d]", all_calls, call_status);

		if (all_calls > 1) {
			CALL_UI_DEBUG("End active conference call");
			if (call_status == CALL_HOLD)
				vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_HELD_CALLS);
			else
				vcall_engine_release_call_by_type(VCALL_ENGINE_RELEASE_ALL_ACTIVE_CALLS);
			ad->call_end_type = CALL_END_TYPE_CONF_CALL;	/*conf call end screen SHOW */
		} else if (all_calls == 1) {
			CALL_UI_DEBUG("End single active call");
			vcall_engine_release_call();
		} else {
			CALL_UI_DEBUG("invalid case");
		}
	} else {
		CALL_UI_DEBUG("dialing/connecting screen end");
		vcall_engine_cancel_call();
	}
}

Evas_Object *_vcui_create_quickpanel_mute_button(void *data)
{
	CALL_UI_DEBUG("..");
	Evas_Object *btn, *layout, *sw;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	CALL_UI_DEBUG("vd->type: %d", vd->type);

	layout = ad->quickpanel_layout;
	VCUI_RETURN_NULL_IF_FAIL(layout);

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_left");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);

	elm_object_part_content_set(layout, "btn_left", btn);
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_MUTE_ABB"));

	if (ad->mute_status == EINA_FALSE) {
		elm_object_style_set(btn, "style_call_qp_text_only_normal");
		evas_object_smart_callback_add(btn, "clicked", __vcui_mute_btn_cb, vd);
	} else {
		Evas_Object *ic = elm_image_add(layout);
		elm_image_file_set(ic, QP_MUTE_ICON, NULL);
		elm_object_content_set(btn, ic);

		elm_object_style_set(btn, "style_call_small_text_with_icon");
		evas_object_smart_callback_add(btn, "clicked", __vcui_mute_press_btn_cb, vd);
	}

	return layout;
}

Evas_Object *_vcui_create_quickpanel_unhold_button(void *data)
{
	CALL_UI_DEBUG("..");
	Evas_Object *btn, *layout, *sw;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	CALL_UI_DEBUG("vd->type: %d", vd->type);

	layout = ad->quickpanel_layout;
	VCUI_RETURN_NULL_IF_FAIL(layout);

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_left");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_left", btn);
	elm_object_style_set(btn, "style_call_qp_text_only_normal");

	elm_object_text_set(btn, _("IDS_CALL_BUTTON_UNHOLD"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_hold_btn_cb, vd);

	return layout;
}

Evas_Object *_vcui_create_quickpanel_end_button(void *data)
{
	CALL_UI_DEBUG("..");
	Evas_Object *btn, *layout, *sw;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	CALL_UI_DEBUG("vd->type: %d", vd->type);

	switch (vd->type) {
	case VIEW_QUICKPANEL_VIEW:
		{
			layout = ad->quickpanel_layout;
		}
		break;

	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_right");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_right", btn);
	elm_object_style_set(btn, "style_call_qp_text_only_end");
	elm_object_text_set(btn, _("IDS_CALL_SK3_END_CALL"));
	evas_object_smart_callback_add(btn, "clicked", __qp_end_btn_cb, vd);

	return layout;
}

static Evas_Object *__vcui_create_caller_info(void *data, Eina_Bool bhold)
{
	Evas_Object *caller_info = NULL;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			caller_info = _vc_ui_view_dialing_get_caller_info(vd);
		}
		break;
		case VIEW_INCOMING_LOCK_VIEW:
		{
			caller_info = _vc_ui_view_incoming_lock_get_caller_info(vd);
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			caller_info = _vc_ui_view_single_call_get_caller_info(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			caller_info = _vc_ui_view_multi_call_conf_get_caller_info(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			if (bhold == EINA_TRUE) {
				caller_info = _vc_ui_view_multi_call_split_get_caller_info_hold(vd);
			} else {
				caller_info = _vc_ui_view_multi_call_split_get_caller_info_unhold(vd);
			}
		}
		break;
	case VIEW_ENDCALL_VIEW:
		{
			caller_info = _vc_ui_view_callend_get_caller_info(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		break;
	}

	return caller_info;
}


/* Caller info name or number*/
void _vcui_show_caller_info_name(void *data, const char *name, Eina_Bool bhold)
{
	Evas_Object *layout = NULL;

	layout = __vcui_create_caller_info(data, bhold);
	edje_object_part_text_set(_EDJ(layout), "txt_call_name", name);
}

/* Caller info number */
void _vcui_show_caller_info_number(void *data, const char *number, Eina_Bool bhold)
{
	Evas_Object *layout = NULL;

	layout = __vcui_create_caller_info(data, bhold);
	edje_object_part_text_set(_EDJ(layout), "txt_phone_num", number);
}

/* Caller info icon*/
Evas_Object *_vcui_show_caller_info_icon(void *data, Eina_Bool bhold)
{
	Evas_Object *sw = NULL;
	Evas_Object *ic = NULL;
	Evas_Object *btn = NULL;
	Evas_Object *layout = NULL;

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	layout = __vcui_create_caller_info(data, bhold);

	sw = edje_object_part_swallow_get(_EDJ(layout), "caller_info_icon");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	if (vd->type == VIEW_DIALLING_VIEW ||
		vd->type == VIEW_INCOMING_LOCK_VIEW) {
		CALL_UI_DEBUG("Call status icon");
		ic = elm_image_add(layout);
		elm_image_file_set(ic, CALLER_INFO_ICON, NULL);
		elm_object_part_content_set(layout, "caller_info_icon", ic);

		return ic;
	} else {
		CALL_UI_DEBUG("More button");

		edje_object_signal_emit(_EDJ(layout), "show_vert_separtor", "call-screen");

		btn = elm_button_add(layout);
		CALL_UI_KPI("elm_object_style_set start :: style_caller_info_morebtn");
		elm_object_style_set(btn, "style_caller_info_morebtn");
		CALL_UI_KPI("elm_object_style_set done :: style_caller_info_morebtn");
		elm_object_part_content_set(layout, "caller_info_icon", btn);

		evas_object_smart_callback_add(btn, "clicked", __vcui_more_btn_cb, vd);

		return btn;
	}
}

/* Caller info status*/
Evas_Object *_vcui_show_caller_info_status(void *data, const char *status, Eina_Bool bhold)
{
	Evas_Object *layout = NULL;

	layout = __vcui_create_caller_info(data, bhold);
	edje_object_part_text_set(_EDJ(layout), "txt_status", status);

	return layout;
}

Evas_Object *_vcui_show_contact_image_split(Evas_Object *contents, const char *path, const char *full_path, Eina_Bool bhold)
{
	Evas_Object *sw = NULL;
	Evas_Object *img = NULL;
	char *part = NULL;

	if (bhold) {
		part = "swl_cid_hold";
	} else {
		part = "swl_cid_unhold";
	}

	sw = edje_object_part_swallow_get(_EDJ(contents), part);
	if (sw) {
		edje_object_part_unswallow(_EDJ(contents), sw);
		evas_object_del(sw);
	}

	if (strcmp(full_path, CONF_SPLIT_ICON) == 0) {
		CALL_UI_DEBUG("Conf Image");
		img = elm_image_add(contents);
		elm_image_file_set(img, CONF_SPLIT_ICON, NULL);
	} else if ((strncmp(full_path, IMGDIR, strlen(IMGDIR)) == 0) || (strlen(full_path) == 0)) {
		CALL_UI_DEBUG("No caller image");
		img = elm_image_add(contents);
		elm_image_file_set(img, NOIMG_SPLIT_ICON, NULL);
	} else {
		CALL_UI_DEBUG("Display : %s", full_path);
		img = elm_bg_add(contents);
		elm_bg_load_size_set(img, MAIN_WIN_HD_W, 445);
		elm_bg_file_set(img, full_path, NULL);
		evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(img, 0.5, 0.5);
		evas_object_show(img);
	}

	elm_object_part_content_set(contents, part, img);
	return img;
}

Evas_Object *_vcui_show_call_bg_img(Evas_Object *contents)
{
	Evas_Object *d_image;
	Evas_Object *sw;

	sw = edje_object_part_swallow_get(_EDJ(contents), "call_bg_img");
	if (sw) {
		edje_object_part_unswallow(_EDJ(contents), sw);
		evas_object_del(sw);
	}

	d_image = elm_image_add(contents);
	elm_image_file_set(d_image, CALL_BG_IMG, NULL);
	elm_object_part_content_set(contents, "call_bg_img", d_image);

	return d_image;
}

Evas_Object *_vcui_show_call_not_saved_bg_img(Evas_Object *contents)
{
	Evas_Object *d_image;
	Evas_Object *sw;

	sw = edje_object_part_swallow_get(_EDJ(contents), "call_bg_img");
	if (sw) {
		edje_object_part_unswallow(_EDJ(contents), sw);
		evas_object_del(sw);
	}

	d_image = elm_image_add(contents);
	elm_image_file_set(d_image, CALL_NOT_SAVED_BG_IMG, NULL);
	elm_object_part_content_set(contents, "call_bg_img", d_image);

	return d_image;
}

void _vcui_elements_check_keypad_n_hide(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *view_ly = NULL;

	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			view_ly = _vc_ui_view_dialing_get_layout(vd);
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			view_ly = _vc_ui_view_single_call_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			view_ly = _vcui_view_multi_call_split_get_layout(vd);
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			view_ly = _vcui_view_multi_call_conf_get_main_layout(vd);
		}
		break;
	default:
		CALL_UI_DEBUG("code should never reach here");
		return;
	}

	if (_vcui_keypad_get_show_status() == EINA_TRUE) {
		/*Hide caller info area*/
		edje_object_signal_emit(_EDJ(view_ly), "HIDE", "CALL_AREA");
	}
}

