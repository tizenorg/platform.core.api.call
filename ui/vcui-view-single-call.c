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
#include "vcui-view-elements.h"
 
static int __vc_ui_view_single_call_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vc_ui_view_single_call_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vc_ui_view_single_call_onhide(voice_call_view_data_t *view_data);
static int __vc_ui_view_single_call_onshow(voice_call_view_data_t *view_data);
static int __vc_ui_view_single_call_ondestroy(voice_call_view_data_t *view_data);

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

	if (priv->now_data == NULL) {
		CALL_UI_DEBUG("Now Data is NULL");
		return;
	}

	/* ============ Check valid Evas Object ============= */
	int valid = 0;
	CALL_UI_KPI("_vcui_check_valid_eo ONEVIEW start");
	valid = _vcui_check_valid_eo(priv->contents, "ONEVIEW");
	CALL_UI_KPI("_vcui_check_valid_eo ONEVIEW done");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== ONEVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
		return;
	}
	/* ============ Check valid Evas Object ============= */

	/* call image */
	if (strcmp((char *)priv->now_data->call_file_path, NOIMG_ICON) == 0) {
		_vcui_show_wallpaper_image(priv->contents);
	} else {
		CALL_UI_DEBUG("some image case!!");
		_vcui_show_wallpaper_image(priv->contents);
		priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, priv->now_data->call_file_path);
		if (priv->now_data->caller_status == CALL_HOLD) {
			double scale_factor = 0.0;
			scale_factor = elm_config_scale_get();
			CALL_UI_DEBUG("scale_factor %f", scale_factor);

			if (scale_factor == 1.0) {
				edje_object_signal_emit(_EDJ(priv->contents), "SHOW_DIM_HD", "CID_HD");
			} else {
				edje_object_signal_emit(_EDJ(priv->contents), "SHOW_DIM_WVGA", "CID_WVGA");
			}
		} else {
			edje_object_signal_emit(_EDJ(priv->contents), "HIDE_DIM", "CID");
		}
	}

	/* call name (if nothing, call number) */
	CALL_UI_KPI("edje_object_part_text_set start");
	if (strlen((char *)priv->now_data->call_display) == 0) {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_num);
	} else {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_display);
		edje_object_part_text_set(_EDJ(eo), "txt_contact_phone_type", (char *)priv->now_data->call_num);
	}
	CALL_UI_KPI("edje_object_part_text_set done");

	CALL_UI_KPI("_vcui_view_common_set_each_time start");
	_vcui_view_common_set_each_time(priv->now_data->start_time);
	CALL_UI_KPI("_vcui_view_common_set_each_time done");

	CALL_UI_KPI("_vcui_create_top_left_button start");
	_vcui_create_top_left_button(vd);
	CALL_UI_KPI("_vcui_create_top_left_button done");

	CALL_UI_KPI("_vcui_create_top_right_button start");
	_vcui_create_top_right_button(vd);
	CALL_UI_KPI("_vcui_create_top_right_button done");

	CALL_UI_KPI("_vcui_create_bottom_left_button start");
	_vcui_create_bottom_left_button(vd);
	CALL_UI_KPI("_vcui_create_bottom_left_button done");

	CALL_UI_KPI("_vcui_create_bottom_middle_button start");
	_vcui_create_bottom_middle_button(vd);
	CALL_UI_KPI("_vcui_create_bottom_middle_button done");

	if ((_vcui_is_phonelock_status() == EINA_TRUE)&&(_vcui_is_idle_lock() == EINA_TRUE)) {
		CALL_UI_KPI("_vcui_create_top_middle_button start");
		_vcui_create_top_middle_button_disabled(vd);
		CALL_UI_KPI("_vcui_create_top_middle_button done");

		CALL_UI_KPI("_vcui_create_bottom_right_button start");
		_vcui_create_bottom_right_button_disabled(vd);
		CALL_UI_KPI("_vcui_create_bottom_right_button done");
	} else {
		CALL_UI_KPI("_vcui_create_top_middle_button start");
		_vcui_create_top_middle_button(vd);
		CALL_UI_KPI("_vcui_create_top_middle_button done");

		_vcui_create_bottom_right_button(vd);
 	}

	CALL_UI_KPI("_vcui_create_button_bigend start");
	_vcui_create_button_bigend(vd);
	CALL_UI_KPI("_vcui_create_button_bigend done");

	CALL_UI_KPI("edje_object_signal_emit effect start");
	if (ad->beffect_needed == EINA_TRUE) {
		edje_object_signal_emit(_EDJ(priv->contents), "SHOW_EFFECT", "ALLBTN");
		ad->beffect_needed = EINA_FALSE;
	} else {
		edje_object_signal_emit(_EDJ(priv->contents), "SHOW_NO_EFFECT", "ALLBTN");
	}
	CALL_UI_KPI("edje_object_signal_emit effect done");

	CALL_UI_KPI("evas_object_show start");
	evas_object_show(eo);
	CALL_UI_KPI("evas_object_show done");
}

static Evas_Object *__vc_ui_view_single_call_create_contents(void *data)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *eo = NULL;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, GRP_INCALL);
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

	priv->now_data = _vcui_doc_get_first();
	if (priv->now_data == NULL) {
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
		priv->contents = __vc_ui_view_single_call_create_contents(vd);
		elm_object_part_content_set(vd->layout, "elm.swallow.content", priv->contents);
		CALL_UI_KPI("__vc_ui_view_single_call_create_contents done");

		CALL_UI_KPI("evas_object_name_set start");
		evas_object_name_set(priv->contents, "ONEVIEW");
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

	priv->now_data = _vcui_doc_get_first();
	if (priv->now_data == NULL) {
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
