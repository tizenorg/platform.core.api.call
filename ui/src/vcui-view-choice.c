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
#include "vcui-app-data.h"
#include "vcui-view-choice.h"
#include "vcui-view-dialing.h"
#include "vcui-view-incoming-lock.h"
#include "vcui-view-single-call.h"
#include "vcui-view-multi-call-list.h"
#include "vcui-view-multi-call-split.h"
#include "vcui-view-multi-call-conf.h"
#include "vcui-view-keypad.h"
#include "vcui-view-callend.h"
static const char *vcui_view_name[] = {
		"VIEW_DIALLING_VIEW",
		"VIEW_INCOMING_LOCK_VIEW",
		"VIEW_INCALL_ONECALL_VIEW",
		"VIEW_INCALL_MULTICALL_SPLIT_VIEW",
		"VIEW_INCALL_MULTICALL_CONF_VIEW",
		"VIEW_INCALL_MULTICALL_LIST_VIEW",
		"VIEW_ENDCALL_VIEW",
		"VIEW_QUICKPANEL_VIEW",
		"VIEW_MAX",
};

void _vcui_view_change(vcui_app_call_view_id_t view_id, int param1, void *param2, void *param3)
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	int valid = 0;
	CALL_UI_DEBUG("view:[%d] -> [%d]", ad->view_top, view_id);
	if (view_id >= VIEW_MAX) {
		CALL_UI_DEBUG("[=========== ERROR!!!! Invalid View ID : %d =================]", view_id);
		return;
	}
	ad->ball_view_hide = EINA_FALSE;
#ifdef _DESTROY_UG_ALL_FORCE_
	if (ad->ugs_array_data.ug_count != 0)
		_vcui_doc_launch_destroy_ug_all(&(ad->ugs_array_data));
#else
	if (ad->ugs_array_data.ug_count != 0) {
		Eina_List *l = NULL;
		ui_gadget_h ug = NULL;
		EINA_LIST_FOREACH(ad->ugs_array_data.ug_lists, l, ug) {
			CALL_UI_DEBUG("Destroy UG due to _vcui_view_change");
			ug_destroy(ug);
		}
		ad->ugs_array_data.ug_lists = eina_list_free(ad->ugs_array_data.ug_lists);
		ad->ugs_array_data.ug_count = 0;
		ad->ugs_array_data.last_ug_type = VCUI_UG_TYPE_NOE;
		ad->contact_ug = NULL;
	}
#endif
	if (ad->view_st[view_id]) {
		CALL_UI_DEBUG("[============  Layout exists !! ============]");
		voice_call_view_data_t *vd = (voice_call_view_data_t *) ad->view_st[view_id];
		if (vd->layout) {
			if (view_id == VIEW_DIALLING_VIEW) {
				valid = _vc_ui_view_dialing_check_valid_eo(vd);
			} else if (view_id == VIEW_INCOMING_LOCK_VIEW) {
				valid = _vc_ui_view_incoming_lock_check_valid_eo(vd);
			} else if (view_id == VIEW_INCALL_ONECALL_VIEW) {
				valid = _vc_ui_view_single_call_check_valid_eo(vd);
			} else if (view_id == VIEW_INCALL_MULTICALL_SPLIT_VIEW) {
				valid = _vcui_view_multi_call_split_check_valid_eo(vd);
			} else if (view_id == VIEW_INCALL_MULTICALL_CONF_VIEW) {
				valid = _vcui_view_multi_call_conf_check_valid_eo(vd);
			} else if (view_id == VIEW_INCALL_MULTICALL_LIST_VIEW) {
				valid = _vcui_view_multi_call_list_check_valid_eo(vd);
			} else if (view_id == VIEW_ENDCALL_VIEW) {
				valid = _vc_ui_view_callend_check_valid_eo(vd);
			} else {
				CALL_UI_DEBUG("[============ BAD INPUT!!!! Check Input Layout!!!!! %d============]", view_id);
			}
			if (valid == -1) {
				CALL_UI_DEBUG("[========== WARNING!! Invalid Evas Object  ==========]");
				ad->view_st[view_id] = NULL;
				/* free priv */
			}
		}
	} else {
		CALL_UI_DEBUG("[============  Create Layout !! ============]");
	}
	ad->view_before_top = ad->view_top;	/* hold the current top window in the before_top pointer */
	ad->view_top = view_id;	/* set the new top window to the view_id which is passed... this step enables in setting 00:00:00 as timer */
	if (view_id == VIEW_DIALLING_VIEW || view_id == VIEW_INCOMING_LOCK_VIEW) {
		_vcui_raise_main_win();
	}
	if ((ad->view_before_top != -1) && (ad->view_before_top != view_id)) {
		CALL_UI_DEBUG("hide & destroy [%d]", ad->view_before_top);
		CALL_UI_KPI("%s onHide start", vcui_view_name[ad->view_before_top]);
		ad->view_st[ad->view_before_top]->onHide(ad->view_st[ad->view_before_top]);
		CALL_UI_KPI("%s onHide done", vcui_view_name[ad->view_before_top]);
		CALL_UI_KPI("%s onDestroy start", vcui_view_name[ad->view_before_top]);
		ad->view_st[ad->view_before_top]->onDestroy(ad->view_st[ad->view_before_top]);
		CALL_UI_KPI("%s onDestroy done", vcui_view_name[ad->view_before_top]);
	}
	if (ad->view_st[view_id] == NULL) {
		CALL_UI_DEBUG("Create view data");
		voice_call_view_data_t *view_data = ad->func_new[view_id] (ad);
		ad->view_st[view_id] = view_data;
	}
	if (ad->view_st[view_id]->layout == NULL) {
		CALL_UI_DEBUG("Create layout");
		CALL_UI_KPI("%s onCreate start", vcui_view_name[view_id]);
		ad->view_st[view_id]->onCreate(ad->view_st[view_id], param1, param2, param3);
		CALL_UI_KPI("%s onCreate done", vcui_view_name[view_id]);
	} else {
		CALL_UI_DEBUG("Update layout");
		CALL_UI_KPI("%s onUpdate start", vcui_view_name[view_id]);
		ad->view_st[view_id]->onUpdate(ad->view_st[view_id], param2, param3);
		CALL_UI_KPI("%s onUpdate done", vcui_view_name[view_id]);
	}
	_vcui_show_main_ui_set_flag();
	CALL_UI_DEBUG("End");
}

void _vcui_view_auto_change()
{
	CALL_UI_DEBUG("..");

	void *data = _vcui_view_get_second_mtcall_popup_data();
	_vcui_view_popup_second_mtcall_unload(data);

	if (_vcui_doc_get_no_status_call_data_count() == 1) {
		call_data_t *call_data = _vcui_doc_get_recent_call_data();
		int call_type = _vcui_doc_get_call_type(call_data);
		if (call_type == CALL_OUTGOING) {
			_vcui_view_change(VIEW_DIALLING_VIEW, 0, NULL, NULL);
		} else if (call_type == CALL_INCOMING) {
			_vcui_view_change(VIEW_INCOMING_LOCK_VIEW, 0, NULL, NULL);
		} else {
			CALL_UI_DEBUG("ERROR");
		}
	} else if (_vcui_doc_get_all_call_data_count() > 1) {
		if (_vcui_doc_get_unhold_call_data_count() == 0 || _vcui_doc_get_hold_call_data_count() == 0) {
			_vcui_view_change(VIEW_INCALL_MULTICALL_CONF_VIEW, 0, NULL, NULL);
		} else {
			_vcui_view_change(VIEW_INCALL_MULTICALL_SPLIT_VIEW, 0, NULL, NULL);
		}
	} else {
		_vcui_view_change(VIEW_INCALL_ONECALL_VIEW, 0, NULL, NULL);
	}
	CALL_UI_DEBUG("End..");
}

void _vcui_view_all_hide()
{
	int i = 0;
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	for (i = 0; i < VIEW_MAX; i++) {
		if (ad->view_st[i] != NULL) {
			ad->view_st[i]->onHide(ad->view_st[i]);
		}
	}
	ad->ball_view_hide = EINA_TRUE;
	_vcui_show_main_ui_set_flag();
}

void _vcui_view_update()
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	if (ad->view_top != -1 && ad->view_st[ad->view_top] != NULL) {
		ad->view_st[ad->view_top]->onUpdate(ad->view_st[ad->view_top], NULL, NULL);
	}
	_vcui_show_main_ui_set_flag();
}

void _vcui_view_destroy(vcui_app_call_view_id_t view_id)
{
	CALL_UI_DEBUG("view_id:[%d]", view_id);
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	if (ad->view_st[view_id]) {
		ad->view_st[view_id]->onHide(ad->view_st[view_id]);
		ad->view_st[view_id]->onDestroy(ad->view_st[view_id]);
	}
}

void _vcui_view_quickpanel_change()
{
#ifdef	_QUICKPANNEL_SUPPORT_
	CALL_UI_DEBUG("..");
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	if (ad->win_quickpanel) {
		if (_vcui_doc_get_all_call_data_count() < 1) {
			CALL_UI_DEBUG("destroy quickpanel");
			ad->view_st[VIEW_QUICKPANEL_VIEW]->onHide(ad->view_st[VIEW_QUICKPANEL_VIEW]);
			ad->view_st[VIEW_QUICKPANEL_VIEW]->onDestroy(ad->view_st[VIEW_QUICKPANEL_VIEW]);
			ad->win_quickpanel = NULL;
		} else {
			if (ad->view_st[VIEW_QUICKPANEL_VIEW]->layout != NULL) {
				CALL_UI_DEBUG("update quickpanel");
				ad->view_st[VIEW_QUICKPANEL_VIEW]->onUpdate(ad->view_st[VIEW_QUICKPANEL_VIEW], NULL, NULL);
			}
		}
	} else {
		CALL_UI_DEBUG("create quickpanel");
		if (ad->view_st[VIEW_QUICKPANEL_VIEW] == NULL) {
			voice_call_view_data_t *view_data = ad->func_new[VIEW_QUICKPANEL_VIEW] (ad);
			ad->view_st[VIEW_QUICKPANEL_VIEW] = view_data;
		}
		if (ad->view_st[VIEW_QUICKPANEL_VIEW]->layout == NULL) {
			CALL_UI_DEBUG("create!!");
			ad->view_st[VIEW_QUICKPANEL_VIEW]->onCreate(ad->view_st[VIEW_QUICKPANEL_VIEW], 0, NULL, NULL);
		}
	}
#else
	CALL_UI_DEBUG("Not applied yet");
#endif
}
