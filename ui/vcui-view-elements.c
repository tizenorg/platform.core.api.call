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
#include "vcui-view-incoming.h"
#include "vcui-view-single-call.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-keypad.h"
#include "vcui-view-callend.h"

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
static void __vcui_accept_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_reject_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_second_incoming_reject_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_second_incoming_hold_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_second_incoming_end_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_second_incoming_end_active_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_second_incoming_end_hold_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_second_incoming_end_all_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_videocall_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_voicecall_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __vcui_msg_btn_cb(void *data, Evas_Object *obj, void *event_info);

static void __vcui_hold_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_engine_hold_unhold_swap_call();
}

static void __vcui_unhold_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_engine_hold_unhold_swap_call();
}

void _vcui_swap_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;

	ad->bholdisleft = ad->bswapped;
	_vcui_view_popup_load(_("IDS_CALL_POP_SWAPPED"), POPUP_TIMEOUT_LONG, EINA_FALSE);
	_vcui_engine_hold_unhold_swap_call();
}

static void __vcui_join_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_create_top_right_button_disabled(data);
	_vcui_engine_join_call();
}

void _vcui_conf_img_cb(void *data, Evas_Object *obj, void *event_info)	/* for multicall list view */
{
	CALL_UI_DEBUG("..");
	_vcui_view_change(VIEW_INCALL_MULTICALL_LIST_VIEW, 0, 0, 0);
}

static void __vcui_keypad_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_view_change(VIEW_INCALL_KEYPAD_VIEW, 0, NULL, NULL);
}

static void __vcui_contacts_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	_vcui_doc_launch_contact_list_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data));
}

static void __vcui_addcall_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;

	_vcui_doc_launch_phoneui_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data));
}

static void __vcui_sound_path_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_view_popup_load_snd_path();
}

static void __vcui_spk_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;

	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	_vcui_engine_speaker_on_off(EINA_TRUE);

	ad->speaker_status = EINA_TRUE;
	_vcui_create_bottom_left_button(vd);
}

static void __vcui_spk_press_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;

	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	_vcui_engine_speaker_on_off(EINA_FALSE);

	ad->speaker_status = EINA_FALSE;
	_vcui_create_bottom_left_button(vd);
}

static void __vcui_mute_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;

	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	_vcui_engine_mute_on_off(EINA_TRUE);

	ad->mute_status = EINA_TRUE;
	_vcui_create_bottom_middle_button(vd);
}

static void __vcui_mute_press_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;

	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	_vcui_engine_mute_on_off(EINA_FALSE);

	ad->mute_status = EINA_FALSE;
	_vcui_create_bottom_middle_button(vd);
}

static void __vcui_bigend_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;
	CALL_UI_DEBUG("vd->type:[%d]", vd->type);

	_vcui_create_button_bigend_disabled(vd);

	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			_vcui_engine_cancel_call();
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			_vcui_engine_end_call();
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			_vcui_engine_end_active_calls();
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			vcui_view_multi_call_conf_priv_t *priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;
			if (priv->call_status == CALL_HOLD)
				_vcui_engine_end_held_calls();
			else
				_vcui_engine_end_active_calls();
			ad->call_end_type = CALL_END_TYPE_CONF_CALL;
		}
		break;
	case VIEW_INCALL_MULTICALL_LIST_VIEW:
		{
			vcui_view_multi_call_list_priv_t *priv = (vcui_view_multi_call_list_priv_t *) vd->priv;
			if (priv->call_status == CALL_HOLD)
				_vcui_engine_end_held_calls();
			else
				_vcui_engine_end_active_calls();

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

static void __vcui_accept_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;

	if (priv->bselected_btn == EINA_TRUE) {
		CALL_UI_DEBUG("already clicked!!");
	} else {
		priv->bselected_btn = EINA_TRUE;
		_vcui_engine_answer_call();
	}
}

static void __vcui_reject_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;

	if (priv->bselected_btn == EINA_TRUE) {
		CALL_UI_DEBUG("already clicked!!");
	} else {
		priv->bselected_btn = EINA_TRUE;
		_vcui_engine_reject_call();
	}
}

static void __vcui_second_incoming_reject_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_engine_reject_call();
}

static void __vcui_second_incoming_hold_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			priv->bdont_refresh = EINA_TRUE;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		break;
	}

	_vcui_engine_answer_call_by_type(1);
}

static void __vcui_second_incoming_end_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			priv->bdont_refresh = EINA_TRUE;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		break;
	}

	_vcui_engine_answer_call_by_type(2);
}

static void __vcui_second_incoming_end_active_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			priv->bdont_refresh = EINA_TRUE;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		break;
	}

	_vcui_engine_answer_call_by_type(2);
}

static void __vcui_second_incoming_end_hold_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			priv->bdont_refresh = EINA_TRUE;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		break;
	}

	_vcui_engine_answer_call_by_type(3);
}

static void __vcui_second_incoming_end_all_and_accept_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			priv->bdont_refresh = EINA_TRUE;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		break;
	}

	_vcui_engine_answer_call_by_type(4);
}

Evas_Object *_vcui_create_top_left_button(void *data)
{
	return NULL;
}

Evas_Object *_vcui_create_top_left_button_disabled(void *data)
{
	return NULL;
}


Evas_Object *_vcui_create_top_middle_button(void *data)
{
	return NULL;
}

Evas_Object *_vcui_create_top_middle_button_disabled(void *data)
{
	return NULL;
}

Evas_Object *_vcui_create_top_right_button(void *data)
{
	return NULL;
}

Evas_Object *_vcui_create_top_right_button_disabled(void *data)
{
	return NULL;
}

/* Speaker Button ENABLED */
Evas_Object *_vcui_create_bottom_left_button(void *data)
{
	CALL_UI_KPI("_vcui_create_bottom_left_button start");
	Evas_Object *btn;
	Evas_Object *ic;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;

	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			incall_one_view_priv_t *priv = (incall_one_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			vcui_view_multi_call_conf_priv_t *priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_speaker");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}
	if (EINA_TRUE == _vcui_is_headset_conected()) {
		btn = elm_button_add(layout);
		CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn");
		elm_object_style_set(btn, "style_call_sixbtn");
		CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn");
		elm_object_part_content_set(layout, "btn_speaker", btn);
		ic = elm_icon_add(layout);
		elm_icon_file_set(ic, SPEAKER_ICON, NULL);
		evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_icon_resizable_set(ic, 1, 1);
		elm_object_part_content_set(btn, "icon", ic);
		elm_object_text_set(btn, _("IDS_MSGC_OPT_SOUND"));
		evas_object_smart_callback_add(btn, "clicked", __vcui_sound_path_btn_cb, vd);
	} else {
		if (ad->speaker_status == EINA_FALSE) {
			btn = elm_button_add(layout);
			CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn");
			elm_object_style_set(btn, "style_call_sixbtn");
			CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn");
			elm_object_part_content_set(layout, "btn_speaker", btn);
			ic = elm_icon_add(layout);
			elm_icon_file_set(ic, SPEAKER_ICON, NULL);
			evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			elm_icon_resizable_set(ic, 1, 1);
			elm_object_part_content_set(btn, "icon", ic);
			elm_object_text_set(btn, _("IDS_CALL_BUTTON_SPEAKER"));
			evas_object_smart_callback_add(btn, "clicked", __vcui_spk_btn_cb, vd);
		} else {
			btn = elm_button_add(layout);
			CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_pressed");
			elm_object_style_set(btn, "style_call_sixbtn_pressed");
			CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_pressed");
			elm_object_part_content_set(layout, "btn_speaker", btn);
			ic = elm_icon_add(layout);
			elm_icon_file_set(ic, SPEAKER_ICON, NULL);
			evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			elm_icon_resizable_set(ic, 1, 1);
			elm_object_part_content_set(btn, "icon", ic);
			elm_object_text_set(btn, _("IDS_CALL_BUTTON_SPEAKER"));
			evas_object_smart_callback_add(btn, "clicked", __vcui_spk_press_btn_cb, vd);
		}
	}
	CALL_UI_KPI("_vcui_create_bottom_left_button done");
	return layout;
}

/* Speaker Button DISABLED */
Evas_Object *_vcui_create_bottom_left_button_disabled(void *data)
{
	CALL_UI_KPI("_vcui_create_bottom_left_button_disabled start");
	Evas_Object *btn;
	Evas_Object *ic;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_ENDCALL_VIEW:
		{
			endcall_view_priv_t *priv = (endcall_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_speaker");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	CALL_UI_KPI("elm_object_style_set start :: style_call_sixbtn_disabled");
	elm_object_style_set(btn, "style_call_sixbtn_disabled");
	CALL_UI_KPI("elm_object_style_set done :: style_call_sixbtn_disabled");
	elm_object_part_content_set(layout, "btn_speaker", btn);
	ic = elm_icon_add(layout);
	elm_icon_file_set(ic, SPEAKER_DISABLED_ICON, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_resizable_set(ic, 1, 1);
	elm_object_part_content_set(btn, "icon", ic);
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_SPEAKER"));
	CALL_UI_KPI("_vcui_create_bottom_left_button_disabled done");
	return layout;
}

Evas_Object *_vcui_create_bottom_middle_button(void *data)
{
	return NULL;
}

Evas_Object *_vcui_create_bottom_middle_button_disabled(void *data)
{
	return NULL;
}

Evas_Object *_vcui_create_bottom_right_button(void *data)
{
	return NULL;
}


Evas_Object *_vcui_create_bottom_right_button_disabled(void *data)
{
	return NULL;
}


Evas_Object *_vcui_create_button_bigend(void *data)
{
	CALL_UI_KPI("_vcui_create_button_bigend start");
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			incall_one_view_priv_t *priv = (incall_one_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			vcui_view_multi_call_conf_priv_t *priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_MULTICALL_LIST_VIEW:
		{
			vcui_view_multi_call_list_priv_t *priv = (vcui_view_multi_call_list_priv_t *) vd->priv;
			layout = priv->contents;
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
	CALL_UI_KPI("elm_object_style_set start :: text_only/style_call_text_only_red");
	elm_object_style_set(btn, "text_only/style_call_text_only_red");
	CALL_UI_KPI("elm_object_style_set done :: text_only/style_call_text_only_red");
	elm_object_text_set(btn, _("IDS_CALL_SK3_END_CALL"));
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

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_DIALLING_VIEW:
		{
			vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_ONECALL_VIEW:
		{
			incall_one_view_priv_t *priv = (incall_one_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_MULTICALL_SPLIT_VIEW:
		{
			incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_INCALL_MULTICALL_CONF_VIEW:
		{
			vcui_view_multi_call_conf_priv_t *priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	case VIEW_ENDCALL_VIEW:
		{
			endcall_view_priv_t *priv = (endcall_view_priv_t *) vd->priv;
			layout = priv->contents;
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
	CALL_UI_KPI("elm_object_style_set start :: text_only/style_call_text_only_red_disabled");
	elm_object_style_set(btn, "text_only/style_call_text_only_red_disabled");
	CALL_UI_KPI("elm_object_style_set done :: text_only/style_call_text_only_red_disabled");
	elm_object_text_set(btn, _("IDS_CALL_SK3_END_CALL"));
	CALL_UI_KPI("_vcui_create_button_bigend_disabled done");
	return layout;
}

Evas_Object *_vcui_create_conf_list_button_hold(void *data)
{
	return NULL;
}

Evas_Object *_vcui_create_button_accept(void *data)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_accept");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_accept", btn);
	CALL_UI_KPI("elm_object_style_set start :: text_only/style_call_text_only_green");
	elm_object_style_set(btn, "text_only/style_call_text_only_green");
	CALL_UI_KPI("elm_object_style_set done :: text_only/style_call_text_only_green");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_ACCEPT"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_accept_btn_cb, vd);

	return layout;
}

Evas_Object *_vcui_create_button_reject(void *data)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_reject");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_reject", btn);
	CALL_UI_KPI("elm_object_style_set start :: text_only/style_call_text_only_red");
	elm_object_style_set(btn, "text_only/style_call_text_only_red");
	CALL_UI_KPI("elm_object_style_set done :: text_only/style_call_text_only_red");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_REJECT"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_reject_btn_cb, vd);

	return layout;
}

Evas_Object *_vcui_create_button_second_incoming_reject(void *data, char *text, char *part_name)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			layout = priv->contents;
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

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, part_name, btn);
	CALL_UI_KPI("elm_object_style_set start :: multiline_text_black");
	elm_object_style_set(btn, "multiline_text_black");
	CALL_UI_KPI("elm_object_style_set done :: multiline_text_black");
	elm_object_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", __vcui_second_incoming_reject_btn_cb, vd);

	return layout;
}

Evas_Object *_vcui_create_button_second_incoming_hold_and_accept(void *data, char *text)
{
	return NULL;
}

Evas_Object *_vcui_create_button_second_incoming_end_and_accept(void *data, char *text)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_incoming4");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_incoming4", btn);
	CALL_UI_KPI("elm_object_style_set start :: multiline_text_red");
	elm_object_style_set(btn, "multiline_text_red");
	CALL_UI_KPI("elm_object_style_set done :: multiline_text_red");
	elm_object_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", __vcui_second_incoming_end_and_accept_btn_cb, vd);

	return layout;
}

Evas_Object *_vcui_create_button_second_incoming_end_active_and_accept(void *data, char *text)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_incoming2");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_incoming2", btn);
	CALL_UI_KPI("elm_object_style_set start :: multiline_text_black");
	elm_object_style_set(btn, "multiline_text_black");
	CALL_UI_KPI("elm_object_style_set done :: multiline_text_black");
	elm_object_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", __vcui_second_incoming_end_active_and_accept_btn_cb, vd);

	return layout;
}

Evas_Object *_vcui_create_button_second_incoming_end_hold_and_accept(void *data, char *text)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_incoming3");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_incoming3", btn);
	CALL_UI_KPI("elm_object_style_set start :: multiline_text_black");
	elm_object_style_set(btn, "multiline_text_black");
	CALL_UI_KPI("elm_object_style_set done :: multiline_text_black");
	elm_object_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", __vcui_second_incoming_end_hold_and_accept_btn_cb, vd);

	return layout;
}

Evas_Object *_vcui_create_button_second_incoming_end_all_and_accept(void *data, char *text)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	switch (vd->type) {
	case VIEW_INCOMING_VIEW:
		{
			incoming_view_priv_t *priv = (incoming_view_priv_t *) vd->priv;
			layout = priv->contents;
		}
		break;
	default:
		CALL_UI_DEBUG("ERROR - wrong vd type:[%d]", vd->type);
		return NULL;
		break;

	}

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_incoming4");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_incoming4", btn);
	CALL_UI_KPI("elm_object_style_set start :: multiline_text_red");
	elm_object_style_set(btn, "multiline_text_red");
	CALL_UI_KPI("elm_object_style_set done :: multiline_text_red");
	elm_object_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", __vcui_second_incoming_end_all_and_accept_btn_cb, vd);

	return layout;
}

Evas_Object *_vcui_show_wallpaper_image(Evas_Object *contents)
{
	return NULL;
}

Evas_Object *_vcui_show_calling_name_bg(Evas_Object *contents)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	Evas_Object *sw = NULL;
	Evas_Object *ic = NULL;

	sw = edje_object_part_swallow_get(_EDJ(contents), "swl_calling_name_bg");
	if (sw) {
		edje_object_part_unswallow(_EDJ(contents), sw);
		evas_object_del(sw);
	}

	ic = elm_icon_add(ad->win_main);
	elm_object_part_content_set(contents, "swl_calling_name_bg", ic);
	elm_icon_file_set(ic, CALLING_NAME_BG_IMAGE, NULL);
	elm_icon_fill_outside_set(ic, EINA_TRUE);

	return ic;
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

	ic = elm_icon_add(win_main);
	elm_object_part_content_set(contents, "swl_cid", ic);
	elm_icon_file_set(ic, path, NULL);

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

	ic = elm_icon_add(win_main);
	elm_object_part_content_set(contents, "swl_cid", ic);
	elm_icon_file_set(ic, path, NULL);

	return ic;
}

void _vcui_set_full_image(Evas_Object *contents, Evas_Object *win_main, char *img_path)
{
	Evas_Object *d_image;
	Evas_Object *sw;
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	CALL_UI_DEBUG("path:[%s]", img_path);

	sw = edje_object_part_swallow_get(_EDJ(contents), "swl_cid_background");
	if (sw) {
		edje_object_part_unswallow(_EDJ(contents), sw);
		evas_object_del(sw);
	}

	d_image = elm_image_add(contents);
	elm_image_file_set(d_image, img_path, NULL);
	elm_object_part_content_set(contents, "swl_cid_background", d_image);
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
	vcui_call_type_t call_type;
	vcui_call_mo_data_t call_data;

	_vcui_view_common_timer_redial_reset();
	ad->speaker_status = EINA_FALSE;
	ad->mute_status = EINA_FALSE;

	memset(&call_data, 0, sizeof(call_data));
	call_type = VCUI_CALL_TYPE_MO;

	snprintf(call_data.call_number, sizeof(call_data.call_number), "%s", tel_num);

	_vcui_engine_interface_process_mo_call(call_type, &call_data);

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
	CALL_UI_DEBUG("__vcui_add_to_contacts_btn_cb..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = ad->view_st[VIEW_ENDCALL_VIEW];
	char *tel_num = (char *)data;

	_vcui_doc_launch_add_to_contacts_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data), tel_num);

	free(tel_num);
	tel_num = NULL;

	evas_object_smart_callback_del(obj, "clicked", __vcui_add_to_contacts_btn_cb);
}
 
static void __qp_end_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;
	int grp_count = 0;

	vcall_engine_get_group_count(&grp_count);
	CALL_UI_DEBUG("No. of groups - %d", grp_count);

	if (grp_count > 1) {
		CALL_UI_DEBUG("multi-party call");
		_vcui_engine_end_active_calls();
	} else if (grp_count == 1) {
		CALL_UI_DEBUG("single-party call");
		int all_calls = 0, call_status = 0;
		all_calls = _vcui_doc_get_count();
		call_status = _vcui_doc_get_show_callstatus();
		CALL_UI_DEBUG("all_calls[%d], call_status[%d]", all_calls, call_status);

		if (all_calls > 1) {
			CALL_UI_DEBUG("End active conference call");
			if (call_status == CALL_HOLD)
				_vcui_engine_end_held_calls();
			else
				_vcui_engine_end_active_calls();
			ad->call_end_type = CALL_END_TYPE_CONF_CALL;	/*conf call end screen SHOW*/
		} else if (all_calls == 1) {
			CALL_UI_DEBUG("End single active call");
			_vcui_engine_end_call();
		} else {
			CALL_UI_DEBUG("invalid case");
		}
	} else {
		CALL_UI_DEBUG("dialing/connecting screen end");
		_vcui_engine_cancel_call();
	}
}
