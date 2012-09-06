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
#include "vcui-document.h"
#include "vcui-view-common.h"
#include "vcui-view-elements.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-keypad.h"

#define	VCUI_VIEW_MULTICALL_CONF_LAYOUT_ID "MULTIVIEWCONF"
typedef struct {
	Evas_Object *contents;
	Evas_Object *caller_info;
	Evas_Object *btn_ly;
	Evas_Object *cid_ly;
	Evas_Object *ic;
	Evas_Object *record_btn;
	vcui_app_call_status_t call_status;
	int	total_members;
} vcui_view_multi_call_conf_priv_t;

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

	priv->call_status = _vcui_doc_get_group_call_status();
	call_data = _vcui_doc_get_call_data_by_call_status(priv->call_status);
	if (call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return;
	}
	priv->total_members = _vcui_doc_get_hold_call_data_count() + _vcui_doc_get_unhold_call_data_count();

	elm_win_rotation_with_resize_set(ad->win_main, ad->rotate_angle);
	elm_win_rotation_with_resize_set(ad->popup_mw, ad->rotate_angle);

	if (ad->rotate_angle == 0 || ad->rotate_angle == 180) {
		CALL_UI_DEBUG("create portrait mode layout items");
		/* call image */
		_vcui_show_wallpaper_image(priv->contents);
		priv->ic = _vcui_show_default_image(priv->contents, vd->app_data->win_main, CONF_ICON);
		if (priv->call_status == CALL_HOLD) {
			if (ad->root_w == MAIN_WIN_HD_W && ad->root_h == MAIN_WIN_HD_H) {
				edje_object_signal_emit(_EDJ(priv->contents), "SHOW_DIM_HD", "CID_HD");
			} else if (ad->root_w == MAIN_WIN_WVGA_W && ad->root_h == MAIN_WIN_WVGA_H) {
				edje_object_signal_emit(_EDJ(priv->contents), "SHOW_DIM_WVGA", "CID_WVGA");
			}
			edje_object_signal_emit(_EDJ(priv->caller_info), "set-hold-state", "call-screen");
		} else {
			Evas_Object *txt_status = edje_object_part_swallow_get(_EDJ(priv->caller_info), "txt_status");
			if (txt_status) {
				edje_object_part_unswallow(_EDJ(priv->caller_info), txt_status);
				evas_object_del(txt_status);
			}
			edje_object_signal_emit(_EDJ(priv->contents), "HIDE_DIM", "CID");
			edje_object_signal_emit(_EDJ(priv->caller_info), "set-unhold-state", "call-screen");
		}

		_vcui_show_call_bg_img(priv->contents);
		_vcui_show_caller_info_status(data, _("IDS_CALL_BODY_ON_HOLD_ABB"), EINA_FALSE);
		_vcui_show_caller_info_name(vd, _("IDS_CALL_OPT_CONFERENCE_CALL"), EINA_FALSE);
		snprintf(buf, DEF_BUF_LEN, "%d %s", priv->total_members, _("IDS_CALL_BODY_PEOPLE"));
		_vcui_show_caller_info_number(vd, buf, EINA_FALSE);

		_vcui_show_caller_info_icon(vd, EINA_FALSE);

		_vcui_view_common_set_each_time(_vcui_doc_get_call_start_time(call_data));

		{
			CALL_UI_DEBUG("..");

			/*create a small button for Hold/Swap*/
			_vcui_create_hold_swap_button(vd);

			if ((_vcui_is_phonelock_status() == EINA_TRUE) && (_vcui_is_idle_lock() == EINA_TRUE)) {
				_vcui_create_top_middle_button_disabled(vd);
			} else {
				_vcui_create_top_middle_button(vd);
			}
			_vcui_create_top_right_button(vd);
			_vcui_create_bottom_left_button(vd);
			_vcui_create_bottom_middle_button(vd);

			{
				if (_vcui_view_common_is_emul_bin() == EINA_TRUE) {
					_vcui_create_top_left_button_disabled(vd);
					_vcui_create_bottom_right_button_disabled(vd);
				} else {
					_vcui_create_top_left_button(vd);
					_vcui_create_bottom_right_button(vd);
				}
			}
		}

		ad->beffect_needed = EINA_FALSE;
	} else if (ad->rotate_angle == 90 || ad->rotate_angle == 270) {
		CALL_UI_DEBUG("landscape mode not supported");
	}

	_vcui_elements_check_keypad_n_hide(vd);

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

	if (!vd->layout) {

		/* Create Main Layout */
		vd->layout = __vcui_view_multi_call_conf_create_layout_main(vd->app_data->win_main);
		if (vd->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
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
	Evas_Object *sep_ly = NULL;

	vcui_app_call_data_t *ad = view_data->app_data;
	/*if (priv->cid_ly) {
		CALL_UI_DEBUG("..");
		evas_object_del(priv->cid_ly);
		priv->cid_ly = NULL;
	}
	if (priv->contents) {
		evas_object_del(priv->contents);
		priv->contents = NULL;
	}*/

	if (ad->rotate_angle == 0 || ad->rotate_angle == 180) {
		CALL_UI_DEBUG("portrait mode layout");
		priv->contents = __vcui_view_multi_call_conf_create_contents(view_data, GRP_MULTICALL_CONF);
		priv->caller_info = __vcui_view_multi_call_conf_create_contents(view_data, GRP_CALLER_INFO);
		elm_object_part_content_set(priv->contents, "caller_info", priv->caller_info);
		sep_ly = __vcui_view_multi_call_conf_create_contents(view_data, GRP_SEPARATOR_LAYOUT);
		elm_object_part_content_set(priv->contents, "btn_bg", sep_ly);
		priv->btn_ly = __vcui_view_multi_call_conf_create_contents(view_data, GRP_BUTTON_LAYOUT);
		elm_object_part_content_set(priv->contents, "btn_region", priv->btn_ly);
	} else if (ad->rotate_angle == 90 || ad->rotate_angle == 270) {
		CALL_UI_DEBUG("landscape mode not supported");
	}
	elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);
	evas_object_name_set(priv->contents, VCUI_VIEW_MULTICALL_CONF_LAYOUT_ID);
	CALL_UI_DEBUG("[========== MULTIVIEWCONF: priv->contents Addr : [%p] ==========]", priv->contents);

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
		if (priv->cid_ly) {
			CALL_UI_DEBUG("..");
			evas_object_del(priv->cid_ly);
			priv->cid_ly = NULL;
		}
		if (priv->btn_ly) {
			evas_object_del(priv->btn_ly);
			priv->btn_ly = NULL;
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

	ad->view_st[VIEW_INCALL_MULTICALL_CONF_VIEW] = NULL;

	_vcui_cache_flush();
	CALL_UI_DEBUG("complete destroy multi view conf");

	return VC_NO_ERROR;
}

int	_vcui_view_multi_call_conf_check_valid_eo(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");

	vcui_view_multi_call_conf_priv_t *priv = NULL;
	int valid = -1;

	VCUI_RETURN_VALUE_IF_FAIL(vd , -1);
	priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;

	valid = _vcui_check_valid_eo(priv->contents, VCUI_VIEW_MULTICALL_CONF_LAYOUT_ID);

	return valid;
}

Evas_Object *_vcui_view_multi_call_conf_get_main_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_multi_call_conf_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->contents;
}

int	_vcui_view_multi_call_conf_get_call_status(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_multi_call_conf_priv_t *priv = NULL;
	int call_status = 0;

	VCUI_RETURN_INVALID_IF_FAIL(vd);
	priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;

	VCUI_RETURN_INVALID_IF_FAIL(priv);
	if (priv->call_status == CALL_HOLD) {
		call_status = CALL_HOLD;
	} else {
		call_status = CALL_UNHOLD;
	}

	return call_status;
}

void _vcui_view_multi_call_conf_set_call_timer(voice_call_view_data_t *vd, char *pcall_dur)
{
	vcui_view_multi_call_conf_priv_t *priv = NULL;

	VCUI_RETURN_IF_FAIL(vd);
	priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;

	VCUI_RETURN_IF_FAIL(priv);
	edje_object_part_text_set(_EDJ(priv->caller_info), "txt_timer", _(pcall_dur));
}

Evas_Object *_vcui_view_multi_call_conf_get_button_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_multi_call_conf_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->btn_ly;
}

Evas_Object *_vcui_view_multi_call_conf_get_cid_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_multi_call_conf_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->cid_ly;
}

int	_vcui_view_multi_call_conf_get_total_members(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_multi_call_conf_priv_t *priv = NULL;

	VCUI_RETURN_INVALID_IF_FAIL(vd);
	priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;

	VCUI_RETURN_INVALID_IF_FAIL(priv);

	return priv->total_members;
}

Evas_Object *_vc_ui_view_multi_call_conf_get_caller_info(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	vcui_view_multi_call_conf_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (vcui_view_multi_call_conf_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->caller_info;
}

