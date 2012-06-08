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
#include "vcui-view-callend.h"
#include "vcui-view-elements.h"

static call_data_t *call_data = NULL;

static int __vcui_view_callend_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_callend_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_callend_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_callend_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_callend_ondestroy(voice_call_view_data_t *view_data);

static voice_call_view_data_t s_view = {
	.type = VIEW_ENDCALL_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vcui_view_callend_oncreate,
	.onUpdate = __vcui_view_callend_onupdate,
	.onHide = __vcui_view_callend_onhide,
	.onShow = __vcui_view_callend_onshow,
	.onDestroy = __vcui_view_callend_ondestroy,
	.priv = NULL,
};

voice_call_view_data_t *_vcui_view_callend_new(vcui_app_call_data_t *ad)
{
	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(endcall_view_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!!");
	}

	return &s_view;
}

static void __vcui_view_callend_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	endcall_view_priv_t *priv = (endcall_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;

	/* ============ Check valid Evas Object ============= */
	int valid = 0;
	valid = _vcui_check_valid_eo(priv->contents, "ENDCALLVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== ENDCALLVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
		return;
	}
	/* ============ Check valid Evas Object ============= */

	if (ad->call_end_type == CALL_END_TYPE_SINGLE_CALL) {
		CALL_UI_DEBUG("CALL_END_TYPE_SINGLE_CALL type end screen");

		/* call image */
		_vcui_delete_contact_image(priv->contents);
		if (strcmp((char *)call_data->call_file_path, NOIMG_ICON) == 0 && strcmp((char *)call_data->call_full_file_path, NOIMG_ICON) == 0) {
			_vcui_show_wallpaper_image(priv->contents);
		} else {
			if (strcmp((char *)call_data->call_full_file_path, NOIMG_ICON) == 0) {	/* normal size */
				_vcui_show_wallpaper_image(priv->contents);
				priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, call_data->call_file_path);
			} else {		/* full size */
				_vcui_set_full_image(priv->contents, vd->app_data->win_main, call_data->call_full_file_path);
			}
		}

		/* call name (if nothing, call number) */
		if (strlen((char *)call_data->call_display) == 0) {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)call_data->call_num);
			edje_object_signal_emit(_EDJ(eo), "show-all-buttons", "end-screen");
		} else {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)call_data->call_display);
			edje_object_part_text_set(_EDJ(eo), "txt_contact_phone_type", (char *)call_data->call_num);
			edje_object_signal_emit(_EDJ(eo), "show-some-buttons", "end-screen");
		}

		_vcui_view_common_set_each_time(call_data->start_time);
	} else if (ad->call_end_type == CALL_END_TYPE_CONF_CALL) {
		CALL_UI_DEBUG("CALL_END_TYPE_CONF_CALL type end screen");
		char buf[DEF_BUF_LEN] = { 0, };

		/* Conference image */
		_vcui_show_wallpaper_image(priv->contents);
		priv->ic = _vcui_show_default_image(priv->contents, vd->app_data->win_main, CONF_ICON);
		if (priv->call_status == CALL_HOLD) {
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

		edje_object_part_text_set(_EDJ(eo), "txt_call_name", _("IDS_CALL_OPT_CONFERENCE_CALL"));
		snprintf(buf, DEF_BUF_LEN, "%d %s", (_vcui_doc_get_count_hold() + _vcui_doc_get_count_unhold()), _("IDS_CALL_BODY_PEOPLE"));
		edje_object_part_text_set(_EDJ(eo), "txt_contact_phone_type", buf);		/* in case of multicallconf, it is num of people */

		_vcui_create_top_left_button_disabled(vd);
		_vcui_create_top_middle_button_disabled(vd);
		_vcui_create_top_right_button_disabled(vd);
		_vcui_create_bottom_left_button_disabled(vd);
		_vcui_create_bottom_middle_button_disabled(vd);
		_vcui_create_bottom_right_button_disabled(vd);

		_vcui_create_button_bigend_disabled(vd);

	} else {
		CALL_UI_DEBUG("invalid type... return");
		return;
	}

	evas_object_show(eo);
}

static Evas_Object *__vcui_view_callend_create_contents(void *data)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	Evas_Object *eo = NULL;

	/* load edje */
	if (ad->call_end_type == CALL_END_TYPE_SINGLE_CALL)
		eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, GRP_END_SINGLECALL);
	else if (ad->call_end_type == CALL_END_TYPE_CONF_CALL)
		eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, GRP_END_CONFCALL);

	if (eo == NULL)
		return NULL;

	return eo;
}

static Evas_Object *__vcui_view_callend_create_layout_main(Evas_Object *parent)
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

static int __vcui_view_callend_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("endcall view create");
	voice_call_view_data_t *vd = view_data;
	vcui_app_call_data_t *ad = vd->app_data;
	endcall_view_priv_t *priv = (endcall_view_priv_t *)vd->priv;

	CALL_UI_DEBUG("call_end_type[%d]", ad->call_end_type);
	if (ad->call_end_type == CALL_END_TYPE_SINGLE_CALL) {
		call_data = (call_data_t *)param2;
	} else if (ad->call_end_type == CALL_END_TYPE_CONF_CALL) {
		priv->call_status = _vcui_doc_get_show_callstatus();
		call_data = _vcui_doc_get_last_status(priv->call_status);
	}

	if (call_data == NULL) {
		CALL_UI_DEBUG("call Data is NULL");
		return;
	}

	if (!vd->layout) {

		/* Create Main Layout */
		vd->layout = __vcui_view_callend_create_layout_main(vd->app_data->win_main);
		if (vd->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
		/* Create Contents */
		priv->contents = __vcui_view_callend_create_contents(vd);
		elm_object_part_content_set(vd->layout, "elm.swallow.content", priv->contents);

		evas_object_name_set(priv->contents, "ENDCALLVIEW");
		CALL_UI_DEBUG("[========== ENDCALLVIEW: priv->contents Addr : [%p] ==========]", priv->contents);

	} else {
		CALL_UI_DEBUG("[UI]ad->layout_end call==NULL case ");
		evas_object_show(vd->layout);
	}

	__vcui_view_callend_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_callend_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("end call view update");

	__vcui_view_callend_onshow(view_data);

	return VC_NO_ERROR;
}

static int __vcui_view_callend_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("end call view hide");

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_callend_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("end call view show");
	CALL_UI_KPI("__vcui_view_callend_onshow start");
	endcall_view_priv_t *priv = (endcall_view_priv_t *)view_data->priv;

	__vcui_view_callend_draw_screen(priv->contents, view_data);

	evas_object_hide(view_data->layout);
	evas_object_show(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_callend_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("endcall view destroy");

	vcui_app_call_data_t *ad = _vcui_get_app_data();

	voice_call_view_data_t *vd = ad->view_st[VIEW_ENDCALL_VIEW];
	endcall_view_priv_t *priv = (endcall_view_priv_t *)vd->priv;

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

	ad->view_st[VIEW_ENDCALL_VIEW] = NULL;
	call_data = NULL;
	ad->call_end_type = CALL_END_TYPE_NONE;

	_vcui_cache_flush();
	CALL_UI_DEBUG("complete destroy one view");
	return VC_NO_ERROR;
}
