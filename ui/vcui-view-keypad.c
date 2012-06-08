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
#include "vcui-view-keypad.h"
#include "vcui-view-common.h"

static int _vcui_view_keypad_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int _vcui_view_keypad_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int _vcui_view_keypad_onhide(voice_call_view_data_t *view_data);
static int _vcui_view_keypad_onshow(voice_call_view_data_t *view_data);
static int _vcui_view_keypad_ondestroy(voice_call_view_data_t *view_data);
static Evas_Object *__vcui_view_keypad_create_single_line_scrolled_entry(void *content);

static voice_call_view_data_t s_view = {
	.type = VIEW_INCALL_KEYPAD_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = _vcui_view_keypad_oncreate,
	.onUpdate = _vcui_view_keypad_onupdate,
	.onHide = _vcui_view_keypad_onhide,
	.onShow = _vcui_view_keypad_onshow,
	.onDestroy = _vcui_view_keypad_ondestroy,
	.priv = NULL,
};

static void __vcui_view_keypad_on_keypad_down(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	CALL_UI_DEBUG("source(%s)",source);

	char before_dest[KEYPAD_STR_DEFINE_OPEN_SIZE+KEYPAD_ENTRY_DISP_DATA_SIZE+1] = { 0, };
	char *sub_buffer_pointer = NULL;
	char entry_dest[KEYPAD_ENTRY_SET_DATA_SIZE+1] = { 0, };
	char *keypad_source;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_keypad_priv_t *priv = NULL;

	if (NULL == vd) {
		CALL_UI_DEBUG("\n data is null \n");
		return;
	}
	priv = (vcui_view_keypad_priv_t *)vd->priv;

	if (strlen(source) >= KEYPAD_ENTRY_DISP_DATA_SIZE) {
		CALL_UI_DEBUG("strlen(source) >= DATA_BUF_SIZE(%d)",KEYPAD_ENTRY_DISP_DATA_SIZE);
		return;
	}

	if (strcmp(source, "star") == 0) {
		keypad_source = "*";
	} else if (strcmp(source, "sharp") == 0) {
		keypad_source = "#";
	} else {
		keypad_source = (char *)source;
	}

	_vcui_engine_interface_send_dtmf_number(keypad_source[0]);

	if (strlen(priv->entry_disp_data) == KEYPAD_ENTRY_DISP_DATA_SIZE) {
		sub_buffer_pointer = &priv->entry_disp_data[1];

		snprintf(priv->entry_disp_data, sizeof(priv->entry_disp_data),
				 "%s", sub_buffer_pointer);
		CALL_UI_DEBUG("priv->entry_disp_data after change [%s]", priv->entry_disp_data);
	}

	priv->entry_disp_data[priv->data_len] = keypad_source[0];
	if(priv->data_len < (KEYPAD_ENTRY_DISP_DATA_SIZE-1)) {
		priv->data_len++;
	}

	snprintf(before_dest, sizeof(before_dest),
		 "<font_size=%d><color=#FFFFFF><shadow_color=#000000><style=outline_shadow>%s",
		 MAX_DIAL_NUMBER_FONT_SIZE, priv->entry_disp_data);

	snprintf(entry_dest, sizeof(entry_dest),
		 "%s</style></shadow_color></color></font_size>",
		 before_dest);

	CALL_UI_DEBUG("entry_dest [%s]", priv->entry_disp_data);
	elm_entry_entry_set(priv->entry, entry_dest);
	elm_entry_cursor_end_set(priv->entry);
}

static void __vcui_view_keypad_hide_btn_effect_done(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	CALL_UI_DEBUG("..");

	_vcui_view_auto_change();
}

static void __vcui_view_keypad_hide_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_keypad_priv_t *priv = (vcui_view_keypad_priv_t *)vd->priv;

	edje_object_signal_emit(_EDJ(priv->contents), "HIDE", "KEYPADBTN");
	edje_object_signal_callback_add(_EDJ(priv->contents), "DONE", "HIDEKEYPAD", __vcui_view_keypad_hide_btn_effect_done, data);
}

static void __vcui_view_keypad_end_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");

	if (_vcui_doc_get_count_unhold() == 1) {
		_vcui_engine_end_call();
	} else if (_vcui_doc_get_count_unhold() > 1) {
		_vcui_engine_end_active_calls();
	} else if (_vcui_doc_get_count_hold() == 1) {
		_vcui_engine_end_call();
	} else if (_vcui_doc_get_count_hold() > 1) {
		_vcui_engine_end_held_calls();
	} else {
		CALL_UI_DEBUG("call data is null");
		return;
	}

}

voice_call_view_data_t *_vcui_view_keypad_new(vcui_app_call_data_t *ad)
{

	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(vcui_view_keypad_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!!");
	}

	return &s_view;
}

static Evas_Object *__vcui_view_keypad_create_hide_button(void *data)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_keypad_priv_t *priv = (vcui_view_keypad_priv_t *)vd->priv;
	layout = priv->contents;

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_hide");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_hide", btn);
	elm_object_style_set(btn, "text_only/style_keypad_hide_button");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON2_HIDE_KEYPAD"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_view_keypad_hide_btn_cb, vd);

	return layout;
}

static Evas_Object *__vcui_view_keypad_create_end_call(void *data)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *sw;

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_keypad_priv_t *priv = (vcui_view_keypad_priv_t *)vd->priv;
	layout = priv->contents;

	sw = edje_object_part_swallow_get(_EDJ(layout), "btn_end");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_end", btn);
	elm_object_style_set(btn, "text_only/style_keypad_end_button");
	elm_object_text_set(btn, _("IDS_CALL_SK3_END_CALL"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_view_keypad_end_btn_cb, vd);

	return layout;
}

static Evas_Object *__vcui_view_keypad_create_contents(void *data)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	Evas_Object *eo;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, GRP_KEYPAD);
	if (eo == NULL)
		return NULL;

	return eo;
}

static Evas_Object *__vcui_view_keypad_create_layout_main(Evas_Object *parent)
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

static void __vcui_view_keypad_draw_keypad_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");

	int valid = 0;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_view_keypad_priv_t *priv = (vcui_view_keypad_priv_t *)vd->priv;

	_vcui_show_wallpaper_image(priv->contents);

	if (_vcui_doc_get_count_unhold() == 1) {
		priv->now_data = _vcui_doc_get_last_status(CALL_UNHOLD);
		if (NULL == priv->now_data) {
			CALL_UI_DEBUG("priv->now_data is NULL \n");
			return;
		}
		if (strlen((char *)priv->now_data->call_display) == 0) {
			edje_object_part_text_set(_EDJ(priv->contents), "txt_call_name", (char *)priv->now_data->call_num);
		} else {
			edje_object_part_text_set(_EDJ(priv->contents), "txt_call_name", (char *)priv->now_data->call_display);
		}
	} else if (_vcui_doc_get_count_unhold() > 1) {
		priv->now_data = _vcui_doc_get_last_status(CALL_UNHOLD);
		edje_object_part_text_set(_EDJ(priv->contents), "txt_call_name", _("IDS_CALL_OPT_CONFERENCE_CALL"));
	} else if (_vcui_doc_get_count_hold() == 1) {
		priv->now_data = _vcui_doc_get_last_status(CALL_HOLD);
		if (NULL == priv->now_data) {
			CALL_UI_DEBUG("priv->now_data is NULL \n");
			return;
		}

		if (strlen((char *)priv->now_data->call_display) == 0) {
			edje_object_part_text_set(_EDJ(priv->contents), "txt_call_name", (char *)priv->now_data->call_num);
		} else {
			edje_object_part_text_set(_EDJ(priv->contents), "txt_call_name", (char *)priv->now_data->call_display);
		}
	} else if (_vcui_doc_get_count_hold() > 1) {
		priv->now_data = _vcui_doc_get_last_status(CALL_HOLD);
		edje_object_part_text_set(_EDJ(priv->contents), "txt_call_name", _("IDS_CALL_OPT_CONFERENCE_CALL"));
	} else {
		CALL_UI_DEBUG("call data is null");
		return;
	}

	valid = _vcui_check_valid_eo(priv->contents, "KEYPADVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== KEYPADVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
		return;
	}

	if (NULL == priv->now_data) {
		CALL_UI_DEBUG("\n priv->now_data is NULL \n");
		return;
	}
	_vcui_view_common_set_each_time(priv->now_data->start_time);

	__vcui_view_keypad_create_hide_button(vd);
	__vcui_view_keypad_create_end_call(vd);

	edje_object_signal_emit(_EDJ(priv->contents), "SHOW", "KEYPADBTN");

	evas_object_show(priv->contents);

}

static int _vcui_view_keypad_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("keypad view create!!");

	vcui_app_call_data_t *ad = view_data->app_data;
	vcui_view_keypad_priv_t *priv = view_data->priv;

	if (!view_data->layout) {
		/* Create Main Layout */
		view_data->layout = __vcui_view_keypad_create_layout_main(ad->win_main);
		if (view_data->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}

		/* Create Contents */
		priv->contents = __vcui_view_keypad_create_contents(view_data);
		elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);

		priv->entry = __vcui_view_keypad_create_single_line_scrolled_entry(priv->contents);
		memset(priv->entry_disp_data, 0x0, sizeof(priv->entry_disp_data));
		priv->data_len = 0;

		edje_object_signal_callback_add(_EDJ(priv->contents),
										"pad_down", "*",
										__vcui_view_keypad_on_keypad_down, 
										view_data);

		edje_object_part_swallow(_EDJ(priv->contents), "textblock/textarea", priv->entry);
		evas_object_name_set(priv->contents, "KEYPADVIEW");
		CALL_UI_DEBUG("[========== KEYPADVIEW: priv->contents Addr : [%p] ==========]", priv->contents);
	}

	_vcui_view_keypad_onshow(view_data);
	return VC_NO_ERROR;
}

static int _vcui_view_keypad_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("keypad view update!!");

	_vcui_view_keypad_onshow(view_data);
	return VC_NO_ERROR;
}

static int _vcui_view_keypad_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("keypad view hide!!");
	int valid = 0;
	vcui_view_keypad_priv_t *priv = (vcui_view_keypad_priv_t *)view_data->priv;

	valid = _vcui_check_valid_eo(priv->contents, "KEYPADVIEW");
	if (valid == -1) {
		CALL_UI_DEBUG("[========== KEYPADVIEW : Invalid Evas Object, priv->contents : %p ==========]", priv->contents);
	}

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int _vcui_view_keypad_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("keypad view show!!");

	vcui_view_keypad_priv_t *priv = view_data->priv;

	__vcui_view_keypad_draw_keypad_screen(priv->contents, view_data);

	evas_object_hide(view_data->layout);
	evas_object_show(view_data->layout);
	elm_object_focus_set(priv->entry, EINA_TRUE);
	return VC_NO_ERROR;
}

static int _vcui_view_keypad_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("keypad view destroy!!");

	vcui_app_call_data_t *ad = _vcui_get_app_data();

	voice_call_view_data_t *vd = ad->view_st[VIEW_INCALL_KEYPAD_VIEW];
	vcui_view_keypad_priv_t *priv = (vcui_view_keypad_priv_t *)vd->priv;

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

	ad->view_st[VIEW_INCALL_KEYPAD_VIEW] = NULL;

	_vcui_cache_flush();

	return VC_NO_ERROR;
}

static Evas_Object *__vcui_view_keypad_create_single_line_scrolled_entry(void *content)
{
	Evas_Object *en;

	if (content == NULL) {
		CALL_UI_DEBUG("content is NULL!");
		return NULL;
	}

	en = elm_entry_add(content);
	elm_entry_scrollable_set(en, EINA_TRUE);

	elm_entry_select_all(en);
	elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF,
						ELM_SCROLLER_POLICY_AUTO);

	elm_entry_bounce_set(en, EINA_FALSE, EINA_FALSE);
	elm_entry_line_wrap_set(en, ELM_WRAP_WORD);
	elm_entry_input_panel_enabled_set(en, EINA_FALSE);
	elm_entry_single_line_set(en, EINA_TRUE);
	elm_entry_cursor_end_set(en);
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(en);

	return en;
}
