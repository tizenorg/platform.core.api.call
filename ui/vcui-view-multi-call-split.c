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
#include "vcui-view-multi-call-split.h"

static int __vcui_view_multi_call_split_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_multi_call_split_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_multi_call_split_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_multi_call_split_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_multi_call_split_ondestroy(voice_call_view_data_t *view_data);

static voice_call_view_data_t s_view = {
	.type = VIEW_INCALL_MULTICALL_SPLIT_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vcui_view_multi_call_split_oncreate,
	.onUpdate = __vcui_view_multi_call_split_onupdate,
	.onHide = __vcui_view_multi_call_split_onhide,
	.onShow = __vcui_view_multi_call_split_onshow,
	.onDestroy = __vcui_view_multi_call_split_ondestroy,
	.priv = NULL,
};

voice_call_view_data_t *_vcui_view_multi_call_split_new(vcui_app_call_data_t *ad)
{
	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(incall_multi_view_split_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!!");
	}

	return &s_view;
}

static void __vcui_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_view_change(VIEW_INCALL_MULTICALL_LIST_VIEW, 0, 0, 0);
}

static void __vcui_view_multi_call_split_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *)vd->priv;

	vcui_app_call_data_t *ad = vd->app_data;

	Evas_Object *sw1;
	Evas_Object *sw2;
	Evas_Object *sw3;
	Evas_Object *sw4;
	Evas_Object *play_ic;
	Evas_Object *pause_ic;
	Evas_Object *more_ic;

	priv->now_data_hold = _vcui_doc_get_last_status(CALL_HOLD);
	priv->now_data_unhold = _vcui_doc_get_last_status(CALL_UNHOLD);

	if ((priv->now_data_hold == NULL) || (priv->now_data_unhold == NULL)) {
		CALL_UI_DEBUG("call data is null");
		return;
	}

	sw1 = edje_object_part_swallow_get(_EDJ(eo), "swl_cid_1");
	if (sw1) {
		edje_object_part_unswallow(_EDJ(eo), sw1);
		evas_object_del(sw1);
	}
	sw2 = edje_object_part_swallow_get(_EDJ(eo), "swl_cid_2");
	if (sw2) {
		edje_object_part_unswallow(_EDJ(eo), sw2);
		evas_object_del(sw2);
	}
	sw3 = edje_object_part_swallow_get(_EDJ(eo), "txt_call_name_1");
	if (sw3) {
		edje_object_part_unswallow(_EDJ(eo), sw3);
		evas_object_del(sw3);
	}
	sw4 = edje_object_part_swallow_get(_EDJ(eo), "txt_call_name_2");
	if (sw4) {
		edje_object_part_unswallow(_EDJ(eo), sw4);
		evas_object_del(sw4);
	}
	play_ic = edje_object_part_swallow_get(_EDJ(eo), "swl_play");
	if (play_ic) {
		edje_object_part_unswallow(_EDJ(eo), play_ic);
		evas_object_del(play_ic);
	}
	pause_ic = edje_object_part_swallow_get(_EDJ(eo), "swl_pause");
	if (pause_ic) {
		edje_object_part_unswallow(_EDJ(eo), pause_ic);
		evas_object_del(pause_ic);
	}
	more_ic = edje_object_part_swallow_get(_EDJ(eo), "swl_more");
	if (more_ic) {
		edje_object_part_unswallow(_EDJ(eo), more_ic);
		evas_object_del(more_ic);
	}

	priv->ic_incall = elm_icon_add(vd->app_data->win_main);
	elm_object_part_content_set(eo, "swl_cid_1", priv->ic_incall);

	priv->ic_onhold = elm_icon_add(vd->app_data->win_main);
	elm_object_part_content_set(eo, "swl_cid_2", priv->ic_onhold);

	play_ic = elm_icon_add(vd->app_data->win_main);
	elm_object_part_content_set(eo, "swl_play", play_ic);
	pause_ic = elm_icon_add(vd->app_data->win_main);
	elm_object_part_content_set(eo, "swl_pause", pause_ic);
	more_ic = elm_icon_add(vd->app_data->win_main);
	elm_object_part_content_set(eo, "swl_more", more_ic);

	CALL_UI_DEBUG("priv->now_data_unhold->call_num:[%s]", priv->now_data_unhold->call_num);
	CALL_UI_DEBUG("priv->now_data_hold->call_num:[%s]", priv->now_data_hold->call_num);
	CALL_UI_DEBUG("priv->now_data_unhold->call_file_path:[%s]", priv->now_data_unhold->call_file_path);
	CALL_UI_DEBUG("priv->now_data_hold->call_file_path:[%s]", priv->now_data_hold->call_file_path);

	_vcui_show_wallpaper_image(priv->contents);
	if (_vcui_doc_get_count_unhold() == 1 && _vcui_doc_get_count_hold() == 1) {
		/* in call */
		elm_icon_file_set(priv->ic_incall, (char *)priv->now_data_unhold->call_file_path, NULL);

		if (strlen((char *)priv->now_data_unhold->call_display) == 0) {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name_1", (char *)priv->now_data_unhold->call_num);
		} else {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name_1", (char *)priv->now_data_unhold->call_display);
		}

		/* on hold */
		elm_icon_file_set(priv->ic_onhold, (char *)priv->now_data_hold->call_file_path, NULL);

		if (strlen((char *)priv->now_data_hold->call_display) == 0) {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name_2", (char *)priv->now_data_hold->call_num);
		} else {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name_2", (char *)priv->now_data_hold->call_display);
		}

		/* on hold callback */
		if (priv->ic_onhold) {
			evas_object_smart_callback_del(priv->ic_onhold, "clicked", _vcui_swap_btn_cb);
		}
		evas_object_smart_callback_add(priv->ic_onhold, "clicked", _vcui_swap_btn_cb, vd);
	} else if (_vcui_doc_get_count_unhold() == 1 && _vcui_doc_get_count_hold() > 1) {
		/* in call */
		elm_icon_file_set(priv->ic_incall, (char *)priv->now_data_unhold->call_file_path, NULL);

		if (strlen((char *)priv->now_data_unhold->call_display) == 0) {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name_1", (char *)priv->now_data_unhold->call_num);
		} else {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name_1", (char *)priv->now_data_unhold->call_display);
		}

		/* on hold */
		elm_icon_file_set(priv->ic_onhold, CONF_ICON, NULL);
		edje_object_part_text_set(_EDJ(eo), "txt_call_name_2", _("IDS_CALL_OPT_CONFERENCE_CALL"));

		/* on hold callback */
		if (priv->ic_onhold) {
			evas_object_smart_callback_del(priv->ic_onhold, "clicked", _vcui_swap_btn_cb);
		}
		evas_object_smart_callback_add(priv->ic_onhold, "clicked", _vcui_swap_btn_cb, vd);
	} else if (_vcui_doc_get_count_unhold() > 1 && _vcui_doc_get_count_hold() == 1) {
		/* in call */
		elm_icon_file_set(priv->ic_incall, CONF_ICON, NULL);
		edje_object_part_text_set(_EDJ(eo), "txt_call_name_1", _("IDS_CALL_OPT_CONFERENCE_CALL"));

		/* on hold */
		elm_icon_file_set(priv->ic_onhold, (char *)priv->now_data_hold->call_file_path, NULL);

		if (strlen((char *)priv->now_data_hold->call_display) == 0) {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name_2", (char *)priv->now_data_hold->call_num);
		} else {
			edje_object_part_text_set(_EDJ(eo), "txt_call_name_2", (char *)priv->now_data_hold->call_display);
		}

		/* on hold callback */
		if (priv->ic_onhold) {
			evas_object_smart_callback_del(priv->ic_onhold, "clicked", _vcui_swap_btn_cb);
		}
		evas_object_smart_callback_add(priv->ic_onhold, "clicked", _vcui_swap_btn_cb, vd);

		/* enter conf list view */
		if (priv->ic_incall) {
			evas_object_smart_callback_del(priv->ic_incall, "clicked", _vcui_conf_img_cb);
		}
		evas_object_smart_callback_add(priv->ic_incall, "clicked", _vcui_conf_img_cb, vd);
	} else {
		CALL_UI_DEBUG("[=========== ERROR : Invalid Status!!! ============]");
		return;
	}

	elm_icon_file_set(play_ic, PLAY_ICON, NULL);
	elm_icon_file_set(pause_ic, PAUSE_ICON, NULL);

	elm_icon_file_set(more_ic, MORE_ICON, NULL);
	evas_object_smart_callback_add(more_ic, "clicked", __vcui_more_btn_cb, vd);

	_vcui_view_common_set_each_time(priv->now_data_unhold->start_time);

	edje_object_part_text_set(_EDJ(eo), "txt_onhold", _("IDS_CALL_BODY_ON_HOLD_ABB"));

	if ((_vcui_doc_get_count_unhold() == 5 && _vcui_doc_get_count_hold() == 1) || (_vcui_doc_get_count_unhold() == 1 && _vcui_doc_get_count_hold() == 5)) {
		_vcui_create_top_right_button_disabled(vd);
	} else {
		_vcui_create_top_right_button(vd);
	}

	_vcui_create_top_left_button(vd);
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

static Evas_Object *__vcui_view_multi_call_split_create_contents(void *data, char *group)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	Evas_Object *eo = NULL;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, group);
	if (eo == NULL)
		return NULL;

	return eo;
}

static Evas_Object *__vcui_view_multi_call_split_create_layout_main(Evas_Object *parent)
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

static int __vcui_view_multi_call_split_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("multi-split view create");

	voice_call_view_data_t *vd = view_data;
	vcui_app_call_data_t *ad = vd->app_data;

	if (!vd->layout) {
		vd->layout = __vcui_view_multi_call_split_create_layout_main(vd->app_data->win_main);
		if (vd->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
	} else {
		CALL_UI_DEBUG("[UI]ad->layout_incall==NULL case ");
		evas_object_show(vd->layout);
	}

	__vcui_view_multi_call_split_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_split_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("multi-split view update");

	__vcui_view_multi_call_split_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_split_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multi-split view hide");

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_split_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multi-split view show");

	incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *)view_data->priv;
	vcui_app_call_data_t *ad = view_data->app_data;

	if (priv->contents) {
		evas_object_del(priv->contents);
		priv->contents = NULL;
	}

	CALL_UI_DEBUG("bswapped(%d)",ad->bswapped);
	if (ad->bswapped == 1) {
		priv->contents = __vcui_view_multi_call_split_create_contents(view_data, GRP_MULTICALL_SPLIT2);
	} else {
		priv->contents = __vcui_view_multi_call_split_create_contents(view_data, GRP_MULTICALL_SPLIT);
	}
	elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);
	evas_object_name_set(priv->contents, "MULTIVIEWSPLIT");
	CALL_UI_DEBUG("[========== MULTIVIEWSPLIT: priv->contents Addr : [%p] ==========]", priv->contents);

	__vcui_view_multi_call_split_draw_screen(priv->contents, view_data);

	evas_object_hide(view_data->layout);

	double scale_factor = 0.0;
	if (ad->bswapped == 1) {
		edje_object_signal_emit(_EDJ(priv->contents), "set-portrait", "incall-split2-view");
		if (_vcui_doc_get_count_unhold() > 1 && _vcui_doc_get_count_hold() == 1) {
			CALL_UI_DEBUG("show-more-portrait mode signal emit");
			edje_object_signal_emit(_EDJ(priv->contents), "show-more-portrait", "incall-split2-conf-call");
		}
	}
	else {
		edje_object_signal_emit(_EDJ(priv->contents), "set-portrait", "incall-split1-view");
		if (_vcui_doc_get_count_hold() > 1 && _vcui_doc_get_count_unhold() == 1) {
			CALL_UI_DEBUG("show-more-portrait mode signal emit");
			edje_object_signal_emit(_EDJ(priv->contents), "show-more-portrait", "incall-split1-conf-call");
		}
	}

	scale_factor = elm_config_scale_get();
	CALL_UI_DEBUG("scale_factor %f", scale_factor);
	if (scale_factor == 1.0) {
		edje_object_signal_emit(_EDJ(priv->contents), "show-swl-dim-hd", "cid-hd");
	} else {
		edje_object_signal_emit(_EDJ(priv->contents), "show-swl-dim-wvga", "cid-wvga");
	}
	evas_object_show(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_split_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multi-split view destroy");

	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = ad->view_st[VIEW_INCALL_MULTICALL_SPLIT_VIEW];

	incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *)vd->priv;
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
	ad->view_st[VIEW_INCALL_MULTICALL_SPLIT_VIEW] = NULL;

	_vcui_cache_flush();

	return VC_NO_ERROR;
}
