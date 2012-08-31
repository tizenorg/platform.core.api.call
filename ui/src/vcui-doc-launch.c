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


#include <app.h>

#include "vcui-application.h"
#include "contacts-ug.h"

#define MSG_COMPOSER_UG		"msg-composer-efl"
#define MSG_PKG				"org.tizen.message"
#define ADD_TO_CONTACTS_UG	"contacts-details-efl"
#define VIEW_CONTACT_UG		"contacts-details-efl"

struct vcui_ug_priv_data *local_priv_data = NULL;

#define COMMON_FUNCTIONS
#ifdef COMMON_FUNCTIONS
#if 0
static void __vcui_doc_launch_ug_common_back_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *navi = data;
	elm_naviframe_item_pop(navi);
}
#endif

#ifdef _LOCAL_UG_EFFECT_
static void __vcui_doc_launch_ug_common_hide_effect_finished(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	struct vcui_ug_priv_data *temp_priv_data = (struct vcui_ug_priv_data *)data;

	Evas_Object *base;
	base = temp_priv_data->need_layout;

	if (temp_priv_data->need_ug != NULL) {
		CALL_UI_DEBUG("__vcui_doc_launch_ug_common_hide_effect_finished start");

		if (base != NULL)
			evas_object_hide(base);

		elm_object_part_content_unset(base, "elm.swallow.content");

		if (temp_priv_data->on_destroy_callback != NULL)
			temp_priv_data->on_destroy_callback(temp_priv_data->destroy_callback_param);

		ug_destroy((ui_gadget_h)(temp_priv_data->need_ug));

		free(temp_priv_data);
		local_priv_data = NULL;

		evas_object_del(base);
		CALL_UI_DEBUG("__vcui_doc_launch_ug_common_hide_effect_finished end");
	}
}
#endif

static void __vcui_doc_launch_ug_common_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	Evas_Object *base, *win;

	if (!ug)
		return;

	base = ug_get_layout(ug);
	if (!base)
		return;

#ifdef _LOCAL_UG_EFFECT_
	Evas_Object *ly = elm_layout_add(temp_priv_data->need_parent);
	win = ug_get_window();
	elm_win_resize_object_add(win, ly);
	elm_layout_file_set(ly, EDJ_NAME, "ug_effect");
	elm_object_part_content_set(ly, "elm.swallow.content", base);
	evas_object_show(ly);
	temp_priv_data->need_layout = ly;

	edje_object_signal_callback_add(elm_layout_edje_get(ly), "elm,action,hide,finished", "", __vcui_doc_launch_ug_common_hide_effect_finished, temp_priv_data);
	edje_object_signal_emit(elm_layout_edje_get(ly), "elm,state,show", "");
#else
	win = ug_get_window();
	evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, base);
	evas_object_show(base);
#endif
}

static void __vcui_doc_launch_ug_common_destroy_cb(ui_gadget_h ug, void *priv)
{
	CALL_UI_DEBUG("");
	struct vcui_ug_priv_data *temp_priv_data = NULL;
	if (priv != NULL)
		temp_priv_data = (struct vcui_ug_priv_data *)priv;
	else
		temp_priv_data = local_priv_data;

#ifdef _LOCAL_UG_EFFECT_
	if (temp_priv_data->need_navi != NULL) {
		Evas_Object *base;
		base = ug_get_layout(ug);

		if (ug != NULL) {
			ug_destroy(ug);

			if (temp_priv_data->on_destroy_callback != NULL)
				temp_priv_data->on_destroy_callback(temp_priv_data->destroy_callback_param);


			struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)(temp_priv_data->need_ug_lists);
			if (pugs_array != NULL) {
				pugs_array->ug_lists = eina_list_remove(pugs_array->ug_lists, temp_priv_data->need_ug);
				pugs_array->ug_count = pugs_array->ug_count - 1;
				pugs_array->last_ug_type = VCUI_UG_TYPE_NOE;
			}
			free(temp_priv_data);
			local_priv_data = NULL;
		}
		CALL_UI_DEBUG("__vcui_doc_launch_ug_common_destroy_cb");
	} else {
		struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)(temp_priv_data->need_ug_lists);
		CALL_UI_DEBUG("pugs_array(0x%x)");
		if (pugs_array != NULL) {
			pugs_array->ug_lists = eina_list_remove(pugs_array->ug_lists, temp_priv_data->need_ug);
			pugs_array->ug_count = pugs_array->ug_count - 1;
			pugs_array->last_ug_type = VCUI_UG_TYPE_NOE;
		}
		edje_object_signal_emit(elm_layout_edje_get(temp_priv_data->need_layout), "elm,state,hide", "");
		CALL_UI_DEBUG("Send Hide");
	}
#else
	Evas_Object *base;
	base = ug_get_layout(ug);

	if (ug != NULL) {
		ug_destroy(ug);

		if (temp_priv_data->on_destroy_callback != NULL)
			temp_priv_data->on_destroy_callback(temp_priv_data->destroy_callback_param);


		struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)(temp_priv_data->need_ug_lists);
		if (pugs_array != NULL) {
			pugs_array->ug_lists = eina_list_remove(pugs_array->ug_lists, temp_priv_data->need_ug);
			pugs_array->ug_count = pugs_array->ug_count - 1;
			pugs_array->last_ug_type = VCUI_UG_TYPE_NOE;
		}
		free(temp_priv_data);
		local_priv_data = NULL;
	}
	CALL_UI_DEBUG("__vcui_doc_launch_ug_common_destroy_cb");
#endif
}
#endif

static void __vcui_doc_launch_ug_contact_list_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	__vcui_doc_launch_ug_common_layout_cb(ug, mode, priv);
	CALL_UI_DEBUG("__vcui_doc_launch_ug_contact_list_layout_cb success");
}

static void __vcui_doc_launch_ug_contact_list_destroy_cb(ui_gadget_h ug, void *priv)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	__vcui_doc_launch_ug_common_destroy_cb(ug, priv);
	ad->contact_ug = NULL;
	CALL_UI_DEBUG("__vcui_doc_launch_ug_contact_list_destroy_cb success");
}

static void __vcui_doc_launch_ug_contact_list_result_cb(ui_gadget_h ug, service_h result, void *priv)
{
	CALL_UI_DEBUG("__vcui_doc_launch_ug_contact_list_result_cb nothing");
}

void _vcui_doc_launch_contact_list_ug(void *parent_ui_gadget, void *navi, void *parent, void (*on_start_callback) (void *), void (*on_destroy_callback) (void *), void *callback_param, void *ugs_array_data)
{
	CALL_UI_DEBUG("_vcui_doc_launch_contact_list_ug start");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	service_h service;

	int ret = service_create(&service);
	if (ret < 0) {
		CALL_UI_DEBUG("service_create() return error : %d", ret);
		return;
	}

	service_add_extra_data(service, "type", "1");

	struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
	if (pugs_array->last_ug_type == VCUI_UG_TYPE_CONTACT_LIST) {
		CALL_UI_DEBUG("launch exit because of same ug execution");
		service_destroy(service);
		return;
	}
	struct ug_cbs cbs = { 0, };
	cbs.layout_cb = __vcui_doc_launch_ug_contact_list_layout_cb;
	cbs.destroy_cb = __vcui_doc_launch_ug_contact_list_destroy_cb;
	cbs.result_cb = __vcui_doc_launch_ug_contact_list_result_cb;

	struct vcui_ug_priv_data *temp_priv_data = malloc(sizeof(struct vcui_ug_priv_data));
	if (temp_priv_data == NULL) {
		CALL_UI_DEBUG("malloc error.");
		service_destroy(service);
		return;
	}
	temp_priv_data->on_start_callback = on_start_callback;
	temp_priv_data->on_destroy_callback = on_destroy_callback;
	temp_priv_data->destroy_callback_param = callback_param;
	temp_priv_data->need_navi = navi;
	temp_priv_data->need_parent = parent;
	temp_priv_data->need_ug_lists = ugs_array_data;
	local_priv_data = cbs.priv = temp_priv_data;

	ui_gadget_h ug = ug_create(parent_ui_gadget, PKGNAME_CONTACT_UG, UG_MODE_FULLVIEW, service, &cbs);
	temp_priv_data->need_ug = ug;
	if (ug == NULL) {
		free(temp_priv_data);
		local_priv_data = NULL;
		CALL_UI_DEBUG("_vcui_doc_launch_contact_list_ug fail");
	} else {
		struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
		pugs_array->ug_count = pugs_array->ug_count + 1;
		pugs_array->last_ug_type = VCUI_UG_TYPE_CONTACT_LIST;
		pugs_array->ug_lists = eina_list_append(pugs_array->ug_lists, (void *)ug);
		ad->contact_ug = ug;
	}
	service_destroy(service);
	CALL_UI_DEBUG("_vcui_doc_launch_contact_list_ug end");
}

static void __vcui_doc_launch_ug_phoneui_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	__vcui_doc_launch_ug_common_layout_cb(ug, mode, priv);
	CALL_UI_DEBUG("__vcui_doc_launch_ug_phoneui_layout_cb success");
}

static void __vcui_doc_launch_ug_phoneui_destroy_cb(ui_gadget_h ug, void *priv)
{
	__vcui_doc_launch_ug_common_destroy_cb(ug, priv);
	CALL_UI_DEBUG("__vcui_doc_launch_ug_phoneui_destroy_cb success");
}

static void __vcui_doc_launch_ug_phoneui_result_cb(ui_gadget_h ug, service_h result, void *priv)
{
	CALL_UI_DEBUG("__vcui_doc_launch_ug_phoneui_result_cb nothing");
}

void _vcui_doc_launch_phoneui_ug(void *parent_ui_gadget, void *navi, void *parent, void (*on_start_callback) (void *), void (*on_destroy_callback) (void *), void *callback_param, void *ugs_array_data)
{
	CALL_UI_DEBUG("launch_PHONEUI UG start");
	service_h service;

	int ret = service_create(&service);
	if (ret < 0) {
		CALL_UI_DEBUG("service_create() return error : %d", ret);
		return;
	}

	service_add_extra_data(service, "type", "51");

	struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
	if (pugs_array->last_ug_type == VCUI_UG_TYPE_ADD_CALL) {
		CALL_UI_DEBUG("launch exit because of same ug execution");
		service_destroy(service);
		return;
	}
	struct ug_cbs cbs = { 0, };
	cbs.layout_cb = __vcui_doc_launch_ug_phoneui_layout_cb;
	cbs.destroy_cb = __vcui_doc_launch_ug_phoneui_destroy_cb;
	cbs.result_cb = __vcui_doc_launch_ug_phoneui_result_cb;

	struct vcui_ug_priv_data *temp_priv_data = malloc(sizeof(struct vcui_ug_priv_data));
	if (temp_priv_data == NULL) {
		CALL_UI_DEBUG("malloc error.");
		service_destroy(service);
		return;
	}
	temp_priv_data->on_start_callback = on_start_callback;
	temp_priv_data->on_destroy_callback = on_destroy_callback;
	temp_priv_data->destroy_callback_param = callback_param;
	temp_priv_data->need_navi = navi;
	temp_priv_data->need_parent = parent;
	temp_priv_data->need_ug_lists = ugs_array_data;
	local_priv_data = cbs.priv = temp_priv_data;

	ui_gadget_h ug = ug_create(parent_ui_gadget, PKGNAME_DIALER_UG, UG_MODE_FULLVIEW, service, &cbs);
	temp_priv_data->need_ug = ug;
	if (ug == NULL) {
		free(temp_priv_data);
		local_priv_data = NULL;
		CALL_UI_DEBUG("_vcui_doc_launch_contact_list_ug fail");
	} else {
		struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
		pugs_array->ug_count = pugs_array->ug_count + 1;
		pugs_array->last_ug_type = VCUI_UG_TYPE_ADD_CALL;
		pugs_array->ug_lists = eina_list_append(pugs_array->ug_lists, (void *)ug);
	}
	service_destroy(service);
	CALL_UI_DEBUG("_vcui_doc_launch_contact_list_ug end");
}

#if 0
static void __vcui_doc_launch_render_flush_post_cb(void *data, Evas *e, void *event_info)
{
	vcui_app_call_data_t *ad = (vcui_app_call_data_t *)data;
	if (!ad)
		return;

	edje_object_signal_emit(_EDJ(ad->ly), "elm,state,show", "");
	evas_event_callback_del(e, EVAS_CALLBACK_RENDER_FLUSH_POST, __vcui_doc_launch_render_flush_post_cb);
}
#endif

void _vcui_doc_launch_msg_composer(void *data, char *number)
{
	CALL_UI_DEBUG("..");

	service_h service;
	int ret = SERVICE_ERROR_NONE;

	ret = service_create(&service);
	if (ret != SERVICE_ERROR_NONE) {
		CALL_UI_DEBUG("service_create() return error : %d", ret);
		return;
	}

	ret = service_set_operation(service, SERVICE_OPERATION_SEND_TEXT);
	if (ret != SERVICE_ERROR_NONE) {
		CALL_UI_DEBUG("service_set_operation() return error : %d", ret);
		ret = service_destroy(service);
		return;
	}

	ret = service_set_package(service, MSG_PKG);
	if (ret != SERVICE_ERROR_NONE) {
		CALL_UI_DEBUG("service_set_package() return error : %d", ret);
		ret = service_destroy(service);
		return;
	}

	ret = service_add_extra_data(service, SERVICE_DATA_TO, number);
	if (ret != SERVICE_ERROR_NONE) {
		CALL_UI_DEBUG("service_add_extra_data() return error : %d", ret);
		ret = service_destroy(service);
		return;
	}

	ret = service_send_launch_request(service, NULL, NULL);
	if (ret != SERVICE_ERROR_NONE) {
		CALL_UI_DEBUG("service_send_launch_request() is failed : %d", ret);
	}

	service_destroy(service);
}

void _vcui_doc_launch_destroy_ug_all(void *ugs_array_data)
{
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();

	struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
	if (pugs_array != NULL) {
		pugs_array->ug_lists = eina_list_free(pugs_array->ug_lists);
		pugs_array->ug_count = 0;
		pugs_array->last_ug_type = VCUI_UG_TYPE_NOE;

#ifdef _LOCAL_UG_EFFECT_
		if (local_priv_data->need_ug != NULL) {
			CALL_UI_DEBUG("__vcui_doc_launch_ug_common_hide_effect_finished start");
			base = local_priv_data->need_layout;

			if (base != NULL) {
				evas_object_hide(base);
				elm_layout_content_unset(base, "elm.swallow.content");
				evas_object_del(base);
			}
			if (local_priv_data->on_destroy_callback != NULL)
				local_priv_data->on_destroy_callback(local_priv_data->destroy_callback_param);

			ug_destroy_all();

			if (ad->contact_ug != NULL) {
				ad->contact_ug = NULL;
				_vcui_view_common_call_terminate_or_view_change();
			}

			free(local_priv_data);
			local_priv_data = NULL;

			CALL_UI_DEBUG("__vcui_doc_launch_ug_common_hide_effect_finished end");
		}
#else
		ug_destroy_all();

		if (ad->contact_ug != NULL) {
			ad->contact_ug = NULL;
			_vcui_view_common_call_terminate_or_view_change();
		}

		if (local_priv_data != NULL) {
			free(local_priv_data);
			local_priv_data = NULL;
		}
#endif
	}
}

static void __vcui_doc_launch_ug_contact_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	__vcui_doc_launch_ug_common_layout_cb(ug, mode, priv);
	CALL_UI_DEBUG("__vcui_doc_launch_ug_contact_layout_cb success");
}

static void __vcui_doc_launch_ug_contact_result_cb(ui_gadget_h ug, service_h result, void *priv)
{
	CALL_UI_DEBUG("__vcui_doc_launch_ug_phoneui_result_cb nothing");
}

static void __vcui_doc_launch_ug_contact_destroy_cb(ui_gadget_h ug, void *priv)
{
	CALL_UI_DEBUG("__vcui_doc_launch_ug_contact_destroy_cb..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	__vcui_doc_launch_ug_common_destroy_cb(ug, priv);
	ad->contact_ug = NULL;
	_vcui_view_common_call_terminate_or_view_change();
}

void _vcui_doc_launch_add_to_contacts_ug(void *parent_ui_gadget, void *navi, void *parent, void (*on_start_callback) (void *), void (*on_destroy_callback) (void *), void *callback_param, void *ugs_array_data, void *data)
{
	CALL_UI_DEBUG("launch_ADD_TO_CONTACTS UG start");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	service_h service;
	char buf[4] = {0,};

	int ret = service_create(&service);
	if (ret < 0) {
		CALL_UI_DEBUG("service_create() return error : %d", ret);
		return;
	}

	snprintf(buf, sizeof(buf), "%d", 22);
	service_add_extra_data(service, "type", buf);
	CALL_UI_DEBUG("number %s", (char *)data);
	service_add_extra_data(service, "ct_num", (char *)data);

	struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
	if (pugs_array->last_ug_type == VCUI_UG_TYPE_ADD_TO_CONTACTS) {
		CALL_UI_DEBUG("launch exit because of same ug execution");
		service_destroy(service);
		return;
	}
	struct ug_cbs cbs = { 0, };
	cbs.layout_cb = __vcui_doc_launch_ug_contact_layout_cb;
	cbs.destroy_cb = __vcui_doc_launch_ug_contact_destroy_cb;
	cbs.result_cb = __vcui_doc_launch_ug_contact_result_cb;

	struct vcui_ug_priv_data *temp_priv_data = malloc(sizeof(struct vcui_ug_priv_data));
	if (temp_priv_data == NULL) {
		CALL_UI_DEBUG("malloc error.");
		service_destroy(service);
		return;
	}
	temp_priv_data->on_start_callback = on_start_callback;
	temp_priv_data->on_destroy_callback = on_destroy_callback;
	temp_priv_data->destroy_callback_param = callback_param;
	temp_priv_data->need_navi = navi;
	temp_priv_data->need_parent = parent;
	temp_priv_data->need_ug_lists = ugs_array_data;
	local_priv_data = cbs.priv = temp_priv_data;

	ui_gadget_h ug = ug_create(parent_ui_gadget, ADD_TO_CONTACTS_UG, UG_MODE_FULLVIEW, service, &cbs);
	temp_priv_data->need_ug = ug;
	if (ug == NULL) {
		free(temp_priv_data);
		local_priv_data = NULL;
		CALL_UI_DEBUG("_vcui_doc_launch_add_to_contacts_ug fail");
	} else {
		struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
		pugs_array->ug_count = pugs_array->ug_count + 1;
		pugs_array->last_ug_type = VCUI_UG_TYPE_ADD_TO_CONTACTS;
		pugs_array->ug_lists = eina_list_append(pugs_array->ug_lists, (void *)ug);
		ad->contact_ug = ug;
	}
	service_destroy(service);
	CALL_UI_DEBUG("_vcui_doc_launch_add_to_contacts_ug end");
}

void _vcui_doc_launch_view_contact_ug(void *parent_ui_gadget, void *navi, void *parent, void (*on_start_callback) (void *), void (*on_destroy_callback) (void *), void *callback_param, void *ugs_array_data, void *data)
{
	CALL_UI_DEBUG("launch_VIEW_CONTACT UG start");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	service_h service;
	char buf[4] = {0,};
	char contactId[128] = {0,};

	int ret = service_create(&service);
	if (ret < 0) {
		CALL_UI_DEBUG("service_create() return error : %d", ret);
		return;
	}

	snprintf(buf, sizeof(buf), "%d", 0);
	service_add_extra_data(service, "type", buf);	/*CT_UG_REQUEST_DETAIL*/
	CALL_UI_DEBUG("ct_id %d", (int)data);
	snprintf(contactId, sizeof(contactId), "%d", (int)data);
	service_add_extra_data(service, "ct_id", contactId);

	struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
	if (pugs_array->last_ug_type == VCUI_UG_TYPE_VIEW_CONTACT) {
		CALL_UI_DEBUG("launch exit because of same ug execution");
		service_destroy(service);
		return;
	}
	struct ug_cbs cbs = { 0, };
	cbs.layout_cb = __vcui_doc_launch_ug_contact_layout_cb;
	cbs.destroy_cb = __vcui_doc_launch_ug_contact_destroy_cb;
	cbs.result_cb = __vcui_doc_launch_ug_contact_result_cb;

	struct vcui_ug_priv_data *temp_priv_data = malloc(sizeof(struct vcui_ug_priv_data));
	if (temp_priv_data == NULL) {
		CALL_UI_DEBUG("malloc error.");
		service_destroy(service);
		return;
	}
	temp_priv_data->on_start_callback = on_start_callback;
	temp_priv_data->on_destroy_callback = on_destroy_callback;
	temp_priv_data->destroy_callback_param = callback_param;
	temp_priv_data->need_navi = navi;
	temp_priv_data->need_parent = parent;
	temp_priv_data->need_ug_lists = ugs_array_data;
	local_priv_data = cbs.priv = temp_priv_data;

	ui_gadget_h ug = ug_create(parent_ui_gadget, VIEW_CONTACT_UG, UG_MODE_FULLVIEW, service, &cbs);
	temp_priv_data->need_ug = ug;
	if (ug == NULL) {
		free(temp_priv_data);
		local_priv_data = NULL;
		CALL_UI_DEBUG("_vcui_doc_launch_view_contact_ug fail");
	} else {
		struct vcui_ugs_array *pugs_array = (struct vcui_ugs_array *)ugs_array_data;
		pugs_array->ug_count = pugs_array->ug_count + 1;
		pugs_array->last_ug_type = VCUI_UG_TYPE_VIEW_CONTACT;
		pugs_array->ug_lists = eina_list_append(pugs_array->ug_lists, (void *)ug);
		ad->contact_ug = ug;
	}
	service_destroy(service);
	CALL_UI_DEBUG("_vcui_doc_launch_view_contact_ug end");
}
