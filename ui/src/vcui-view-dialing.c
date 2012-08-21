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
#include "vcui-view-common.h"
#include "vcui-app-window.h"
#include "vcui-view-keypad.h"

#define	VIEW_DIALING_LAYOUT_ID "DIALVIEW"
typedef struct {
	Evas_Object *contents;
	Evas_Object *caller_info;
	Evas_Object *btn_ly;
	Evas_Object *ic;
} vcui_view_dialing_priv_t;

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
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, VIEW_DIALING_LAYOUT_ID);
	if (valid == -1) {
		CALL_UI_DEBUG("[========== DIALVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
	} else {
		char data[VC_DATA_LENGTH_MAX] = { 0, };
		edje_object_part_text_set(_EDJ(priv->caller_info), "txt_status", _(_vcui_get_endcause_string(end_type, data)));
		ad->quickpanel_text = data;
	}
	/* ============ Check valid Evas Object ============= */
}

static void __vcui_view_dialing_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)vd->priv;

	call_data_t *now_call_data = _vcui_doc_get_recent_mo_call_data();
	Eina_Bool bshift_alert = EINA_FALSE;

	/* ============ Check valid Evas Object ============= */
	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, VIEW_DIALING_LAYOUT_ID);
	if (valid == -1) {
		CALL_UI_DEBUG("[========== DIALVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
		return;
	}
	/* ============ Check valid Evas Object ============= */

	if (now_call_data == NULL) {
		CALL_UI_DEBUG("Now Data is NULL");
		return;
	}

	char *file_path = _vcui_doc_get_caller_id_file_path(now_call_data);
	CALL_UI_DEBUG("file_path: %s", file_path);

	/* call image */
	_vcui_delete_contact_image(priv->contents);
	if (strcmp(file_path, NOIMG_ICON) == 0) {
		_vcui_view_common_show_noid_image(priv->contents);
		bshift_alert = EINA_TRUE;
	} else {
		{
			_vcui_show_wallpaper_image(priv->contents);
			priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, file_path);
		}
	}

	_vcui_show_call_bg_img(priv->contents);

	char *call_number = _vcui_doc_get_call_number(now_call_data);
	char *call_name = _vcui_doc_get_call_display_name(now_call_data);
	/* call name (if nothing, call number) */
	if (strlen(call_name) == 0) {
		_vcui_show_caller_info_name(vd, call_number, EINA_FALSE);
	} else {
		_vcui_show_caller_info_name(vd, call_name, EINA_FALSE);
		_vcui_show_caller_info_number(vd, call_number, EINA_FALSE);
	}

	_vcui_show_caller_info_icon(vd, EINA_FALSE);
	_vcui_show_caller_info_status(vd, _("IDS_CALL_POP_CALLING"), EINA_FALSE);

	_vcui_create_top_left_button_disabled(vd);
	_vcui_create_top_middle_button(vd);
	_vcui_create_top_right_button(vd);
	_vcui_create_bottom_left_button(vd);
	_vcui_create_bottom_middle_button_disabled(vd);

	_vcui_create_bottom_right_button(vd);

	_vcui_elements_check_keypad_n_hide(vd);

	evas_object_show(eo);
}

static Evas_Object *__vcui_view_dialing_create_contents(void *data, char *grpname)
{
	CALL_UI_KPI("__vcui_view_dialing_create_contents start");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *eo = NULL;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, grpname);
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
	Evas_Object *sep_ly = NULL;

	if (!view_data->layout) {

		/* Create Main Layout */
		view_data->layout = __vcui_view_dialing_create_layout_main(ad->win_main);

		if (view_data->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
		/* Create Contents */
		priv->contents = __vcui_view_dialing_create_contents(view_data, GRP_MOVIEW);
		elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);

		priv->caller_info = __vcui_view_dialing_create_contents(view_data, GRP_CALLER_INFO);
		elm_object_part_content_set(priv->contents, "caller_info", priv->caller_info);

		sep_ly = __vcui_view_dialing_create_contents(view_data, GRP_SEPARATOR_LAYOUT);
		elm_object_part_content_set(priv->contents, "btn_bg", sep_ly);

		priv->btn_ly = __vcui_view_dialing_create_contents(view_data, GRP_BUTTON_LAYOUT);
		elm_object_part_content_set(priv->contents, "btn_region", priv->btn_ly);

		evas_object_name_set(priv->contents, VIEW_DIALING_LAYOUT_ID);
		CALL_UI_DEBUG("[========== DIALVIEW: priv->contents Addr : [%p] ==========]", priv->contents);
	}

	__vcui_view_dialing_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_dialing_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("dialling view update");
	vcui_view_dialing_priv_t *priv = view_data->priv;

	__vcui_view_dialing_draw_screen(priv->contents, view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_dialing_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("dialling view hide");

	int valid = 0;
	vcui_view_dialing_priv_t *priv = (vcui_view_dialing_priv_t *)view_data->priv;
	valid = _vcui_check_valid_eo(priv->contents, VIEW_DIALING_LAYOUT_ID);

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
		if (priv->btn_ly) {
			evas_object_del(priv->btn_ly);
			priv->btn_ly = NULL;
		}

		if (priv->caller_info) {
			evas_object_del(priv->caller_info);
			priv->caller_info = NULL;
		}

		/*Delete keypad layout*/
		_vcui_keypad_delete_layout(priv->contents);

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

int	_vc_ui_view_dialing_check_valid_eo(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");

	vcui_view_dialing_priv_t *priv = NULL;
	int valid = -1;

	VCUI_RETURN_VALUE_IF_FAIL(vd , -1);
	priv = (vcui_view_dialing_priv_t *) vd->priv;

	valid = _vcui_check_valid_eo(priv->contents, VIEW_DIALING_LAYOUT_ID);

	return valid;
}

Evas_Object *_vc_ui_view_dialing_get_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_dialing_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (vcui_view_dialing_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->contents;
}

Evas_Object *_vc_ui_view_dialing_get_button_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_dialing_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (vcui_view_dialing_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->btn_ly;
}

Evas_Object *_vc_ui_view_dialing_get_caller_info(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_dialing_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (vcui_view_dialing_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->caller_info;
}

