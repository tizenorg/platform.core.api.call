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
#include "vcui-view-incoming-lock.h"

#define ACCEPT_DIFF_VALUE (130)
#define ACCEPT_DIFF_VALUE_MAX (182)

#define REJECT_DIFF_VALUE (-130)
#define REJECT_DIFF_VALUE_MAX (-182)

static int __vcui_view_incoming_lock_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_incoming_lock_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_incoming_lock_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_incoming_lock_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_incoming_lock_ondestroy(voice_call_view_data_t *view_data);

static voice_call_view_data_t s_view = {
	.type = VIEW_INCOMING_LOCK_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vcui_view_incoming_lock_oncreate,
	.onUpdate = __vcui_view_incoming_lock_onupdate,
	.onHide = __vcui_view_incoming_lock_onhide,
	.onShow = __vcui_view_incoming_lock_onshow,
	.onDestroy = __vcui_view_incoming_lock_ondestroy,
	.priv = NULL,
};

voice_call_view_data_t *_vcui_view_incoming_lock_new(vcui_app_call_data_t *ad)
{

	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(incoming_lock_view_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!!");
	}

	return &s_view;
}

static Evas_Object *__vcui_view_incoming_lock_load_edj(Evas *evas, char *edjname, const char *grpname)
{
	Evas_Object *edj;

	edj = edje_object_add(evas);
	if (!edje_object_file_set(edj, edjname, grpname)) {
		CALL_UI_DEBUG("ERROR!!");
		return NULL;
	}

	return edj;
}

static void __vcui_view_incoming_lock_accept_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Event_Mouse_Move *ev = event_info;

	priv->accept_start_x = ev->cur.canvas.x;
}

static void __vcui_view_incoming_lock_accept_mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;
	Evas_Event_Mouse_Move *ev = event_info;

	int diff_x = 0;

	priv->accept_cur_x = ev->cur.canvas.x;
	diff_x = priv->accept_cur_x - priv->accept_start_x;


	if (diff_x >= 0) {
		if (diff_x < (ACCEPT_DIFF_VALUE * ad->scale_factor)) {
			evas_object_move(priv->lock_accept, diff_x, 0);

			edje_object_signal_emit(priv->lock_reject, "lock_reject,default", "prog");
		} else if (diff_x < (ACCEPT_DIFF_VALUE_MAX * ad->scale_factor)) {
			evas_object_move(priv->lock_accept, diff_x, 0);
			evas_object_move(priv->lock_reject, (diff_x - (ACCEPT_DIFF_VALUE * ad->scale_factor)), 0);

			edje_object_signal_emit(priv->lock_reject, "lock_reject,default", "prog");
		} else {
			evas_object_move(priv->lock_accept, (ACCEPT_DIFF_VALUE_MAX * ad->scale_factor), 0);
			evas_object_move(priv->lock_reject, ((ACCEPT_DIFF_VALUE_MAX * ad->scale_factor) - (ACCEPT_DIFF_VALUE * ad->scale_factor)), 0);

			edje_object_signal_emit(priv->lock_reject, "lock_reject,alpha", "prog");
		}
	}
}

static void __vcui_view_incoming_lock_accept_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;

	int diff_x = priv->accept_cur_x - priv->accept_start_x;

	if (diff_x >= (ACCEPT_DIFF_VALUE_MAX * ad->scale_factor)) {
		if(_vcui_is_phonelock_status() == EINA_FALSE)
			vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_UNLOCK);
		_vcui_engine_answer_call();
	} else {
		evas_object_move(priv->lock_accept, 0, 0);
		evas_object_move(priv->lock_reject, 0, 0);
	}

}

static Evas_Object *__vcui_view_incoming_lock_create_button_accept(void *data)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;

	if(priv->lock_accept != NULL) {
		evas_object_del(priv->lock_accept);
		priv->lock_accept = NULL;
	}	
	priv->lock_accept = __vcui_view_incoming_lock_load_edj(evas_object_evas_get(ad->win_main), EDJ_NAME, GRP_LOCK_ACCEPT);
	evas_object_resize(priv->lock_accept, ad->root_w, ad->root_h);

	evas_object_event_callback_add(priv->lock_accept, EVAS_CALLBACK_MOUSE_DOWN, __vcui_view_incoming_lock_accept_mouse_down_cb, vd);
	evas_object_event_callback_add(priv->lock_accept, EVAS_CALLBACK_MOUSE_MOVE, __vcui_view_incoming_lock_accept_mouse_move_cb, vd);
	evas_object_event_callback_add(priv->lock_accept, EVAS_CALLBACK_MOUSE_UP, __vcui_view_incoming_lock_accept_mouse_up_cb, vd);

	edje_object_part_text_set(priv->lock_accept, "accept_text", _("IDS_CALL_BUTTON_ACCEPT"));

	evas_object_show(priv->lock_accept);

	return priv->lock_accept;
}

static void __vcui_view_incoming_lock_reject_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Event_Mouse_Move *ev = event_info;

	priv->reject_start_x = ev->cur.canvas.x;
}

static void __vcui_view_incoming_lock_reject_mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;
	Evas_Event_Mouse_Move *ev = event_info;

	int diff_x = 0;

	priv->reject_cur_x = ev->cur.canvas.x;
	diff_x = priv->reject_cur_x - priv->reject_start_x;

	if (diff_x <= 0) {
		if (diff_x > (REJECT_DIFF_VALUE * ad->scale_factor)) {
			evas_object_move(priv->lock_reject, diff_x, 0);

			edje_object_signal_emit(priv->lock_accept, "lock_accept,default", "prog");
		} else if (diff_x > (REJECT_DIFF_VALUE_MAX * ad->scale_factor)) {
			evas_object_move(priv->lock_reject, diff_x, 0);
			evas_object_move(priv->lock_accept, (diff_x - (REJECT_DIFF_VALUE * ad->scale_factor)), 0);

			edje_object_signal_emit(priv->lock_accept, "lock_accept,default", "prog");
		} else {
			evas_object_move(priv->lock_reject, (REJECT_DIFF_VALUE_MAX * ad->scale_factor), 0);
			evas_object_move(priv->lock_accept, ((REJECT_DIFF_VALUE_MAX * ad->scale_factor) - (REJECT_DIFF_VALUE * ad->scale_factor)), 0);

			edje_object_signal_emit(priv->lock_accept, "lock_accept,alpha", "prog");

		}
	}
}

static void __vcui_view_incoming_lock_reject_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;

	int diff_x = priv->reject_cur_x - priv->reject_start_x;

	if (diff_x <= (REJECT_DIFF_VALUE_MAX * ad->scale_factor)) {
/*		vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_UNLOCK);*/
		_vcui_engine_reject_call();
	} else {
		evas_object_move(priv->lock_reject, 0, 0);
		evas_object_move(priv->lock_accept, 0, 0);
	}

}

static Evas_Object *__vcui_view_incoming_lock_create_button_reject(void *data)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;

	if(priv->lock_reject != NULL) {
		evas_object_del(priv->lock_reject);
		priv->lock_reject = NULL;
	}
	priv->lock_reject = __vcui_view_incoming_lock_load_edj(evas_object_evas_get(ad->win_main), EDJ_NAME, GRP_LOCK_REJECT);
	evas_object_resize(priv->lock_reject, ad->root_w, ad->root_h);

	evas_object_event_callback_add(priv->lock_reject, EVAS_CALLBACK_MOUSE_DOWN, __vcui_view_incoming_lock_reject_mouse_down_cb, vd);
	evas_object_event_callback_add(priv->lock_reject, EVAS_CALLBACK_MOUSE_MOVE, __vcui_view_incoming_lock_reject_mouse_move_cb, vd);
	evas_object_event_callback_add(priv->lock_reject, EVAS_CALLBACK_MOUSE_UP, __vcui_view_incoming_lock_reject_mouse_up_cb, vd);

	edje_object_part_text_set(priv->lock_reject, "reject_text", _("IDS_CALL_BUTTON_REJECT"));

	evas_object_show(priv->lock_reject);

	return priv->lock_reject;
}

static void __vcui_view_incoming_lock_draw_screen(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");

	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Object *eo = priv->contents;
	vcui_app_call_data_t *ad = vd->app_data;

	priv->now_data = _vcui_doc_get_recent_mt();
	if (priv->now_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return;
	}
	if (priv->now_data->mo_mt_status != CALL_INCOMING) {
		CALL_UI_DEBUG("it is not call_incoming.");
		return;
	}

	/* call image */
	if (strcmp((char *)priv->now_data->call_file_path, NOIMG_ICON) == 0) {
		_vcui_show_wallpaper_image(priv->contents);
	} else {
		CALL_UI_DEBUG("there is caller image.");
		_vcui_show_wallpaper_image(priv->contents);
		priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, priv->now_data->call_file_path);
	}

	/* call name (if nothing, call number) */
	if (strlen((char *)priv->now_data->call_display) == 0) {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_num);
	} else {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", (char *)priv->now_data->call_display);
		edje_object_part_text_set(_EDJ(eo), "txt_contact_phone_type", (char *)priv->now_data->call_num);
	}

	__vcui_view_incoming_lock_create_button_accept(vd);
	__vcui_view_incoming_lock_create_button_reject(vd);

	ad->beffect_needed = EINA_TRUE;

	evas_object_show(eo);

}

static Evas_Object *__vcui_view_incoming_lock_create_contents(void *data)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *eo;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, GRP_MTLOCK);
	if (eo == NULL)
		return NULL;

	return eo;
}

static Evas_Object *__vcui_view_incoming_lock_create_layout_main(Evas_Object *parent)
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

static int __vcui_view_incoming_lock_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("mt-lock view create!!");

	vcui_app_call_data_t *ad = view_data->app_data;
	incoming_lock_view_priv_t *priv = view_data->priv;

	if (!view_data->layout) {
		view_data->layout = __vcui_view_incoming_lock_create_layout_main(ad->win_main);
		if (view_data->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
		priv->contents = __vcui_view_incoming_lock_create_contents(view_data);
		elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);

		evas_object_name_set(priv->contents, "INCOMINGLOCKVIEW");
		CALL_UI_DEBUG("[========== INCOMINGLOCKVIEW: priv->contents Addr : [%p] ==========]", priv->contents);

	}

	__vcui_view_incoming_lock_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_incoming_lock_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("mt-lock view update!!");

	__vcui_view_incoming_lock_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_incoming_lock_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("mt-lock view hide!!");

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_incoming_lock_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("mt-lock view show!!");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	int result = 0;

	__vcui_view_incoming_lock_draw_screen(view_data);

	evas_object_hide(view_data->layout);
	evas_object_show(view_data->layout);
	_vcui_app_win_set_noti_type(EINA_TRUE);

	return VC_NO_ERROR;
}

static int __vcui_view_incoming_lock_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("mt-lock view destroy!!");

	vcui_app_call_data_t *ad = _vcui_get_app_data();

	voice_call_view_data_t *vd = ad->view_st[VIEW_INCOMING_LOCK_VIEW];
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;

	if (_vcui_is_idle_lock() == CALL_LOCK)
		_vcui_app_win_set_noti_type(EINA_TRUE);
	else
		_vcui_app_win_set_noti_type(EINA_FALSE);

	ad->bmute_ringtone = EINA_FALSE;
	if (priv != NULL) {
		if (priv->lock_accept) {
			evas_object_del(priv->lock_accept);
			priv->lock_accept = NULL;
		}
		if (priv->lock_reject) {
			evas_object_del(priv->lock_reject);
			priv->lock_reject = NULL;
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

	ad->view_st[VIEW_INCOMING_LOCK_VIEW] = NULL;

	_vcui_cache_flush();

	return VC_NO_ERROR;
}
