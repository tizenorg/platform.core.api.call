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
#include "vcui-view-dialing.h"
#include "vcui-view-elements.h"

static int __vcui_view_dialing_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_dialing_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_dialing_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_dialing_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_dialing_ondestroy(voice_call_view_data_t *view_data);

static voice_call_view_data_t s_view = {
	.type = VIEW_DIALLING_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vcui_view_dialing_oncreate,
	.onUpdate = __vcui_view_dialing_onupdate,
	.onHide = __vcui_view_dialing_onhide,
	.onShow = __vcui_view_dialing_onshow,
	.onDestroy = __vcui_view_dialing_ondestroy,
	.priv = NULL,
};

voice_call_view_data_t *_vcui_view_dialing_new(vcui_app_call_data_t *ad)
{

	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(vcui_view_dialing_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!! ");
	}

	return &s_view;
}

void _vcui_view_dialing_draw_txt_ended(voice_call_view_data_t *vd, int end_type)
{
	/* ============ Check valid Evas Object ============= */
	if (vd == NULL) {
		CALL_UI_DEBUG("View Data is NULL");
		return;
	}

	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)vd->priv;

	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, "DIALVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== DIALVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
	} else {
		char data[VC_DATA_LENGTH_MAX] = { 0, };
		edje_object_part_text_set(_EDJ(priv->contents), "txt_mo_status", _(_vcui_get_endcause_string(end_type, data)));
	}
	/* ============ Check valid Evas Object ============= */
}

void _vcui_view_dialing_draw_txt_connecting(voice_call_view_data_t *vd)
{
	/* ============ Check valid Evas Object ============= */
	if (vd == NULL) {
		CALL_UI_DEBUG("View Data is NULL");
		return;
	}

	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)vd->priv;

	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, "DIALVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== DIALVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
	} else {
		edje_object_part_text_set(_EDJ(priv->contents), "txt_mo_status", _("IDS_CALL_POP_CONNECTING"));
	}
	/* ============ Check valid Evas Object ============= */
}

void _vcui_view_dialing_draw_txt_dialing(voice_call_view_data_t *vd)
{
	/* ============ Check valid Evas Object ============= */
	if (vd == NULL) {
		CALL_UI_DEBUG("View Data is NULL");
		return;
	}

	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)vd->priv;

	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, "DIALVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== DIALVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
	} else {
		edje_object_part_text_set(_EDJ(priv->contents), "txt_mo_status", _("IDS_CALL_POP_DIALLING"));
	}
	/* ============ Check valid Evas Object ============= */
}

static void __vcui_view_dialing_update_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)vd->priv;

	call_data_t *now_call_data = _vcui_doc_get_recent_mo();

	/* ============ Check valid Evas Object ============= */
	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, "DIALVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== DIALVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
		return;
	}
	/* ============ Check valid Evas Object ============= */

	if (now_call_data == NULL) {
		CALL_UI_DEBUG("Now Data is NULL");
		return;
	}

	/* call image */
	if (strcmp((char *)priv->now_data->call_file_path, NOIMG_ICON) == 0) {
		_vcui_show_wallpaper_image(priv->contents);
	} else {
		CALL_UI_DEBUG("there is caller image.");

		_vcui_show_wallpaper_image(priv->contents);
		priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, priv->now_data->call_file_path);
	}

	/* call name (if nothing, call number) */
	if (strlen((char *)priv->now_data->call_display) == 0) {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_num);
	} else {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_display);
		edje_object_part_text_set(_EDJ(eo), "txt_contact_phone_type", (char *)priv->now_data->call_num);
	}

	_vcui_create_top_left_button_disabled(vd);
	_vcui_create_top_middle_button_disabled(vd);
	_vcui_create_top_right_button_disabled(vd);
	_vcui_create_bottom_left_button(vd);
	_vcui_create_bottom_middle_button_disabled(vd);
	_vcui_create_bottom_right_button_disabled(vd);

	_vcui_create_button_bigend(vd);

	evas_object_show(eo);
}

static void __vcui_view_dialing_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)vd->priv;

	call_data_t *now_call_data = _vcui_doc_get_recent_mo();

	/* ============ Check valid Evas Object ============= */
	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, "DIALVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== DIALVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
		return;
	}
	/* ============ Check valid Evas Object ============= */

	if (now_call_data == NULL) {
		CALL_UI_DEBUG("Now Data is NULL");
		return;
	}

	/* call image */
	if (strcmp((char *)priv->now_data->call_file_path, NOIMG_ICON) == 0) {
		_vcui_show_wallpaper_image(priv->contents);
	} else {
		CALL_UI_DEBUG("there is caller image.");

		_vcui_show_wallpaper_image(priv->contents);
		priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, priv->now_data->call_file_path);
	}

	/* call name (if nothing, call number) */
	if (strlen((char *)priv->now_data->call_display) == 0) {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_num);
	} else {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_display);
		edje_object_part_text_set(_EDJ(eo), "txt_contact_phone_type", (char *)priv->now_data->call_num);
	}

	edje_object_part_text_set(_EDJ(priv->contents), "txt_mo_status", _("IDS_CALL_POP_DIALLING"));

	_vcui_create_top_left_button_disabled(vd);
	_vcui_create_top_middle_button_disabled(vd);
	_vcui_create_top_right_button_disabled(vd);
	_vcui_create_bottom_left_button(vd);
	_vcui_create_bottom_middle_button_disabled(vd);
	_vcui_create_bottom_right_button_disabled(vd);

	_vcui_create_button_bigend(vd);

	evas_object_show(eo);
}

static Evas_Object *__vcui_view_dialing_create_contents(void *data)
{
	CALL_UI_KPI("__vcui_view_dialing_create_contents start");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *eo = NULL;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, GRP_MOVIEW);
	if (eo == NULL)
		return NULL;
	CALL_UI_KPI("__vcui_view_dialing_create_contents done");
	return eo;
}

static Evas_Object *__vcui_view_dialing_create_layout_main(Evas_Object *parent)
{
	CALL_UI_KPI("__vcui_view_dialing_create_layout_main start");
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
	CALL_UI_KPI("__vcui_view_dialing_create_layout_main done");
	return ly;
}

static int __vcui_view_dialing_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("dialling view create!!");

	vcui_app_call_data_t *ad = view_data->app_data;
	vcui_view_dialing_priv_t *priv = view_data->priv;

	if (!view_data->layout) {

		/* Create Main Layout */
		view_data->layout = __vcui_view_dialing_create_layout_main(ad->win_main);

		if (view_data->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
		/* Create Contents */
		priv->contents = __vcui_view_dialing_create_contents(view_data);
		elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);

		evas_object_name_set(priv->contents, "DIALVIEW");
		CALL_UI_DEBUG("[========== DIALVIEW: priv->contents Addr : [%p] ==========]", priv->contents);
	}

	__vcui_view_dialing_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_dialing_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("dialling view update");
	vcui_view_dialing_priv_t *priv = view_data->priv;

	__vcui_view_dialing_update_screen(priv->contents, view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_dialing_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("dialling view hide");

	int valid = 0;
	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)view_data->priv;
	valid = _vcui_check_valid_eo(priv->contents, "DIALVIEW");

	if (valid == -1) {
		CALL_UI_DEBUG("[========== DIALVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
	}

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_dialing_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_KPI("__vcui_view_dialing_onshow start");
	CALL_UI_DEBUG("dialling view show");

	vcui_view_dialing_priv_t *priv = view_data->priv;
	priv->now_data = _vcui_doc_get_recent_mo();
	if (priv->now_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return VC_ERROR;
	}

	__vcui_view_dialing_draw_screen(priv->contents, view_data);

	evas_object_hide(view_data->layout);
	evas_object_show(view_data->layout);

	if (_vcui_is_idle_lock() == CALL_LOCK)
		_vcui_app_win_set_noti_type(EINA_TRUE);
	else
		_vcui_app_win_set_noti_type(EINA_FALSE);

	CALL_UI_KPI("__vcui_view_dialing_onshow done");

	return VC_NO_ERROR;
}

static int __vcui_view_dialing_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("dialling view destroy");

	vcui_app_call_data_t *ad = _vcui_get_app_data();

	voice_call_view_data_t *vd = ad->view_st[VIEW_DIALLING_VIEW];
	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)vd->priv;

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

	ad->view_st[VIEW_DIALLING_VIEW] = NULL;

	_vcui_cache_flush();
	return VC_NO_ERROR;
}
