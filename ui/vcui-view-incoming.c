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
#include "vcui-view-incoming.h"
#include "vcui-view-elements.h"

static int __vcui_view_incoming_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_incoming_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_incoming_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_incoming_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_incoming_destroy(voice_call_view_data_t *view_data);

static voice_call_view_data_t s_view = {
	.type = VIEW_INCOMING_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vcui_view_incoming_oncreate,
	.onUpdate = __vcui_view_incoming_onupdate,
	.onHide = __vcui_view_incoming_onhide,
	.onShow = __vcui_view_incoming_onshow,
	.onDestroy = __vcui_view_incoming_destroy,
	.priv = NULL,
};

voice_call_view_data_t *_vcui_view_incoming_new(vcui_app_call_data_t *ad)
{

	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(incoming_view_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!! ");
	}

	return &s_view;
}

static void __vcui_view_incoming_clear_button(Evas_Object *eo)
{
	CALL_UI_DEBUG("..");
	Evas_Object *sw;
	sw = edje_object_part_swallow_get(_EDJ(eo), "btn_accept");
	if (sw) {
		edje_object_part_unswallow(_EDJ(eo), sw);
		evas_object_del(sw);
	}
	sw = edje_object_part_swallow_get(_EDJ(eo), "btn_reject");
	if (sw) {
		edje_object_part_unswallow(_EDJ(eo), sw);
		evas_object_del(sw);
	}
	sw = edje_object_part_swallow_get(_EDJ(eo), "btn_incoming1");
	if (sw) {
		edje_object_part_unswallow(_EDJ(eo), sw);
		evas_object_del(sw);
	}
	sw = edje_object_part_swallow_get(_EDJ(eo), "btn_incoming2");
	if (sw) {
		edje_object_part_unswallow(_EDJ(eo), sw);
		evas_object_del(sw);
	}
	sw = edje_object_part_swallow_get(_EDJ(eo), "btn_incoming3");
	if (sw) {
		edje_object_part_unswallow(_EDJ(eo), sw);
		evas_object_del(sw);
	}
	sw = edje_object_part_swallow_get(_EDJ(eo), "btn_incoming4");
	if (sw) {
		edje_object_part_unswallow(_EDJ(eo), sw);
		evas_object_del(sw);
	}
}

static void __vcui_view_incoming_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_view_priv_t *priv = (incoming_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;
	int hold_cnt = 0;
	int active_cnt = 0;

	/* ============ Check valid Evas Object ============= */
	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, "INCOMINGVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== INCOMINGVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
		return;
	}
	/* ============ Check valid Evas Object ============= */

	if (strcmp((char *)priv->now_data->call_file_path, NOIMG_ICON) == 0) {
		_vcui_show_wallpaper_image(priv->contents);
		_vcui_show_calling_name_bg(priv->contents);
	} else {
		CALL_UI_DEBUG("there is caller image.");
		_vcui_show_wallpaper_image(priv->contents);
		_vcui_show_calling_name_bg(priv->contents);
		priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, priv->now_data->call_file_path);
	}

	/* call name (if nothing, call number) */
	if (strlen((char *)priv->now_data->call_display) == 0) {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_num);
	} else {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_display);
		edje_object_part_text_set(_EDJ(eo), "txt_contact_phone_type", (char *)priv->now_data->call_num);
	}

	active_cnt = _vcui_doc_get_count_unhold();
	hold_cnt = _vcui_doc_get_count_hold();
	CALL_UI_DEBUG("unhold:[%d], hold:[%d]",active_cnt, hold_cnt );
	__vcui_view_incoming_clear_button(priv->contents);	/* to change 2nd incoming call view -> normal incoming call view. */
	if (active_cnt == 0) {	/* no active call  - single mt call or 2nd call(1hold) */
		CALL_UI_DEBUG("normal incoming call");

		_vcui_create_button_accept(vd);
		_vcui_create_button_reject(vd);

		ad->beffect_needed = EINA_TRUE;
	} else if (hold_cnt == 0) {	/* active call & no hold call - 2nd call(1active) */
		CALL_UI_DEBUG("2nd incoming call-1");
		char text[256] = { 0, };
		_vcui_create_button_second_incoming_reject(vd, _("IDS_CALL_BUTTON_REJECT"), "btn_incoming3");
		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_BODY_HOLD_ACTIVE_CALL_ABB"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_hold_and_accept(vd, text);
		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_POP_END_ACTIVE_CALL"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_and_accept(vd, text);
	} else if (active_cnt == 1 && hold_cnt == 1) {	/* 1active & 1hold */
		CALL_UI_DEBUG("2nd incoming call - 1ACT & 1HLD");
		char text[256] = { 0, };
		_vcui_create_button_second_incoming_reject(vd, _("IDS_CALL_BUTTON_REJECT"), "btn_incoming1");

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_POP_END_ACTIVE_CALL"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_active_and_accept(vd, text);

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_OPT_END_HELD_CALL"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_hold_and_accept(vd, text);

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_OPT_END_ALL_CALLS"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_all_and_accept(vd, text);
	} else if (active_cnt >= 1 && hold_cnt == 1) {	/* 1+active & 1hold */
		CALL_UI_DEBUG("2nd incoming call - [1+]ACT & 1HLD");
		char text[256] = { 0, };
		_vcui_create_button_second_incoming_reject(vd, _("IDS_CALL_BUTTON_REJECT"), "btn_incoming1");

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s(%d) + %s", _("IDS_CALL_POP_END_ACTIVE_CALL"), active_cnt, _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_active_and_accept(vd, text);

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_OPT_END_HELD_CALL"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_hold_and_accept(vd, text);

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_OPT_END_ALL_CALLS"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_all_and_accept(vd, text);
	} else if (active_cnt == 1 && hold_cnt >= 1) {	/*  1active & 1+hold */
		CALL_UI_DEBUG("2nd incoming call - 1ACT & [1+]HLD");
		char text[256] = { 0, };
		_vcui_create_button_second_incoming_reject(vd, _("IDS_CALL_BUTTON_REJECT"), "btn_incoming1");

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_POP_END_ACTIVE_CALL"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_active_and_accept(vd, text);

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s(%d) + %s", _("IDS_CALL_OPT_END_HELD_CALL"), hold_cnt, _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_hold_and_accept(vd, text);

		memset(text, 0x00, 256);
		snprintf(text, 256, "%s + %s", _("IDS_CALL_OPT_END_ALL_CALLS"), _("IDS_CALL_BUTTON_ACCEPT"));
		_vcui_create_button_second_incoming_end_all_and_accept(vd, text);
	}
	CALL_UI_DEBUG("unhold:[%d], hold:[%d]", active_cnt, hold_cnt);

	evas_object_show(eo);
}

static Evas_Object *__vcui_view_incoming_create_contents(void *data)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *eo = NULL;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, GRP_MTVIEW);
	if (eo == NULL)
		return NULL;

	return eo;
}

static Evas_Object *__vcui_view_incoming_create_layout_main(Evas_Object *parent)
{
	if (parent == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}

	Evas_Object *ly;
	ly = elm_layout_add(parent);
	retvm_if(ly == NULL, NULL, "Failed elm_layout_add.");

	elm_layout_theme_set(ly, "standard", "window", "integration");
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, ly);

	edje_object_signal_emit(_EDJ(ly), "elm,state,show,indicator", "elm");
	edje_object_signal_emit(_EDJ(ly), "elm,state,show,content", "elm");
	evas_object_show(ly);

	return ly;
}

static int __vcui_view_incoming_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("incoming view create!!");

	vcui_app_call_data_t *ad = view_data->app_data;
	incoming_view_priv_t *priv = view_data->priv;

	if (!view_data->layout) {

		view_data->layout = __vcui_view_incoming_create_layout_main(ad->win_main);
		if (view_data->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}

		priv->contents = __vcui_view_incoming_create_contents(view_data);
		elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);

		evas_object_name_set(priv->contents, "INCOMINGVIEW");
		CALL_UI_DEBUG("[========== INCOMINGVIEW: priv->contents Addr : [%p] ==========]", priv->contents);

	}

	__vcui_view_incoming_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_incoming_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("incoming view update");
	incoming_view_priv_t *priv = view_data->priv;
	if (priv->bdont_refresh == EINA_TRUE) {
		return VC_NO_ERROR;
	}
	priv->bselected_btn = EINA_FALSE;
	__vcui_view_incoming_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_incoming_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("incoming view hide");

	int valid = 0;
	incoming_view_priv_t *priv = (incoming_view_priv_t *)view_data->priv;
	valid = _vcui_check_valid_eo(priv->contents, "INCOMINGVIEW");

	if (valid == -1) {
		CALL_UI_DEBUG("[========== INCOMINGVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
	}

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_incoming_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("incoming view show");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	int result = 0;

	incoming_view_priv_t *priv = view_data->priv;
	priv->now_data = _vcui_doc_get_recent_mt();
	if (priv->now_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return VC_ERROR;
	}

	__vcui_view_incoming_draw_screen(priv->contents, view_data);

	evas_object_hide(view_data->layout);
	evas_object_show(view_data->layout);

	_vcui_app_win_set_noti_type(EINA_TRUE);

	return VC_NO_ERROR;
}

static int __vcui_view_incoming_destroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("incoming view destroy");

	vcui_app_call_data_t *ad = _vcui_get_app_data();

	voice_call_view_data_t *vd = ad->view_st[VIEW_INCOMING_VIEW];
	incoming_view_priv_t *priv = (incoming_view_priv_t *)vd->priv;

	if (_vcui_is_idle_lock() == CALL_LOCK)
		_vcui_app_win_set_noti_type(EINA_TRUE);
	else
		_vcui_app_win_set_noti_type(EINA_FALSE);

	ad->bmute_ringtone = EINA_FALSE;

	if (priv != NULL) {
		if (priv->contents) {
			evas_object_del(priv->contents);
			priv->contents = NULL;
		}

		free(priv);
		priv = NULL;
	}

	if (vd->layout != NULL) {
		evas_object_hide(vd->layout);
		evas_object_del(vd->layout);
		vd->layout = NULL;
	}

	ad->view_st[VIEW_INCOMING_VIEW] = NULL;
	_vcui_cache_flush();

	return VC_NO_ERROR;
}
