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

static call_data_t *recent_mo;
static call_data_t *recent_mt;
static call_data_t *recent_mt_mo;
static Eina_List *caller_list;

void _vcui_doc_recent_init()
{
	recent_mo = NULL;
	recent_mt = NULL;
	recent_mt_mo = NULL;
}

call_data_t *_vcui_doc_get_recent_mo()
{
	return recent_mo;
}

call_data_t *_vcui_doc_get_recent_mt()
{
	return recent_mt;
}

call_data_t *_vcui_doc_get_all_recent()
{
	CALL_UI_DEBUG("..");
	if (recent_mt_mo == NULL) {
		CALL_UI_DEBUG("recent is NULL");
		if (recent_mo != NULL) {
			recent_mt_mo = recent_mo;
			CALL_UI_DEBUG("recent is mo");
		} else if (recent_mt != NULL) {
			recent_mt_mo = recent_mt;
			CALL_UI_DEBUG("recent is mt");
		}
	}
	return recent_mt_mo;
}

void _vcui_doc_set_all_recent(call_data_t *in)
{
	CALL_UI_DEBUG("..");
	if (in == NULL) {
		CALL_UI_DEBUG("set recent_mt_mo to null");
	}
	recent_mt_mo = in;
}

void _vcui_doc_set_mo_recent(call_data_t *in)
{
	CALL_UI_DEBUG("..");
	if (in == NULL) {
		CALL_UI_DEBUG("set recent_mo to null");
	}
	if (recent_mo != NULL && recent_mo->call_handle == NO_HANDLE) {
		CALL_UI_DEBUG("Set_recent 1");
		free(recent_mo);
		recent_mo = NULL;
	}
	_vcui_doc_set_all_recent(in);
	recent_mo = in;
}

void _vcui_doc_set_mt_recent(call_data_t *in)
{
	CALL_UI_DEBUG("..");
	if (in == NULL) {
		CALL_UI_DEBUG("set recent_mt to null");
	}
	_vcui_doc_set_all_recent(in);
	recent_mt = in;
}

void _vcui_doc_caller_list_init()
{
	caller_list = NULL;
}

int _vcui_doc_is_call_data(call_data_t *in)
{
	if (in == NULL)
		return EINA_FALSE;
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			return EINA_TRUE;
		}
	}

	return EINA_FALSE;
}

void _vcui_doc_add_call_data(call_data_t *in)
{
	if (in == NULL)
		return;
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			return;
		}
	}

	caller_list = eina_list_append(caller_list, (void *)in);
}

void _vcui_doc_update_call_data(call_data_t *in)
{
	if (in == NULL)
		return;
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			*cd = *in;
			return;
		}
	}
}

void _vcui_doc_remove_all_data()
{
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			free(cd);
			cd = NULL;
		}
	}
	eina_list_free(caller_list);
	caller_list = NULL;
}

call_data_t *_vcui_doc_remove_call_data_only_list(call_data_t *in)
{
	if (in == NULL) {
		CALL_UI_DEBUG("Call data is Null");
		return NULL;
	}
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			caller_list = eina_list_remove(caller_list, in);
			return in;
		}
	}
	return NULL;
}

void _vcui_doc_remove_call_data(call_data_t *in)
{
	if (in == NULL) {
		CALL_UI_DEBUG("Call data is Null");
		return;
	}
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			if (in == _vcui_doc_get_recent_mo())
				_vcui_doc_set_mo_recent(NULL);
			if (in == _vcui_doc_get_recent_mt())
				_vcui_doc_set_mt_recent(NULL);

			caller_list = eina_list_remove(caller_list, in);
			free(in);
			in = NULL;
			CALL_UI_DEBUG("Call data removed");
			break;
		}

	}

	if (_vcui_doc_get_count() == 0) {
		eina_list_free(caller_list);
		caller_list = NULL;
	}

}

call_data_t *_vcui_doc_get_call_handle(int handle)
{
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->call_handle == handle) {
				return cd;
			}
		}
	}
	return NULL;
}

call_data_t *_vcui_doc_get_first_unhold()
{
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_UNHOLD) {
				return cd;
			}
		}
	}
	return NULL;
}

call_data_t *_vcui_doc_get_first_hold()
{
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_HOLD) {
				return cd;
			}
		}
	}
	return NULL;
}

call_data_t *_vcui_doc_get_last_status(int call_status)
{

	Eina_List *l;
	call_data_t *fast_cd = NULL;
	call_data_t *cd;
	int i = 0;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == call_status) {
				if (i == 0) {
					fast_cd = cd;
					i++;
					continue;
				} else {
					if (fast_cd->start_time > cd->start_time) {
						fast_cd = cd;
					}
				}
				i++;
			}
		}
	}
	return fast_cd;
}

call_data_t *_vcui_doc_get_last_type_mo()
{

	Eina_List *l;
	call_data_t *last_cd = NULL;
	call_data_t *cd;
	int i = 0;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->mo_mt_status == CALL_OUTGOING) {
				if (i == 0) {
					last_cd = cd;
					i++;
					continue;
				} else {
					if (last_cd->start_time < cd->start_time) {
						last_cd = cd;
					}
				}
				i++;
			}
		}
	}
	return last_cd;
}

call_data_t *_vcui_doc_get_first()
{
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd)
	    if (cd != NULL) {
		return cd;
	}
	return NULL;
}

void _vcui_doc_set_unhold_all()
{
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_HOLD) {
				cd->caller_status = CALL_UNHOLD;
			}
		}
	}
}

void _vcui_doc_set_hold_all()
{
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_UNHOLD) {
				cd->caller_status = CALL_HOLD;
			}
		}
	}
}

void _vcui_doc_set_swap_all()
{
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_HOLD) {
				cd->caller_status = CALL_UNHOLD;
			} else if (cd->caller_status == CALL_UNHOLD) {
				cd->caller_status = CALL_HOLD;
			}
		}
	}
}

int _vcui_doc_get_show_callstatus()
{
	if (_vcui_doc_get_count() > 1) {
		if (_vcui_doc_get_count_hold() > 1)
			return CALL_HOLD;
		else
			return CALL_UNHOLD;
	} else
		return CALL_UNHOLD;
}

int _vcui_doc_get_count()
{
	int i = eina_list_count(caller_list);
	return i;
}

int _vcui_doc_get_count_hold()
{
	int i = 0;
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_HOLD) {
				i++;
			}
		}
	}
	CALL_UI_DEBUG("(%d)",i);
	return i;
}

int _vcui_doc_get_count_unhold()
{
	int i = 0;
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_UNHOLD) {
				i++;
			}
		}
	}
	CALL_UI_DEBUG("(%d)",i);
	return i;
}

int _vcui_doc_get_count_nostatus()
{
	int i = 0;
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == NO_STATUS) {
				i++;
			}
		}
	}
	return i;
}

Eina_List *_vcui_doc_get_hold_caller()
{
	if (_vcui_doc_get_count() == 0)
		return NULL;
	Eina_List *hold_list = NULL;
	Eina_List *l;
	call_data_t *cd;

	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_HOLD) {
				hold_list = eina_list_append(hold_list, cd);
			}
		}
	}
	return hold_list;
}

Eina_List *_vcui_doc_get_unhold_caller()
{
	if (_vcui_doc_get_count() == 0)
		return NULL;
	Eina_List *unhold_list = NULL;
	Eina_List *l;
	call_data_t *cd;

	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			CALL_UI_DEBUG("_vcui_doc_get_unhold_caller");
			if (cd->caller_status == CALL_UNHOLD) {
				CALL_UI_DEBUG("find it");
				unhold_list = eina_list_append(unhold_list, cd);
			}
		}
	}
	return unhold_list;
}

Eina_List *_vcui_doc_get_caller()
{
	if (_vcui_doc_get_count() == 0)
		return NULL;

	Eina_List *list = NULL;
	Eina_List *l;
	call_data_t *cd;

	EINA_LIST_FOREACH(caller_list, l, cd) {
		list = eina_list_append(list, cd);
	}
	return list;
}

void _vcui_doc_all_print_address()
{
	vcui_app_call_data_t *ad = _vcui_get_app_data();
	CALL_UI_DEBUG("----------address----------");
	CALL_UI_DEBUG("View Data : DIALING VIEW %p", ad->view_st[VIEW_DIALLING_VIEW]);
	CALL_UI_DEBUG("View Data : INCOMING VIEW %p", ad->view_st[VIEW_INCOMING_VIEW]);
	CALL_UI_DEBUG("View Data : INCOMING LOCK VIEW %p", ad->view_st[VIEW_INCOMING_LOCK_VIEW]);
	CALL_UI_DEBUG("View Data : INCALL ONEVIEW %p", ad->view_st[VIEW_INCALL_ONECALL_VIEW]);
	CALL_UI_DEBUG("View Data : INCALL MULTIVIEW SPLIT %p", ad->view_st[VIEW_INCALL_MULTICALL_SPLIT_VIEW]);
	CALL_UI_DEBUG("View Data : INCALL MULTIVIEW CONF %p", ad->view_st[VIEW_INCALL_MULTICALL_CONF_VIEW]);
	CALL_UI_DEBUG("View Data : INCALL MULTIVIEW LIST %p", ad->view_st[VIEW_INCALL_MULTICALL_LIST_VIEW]);
	CALL_UI_DEBUG("View Data : INCALL KEYPAD %p", ad->view_st[VIEW_INCALL_KEYPAD_VIEW]);
	CALL_UI_DEBUG(" --------------------------");

}

void _vcui_doc_all_print(char *msg_pos)
{
	CALL_UI_DEBUG(" --------%s------------", msg_pos);
	Eina_List *l;
	call_data_t *cd;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			CALL_UI_DEBUG(" call_handle %d", cd->call_handle);
			CALL_UI_DEBUG(" call_num %s", cd->call_num);
			CALL_UI_DEBUG(" call_display %s", cd->call_display);
			CALL_UI_DEBUG(" call_file_path %s", cd->call_file_path);
			CALL_UI_DEBUG(" call_full_file_path %s", cd->call_full_file_path);
			CALL_UI_DEBUG(" caller_status %d", cd->caller_status);
			CALL_UI_DEBUG(" call_time %d", (int)(cd->start_time));
		}
	}
	CALL_UI_DEBUG(" --------------------------");
}

