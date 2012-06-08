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
#include "vcui-view-elements.h"
#include "vcui-view-multi-call-conf.h"

static int __vcui_view_multi_call_conf_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_multi_call_conf_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_multi_call_conf_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_multi_call_conf_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_multi_call_conf_ondestroy(voice_call_view_data_t *view_data);

static voice_call_view_data_t s_view = {
	.type = VIEW_INCALL_MULTICALL_CONF_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vcui_view_multi_call_conf_oncreate,
	.onUpdate = __vcui_view_multi_call_conf_onupdate,
	.onHide = __vcui_view_multi_call_conf_onhide,
	.onShow = __vcui_view_multi_call_conf_onshow,
	.onDestroy = __vcui_view_multi_call_conf_ondestroy,
	.priv = NULL,
};

voice_call_view_data_t *_vcui_view_multi_call_conf_new(vcui_app_call_data_t *ad)
{
	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(vcui_view_multi_call_conf_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!! ");
	}

	return &s_view;
}

static void __vcui_view_multi_call_conf_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_multi_call_conf_priv_t *priv = (vcui_view_multi_call_conf_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;
	char buf[DEF_BUF_LEN] = { 0, };
	call_data_t *call_data = NULL;

	priv->call_status = _vcui_doc_get_show_callstatus();
	call_data = _vcui_doc_get_last_status(priv->call_status);
	if (call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return;
	}
	priv->total_members = _vcui_doc_get_count_hold()+_vcui_doc_get_count_unhold();

		/* call image */
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
		evas_object_smart_callback_add(priv->ic, "clicked", _vcui_conf_img_cb, vd);

		edje_object_part_text_set(_EDJ(eo), "txt_call_name", _("IDS_CALL_OPT_CONFERENCE_CALL"));
		snprintf(buf, DEF_BUF_LEN, "%d %s", priv->total_members, _("IDS_CALL_BODY_PEOPLE"));
		edje_object_part_text_set(_EDJ(eo), "txt_contact_phone_type", buf);		/* in case of multicallconf, it is num of people */

		_vcui_view_common_set_each_time(call_data->start_time);

		_vcui_create_top_left_button(vd);
		_vcui_create_top_right_button(vd);
		_vcui_create_bottom_left_button(vd);
		_vcui_create_bottom_middle_button(vd);
		if ((_vcui_is_phonelock_status() == EINA_TRUE)&&(_vcui_is_idle_lock() == EINA_TRUE)) {
			_vcui_create_top_middle_button_disabled(vd);
			_vcui_create_bottom_right_button_disabled(vd);
		} else {
			_vcui_create_top_middle_button(vd);
			_vcui_create_bottom_right_button(vd);
		}
		_vcui_create_button_bigend(vd);

		ad->beffect_needed = EINA_FALSE;

	evas_object_show(eo);
}

static Evas_Object *__vcui_view_multi_call_conf_create_contents(void *data, char *group)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	Evas_Object *eo;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, group);
	if (eo == NULL)
		return NULL;

	return eo;
}

static Evas_Object *__vcui_view_multi_call_conf_create_layout_main(Evas_Object *parent)
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

static int __vcui_view_multi_call_conf_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	voice_call_view_data_t *vd = view_data;
	vcui_view_multi_call_conf_priv_t *priv = (vcui_view_multi_call_conf_priv_t *)vd->priv;

	if (!vd->layout) {

		/* Create Main Layout */
		vd->layout = __vcui_view_multi_call_conf_create_layout_main(vd->app_data->win_main);
		if (vd->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
		/* Create Contents */
		priv->contents = __vcui_view_multi_call_conf_create_contents(vd, GRP_MULTICALL_CONF);
		elm_object_part_content_set(vd->layout, "elm.swallow.content", priv->contents);

		evas_object_name_set(priv->contents, "MULTIVIEWCONF");
		CALL_UI_DEBUG("[========== MULTIVIEWCONF: priv->contents Addr : [%p] ==========]", priv->contents);
	} else {
		CALL_UI_DEBUG("[UI]ad->layout_multicallconf==NULL case ");
		evas_object_show(vd->layout);
	}

	__vcui_view_multi_call_conf_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_conf_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("multicall-conf view update");

	__vcui_view_multi_call_conf_onshow(view_data);

	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_conf_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multicall-conf view hide");

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_conf_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multicall-conf view show");

	vcui_view_multi_call_conf_priv_t *priv = (vcui_view_multi_call_conf_priv_t *)view_data->priv;

	__vcui_view_multi_call_conf_draw_screen(priv->contents, view_data);

	evas_object_hide(view_data->layout);
	evas_object_show(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_conf_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multicall-conf view destroy");
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	voice_call_view_data_t *vd = ad->view_st[VIEW_INCALL_MULTICALL_CONF_VIEW];
	vcui_view_multi_call_conf_priv_t *priv = (vcui_view_multi_call_conf_priv_t *)vd->priv;

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

	ad->view_st[VIEW_INCALL_MULTICALL_CONF_VIEW] = NULL;

	_vcui_cache_flush();
	CALL_UI_DEBUG("complete destroy multi view conf");

	return VC_NO_ERROR;
}
