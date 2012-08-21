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
#include "vcui-view-common.h"
#include "vcui-view-single-call.h"
#include "vcui-view-keypad.h"
#include "vcui-view-elements.h"

#define	VIEW_SINGLE_CALL_LAYOUT_ID "ONEVIEW"
typedef struct {
	Evas_Object *contents;
	Evas_Object *caller_info;
	Evas_Object *btn_ly;
	Evas_Object *ic;
	Evas_Object *record_btn;
} incall_one_view_priv_t;

static int __vc_ui_view_single_call_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vc_ui_view_single_call_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vc_ui_view_single_call_onhide(voice_call_view_data_t *view_data);
static int __vc_ui_view_single_call_onshow(voice_call_view_data_t *view_data);
static int __vc_ui_view_single_call_ondestroy(voice_call_view_data_t *view_data);
static Evas_Object *__vc_ui_view_single_call_create_contents(void *data, char *grpname);

static voice_call_view_data_t s_view = {
	.type = VIEW_INCALL_ONECALL_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vc_ui_view_single_call_oncreate,
	.onUpdate = __vc_ui_view_single_call_onupdate,
	.onHide = __vc_ui_view_single_call_onhide,
	.onShow = __vc_ui_view_single_call_onshow,
	.onDestroy = __vc_ui_view_single_call_ondestroy,
	.priv = NULL,
};

voice_call_view_data_t *_vc_ui_view_single_call_new(vcui_app_call_data_t *ad)
{
	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(incall_one_view_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!!");
	}

	return &s_view;
}

static void __vc_ui_view_single_call_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incall_one_view_priv_t *priv = (incall_one_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;
	call_data_t *call_data = _vcui_doc_get_first_call_data_from_list();

	if (call_data == NULL) {
		CALL_UI_DEBUG("call Data is NULL");
		return;
	}

	/* ============ Check valid Evas Object ============= */
	int valid = 0;
	CALL_UI_KPI("_vcui_check_valid_eo ONEVIEW start");
	valid = _vcui_check_valid_eo(priv->contents, VIEW_SINGLE_CALL_LAYOUT_ID);
	CALL_UI_KPI("_vcui_check_valid_eo ONEVIEW done");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== ONEVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
		return;
	}
	/* ============ Check valid Evas Object ============= */

	char *file_path = _vcui_doc_get_caller_id_file_path(call_data);
	int caller_status = _vcui_doc_get_call_status(call_data);

	/* call image */
	_vcui_delete_contact_image(priv->contents);
	if (strcmp(file_path, NOIMG_ICON) == 0) {
		_vcui_show_wallpaper_image(priv->contents);
	} else {
		CALL_UI_DEBUG("Caller ID file_path: %s", file_path);
		{
			_vcui_show_wallpaper_image(priv->contents);
			priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, file_path);
		}
	}

	_vcui_show_call_bg_img(priv->contents);
	_vcui_show_caller_info_status(data, _("IDS_CALL_BODY_ON_HOLD_ABB"), EINA_FALSE);
	if (caller_status == CALL_HOLD) {
		CALL_UI_DEBUG("=================HOLD====================== ");
		edje_object_signal_emit(_EDJ(priv->caller_info), "set-hold-state", "call-screen");
	} else {
		CALL_UI_DEBUG("=================UNHOLD====================== ");

		edje_object_signal_emit(_EDJ(priv->caller_info), "set-unhold-state", "call-screen");
	}

	char *call_number = _vcui_doc_get_call_number(call_data);
	char *call_name = _vcui_doc_get_call_display_name(call_data);
	/* call name (if nothing, call number) */
	CALL_UI_KPI("edje_object_part_text_set start");
	if (strlen(call_name) == 0) {
		_vcui_show_caller_info_name(vd, call_number, EINA_FALSE);
	} else {
		_vcui_show_caller_info_name(vd, call_name, EINA_FALSE);
		_vcui_show_caller_info_number(vd, call_number, EINA_FALSE);
	}
	CALL_UI_KPI("edje_object_part_text_set done");

	_vcui_show_caller_info_icon(vd, EINA_FALSE);

	CALL_UI_KPI("_vcui_view_common_set_each_time start");
	_vcui_view_common_set_each_time(_vcui_doc_get_call_start_time(call_data));
	CALL_UI_KPI("_vcui_view_common_set_each_time done");

	{
		CALL_UI_DEBUG("..");

		/*create a small button for Hold/Swap*/
		_vcui_create_hold_swap_button(vd);

		if ((_vcui_is_phonelock_status() == EINA_TRUE) && (_vcui_is_idle_lock() == EINA_TRUE)) {
			CALL_UI_KPI("_vcui_create_top_middle_button start");
			_vcui_create_top_middle_button_disabled(vd);
			CALL_UI_KPI("_vcui_create_top_middle_button done");

		} else {
			CALL_UI_KPI("_vcui_create_top_middle_button start");
			_vcui_create_top_middle_button(vd);
			CALL_UI_KPI("_vcui_create_top_middle_button done");

		}

		CALL_UI_KPI("_vcui_create_top_right_button start");
		_vcui_create_top_right_button(vd);
		CALL_UI_KPI("_vcui_create_top_right_button done");

		CALL_UI_KPI("_vcui_create_bottom_left_button start");
		_vcui_create_bottom_left_button(vd);
		CALL_UI_KPI("_vcui_create_bottom_left_button done");

		CALL_UI_KPI("_vcui_create_bottom_middle_button start");
		_vcui_create_bottom_middle_button(vd);
		CALL_UI_KPI("_vcui_create_bottom_middle_button done");
		{
			_vcui_create_top_left_button(vd);
			_vcui_create_bottom_right_button(vd);
		}
	}
	CALL_UI_KPI("edje_object_signal_emit effect start");
	if (ad->beffect_needed == EINA_TRUE) {
		edje_object_signal_emit(_EDJ(priv->contents), "SHOW_EFFECT", "ALLBTN");
		ad->beffect_needed = EINA_FALSE;
	} else {
		edje_object_signal_emit(_EDJ(priv->contents), "SHOW_NO_EFFECT", "ALLBTN");
	}
	CALL_UI_KPI("edje_object_signal_emit effect done");

	_vcui_elements_check_keypad_n_hide(vd);

	CALL_UI_KPI("evas_object_show start");
	evas_object_show(eo);
	CALL_UI_KPI("evas_object_show done");
}

static Evas_Object *__vc_ui_view_single_call_create_contents(void *data, char *grpname)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *eo = NULL;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, grpname);
	if (eo == NULL)
		return NULL;

	return eo;
}

static Evas_Object *__vc_ui_view_single_call_create_layout_main(Evas_Object *parent)
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

static int __vc_ui_view_single_call_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("incall view create");
	voice_call_view_data_t *vd = view_data;
	incall_one_view_priv_t *priv = (incall_one_view_priv_t *)vd->priv;
	call_data_t *call_data = _vcui_doc_get_first_call_data_from_list();
	Evas_Object *sep_ly = NULL;

	if (call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return VC_ERROR;
	}

	if (!vd->layout) {

		CALL_UI_KPI("__vc_ui_view_single_call_create_layout_main start");
		vd->layout = __vc_ui_view_single_call_create_layout_main(vd->app_data->win_main);
		CALL_UI_KPI("__vc_ui_view_single_call_create_layout_main done");
		if (vd->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}

		CALL_UI_KPI("__vc_ui_view_single_call_create_contents start");
		priv->contents = __vc_ui_view_single_call_create_contents(vd, GRP_INCALL);
		elm_object_part_content_set(vd->layout, "elm.swallow.content", priv->contents);
		CALL_UI_KPI("__vc_ui_view_single_call_create_contents done");

		sep_ly = __vc_ui_view_single_call_create_contents(vd, "separator-layout");
		elm_object_part_content_set(priv->contents, "btn_bg", sep_ly);

		priv->btn_ly = __vc_ui_view_single_call_create_contents(vd, GRP_BUTTON_LAYOUT);
		elm_object_part_content_set(priv->contents, "btn_region", priv->btn_ly);

		priv->caller_info = __vc_ui_view_single_call_create_contents(vd, GRP_CALLER_INFO);
		elm_object_part_content_set(priv->contents, "caller_info", priv->caller_info);

		CALL_UI_KPI("evas_object_name_set start");
		evas_object_name_set(priv->contents, VIEW_SINGLE_CALL_LAYOUT_ID);
		CALL_UI_KPI("evas_object_name_set done");
		CALL_UI_DEBUG("[========== ONEVIEW: priv->contents Addr : [%p] ==========]", priv->contents);

	} else {
		CALL_UI_DEBUG("[UI]ad->layout_incall==NULL case ");
		evas_object_show(vd->layout);
	}

	__vc_ui_view_single_call_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vc_ui_view_single_call_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("incall view update");

	__vc_ui_view_single_call_onshow(view_data);

	return VC_NO_ERROR;
}

static int __vc_ui_view_single_call_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("incall view hide");

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vc_ui_view_single_call_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("incall view show");
	CALL_UI_KPI("__vc_ui_view_single_call_onshow start");
	incall_one_view_priv_t *priv = (incall_one_view_priv_t *)view_data->priv;
	call_data_t *call_data = _vcui_doc_get_first_call_data_from_list();

	/*change the orientation to normal - 0 */
	elm_win_rotation_with_resize_set(view_data->app_data->win_main, 0);
	elm_win_rotation_with_resize_set(view_data->app_data->popup_mw, 0);

	if (call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return VC_ERROR;
	}
	CALL_UI_KPI("__vc_ui_view_single_call_draw_screen start");
	__vc_ui_view_single_call_draw_screen(priv->contents, view_data);
	CALL_UI_KPI("__vc_ui_view_single_call_draw_screen done");

	CALL_UI_KPI("incall-one-view hide start");
	evas_object_hide(view_data->layout);
	CALL_UI_KPI("incall-one-view hide done");
	CALL_UI_KPI("incall-one-view show start");
	evas_object_show(view_data->layout);
	CALL_UI_KPI("incall-one-view show done");
	CALL_UI_KPI("__vc_ui_view_single_call_onshow done");
	return VC_NO_ERROR;
}

static int __vc_ui_view_single_call_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("incall view destroy");

	vcui_app_call_data_t *ad = _vcui_get_app_data();

	voice_call_view_data_t *vd = ad->view_st[VIEW_INCALL_ONECALL_VIEW];
	incall_one_view_priv_t *priv = (incall_one_view_priv_t *)vd->priv;

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

	ad->view_st[VIEW_INCALL_ONECALL_VIEW] = NULL;

	_vcui_cache_flush();
	CALL_UI_DEBUG("complete destroy one view");
	return VC_NO_ERROR;
}

int	_vc_ui_view_single_call_check_valid_eo(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");

	incall_one_view_priv_t *priv = NULL;
	int valid = -1;

	VCUI_RETURN_VALUE_IF_FAIL(vd , -1);
	priv = (incall_one_view_priv_t *) vd->priv;

	valid = _vcui_check_valid_eo(priv->contents, VIEW_SINGLE_CALL_LAYOUT_ID);

	return valid;
}

Evas_Object *_vc_ui_view_single_call_get_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incall_one_view_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incall_one_view_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->contents;
}

Evas_Object *_vc_ui_view_single_call_get_button_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incall_one_view_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incall_one_view_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->btn_ly;
}

int	_vc_ui_view_single_call_get_call_status(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incall_one_view_priv_t *priv = NULL;
	call_data_t *call_data = _vcui_doc_get_first_call_data_from_list();
	int call_status = 0;

	VCUI_RETURN_INVALID_IF_FAIL(vd);
	priv = (incall_one_view_priv_t *) vd->priv;

	VCUI_RETURN_INVALID_IF_FAIL(call_data);
	call_status = _vcui_doc_get_call_status(call_data);
	return call_status;
}

void _vc_ui_view_single_call_set_call_timer(voice_call_view_data_t *vd, char *pcall_dur)
{
	incall_one_view_priv_t *priv = NULL;

	VCUI_RETURN_IF_FAIL(vd);
	priv = (incall_one_view_priv_t *) vd->priv;

	VCUI_RETURN_IF_FAIL(priv);
	edje_object_part_text_set(_EDJ(priv->caller_info), "txt_timer", _(pcall_dur));
}

Evas_Object *_vc_ui_view_single_call_get_caller_info(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incall_one_view_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incall_one_view_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->caller_info;
}

