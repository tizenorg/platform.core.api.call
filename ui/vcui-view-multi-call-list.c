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
#include "vcui-view-multi-call-list.h"

static Elm_Genlist_Item_Class *itc_call;

static int __vcui_view_multi_call_list_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_multi_call_list_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_multi_call_list_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_multi_call_list_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_multi_call_list_ondestroy(voice_call_view_data_t *view_data);

static voice_call_view_data_t s_view = {
	.type = VIEW_INCALL_MULTICALL_LIST_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vcui_view_multi_call_list_oncreate,
	.onUpdate = __vcui_view_multi_call_list_onupdate,
	.onHide = __vcui_view_multi_call_list_onhide,
	.onShow = __vcui_view_multi_call_list_onshow,
	.onDestroy = __vcui_view_multi_call_list_ondestroy,
	.priv = NULL,
};

voice_call_view_data_t *_vcui_view_multi_call_list_new(vcui_app_call_data_t *ad)
{
	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(vcui_view_multi_call_list_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR ");
	}

	return &s_view;
}

static void __vcui_view_multi_call_list_small_end_call_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	call_data_t *call_data = data;
	_vcui_engine_end_call_by_handle(call_data->call_handle);
	call_data->bno_end_show = EINA_TRUE;

	_vcui_view_popup_load_endcall_time(call_data);
}

static void __vcui_view_multi_call_list_split_call_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	call_data_t *call_data = data;
	_vcui_engine_split_call(call_data->call_handle);
}

static void __vcui_view_multi_call_list_gl_sel_call(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	elm_object_item_data_get(item);
	elm_genlist_item_selected_set(item, 0);
}

static void __vcui_view_multi_call_list_gl_del_call(void *data, Evas_Object *obj)
{

	return;
}

static Eina_Bool __vcui_view_multi_call_list_gl_state_get_call(void *data, Evas_Object *obj, const char *part)
{

	return EINA_FALSE;
}

static Evas_Object *__vcui_view_multi_call_list_gl_content_get_call(void *data, Evas_Object *obj, const char *part)
{
	call_data_t *call_data = (call_data_t *)data;

	Evas_Object *icon = NULL;
	Evas_Object *btn = NULL;

	if (strcmp(part, "elm.swallow.end") == 0) {
		btn = elm_button_add(obj);
		elm_object_style_set(btn, "style_call_smallbtn_red");
		icon = elm_icon_add(obj);
		elm_icon_file_set(icon, CONF_CALL_END_ICON, NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_icon_resizable_set(icon, 1, 1);
		elm_object_part_content_set(btn, "icon", icon);

		evas_object_smart_callback_add(btn, "clicked", __vcui_view_multi_call_list_small_end_call_cb, call_data);
		evas_object_propagate_events_set(btn, EINA_FALSE);

		return btn;
	} else if (strcmp(part, "elm.swallow.private") == 0) {
		if (_vcui_doc_get_count_hold() > 0) {
			return NULL;
		}
		btn = elm_button_add(obj);
		icon = elm_icon_add(obj);
		elm_icon_file_set(icon, PRIVATE_ICON, NULL);
		elm_object_style_set(btn, "icon_only/style_call_icon_only_private");

		elm_icon_resizable_set(icon, 1, 1);
		elm_object_part_content_set(btn, "icon", icon);

		evas_object_smart_callback_add(btn, "clicked", __vcui_view_multi_call_list_split_call_cb, call_data);
		evas_object_propagate_events_set(btn, EINA_FALSE);

		return btn;
	}

	return NULL;
}

static char *__vcui_view_multi_call_list_gl_text_get_call(void *data, Evas_Object *obj, const char *part)
{
	char buf[DEF_BUF_LEN] = { 0, };
	call_data_t *call_data = (call_data_t *)data;

	if (strcmp(part, "elm.text") == 0) {
		if (strlen((char *)call_data->call_display) == 0)
			snprintf(buf, sizeof(buf), "%s", call_data->call_num);
		else
			snprintf(buf, sizeof(buf), "%s", call_data->call_display);
	}

	return strdup(buf);
}

static void __vcui_view_multi_call_list_genlist_init(void *data)
{
	itc_call->item_style = "multicall_list";
	itc_call->func.text_get = __vcui_view_multi_call_list_gl_text_get_call;
	itc_call->func.content_get = __vcui_view_multi_call_list_gl_content_get_call;
	itc_call->func.state_get = __vcui_view_multi_call_list_gl_state_get_call;
	itc_call->func.del = __vcui_view_multi_call_list_gl_del_call;
}

static void __vcui_view_multi_call_list_genlist_add(void *data)
{
	voice_call_view_data_t *vd = data;
	vcui_view_multi_call_list_priv_t *priv = (vcui_view_multi_call_list_priv_t *)vd->priv;
	Evas_Object *genlist = NULL;

	genlist = elm_genlist_add(priv->contents);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(priv->contents, "swl_calllist", genlist);
	priv->multibox_gl = genlist;
}

void __vcui_view_multi_call_list_genlist_item_append(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = data;
	vcui_view_multi_call_list_priv_t *priv = (vcui_view_multi_call_list_priv_t *)vd->priv;
	Eina_List *list = NULL;

	if (priv->multibox_gl) {
		elm_genlist_clear(priv->multibox_gl);
	}

	if (priv->call_status == CALL_UNHOLD) {
		list = _vcui_doc_get_unhold_caller();
	} else {
		list = _vcui_doc_get_hold_caller();
	}
	if (list == NULL) {
		CALL_UI_DEBUG("error");
		return;
	}

	Eina_List *l;
	call_data_t *call_data;
	EINA_LIST_FOREACH(list, l, call_data) {
		elm_genlist_item_append(priv->multibox_gl, itc_call, (void *)call_data, NULL, ELM_GENLIST_ITEM_NONE, __vcui_view_multi_call_list_gl_sel_call, data);
	}
	eina_list_free(list);
}

static void __vcui_view_multi_call_list_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	_vcui_view_auto_change();
}

static void __vcui_view_multi_call_list_draw_screen(void *data)
{
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	vcui_view_multi_call_list_priv_t *priv = (vcui_view_multi_call_list_priv_t *)vd->priv;
	call_data_t *call_data;

	priv->total_members = _vcui_doc_get_count_hold()+_vcui_doc_get_count_unhold();
	priv->call_status = _vcui_doc_get_show_callstatus();
	call_data = _vcui_doc_get_last_status(priv->call_status);
	if (call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return;
	}
	_vcui_view_common_set_each_time(call_data->start_time);

		_vcui_show_wallpaper_image(priv->contents);

		__vcui_view_multi_call_list_genlist_item_append(vd);

		_vcui_create_conf_list_button_hold(vd);
		_vcui_create_button_bigend(vd);
	evas_object_show(priv->contents);
}

static Evas_Object *__vcui_view_multi_call_list_create_contents(void *data, char *group)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *eo;

	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}

	/* load edje */
	eo = _vcui_load_edj(vd->layout, EDJ_NAME, group);
	if (eo == NULL)
		return NULL;

	return eo;
}

static void __vcui_view_multi_call_list_create_navigationframe_layout(voice_call_view_data_t *vd)
{
	Evas_Object *navi_ly;
	Evas_Object *btn1;
	vcui_view_multi_call_list_priv_t *priv = (vcui_view_multi_call_list_priv_t *)vd->priv;

	navi_ly = _vcui_load_edj(priv->navi_frame, EDJ_NAME, "navigation_view");
	if (navi_ly == NULL)
		return;
	CALL_UI_DEBUG("navi_layout created...");

	/* Load default button */
	btn1 = elm_button_add(priv->navi_frame);
	elm_object_style_set(btn1, "vcui_naviframe_new_backbutton/default");
	evas_object_smart_callback_add(btn1, "clicked", __vcui_view_multi_call_list_back_cb, vd);

	elm_naviframe_item_push(priv->navi_frame, _("IDS_CALL_OPT_CONFERENCE_CALL"), btn1, NULL, navi_ly, "vcui_naviframe_new_item_1line");
}

static Evas_Object *__vcui_view_multi_call_list_create_navigation_layout(Evas_Object *parent)
{
	Evas_Object *navi_frame;

	if (parent == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}

	navi_frame = elm_naviframe_add(parent);
	elm_object_style_set(navi_frame, "vcui_naviframe_new/default");

	return navi_frame;
}

static Evas_Object *__vcui_view_multi_call_list_create_layout_main(Evas_Object *parent)
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

static int __vcui_view_multi_call_list_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("multicall-list view create");

	voice_call_view_data_t *vd = view_data;
	vcui_view_multi_call_list_priv_t *priv = (vcui_view_multi_call_list_priv_t *)vd->priv;

	itc_call = elm_genlist_item_class_new();

	if (!vd->layout) {
		vd->layout = __vcui_view_multi_call_list_create_layout_main(vd->app_data->win_main);
		if (vd->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
		priv->contents = __vcui_view_multi_call_list_create_contents(vd, GRP_MULTICALL);
		elm_object_part_content_set(vd->layout, "elm.swallow.content", priv->contents);

		priv->navi_frame = __vcui_view_multi_call_list_create_navigation_layout(vd->layout);
		elm_object_part_content_set(priv->contents, "navigation_bar", priv->navi_frame);

		__vcui_view_multi_call_list_create_navigationframe_layout(vd);

		evas_object_name_set(priv->contents, "MULTIVIEWLIST");
		CALL_UI_DEBUG("[========== MULTIVIEWLIST: priv->contents Addr : [%p] ==========]", priv->contents);

		__vcui_view_multi_call_list_genlist_add(vd);
		__vcui_view_multi_call_list_genlist_init(vd);
	} else {
		CALL_UI_DEBUG("[UI]ad->layout_multicall != NULL case ");
		evas_object_show(vd->layout);
	}

	__vcui_view_multi_call_list_onshow(view_data);

	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_list_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("multicall-list view update");

	__vcui_view_multi_call_list_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_list_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multicall-list view hide");

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_list_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multicall-list view show");

	__vcui_view_multi_call_list_draw_screen(view_data);

	evas_object_hide(view_data->layout);
	evas_object_show(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_multi_call_list_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("multicall-list view destroy");

	vcui_app_call_data_t *ad = _vcui_get_app_data();

	voice_call_view_data_t *vd = ad->view_st[VIEW_INCALL_MULTICALL_LIST_VIEW];
	vcui_view_multi_call_list_priv_t *priv = (vcui_view_multi_call_list_priv_t *)vd->priv;

	if (itc_call)
		elm_genlist_item_class_free(itc_call);

	if (priv != NULL) {
		if (priv->multibox_gl != NULL) {
			elm_genlist_clear(priv->multibox_gl);
			evas_object_del(priv->multibox_gl);
			priv->multibox_gl = NULL;
		}

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

	ad->view_st[VIEW_INCALL_MULTICALL_LIST_VIEW] = NULL;

	_vcui_cache_flush();
	CALL_UI_DEBUG("complete destroy multi view list");
	return VC_NO_ERROR;
}
