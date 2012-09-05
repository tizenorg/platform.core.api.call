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
#include "vcui-view-quickpanel.h"
#include <minicontrol-provider.h>

#define	VIEW_QUICKPANEL_LAYOUT_ID "QUICKPANELVIEW"
typedef struct {
	Evas_Object *caller_id;
	int rotate_angle;
	Ecore_Event_Handler *client_msg_handler;
} vcui_view_qp_priv_t;

static int __vcui_view_qp_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3);
static int __vcui_view_qp_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2);
static int __vcui_view_qp_onhide(voice_call_view_data_t *view_data);
static int __vcui_view_qp_onshow(voice_call_view_data_t *view_data);
static int __vcui_view_qp_ondestroy(voice_call_view_data_t *view_data);
static void __vcui_view_qp_caller_id_cb(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool __vcui_qp_client_message_cb(void *data, int type, void *event);

static voice_call_view_data_t s_view = {
	.type = VIEW_QUICKPANEL_VIEW,
	.app_data = NULL,
	.layout = NULL,
	.onCreate = __vcui_view_qp_oncreate,
	.onUpdate = __vcui_view_qp_onupdate,
	.onHide = __vcui_view_qp_onhide,
	.onShow = __vcui_view_qp_onshow,
	.onDestroy = __vcui_view_qp_ondestroy,
	.priv = NULL,
};

static Eina_Bool __vcui_qp_client_message_cb(void *data, int type, void *event)
{
	CALL_UI_DEBUG("..");
	int new_angle = 0;
	Ecore_X_Event_Client_Message *ev = (Ecore_X_Event_Client_Message *) event;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	vcui_view_qp_priv_t *priv = (vcui_view_qp_priv_t *)vd->priv;

	CALL_UI_DEBUG("message_type: %d", ev->message_type);
	if ((ev->message_type == ECORE_X_ATOM_E_ILLUME_ROTATE_WINDOW_ANGLE)
		|| (ev->message_type == ECORE_X_ATOM_E_ILLUME_ROTATE_ROOT_ANGLE)) {
		new_angle = ev->data.l[0];
		CALL_UI_DEBUG("ROTATION: %d", new_angle);
		priv->rotate_angle = new_angle;
		elm_win_rotation_with_resize_set(ad->win_quickpanel, new_angle);
		__vcui_view_qp_onshow(vd);
	}

	return ECORE_CALLBACK_RENEW;
}

static void __vcui_view_qp_caller_id_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;

	elm_win_raise(ad->win_main);
}

voice_call_view_data_t *_vcui_view_qp_new(vcui_app_call_data_t *ad)
{
	s_view.app_data = ad;
	s_view.priv = calloc(1, sizeof(vcui_view_qp_priv_t));

	if (!s_view.priv) {
		CALL_UI_DEBUG("ERROR!!!!!!!!!!! ");
	}

	return &s_view;
}

static void __vcui_view_qp_draw_contact_image(Evas_Object *eo, call_data_t *pcall_data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = ad->view_st[VIEW_QUICKPANEL_VIEW];

	vcui_view_qp_priv_t *priv = (vcui_view_qp_priv_t *)vd->priv;

	if (pcall_data == NULL)
		return;

	_vcui_delete_contact_image(eo);
	char *file_path = _vcui_doc_get_caller_id_file_path(pcall_data);
	if (strcmp(file_path, NOIMG_ICON) == 0) {
		priv->caller_id = _vcui_show_default_image(eo, ad->win_quickpanel, QP_NOIMG_ICON);
	} else {
		priv->caller_id = _vcui_show_default_image(eo, ad->win_quickpanel, file_path);
	}
}

static void __vcui_view_qp_update_caller(Evas_Object *eo, call_data_t *pcall_data)
{
	CALL_UI_DEBUG("..");

	if (pcall_data == NULL)
		return;

	char *call_number = _vcui_doc_get_call_number(pcall_data);
	char *call_name = _vcui_doc_get_call_display_name(pcall_data);
	if (strlen(call_name) == 0) {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", call_number);
	} else {
		edje_object_part_text_set(_EDJ(eo), "txt_call_name", call_name);
	}
}

static void __vcui_view_qp_draw_screen(Evas_Object *eo, void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	vcui_view_qp_priv_t *priv = (vcui_view_qp_priv_t *)vd->priv;
	call_data_t *call_data = NULL;
	char buf[DEF_BUF_LEN] = {0, };

	int grp_count = 0;

	vcall_engine_get_group_count(&grp_count);
	CALL_UI_DEBUG("No. of groups - %d", grp_count);

	if (_vcui_doc_get_no_status_call_data_count() == 1) {
		CALL_UI_DEBUG("dialing/connecting view");
		call_data = _vcui_doc_get_call_data_mo_type();
		__vcui_view_qp_update_caller(eo, call_data);
		__vcui_view_qp_draw_contact_image(eo, call_data);

		_vcui_view_qp_update_text_status(vd, _("IDS_CALL_POP_CALLING"));
	} else if (_vcui_doc_get_all_call_data_count() > 1) {
		if (_vcui_doc_get_unhold_call_data_count() == 0 || _vcui_doc_get_hold_call_data_count() == 0) {
			CALL_UI_DEBUG("multi conf call");

			int all_calls = _vcui_doc_get_all_call_data_count();
			char *temp = _("IDS_QP_BODY_GROUP_CALL_HPD_ABB");
			call_data = _vcui_doc_get_first_call_data_from_list();
			snprintf(buf, DEF_BUF_LEN, temp, all_calls);
			edje_object_part_text_set(_EDJ(eo), "txt_call_name", buf);
			priv->caller_id = _vcui_show_default_image(eo, ad->win_quickpanel, QP_CONF_ICON);

			int caller_status = _vcui_doc_get_call_status(call_data);
			if (caller_status == CALL_HOLD) {
				CALL_UI_DEBUG("Hold call status");
				_vcui_create_quickpanel_unhold_button(vd);
			} else {
				_vcui_create_quickpanel_mute_button(vd);
			}
		} else {
			CALL_UI_DEBUG("multi split call");
			int active_calls = _vcui_doc_get_unhold_call_data_count();
			CALL_UI_DEBUG("active_calls[%d]", active_calls);
			if (active_calls > 1) {
				char *temp = _("IDS_QP_BODY_GROUP_CALL_HPD_ABB");
				call_data = _vcui_doc_get_first_call_data_by_unhold_status();
				snprintf(buf, DEF_BUF_LEN, temp, active_calls);
				edje_object_part_text_set(_EDJ(eo), "txt_call_name", buf);
				priv->caller_id = _vcui_show_default_image(eo, ad->win_quickpanel, QP_CONF_ICON);
			} else if (active_calls == 1) {
				call_data = _vcui_doc_get_first_call_data_by_unhold_status();
				__vcui_view_qp_update_caller(eo, call_data);
				__vcui_view_qp_draw_contact_image(eo, call_data);
			} else {
				CALL_UI_DEBUG("invalid case");
			}
			_vcui_create_quickpanel_mute_button(vd);
		}

		_vcui_view_common_set_each_time(_vcui_doc_get_call_start_time(call_data));
	} else {
		CALL_UI_DEBUG("single call");
		call_data = _vcui_doc_get_first_call_data_from_list();
		__vcui_view_qp_update_caller(eo, call_data);
		__vcui_view_qp_draw_contact_image(eo, call_data);

		int caller_status = _vcui_doc_get_call_status(call_data);
		if (caller_status == CALL_HOLD) {
			CALL_UI_DEBUG("Hold call status");
			_vcui_create_quickpanel_unhold_button(vd);
		} else {
			_vcui_create_quickpanel_mute_button(vd);
		}

		_vcui_view_common_set_each_time(_vcui_doc_get_call_start_time(call_data));
	}
	_vcui_create_quickpanel_end_button(vd);
	evas_object_smart_callback_add(priv->caller_id, "clicked", __vcui_view_qp_caller_id_cb, vd);
}

static Evas_Object *__vcui_view_qp_create_window(void)
{
	Evas_Object *win;
	double scale = elm_config_scale_get();

	win = minicontrol_win_add("voicecall-quickpanel");
	evas_object_resize(win, QUICKPANEL_WIDTH * scale, QUICKPANEL_HEIGHT * scale);

	return win;
}

static Evas_Object *__vcui_view_qp_create_layout_main(Evas_Object *parent)
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

	return ly;
}

static Evas_Object *__vcui_view_qp_create_contents(void *data, char *group)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	vcui_app_call_data_t *ad = vd->app_data;
	Evas_Object *eo = NULL;

	/* load edje */
	eo = _vcui_load_edj(ad->win_quickpanel, EDJ_NAME, group);

	if (eo == NULL)
		return NULL;

	return eo;
}

static int __vcui_view_qp_oncreate(voice_call_view_data_t *view_data, int param1, void *param2, void *param3)
{
	CALL_UI_DEBUG("quickpanel view create!!");

	voice_call_view_data_t *vd = view_data;
	vcui_app_call_data_t *ad = vd->app_data;
	vcui_view_qp_priv_t *priv = (vcui_view_qp_priv_t *)vd->priv;

	if (!vd->layout) {
		ad->win_quickpanel = __vcui_view_qp_create_window();
		if (ad->win_quickpanel == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
		priv->rotate_angle = elm_win_rotation_get(ad->win_quickpanel);
		CALL_UI_DEBUG("current rotate angle(%d)", priv->rotate_angle);

		vd->layout = __vcui_view_qp_create_layout_main(ad->win_quickpanel);
		if (vd->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
	}

	if (priv->client_msg_handler == NULL)
		priv->client_msg_handler = ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, __vcui_qp_client_message_cb, vd);

	__vcui_view_qp_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_qp_onupdate(voice_call_view_data_t *view_data, void *update_data1, void *update_data2)
{
	CALL_UI_DEBUG("quickpanel view update!!");

	__vcui_view_qp_onshow(view_data);
	return VC_NO_ERROR;
}

static int __vcui_view_qp_onhide(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("quickpanel view hide!!");

	evas_object_hide(view_data->layout);
	return VC_NO_ERROR;
}

static int __vcui_view_qp_onshow(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("quickpanel view show!!");
	vcui_app_call_data_t *ad = view_data->app_data;
	vcui_view_qp_priv_t *priv = (vcui_view_qp_priv_t *)view_data->priv;

	if (ad->quickpanel_layout) {
		evas_object_del(ad->quickpanel_layout);
		ad->quickpanel_layout = NULL;
	}

	if (priv->rotate_angle == 0 || priv->rotate_angle == 180) {
		CALL_UI_DEBUG("portrait mode layout");
		evas_object_resize(ad->win_quickpanel, QUICKPANEL_WIDTH * ad->scale_factor, QUICKPANEL_HEIGHT * ad->scale_factor);
		ad->quickpanel_layout = __vcui_view_qp_create_contents(view_data, GRP_QUICKPANEL);
	} else if (priv->rotate_angle == 90 || priv->rotate_angle == 270) {
		CALL_UI_DEBUG("landscape mode layout");
		evas_object_resize(ad->win_quickpanel, LSCAPE_QUICKPANEL_WIDTH * ad->scale_factor, QUICKPANEL_HEIGHT * ad->scale_factor);
		ad->quickpanel_layout = __vcui_view_qp_create_contents(view_data, GRP_QUICKPANEL_LSCAPE);
	}

	elm_object_part_content_set(view_data->layout, "elm.swallow.content", ad->quickpanel_layout);
	evas_object_name_set(ad->quickpanel_layout, VIEW_QUICKPANEL_LAYOUT_ID);
	CALL_UI_DEBUG("[========== QUICKPANEL:ad->quickpanel_layout Addr : [%p] ==========]", ad->quickpanel_layout);

	__vcui_view_qp_draw_screen(ad->quickpanel_layout, view_data);

	return VC_NO_ERROR;
}

static int __vcui_view_qp_ondestroy(voice_call_view_data_t *view_data)
{
	CALL_UI_DEBUG("quickpanel view destroy!!");

	vcui_app_call_data_t *ad = _vcui_get_app_data();
	voice_call_view_data_t *vd = ad->view_st[VIEW_QUICKPANEL_VIEW];

	vcui_view_qp_priv_t *priv = (vcui_view_qp_priv_t *)vd->priv;

	if (priv != NULL) {
		if (priv->caller_id) {
			evas_object_del(priv->caller_id);
			priv->caller_id = NULL;
		}

		if (priv->client_msg_handler != NULL)
			ecore_event_handler_del(priv->client_msg_handler);

		free(priv);
		priv = NULL;
	}

	if (ad->quickpanel_layout) {
		evas_object_del(ad->quickpanel_layout);
		ad->quickpanel_layout = NULL;
	}

	if (vd->layout != NULL) {
		evas_object_hide(vd->layout);
		evas_object_del(vd->layout);
		vd->layout = NULL;
	}

	ad->view_st[VIEW_QUICKPANEL_VIEW] = NULL;

	if (ad->win_quickpanel) {
		evas_object_del(ad->win_quickpanel);
		ad->win_quickpanel = NULL;
	}

	_vcui_cache_flush();
	return VC_NO_ERROR;
}

void _vcui_view_qp_update_text_status(voice_call_view_data_t *vd, char *txt_status)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = NULL;
	int valid = 0;

	if (vd == NULL) {
		CALL_UI_DEBUG("View Data is NULL");
		return;
	}
	ad = vd->app_data;

	if (ad->win_quickpanel && ad->quickpanel_layout) {
		valid = _vcui_check_valid_eo(ad->quickpanel_layout, VIEW_QUICKPANEL_LAYOUT_ID);
		if (valid == -1) {
			CALL_UI_DEBUG("[========== QUICKPANELVIEW : Invalid Evas Object, ad->quickpanel_layout : %p ==========]", ad->quickpanel_layout);
			return;
		}
		edje_object_part_text_set(_EDJ(ad->quickpanel_layout), "txt_timer", txt_status);
	}

}

void _vc_ui_view_qp_set_call_timer(Evas_Object *qp_layout, char *pcall_timer)
{
	int valid = -1;

	VCUI_RETURN_IF_FAIL(qp_layout);
	valid = _vcui_check_valid_eo(qp_layout, VIEW_QUICKPANEL_LAYOUT_ID);
	if (valid == -1) {
		CALL_UI_DEBUG("[========== QUICKPANELVIEW : Invalid Evas Object, ad->quickpanel_layout : %p ==========]", qp_layout);
		return;
	}
	if (!_vcui_doc_get_no_status_call_data_count())
		edje_object_part_text_set(_EDJ(qp_layout), "txt_timer", _(pcall_timer));
}

void _vcui_view_qp_install_window(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	Evas_Object *win;
	vcui_app_call_data_t *ad = NULL;

	if (vd == NULL) {
		CALL_UI_DEBUG("View Data is NULL");
		return;
	}

	ad = vd->app_data;
	if (ad == NULL) {
		CALL_UI_DEBUG("app Data is NULL");
		return;
	}

	win = ad->win_quickpanel;
	if (ad->win_quickpanel == NULL) {
		CALL_UI_DEBUG("QP win is NULL");
		return;
	}
	evas_object_show(win);
	evas_object_show(vd->layout);
}

void _vcui_view_qp_uninstall_window(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	Evas_Object *win;
	vcui_app_call_data_t *ad = NULL;

	if (vd == NULL) {
		CALL_UI_DEBUG("View Data is NULL");
		return;
	}

	ad = vd->app_data;
	if (ad == NULL) {
		CALL_UI_DEBUG("app Data is NULL");
		return;
	}

	win = ad->win_quickpanel;
	if (ad->win_quickpanel == NULL) {
		CALL_UI_DEBUG("QP win is NULL");
		return;
	}
	elm_win_quickpanel_set(win, 0);

	evas_object_hide(ad->win_quickpanel);
	evas_object_hide(vd->layout);
}
