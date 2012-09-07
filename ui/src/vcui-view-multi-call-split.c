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
#include "vcui-view-elements.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-keypad.h"

#define	VCUI_VIEW_MULTICALL_SPLIT_LAYOUT_ID "MULTIVIEWSPLIT"
typedef struct {
	Evas_Object *contents;
	Evas_Object *caller_info_unhold;
	Evas_Object *caller_info_hold;
	Evas_Object *btn_ly;

	Evas_Object *record_btn;

} incall_multi_view_split_priv_t;

static int __vcui_view_multi_call_split_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_multi_call_split_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_multi_call_split_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_multi_call_split_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_multi_call_split_ondestroy(voice_call_view_data_t *view_data);
static void __vcui_multi_view_split_rotation_with_resize();

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

static void __vcui_view_multi_call_split_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *)vd->priv;

	vcui_app_call_data_t *ad = vd->app_data;

	call_data_t *hold_call_data = NULL;
	call_data_t *unhold_call_data = NULL;

	char buf[DEF_BUF_LEN] = { 0, };

	hold_call_data = _vcui_doc_get_call_data_by_call_status(CALL_HOLD);
	unhold_call_data = _vcui_doc_get_call_data_by_call_status(CALL_UNHOLD);

	if ((hold_call_data == NULL) || (unhold_call_data == NULL)) {
		CALL_UI_DEBUG("call data is null");
		return;
	}

	char *hold_file_path = _vcui_doc_get_caller_id_file_path(hold_call_data);
	char *hold_full_file_path = _vcui_doc_get_caller_id_full_file_path(hold_call_data);
	char *unhold_file_path = _vcui_doc_get_caller_id_file_path(unhold_call_data);
	char *unhold_full_file_path = _vcui_doc_get_caller_id_full_file_path(unhold_call_data);
	char *hold_call_number = _vcui_doc_get_call_number(hold_call_data);
	char *hold_call_name = _vcui_doc_get_call_display_name(hold_call_data);
	char *unhold_call_number = _vcui_doc_get_call_number(unhold_call_data);
	char *unhold_call_name = _vcui_doc_get_call_display_name(unhold_call_data);

	if (_vcui_doc_get_unhold_call_data_count() == 1 && _vcui_doc_get_hold_call_data_count() == 1) {
		/* in call */
		_vcui_show_contact_image_split(eo, unhold_file_path, unhold_full_file_path, EINA_FALSE);

		if (strlen(unhold_call_name) == 0) {
			_vcui_show_caller_info_name(vd, unhold_call_number, EINA_FALSE);
			_vcui_show_caller_info_number(vd, NULL, EINA_FALSE);
		} else {
			_vcui_show_caller_info_name(vd, unhold_call_name, EINA_FALSE);
			_vcui_show_caller_info_number(vd, unhold_call_number, EINA_FALSE);
		}

		/* on hold */
		_vcui_show_contact_image_split(eo, hold_file_path, hold_full_file_path, EINA_TRUE);
		if (strlen(hold_call_name) == 0) {
			_vcui_show_caller_info_name(vd, hold_call_number, EINA_TRUE);
			_vcui_show_caller_info_number(vd, NULL, EINA_TRUE);
		} else {
			_vcui_show_caller_info_name(vd, hold_call_name, EINA_TRUE);
			_vcui_show_caller_info_number(vd, hold_call_number, EINA_TRUE);
		}

	} else if (_vcui_doc_get_unhold_call_data_count() == 1 && _vcui_doc_get_hold_call_data_count() > 1) {
		/* in call */
		_vcui_show_contact_image_split(eo, hold_file_path, hold_full_file_path, EINA_FALSE);

		if (strlen(unhold_call_name) == 0) {
			_vcui_show_caller_info_name(vd, unhold_call_number, EINA_FALSE);
			_vcui_show_caller_info_number(vd, NULL, EINA_FALSE);
		} else {
			_vcui_show_caller_info_name(vd, unhold_call_name, EINA_FALSE);
			_vcui_show_caller_info_number(vd, unhold_call_number, EINA_FALSE);
		}

		/* on hold */
		_vcui_show_contact_image_split(eo, CONF_SPLIT_ICON, CONF_SPLIT_ICON, EINA_TRUE);
		_vcui_show_caller_info_name(vd, _("IDS_CALL_OPT_CONFERENCE_CALL"), EINA_TRUE);
		snprintf(buf, DEF_BUF_LEN, "%d %s", _vcui_doc_get_hold_call_data_count(), _("IDS_CALL_BODY_PEOPLE"));
		_vcui_show_caller_info_number(vd, buf, EINA_TRUE);

	} else if (_vcui_doc_get_unhold_call_data_count() > 1 && _vcui_doc_get_hold_call_data_count() == 1) {
		/* in call */
		_vcui_show_contact_image_split(eo, CONF_SPLIT_ICON, CONF_SPLIT_ICON, EINA_FALSE);
		_vcui_show_caller_info_name(vd, _("IDS_CALL_OPT_CONFERENCE_CALL"), EINA_FALSE);
		snprintf(buf, DEF_BUF_LEN, "%d %s", _vcui_doc_get_unhold_call_data_count(), _("IDS_CALL_BODY_PEOPLE"));
		_vcui_show_caller_info_number(vd, buf, EINA_FALSE);

		/* on hold */
		_vcui_show_contact_image_split(eo, hold_file_path, hold_full_file_path, EINA_TRUE);
		if (strlen(hold_call_name) == 0) {
			_vcui_show_caller_info_name(vd, hold_call_number, EINA_TRUE);
			_vcui_show_caller_info_number(vd, NULL, EINA_TRUE);
		} else {
			_vcui_show_caller_info_name(vd, hold_call_name, EINA_TRUE);
			_vcui_show_caller_info_number(vd, hold_call_number, EINA_TRUE);
		}

		/* enter conf list view */
	} else {
		CALL_UI_DEBUG("[=========== ERROR : Invalid Status!!! ============]");
		return;
	}

	_vcui_show_call_bg_img(priv->contents);
	_vcui_show_caller_info_status(vd, _("IDS_CALL_BODY_ON_HOLD_ABB"), EINA_TRUE);

	_vcui_show_caller_info_icon(vd, EINA_FALSE);

	_vcui_view_common_set_each_time(_vcui_doc_get_call_start_time(unhold_call_data));

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
		if ((_vcui_doc_get_unhold_call_data_count() == 5 && _vcui_doc_get_hold_call_data_count() == 1)
			|| (_vcui_doc_get_unhold_call_data_count() == 1 && _vcui_doc_get_hold_call_data_count() == 5)) {
			_vcui_create_top_left_button_disabled(vd);
		} else {
			_vcui_create_top_left_button(vd);
		}
		if (_vcui_view_common_is_emul_bin() == EINA_TRUE) {
			_vcui_create_bottom_right_button_disabled(vd);
		} else {
			_vcui_create_bottom_right_button(vd);
		}
	}

	ad->beffect_needed = EINA_FALSE;

	__vcui_multi_view_split_rotation_with_resize();

	_vcui_elements_check_keypad_n_hide(vd);

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
	incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *)view_data->priv;

	if (!vd->layout) {
		Evas_Object * sep_ly = NULL;
		vd->layout = __vcui_view_multi_call_split_create_layout_main(vd->app_data->win_main);
		if (vd->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}

		priv->contents = __vcui_view_multi_call_split_create_contents(view_data, GRP_MULTICALL_SPLIT);
		elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);

		sep_ly = __vcui_view_multi_call_split_create_contents(view_data, GRP_SEPARATOR_LAYOUT);
		elm_object_part_content_set(priv->contents, "btn_bg", sep_ly);

		priv->btn_ly = __vcui_view_multi_call_split_create_contents(view_data, GRP_BUTTON_LAYOUT);
		elm_object_part_content_set(priv->contents, "btn_region", priv->btn_ly);

		priv->caller_info_unhold = __vcui_view_multi_call_split_create_contents(view_data, GRP_CALLER_INFO);
		elm_object_part_content_set(priv->contents, "caller_info_unhold", priv->caller_info_unhold);
		edje_object_signal_emit(_EDJ(priv->caller_info_unhold), "set-unhold-state", "call-screen");

		priv->caller_info_hold = __vcui_view_multi_call_split_create_contents(view_data, GRP_CALLER_INFO);
		elm_object_part_content_set(priv->contents, "caller_info_hold", priv->caller_info_hold);
		edje_object_signal_emit(_EDJ(priv->caller_info_hold), "set-hold-caller_ifno", "call-screen");

		evas_object_name_set(priv->contents, VCUI_VIEW_MULTICALL_SPLIT_LAYOUT_ID);
		CALL_UI_DEBUG("[========== MULTIVIEWSPLIT: priv->contents Addr : [%p] ==========]", priv->contents);

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

	__vcui_view_multi_call_split_draw_screen(priv->contents, view_data);

	evas_object_hide(view_data->layout);
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
		if (priv->btn_ly) {
			evas_object_del(priv->btn_ly);
			priv->btn_ly = NULL;
		}

		if (priv->caller_info_hold) {
			evas_object_del(priv->caller_info_hold);
			priv->caller_info_hold = NULL;
		}

		if (priv->caller_info_unhold) {
			evas_object_del(priv->caller_info_unhold);
			priv->caller_info_unhold = NULL;
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
	ad->view_st[VIEW_INCALL_MULTICALL_SPLIT_VIEW] = NULL;

	_vcui_cache_flush();

	return VC_NO_ERROR;
}

static void __vcui_multi_view_split_rotation_with_resize()
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = ad->view_st[VIEW_INCALL_MULTICALL_SPLIT_VIEW];
	incall_multi_view_split_priv_t *priv = (incall_multi_view_split_priv_t *)vd->priv;

	elm_win_rotation_with_resize_set(ad->win_main, ad->rotate_angle);
	elm_win_rotation_with_resize_set(ad->popup_mw, ad->rotate_angle);

	if (ad->rotate_angle == 0 || ad->rotate_angle == 180) {
		CALL_UI_DEBUG("portrait mode signal emit");
		edje_object_signal_emit(_EDJ(priv->contents), "set-portrait", "incall-split-view");
	} else if (ad->rotate_angle == 90 || ad->rotate_angle == 270) {
		CALL_UI_DEBUG("landscape mode not supported");
	}

	evas_object_show(vd->layout);
}

int	_vcui_view_multi_call_split_check_valid_eo(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");

	incall_multi_view_split_priv_t *priv = NULL;
	int valid = -1;

	VCUI_RETURN_VALUE_IF_FAIL(vd , -1);
	priv = (incall_multi_view_split_priv_t *) vd->priv;

	valid = _vcui_check_valid_eo(priv->contents, VCUI_VIEW_MULTICALL_SPLIT_LAYOUT_ID);

	return valid;
}

Evas_Object *_vcui_view_multi_call_split_get_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incall_multi_view_split_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incall_multi_view_split_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->contents;
}

Evas_Object *_vcui_view_multi_call_split_get_button_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incall_multi_view_split_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incall_multi_view_split_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->btn_ly;
}

void _vcui_view_multi_call_split_set_call_timer(voice_call_view_data_t *vd, char *pcall_dur)
{
	incall_multi_view_split_priv_t *priv = NULL;

	VCUI_RETURN_IF_FAIL(vd);
	priv = (incall_multi_view_split_priv_t *) vd->priv;

	VCUI_RETURN_IF_FAIL(priv);
	edje_object_part_text_set(_EDJ(priv->caller_info_unhold), "txt_timer", _(pcall_dur));
}

Evas_Object *_vc_ui_view_multi_call_split_get_caller_info_hold(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incall_multi_view_split_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incall_multi_view_split_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->caller_info_hold;
}

Evas_Object *_vc_ui_view_multi_call_split_get_caller_info_unhold(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incall_multi_view_split_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incall_multi_view_split_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->caller_info_unhold;
}


