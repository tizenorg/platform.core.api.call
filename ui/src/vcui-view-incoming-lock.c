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

#include <msg.h>
#include <msg_transport.h>
#include <dlfcn.h>

#include "vcui-application.h"
#include "vcui-app-window.h"
#include "vcui-view-incoming-lock.h"
#include "vcui-view-layout-hd.h"

#define VCUI_REJECT_MSG_LENGTH_MAX (140+1)
#define REJ_MSG_GENLIST_DATA "reject_msg_genlist_data"
#define REJ_MSG_LIST_OPEN_STATUS_KEY "list_open_status_key"
#define REJ_MSG_GENLIST_INDEX "reject_msg_genlist_index"
#define VCUI_CST_REJECT_MSG_GET	"cst_reject_msg_get"
#define VCUI_CST_SO_PATH	"/opt/ug/lib/libug-setting-call-efl.so"
#define VIEW_INCOMING_LOCK_LAYOUT_ID "INCOMINGLOCKVIEW"

typedef struct {
	Evas_Object *contents;
	Evas_Object *caller_info;
	Evas_Object *ic;
	Evas_Object *lock_reject_with_msg;
	Evas_Object *msg_glist;

	double y_scale_factor;

	int reject_with_msg_start_y;
	int reject_with_msg_cur_y;

	int msg_list_height;

	gboolean bmouse_down_pressed;	/*Tracks the mouse-down, mouse-move and mouse-up events are executed in sequence*/

	void *dl_handle;
	Elm_Genlist_Item_Class *itc_reject_msg;
	Evas_Coord y_momentum;
	char *(*msg_func_ptr) (int);
	char reject_msg[VCUI_REJECT_MSG_LENGTH_MAX];
} incoming_lock_view_priv_t;

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

static void __vcui_view_incoming_lock_accept_cb(void *data, Evas_Object *obj, void *event_info)
{
	int all_call_count = 0;
	int nostatus_call_count = 0;
	int unhold_call_count = 0;
	int hold_call_count = 0;

	if (_vcui_is_phonelock_status() == EINA_FALSE)
		vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_UNLOCK);

	all_call_count = _vcui_doc_get_all_call_data_count();
	nostatus_call_count = _vcui_doc_get_no_status_call_data_count();
	unhold_call_count = _vcui_doc_get_unhold_call_data_count();
	hold_call_count = _vcui_doc_get_hold_call_data_count();
	CALL_UI_DEBUG("all_call_count:%d >>>> nostatus_call_count:%d >>>> unhold_call_count:%d >>>> hold_call_count:%d", \
			all_call_count, nostatus_call_count, unhold_call_count, hold_call_count);

	if (unhold_call_count == 0) {
		CALL_UI_DEBUG("No Call Or Held call - Accept");
		vcall_engine_answer_call();
	} else {
		CALL_UI_DEBUG("Show popup - 2nd MT call - test volume popup");
		_vcui_view_popup_second_mtcall_load(_("IDS_CALL_HEADER_ACCEPT_CALL_AFTER"), unhold_call_count, hold_call_count);
	}
}

static Evas_Object *__vcui_view_incoming_lock_create_accept_btn(Evas_Object *layout)
{
	Evas_Object *btn;

	Evas_Object *sw = edje_object_part_swallow_get(_EDJ(layout), "btn_accept");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_accept", btn);
	elm_object_style_set(btn, "sweep");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_ACCEPT"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_view_incoming_lock_accept_cb, NULL);

	return btn;
}

static void __vcui_view_incoming_lock_reject_cb(void *data, Evas_Object *obj, void *event_info)
{
	vcall_engine_reject_call();
}

static Evas_Object *__vcui_view_incoming_lock_create_reject_btn(Evas_Object *layout)
{
	Evas_Object *btn;

	Evas_Object *sw = edje_object_part_swallow_get(_EDJ(layout), "btn_reject");
	if (sw) {
		edje_object_part_unswallow(_EDJ(layout), sw);
		evas_object_del(sw);
	}

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_reject", btn);
	elm_object_style_set(btn, "sweep");
	elm_object_text_set(btn, _("IDS_CALL_BUTTON_REJECT"));
	evas_object_smart_callback_add(btn, "clicked", __vcui_view_incoming_lock_reject_cb, NULL);

	return btn;
}

static void __reject_msg_list_param_reset(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;

	priv->y_momentum = 0;
	priv->reject_with_msg_start_y = 0;
	priv->reject_with_msg_cur_y = 0;
	priv->bmouse_down_pressed = EINA_FALSE;
}

static void __reject_screen_transit_complete_cb(void *data, Elm_Transit *transit)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;

	if (EINA_FALSE == evas_object_data_get(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY)) {
		edje_object_signal_emit(priv->lock_reject_with_msg, "show-up-arrow", "reject_msg");
	} else {
		edje_object_signal_emit(priv->lock_reject_with_msg, "show-down-arrow", "reject_msg");
	}

	__reject_msg_list_param_reset(vd);

	return;
}

static Eina_Bool __rej_msg_show_sliding_effect(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Object *screen_ly;
	Elm_Transit *transit;
	int xpos = 0;
	int ypos = 0;
	int width = 0;
	int height = 0;
	int transit_y = 0;
	int max_height_limit = 0;

	screen_ly = priv->lock_reject_with_msg;
	transit = elm_transit_add();
	elm_transit_object_add(transit, screen_ly);

	evas_object_geometry_get(priv->lock_reject_with_msg, &xpos, &ypos, &width, &height);
	CALL_UI_DEBUG("reject_w_msg dimensions ---> x[%d] y[%d] w[%d] h[%d]", xpos, ypos, width, height);
	CALL_UI_DEBUG("priv->y_momentum: %d", priv->y_momentum);

	/*Max height possible*/
	max_height_limit = (priv->msg_list_height) * (priv->y_scale_factor);
	CALL_UI_DEBUG("max_height_limit: %d", max_height_limit);

	if (EINA_FALSE == evas_object_data_get(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY)) {
		CALL_UI_DEBUG("Close list... Check if opening is feasible");

		if (priv->y_momentum) {
			CALL_UI_DEBUG("Momentum...");

			if (priv->y_momentum < -1500) {
				CALL_UI_DEBUG("Huge Momentum... Move the layout");

				/*effect to pull up the window.*/
				transit_y = -(max_height_limit + ypos);

				elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
				evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_TRUE);
			} else {
				CALL_UI_DEBUG("Small Momentum..");

				if (ypos != 0) {	/*Reject msg layout is displaced from its original position*/
					if (-ypos < max_height_limit/2) {	/*Layout position is lesser than half of the height*/
						CALL_UI_DEBUG("Movement L.T. HALF the height..");

						/*effect to pull down the window.*/
						transit_y = -ypos;

						elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
						evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_FALSE);
					} else if (-ypos >= max_height_limit/2 && -ypos <= max_height_limit) {	/*Layout position is greater than half of the height*/
						CALL_UI_DEBUG("Movement G.T. HALF the height..");

						/*effect to pull up the window.*/
						transit_y = -(max_height_limit + ypos);

						elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
						evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_TRUE);
					}
				}
			}
		} else {
			CALL_UI_DEBUG("NO Momentum... Dont open");

			if (ypos != 0) {	/*Reject msg layout is displaced from its original position*/
				if (-ypos < max_height_limit/2) {	/*Layout position is lesser than half of the height*/
					CALL_UI_DEBUG("Movement L.T. HALF the height..");

					/*effect to pull down the window.*/
					transit_y = -ypos;

					elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
					evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_FALSE);
				} else if (-ypos >= max_height_limit/2 && -ypos <= max_height_limit) {	/*Layout position is greater than half of the height*/
					CALL_UI_DEBUG("Movement G.T. HALF the height..");

					/*effect to pull up the window.*/
					transit_y = -(max_height_limit + ypos);

					elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
					evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_TRUE);
				}
			}
		}
	} else {
		CALL_UI_DEBUG("Open list... Check if closing is feasible");

		if (priv->y_momentum) {
			CALL_UI_DEBUG("Momentum...");

			if (priv->y_momentum > 1500) {
				CALL_UI_DEBUG("Huge Momentum... Move the layout");

				/*effect to pull down the window.*/
				transit_y = -ypos;

				elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
				evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_FALSE);
			} else {
				CALL_UI_DEBUG("Small Momentum..");

				if (ypos != 0) {	/*Reject msg layout is displaced from its original position*/
					if (-ypos < (max_height_limit * 0.8)) {	/*Layout position is lesser than 80% of the height*/
						CALL_UI_DEBUG("Movement L.T. 80 percent of the height..");

						/*effect to pull down the window.*/
						transit_y = -ypos;

						elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
						evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_FALSE);
					} else if (-ypos >= (max_height_limit * 0.8) && -ypos <= max_height_limit) {	/*Layout position is greater than half of the height*/
						CALL_UI_DEBUG("Movement G.T. 80 percent of the height..");

						/*effect to pull up the window.*/
						transit_y = -(max_height_limit + ypos);

						elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
						evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_TRUE);
					}
				}
			}
		} else {
			CALL_UI_DEBUG("NO Momentum... Dont close");

			if (ypos != 0) {	/*Reject msg layout is displaced from its original position*/
				if (-ypos < (max_height_limit * 0.8)) {	/*Layout position is lesser than 80% of the height*/
					CALL_UI_DEBUG("Movement L.T. 80 percent of the height..");

					/*effect to pull down the window.*/
					transit_y = -ypos;

					elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
					evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_FALSE);
				} else if (-ypos >= (max_height_limit * 0.8) && -ypos <= max_height_limit) {	/*Layout position is greater than half of the height*/
					CALL_UI_DEBUG("Movement G.T. 80 percent of the height..");

					/*effect to pull up the window.*/
					transit_y = -(max_height_limit + ypos);

					elm_transit_effect_translation_add(transit, 0, 0, 0, transit_y);
					evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_TRUE);
				}
			}
		}
	}

	elm_transit_del_cb_set(transit, __reject_screen_transit_complete_cb, vd);

	if (priv->y_momentum < 0)
		priv->y_momentum = -priv->y_momentum;

	if (priv->y_momentum < 1500) {
		elm_transit_duration_set(transit, 0.5);
	} else if (priv->y_momentum >= 1500 && priv->y_momentum < 3000) {
		elm_transit_duration_set(transit, 0.4);
	} else if (priv->y_momentum >= 3000 && priv->y_momentum < 4500) {
		elm_transit_duration_set(transit, 0.3);
	} else if (priv->y_momentum >= 4500) {
		elm_transit_duration_set(transit, 0.2);
	}
	evas_object_show(screen_ly);	/*It must be called before elm_transit_go(). transit policy*/
	elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);	/*Keep the window position as end of effect.*/
	elm_transit_go(transit);

	return ECORE_CALLBACK_CANCEL;
}

static void __vcui_view_incoming_lock_reject_with_msg_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Event_Mouse_Move *ev = event_info;

	priv->reject_with_msg_start_y = ev->cur.canvas.y;
	priv->bmouse_down_pressed = EINA_TRUE;
}

static void __vcui_view_incoming_lock_reject_with_msg_mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Event_Mouse_Move *ev = event_info;
	int diff_y = 0;
	int max_height_limit = 0;

	/*Max height possible*/
	max_height_limit = (priv->msg_list_height) * (priv->y_scale_factor);
	CALL_UI_DEBUG("max_height_limit: %d", max_height_limit);

	if (priv->bmouse_down_pressed) {
		CALL_UI_DEBUG("mouse down was pressed - handle move event");
		priv->reject_with_msg_cur_y = ev->cur.canvas.y;

		diff_y = (priv->reject_with_msg_cur_y - priv->reject_with_msg_start_y) * (priv->y_scale_factor);
		CALL_UI_DEBUG("diff_y [<<< %d >>>>]", diff_y);

		if (EINA_FALSE == evas_object_data_get(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY)) {
			if ((diff_y > -max_height_limit) && (diff_y <= 0)) {
				/*Lies between 0 and msg-list layout height*/
				evas_object_move(priv->lock_reject_with_msg, 0, diff_y);
			} else if (diff_y <= -max_height_limit) {
				/*Special case - Move the max distance - msg-list height*/
				evas_object_move(priv->lock_reject_with_msg, 0, -max_height_limit);
				evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_TRUE);
				__reject_screen_transit_complete_cb(vd, NULL);
			}
		} else {
			if ((diff_y >= 0) && (diff_y < max_height_limit)) {
				/*Lies between 0 and msg-list layout height*/
				evas_object_move(priv->lock_reject_with_msg, 0, -(max_height_limit - diff_y));
			} else if (diff_y >= max_height_limit) {
				/*Special case - Move the max distance - msg-list height*/
				evas_object_move(priv->lock_reject_with_msg, 0, 0);
				evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, (const void *)EINA_FALSE);
				__reject_screen_transit_complete_cb(vd, NULL);
			}
		}
	} else {
		CALL_UI_DEBUG("mouse down was NOT pressed - DONT handle move event");
	}
}

static void __vcui_view_incoming_lock_reject_with_msg_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;

	if (priv->bmouse_down_pressed) {
		ecore_idler_add(__rej_msg_show_sliding_effect, vd);
	} else {
		CALL_UI_DEBUG("mouse down was NOT pressed - DONT handle up event");
	}
}

static void __send_reject_msg_status_cb(msg_handle_t Handle, msg_struct_t pStatus, void *pUserParam)
{
	VCUI_RETURN_IF_FAIL(pStatus != NULL);
	int status = MSG_NETWORK_SEND_FAIL;

	msg_get_int_value(pStatus, MSG_SENT_STATUS_NETWORK_STATUS_INT, &status);
	CALL_UI_DEBUG("status:[%d]", status);
	if (status != MSG_NETWORK_SEND_SUCCESS) {
		CALL_UI_DEBUG("..");
		_vcui_view_popup_replace(_("IDS_MSGC_POP_SENDINGFAILED"), POPUP_TIMEOUT_SHORT, EINA_TRUE);
	}
}

static Eina_Bool __send_reject_msg_on_idle(void *data)
{
	VCUI_RETURN_FALSE_IF_FAIL(data != NULL);

	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	call_data_t *call_data = NULL;

	msg_error_t err = MSG_SUCCESS;
	msg_handle_t msgHandle = NULL;

	call_data = _vcui_doc_get_recent_mt_call_data();
	if (call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return EINA_FALSE;
	}

	err = msg_open_msg_handle(&msgHandle);
	if (err != MSG_SUCCESS) {
		CALL_UI_DEBUG("msg_open_msg_handle()- failed [%d]", err);
		return EINA_FALSE;
	}

	err = msg_reg_sent_status_callback(msgHandle, &__send_reject_msg_status_cb, NULL);
	if (err != MSG_SUCCESS) {
		CALL_UI_DEBUG("msg_reg_sent_status_callback()- failed [%d]", err);
		msg_close_msg_handle(&msgHandle);
		return EINA_FALSE;
	}
	msg_struct_t msgInfo = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
	msg_struct_t sendOpt = msg_create_struct(MSG_STRUCT_SENDOPT);
	msg_struct_t pReq = msg_create_struct(MSG_STRUCT_REQUEST_INFO);

	/* Set message type to SMS reject*/
	msg_set_int_value(msgInfo, MSG_MESSAGE_TYPE_INT, MSG_TYPE_SMS_REJECT);

	/* No setting send option */
	msg_set_bool_value(sendOpt, MSG_SEND_OPT_SETTING_BOOL, FALSE);

	/* Set message body */
	if (msg_set_str_value(msgInfo, MSG_MESSAGE_SMS_DATA_STR, priv->reject_msg, strlen(priv->reject_msg))!= MSG_SUCCESS)	{
		return EINA_FALSE;
	}

	/* Create address list*/
	msg_struct_list_s *addr_list;
	msg_get_list_handle(msgInfo, MSG_MESSAGE_ADDR_LIST_STRUCT, (void **)&addr_list);
	msg_struct_t addr_info = addr_list->msg_struct_info[0];
	char *call_number = _vcui_doc_get_call_number(call_data);

	/* Set message address */
	msg_set_int_value(addr_info, MSG_ADDRESS_INFO_RECIPIENT_TYPE_INT, MSG_RECIPIENTS_TYPE_TO);
	msg_set_str_value(addr_info, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR, call_number, strlen(call_number));
	addr_list->nCount = 1;

	/* Set message struct to Request*/
	msg_set_struct_handle(pReq, MSG_REQUEST_MESSAGE_HND, msgInfo);
	msg_set_struct_handle(pReq, MSG_REQUEST_SENDOPT_HND, sendOpt);

	/* Send message */
	err = msg_sms_send_message(msgHandle, pReq);
	if (err != MSG_SUCCESS) {
		CALL_UI_DEBUG("msg_sms_send_message() - failed [%d]", err);
		msg_close_msg_handle(&msgHandle);
		_vcui_view_popup_replace(_("IDS_MSGC_POP_SENDINGFAILED"), POPUP_TIMEOUT_SHORT, EINA_TRUE);
	} else {
		CALL_UI_DEBUG("Sending...");
	}

	msg_release_struct(&pReq);
	msg_release_struct(&msgInfo);
	msg_release_struct(&sendOpt);

	return EINA_FALSE;
}

static char *__reject_list_get_msg_by_index(void *data, int index)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;

	char *ret_str = NULL;

	if (priv->msg_func_ptr) {
		ret_str = (char *) priv->msg_func_ptr(index);  /* i : reject msg index(0 ~ 4)*/
		CALL_UI_DEBUG("ret_str(%s)", ret_str);
	}

	return ret_str;
}

static char *__reject_msg_gl_label_get_msg(void *data, Evas_Object *obj, const char *part)
{
	VCUI_RETURN_NULL_IF_FAIL(part != NULL);
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)evas_object_data_get(obj, REJ_MSG_GENLIST_DATA);

	int index = (int)data;
	CALL_UI_DEBUG("index: %d", index);
	char *entry_label = NULL;
	char *msg_str = NULL;

	if (!strcmp(part, "elm.text")) {
		if (index != -1) {
			msg_str = __reject_list_get_msg_by_index(vd, index);
			CALL_UI_DEBUG("msg_str(%s)", msg_str);

			if (msg_str != NULL) {
				entry_label = elm_entry_markup_to_utf8(msg_str);
				if (entry_label == NULL) {
					free(msg_str);
					msg_str = NULL;
					return NULL;
				}
				free(msg_str);
				msg_str = NULL;
				return entry_label;
			}
		} else {
			CALL_UI_DEBUG("..");
			msg_str = _("No message");
			return strdup(msg_str);
		}
	}

	return NULL;
}

static void __reject_msg_send_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	VCUI_RETURN_IF_FAIL(data != NULL);
	CALL_UI_DEBUG("..");

	char *ret_str = NULL;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	int index = (int)evas_object_data_get(obj, REJ_MSG_GENLIST_INDEX);

	ret_str =  __reject_list_get_msg_by_index(vd, index);
	if (ret_str) {
		snprintf(priv->reject_msg, sizeof(priv->reject_msg), "%s", ret_str);
		free(ret_str);
	}

	vcall_engine_reject_call();

	ecore_timer_add(TIMER_TIMEOUT_0_3_SEC, __send_reject_msg_on_idle, vd);
}

static Evas_Object *__reject_msg_gl_content_get_msg(void *data, Evas_Object *obj, const char *part)
{
	CALL_UI_DEBUG("..");

	Evas_Object *button;
	int index = (int)data;
	CALL_UI_DEBUG("index: %d", index);

	if (!strcmp(part, "elm.swallow.end")) {
		if (index != -1) {
			voice_call_view_data_t *vd = (voice_call_view_data_t *)evas_object_data_get(obj, REJ_MSG_GENLIST_DATA);
			button = elm_button_add(obj);
			evas_object_data_set(button, REJ_MSG_GENLIST_INDEX, data);
			elm_object_text_set(button, dgettext("sys_string", "IDS_COM_SK_SEND"));
			evas_object_propagate_events_set(button, EINA_FALSE);
			evas_object_smart_callback_add(button, "clicked", __reject_msg_send_btn_cb, vd);
			return button;
		} else {
			CALL_UI_DEBUG("do nothing for index [%d]", index);
		}
	}

	return NULL;
}

static void __reject_msg_gl_sel_msg(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}

static Evas_Event_Flags __reject_msg_flick_gesture_move_event_cb(void *data, void *event_info)
{
	CALL_UI_DEBUG("Flick_Gesture Move");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Elm_Gesture_Line_Info *info = (Elm_Gesture_Line_Info *)event_info;

	CALL_UI_DEBUG("*********************************************");
	CALL_UI_DEBUG("info->angle = %lf", info->angle);
	CALL_UI_DEBUG("info->momentum.mx = %d, info->momentum.my = %d", info->momentum.mx, info->momentum.my);
	CALL_UI_DEBUG("info->momentum.n = %d", info->momentum.n);
	CALL_UI_DEBUG("info->momentum.tx = %d, info->momentum.ty = %d", info->momentum.tx, info->momentum.ty);
	CALL_UI_DEBUG("info->momentum.x1 = %d, info->momentum.x2 = %d", info->momentum.x1, info->momentum.x2);
	CALL_UI_DEBUG("info->momentum.y1 = %d, info->momentum.y2 = %d", info->momentum.y1, info->momentum.y2);
	CALL_UI_DEBUG("*********************************************");

	priv->y_momentum = info->momentum.my;

	return EVAS_EVENT_FLAG_NONE;
}

static void __reject_msg_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;

	if (NULL == vd) {
		CALL_UI_DEBUG("view data is NULL");
		return;
	}
	evas_object_move(priv->lock_reject_with_msg, 0, 0);
	edje_object_signal_emit(priv->lock_reject_with_msg, "show-up-arrow", "reject_msg");
	evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, EINA_FALSE);

}

static void __create_new_msg_cb(void *data, Evas_Object *obj, void *event_info)
{
	VCUI_RETURN_IF_FAIL(data != NULL);
	CALL_UI_DEBUG("..");

	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	char *tel_number = NULL;
	call_data_t *call_data = NULL;

	vcall_engine_reject_call();

	vconf_set_int(VCONFKEY_IDLE_LOCK_STATE, VCONFKEY_IDLE_UNLOCK);

	call_data = _vcui_doc_get_recent_mt_call_data();
	if (call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return;
	}
	tel_number = _vcui_doc_get_call_number(call_data);
	_vcui_doc_launch_msg_composer(vd, tel_number);
}

static void __reject_msg_create_call_setting_handle(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	char *error = NULL;

	priv->dl_handle = dlopen(VCUI_CST_SO_PATH, RTLD_LAZY);
	if (priv->dl_handle) {
		priv->msg_func_ptr = dlsym(priv->dl_handle, VCUI_CST_REJECT_MSG_GET);
		if ((error = dlerror()) != NULL) {
			CALL_UI_DEBUG("dlsym failed!!! error = %s", error);
			priv->msg_func_ptr = NULL;
			dlclose(priv->dl_handle);
			priv->dl_handle = NULL;
		}
	} else {
		CALL_UI_DEBUG("failed to open libug-setting-call-efl.so");
	}
}

static void __reject_msg_list_height_update(void *data, int msg_cnt)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;

	CALL_UI_DEBUG("msg_cnt[%d]", msg_cnt);

	switch (msg_cnt) {
	case 0:
	case 1:
		priv->msg_list_height = MTLOCK_REJECT_MSG_ONE_LINE_LIST_HEIGHT+MTLOCK_REJECT_MSG_TOOLBAR_HEIGHT;
		edje_object_signal_emit(priv->lock_reject_with_msg, "show-one-line", "reject-list");
		break;
	case 2:
		priv->msg_list_height = MTLOCK_REJECT_MSG_TWO_LINE_LIST_HEIGHT+MTLOCK_REJECT_MSG_TOOLBAR_HEIGHT;
		edje_object_signal_emit(priv->lock_reject_with_msg, "show-two-line", "reject-list");
		break;
	case 3:
		priv->msg_list_height = MTLOCK_REJECT_MSG_THREE_LINE_LIST_HEIGHT+MTLOCK_REJECT_MSG_TOOLBAR_HEIGHT;
		edje_object_signal_emit(priv->lock_reject_with_msg, "show-three-line", "reject-list");
		break;
	case 4:
		priv->msg_list_height = MTLOCK_REJECT_MSG_FOUR_LINE_LIST_HEIGHT+MTLOCK_REJECT_MSG_TOOLBAR_HEIGHT;
		edje_object_signal_emit(priv->lock_reject_with_msg, "show-four-line", "reject-list");
		break;
	case 5:
		priv->msg_list_height = MTLOCK_REJECT_MSG_FIVE_LINE_LIST_HEIGHT+MTLOCK_REJECT_MSG_TOOLBAR_HEIGHT;
		edje_object_signal_emit(priv->lock_reject_with_msg, "show-five-line", "reject-list");
		break;
	case 6:
		priv->msg_list_height = MTLOCK_REJECT_MSG_SIX_LINE_LIST_HEIGHT+MTLOCK_REJECT_MSG_TOOLBAR_HEIGHT;
		edje_object_signal_emit(priv->lock_reject_with_msg, "show-six-line", "reject-list");
		break;
	default:
		break;
	}
}

static void __reject_msg_create_glist(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	int msg_cnt = 0;
	int index = 0;

	/* msg count from setting */
	vconf_get_int(VCONFKEY_CISSAPPL_REJECT_CALL_MSG_INT, &msg_cnt);
	CALL_UI_DEBUG("msg_cnt: %d", msg_cnt);

	/* create gen list */
	priv->msg_glist = elm_genlist_add(priv->lock_reject_with_msg);
	evas_object_data_set(priv->msg_glist, REJ_MSG_GENLIST_DATA, (const void *)vd);
	evas_object_size_hint_weight_set(priv->msg_glist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(priv->msg_glist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	edje_object_part_swallow(priv->lock_reject_with_msg, "swl_msglist", priv->msg_glist);

	priv->itc_reject_msg = elm_genlist_item_class_new();

	priv->itc_reject_msg->item_style = "call/reject_msg";
	priv->itc_reject_msg->func.text_get = __reject_msg_gl_label_get_msg;
	priv->itc_reject_msg->func.content_get = __reject_msg_gl_content_get_msg;
	priv->itc_reject_msg->func.state_get = NULL;
	priv->itc_reject_msg->func.del = NULL;

	if (msg_cnt == 0 ) {
		index = -1;
		elm_genlist_item_append(priv->msg_glist, priv->itc_reject_msg, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, __reject_msg_gl_sel_msg, NULL);
	} else {
		for (index = 0; index < msg_cnt; index++) {
			elm_genlist_item_append(priv->msg_glist, priv->itc_reject_msg, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, __reject_msg_gl_sel_msg, NULL);
		}
	}

	/*Adjust the list height*/
	__reject_msg_list_height_update(vd, msg_cnt);
}

static void __reject_msg_create_gesture_layer(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Object *rej_msg_gesture_layer = NULL;
	Evas_Object *rej_msg_bg = NULL;

	rej_msg_bg = (Evas_Object *)edje_object_part_object_get(priv->lock_reject_with_msg, "reject_msg_bg");
	rej_msg_gesture_layer = elm_gesture_layer_add(priv->lock_reject_with_msg);
	if (FALSE == elm_gesture_layer_attach(rej_msg_gesture_layer, (Evas_Object *)rej_msg_bg)) {
		CALL_UI_DEBUG("elm_gesture_layer_attach failed !!");
		evas_object_del(rej_msg_gesture_layer);
		rej_msg_gesture_layer = NULL;
	} else {
		elm_gesture_layer_cb_set(rej_msg_gesture_layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_MOVE, __reject_msg_flick_gesture_move_event_cb, vd);
	}
}

static void __reject_msg_create_toolbar(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Object *rej_msg_toolbar = NULL;
	Evas_Object *btn = NULL;
	Elm_Object_Item *item = NULL;

	rej_msg_toolbar = elm_toolbar_add(priv->lock_reject_with_msg);
	elm_toolbar_homogeneous_set(rej_msg_toolbar, TRUE);
	elm_toolbar_shrink_mode_set(rej_msg_toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	if (_vcui_view_common_is_emul_bin() == EINA_TRUE) {
		item = elm_toolbar_item_append(rej_msg_toolbar, NULL, NULL, NULL, NULL);
		elm_object_item_disabled_set(item, EINA_TRUE);
	} else {
		elm_toolbar_item_append(rej_msg_toolbar, REJ_MSG_CREATE_ICON, NULL, __create_new_msg_cb, vd);
	}
	item = elm_toolbar_item_append(rej_msg_toolbar, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(item, EINA_TRUE);
	item = elm_toolbar_item_append(rej_msg_toolbar, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(item, EINA_TRUE);
	item = elm_toolbar_item_append(rej_msg_toolbar, NULL, NULL, NULL, NULL);
	btn = elm_button_add(rej_msg_toolbar);
	elm_object_style_set(btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(btn, "clicked", __reject_msg_back_cb, vd);
	elm_object_item_content_set(item, btn);
	edje_object_part_swallow(priv->lock_reject_with_msg, "swl_msgtoolbar", rej_msg_toolbar);
}

static Evas_Object *__vcui_view_incoming_lock_create_reject_msg_layout(void *data)
{
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	vcui_app_call_data_t *ad = vd->app_data;
	Evas_Object *rej_msg_bg = NULL;

	if (priv->lock_reject_with_msg != NULL) {
		evas_object_del(priv->lock_reject_with_msg);
		priv->lock_reject_with_msg = NULL;
	}
	priv->lock_reject_with_msg = __vcui_view_incoming_lock_load_edj(evas_object_evas_get(ad->win_main), EDJ_NAME, GRP_LOCK_REJECT_WITH_MSG);
	evas_object_resize(priv->lock_reject_with_msg, ad->root_w, ad->root_h);

	__reject_msg_create_call_setting_handle(vd);

	__reject_msg_create_glist(vd);

	__reject_msg_create_gesture_layer(vd);

	__reject_msg_create_toolbar(vd);

	rej_msg_bg = (Evas_Object *)edje_object_part_object_get(priv->lock_reject_with_msg, "reject_msg_bg");
	evas_object_event_callback_add(rej_msg_bg, EVAS_CALLBACK_MOUSE_DOWN, __vcui_view_incoming_lock_reject_with_msg_mouse_down_cb, vd);
	evas_object_event_callback_add(rej_msg_bg, EVAS_CALLBACK_MOUSE_MOVE, __vcui_view_incoming_lock_reject_with_msg_mouse_move_cb, vd);
	evas_object_event_callback_add(rej_msg_bg, EVAS_CALLBACK_MOUSE_UP, __vcui_view_incoming_lock_reject_with_msg_mouse_up_cb, vd);

	edje_object_part_text_set(priv->lock_reject_with_msg, "reject_msg_text", _("IDS_VCALL_BUTTON_REJECT_WITH_MESSAGE"));
	edje_object_signal_emit(priv->lock_reject_with_msg, "show-up-arrow", "reject_msg");
	evas_object_data_set(priv->lock_reject_with_msg, REJ_MSG_LIST_OPEN_STATUS_KEY, EINA_FALSE);

	__reject_msg_list_param_reset(vd);

	evas_object_show(priv->lock_reject_with_msg);

	return priv->lock_reject_with_msg;
}

static void __vcui_view_incoming_lock_draw_screen(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");

	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)vd->priv;
	Evas_Object *eo = priv->contents;
	vcui_app_call_data_t *ad = vd->app_data;

	call_data_t *call_data = _vcui_doc_get_recent_mt_call_data();
	if (call_data == NULL) {
		CALL_UI_DEBUG("call data is null");
		return;
	}
	if (_vcui_doc_get_call_type(call_data) != CALL_INCOMING) {
		CALL_UI_DEBUG("it is not call_incoming.");
		return;
	}

	char *file_path = _vcui_doc_get_caller_id_file_path(call_data);

	/* call image */
	_vcui_delete_contact_image(priv->contents);
	if (strcmp(file_path, NOIMG_ICON) == 0) {
		_vcui_show_wallpaper_image(priv->contents);
	} else {
		CALL_UI_DEBUG("Caller ID file_path: %s", file_path);
		{
			_vcui_show_wallpaper_image(priv->contents);
			priv->ic = _vcui_show_contact_image(priv->contents, vd->app_data->win_main, file_path);
		}
	}
	_vcui_show_call_bg_img(priv->contents);

	char *call_number = _vcui_doc_get_call_number(call_data);
	char *call_name = _vcui_doc_get_call_display_name(call_data);
	/* call name (if nothing, call number) */
	if (strlen(call_name) == 0) {
		_vcui_show_caller_info_name(vd, call_number, EINA_FALSE);
	} else {
		_vcui_show_caller_info_name(vd, call_name, EINA_FALSE);
		_vcui_show_caller_info_number(vd, call_number, EINA_FALSE);
	}

	_vcui_show_caller_info_icon(vd, EINA_FALSE);
	_vcui_show_caller_info_status(vd, dgettext("sys_string", "IDS_COM_BODY_VOICE_CALL"), EINA_FALSE);

	__vcui_view_incoming_lock_create_accept_btn(priv->contents);
	__vcui_view_incoming_lock_create_reject_btn(priv->contents);
		
	if (!(_vcui_is_phonelock_status() == EINA_TRUE)) {
		if (strlen(call_number) > 0) {
			__vcui_view_incoming_lock_create_reject_msg_layout(vd);
		}
	}
	ad->beffect_needed = EINA_TRUE;

	evas_object_show(eo);

}

static Evas_Object *__vcui_view_incoming_lock_create_contents(void *data, char *grpname)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	Evas_Object *eo;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, grpname);
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

	evas_object_pointer_mode_set(ad->win_main, EVAS_OBJECT_POINTER_MODE_NOGRAB);
	if (!view_data->layout) {
		view_data->layout = __vcui_view_incoming_lock_create_layout_main(ad->win_main);
		if (view_data->layout == NULL) {
			CALL_UI_DEBUG("ERROR");
			return VC_ERROR;
		}
		priv->contents = __vcui_view_incoming_lock_create_contents(view_data, GRP_MTLOCK);
		elm_object_part_content_set(view_data->layout, "elm.swallow.content", priv->contents);

		priv->caller_info = __vcui_view_incoming_lock_create_contents(view_data, GRP_CALLER_INFO);
		elm_object_part_content_set(priv->contents, "caller_info", priv->caller_info);

		evas_object_name_set(priv->contents, VIEW_INCOMING_LOCK_LAYOUT_ID);
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
	incoming_lock_view_priv_t *priv = (incoming_lock_view_priv_t *)view_data->priv;
	int result = 0;

	/*change the orientation to normal - 0*/
	elm_win_rotation_with_resize_set(view_data->app_data->win_main, 0);
	elm_win_rotation_with_resize_set(view_data->app_data->popup_mw, 0);

	if (ad->root_w == MAIN_WIN_HD_W && ad->root_h == MAIN_WIN_HD_H) {
		priv->y_scale_factor = 1;
	} else if (ad->root_w == MAIN_WIN_WVGA_W && ad->root_h == MAIN_WIN_WVGA_H) {
		priv->y_scale_factor = (double) MAIN_WIN_WVGA_H/MAIN_WIN_HD_H;
	}
	CALL_UI_DEBUG("y_scale_factor %f", priv->y_scale_factor);

	__vcui_view_incoming_lock_draw_screen(view_data);

	evas_object_hide(view_data->layout);
	evas_object_show(view_data->layout);
	_vcui_app_win_set_noti_type(EINA_TRUE);

	result = utilx_grab_key(ad->disp, ad->win, KEY_POWER, TOP_POSITION_GRAB);
	if (result)
		CALL_UI_DEBUG("KEY_VOLUMEUP key grab failed");

	result = utilx_grab_key(ad->disp, ad->win, KEY_SELECT, TOP_POSITION_GRAB);
	if (result)
		CALL_UI_DEBUG("KEY_VOLUMEUP key grab failed");

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

		if (priv->itc_reject_msg) {
			elm_genlist_item_class_free(priv->itc_reject_msg);
			priv->itc_reject_msg = NULL;
		}
		if (priv->lock_reject_with_msg) {
			evas_object_del(priv->lock_reject_with_msg);
			priv->lock_reject_with_msg = NULL;
		}
		if (priv->caller_info) {
			evas_object_del(priv->caller_info);
			priv->caller_info = NULL;
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

	utilx_ungrab_key(ad->disp, ad->win, KEY_SELECT);
	utilx_ungrab_key(ad->disp, ad->win, KEY_POWER);

	return VC_NO_ERROR;
}

int	_vc_ui_view_incoming_lock_check_valid_eo(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");

	incoming_lock_view_priv_t *priv = NULL;
	int valid = -1;

	VCUI_RETURN_VALUE_IF_FAIL(vd , -1);
	priv = (incoming_lock_view_priv_t *) vd->priv;

	valid = _vcui_check_valid_eo(priv->contents, VIEW_INCOMING_LOCK_LAYOUT_ID);

	return valid;
}

Evas_Object *_vc_ui_view_incoming_lock_get_caller_info(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incoming_lock_view_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incoming_lock_view_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->caller_info;
}

Evas_Object *_vc_ui_view_incoming_lock_get_layout(voice_call_view_data_t *vd)
{
	CALL_UI_DEBUG("..");
	incoming_lock_view_priv_t *priv = NULL;

	VCUI_RETURN_NULL_IF_FAIL(vd);
	priv = (incoming_lock_view_priv_t *) vd->priv;

	VCUI_RETURN_NULL_IF_FAIL(priv);
	return priv->contents;
}
