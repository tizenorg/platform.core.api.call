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

struct _call_data_t {
	unsigned char call_handle;
	int contact_id;
	int contact_phone_type;
	int bday_remaining_days;
	time_t start_time;

	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];
	char call_display[VC_DISPLAY_NAME_LENGTH_MAX];
	char call_file_path[VC_IMAGE_PATH_LENGTH_MAX];
	char call_full_file_path[VC_IMAGE_PATH_LENGTH_MAX];

	gboolean brejected;	/* auto reject */

	/* The below 2 fields are required on UI side as well, even though we can get these values from engine data structure
	 * When call is ended, engine will clear the call object info and hence we cannot get these status at this moment
	 * So we have to maintain a copy in UI side as well */
	int call_type;				/* whether MO or MT call*/
	int caller_status;			/* HOLD/UNHOLD */
};

static call_data_t *recent_mo;
static call_data_t *recent_mt;
static call_data_t *recent_call;
static Eina_List *caller_list;

/**
 * This function initializes the data structure pointers maintained in the document file
 *
 * @return	nothing
 * @param[in]	nothing
*/
void _vcui_doc_data_init()
{
	recent_mo = NULL;
	recent_mt = NULL;
	recent_call = NULL;
	caller_list = NULL;
}

/**
 * This function allocates memory for the call data structure - call_data_t
 *
 * @return	pointer to the memory allocated for call-data structure, or NULL if memory allocation fails
 * @param[in]	nothing
*/
call_data_t *_vcui_doc_allocate_call_data_memory(void)
{
	call_data_t *pcall_data = NULL;

	pcall_data = (call_data_t *)calloc(1, sizeof(call_data_t));
	if (pcall_data == NULL) {
		CALL_UI_DEBUG("memory allocation failed...");
	}
	return pcall_data;
}

/**
 * This function retrieves the value of the call handle
 *
 * @return	value of the call handle
 * @param[in]	pcall_data	pointer to the call-data structure
*/
int _vcui_doc_get_call_handle(call_data_t *pcall_data)
{
	VCUI_RETURN_INVALID_IF_FAIL(pcall_data);
	return pcall_data->call_handle;
}

/**
 * This function assigns the value of call handle for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data	pointer to the call-data structure
 * @param[in]	call_handle	new value of the call handle
*/
void _vcui_doc_set_call_handle(call_data_t *pcall_data, int call_handle)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	pcall_data->call_handle = call_handle;
}

/**
 * This function retrieves the value of the call number
 *
 * @return	pointer to the call number string
 * @param[in]	pcall_data	pointer to the call-data structure
*/
char *_vcui_doc_get_call_number(call_data_t *pcall_data)
{
	VCUI_RETURN_NULL_IF_FAIL(pcall_data);
	return (char *) pcall_data->call_num;
}

/**
 * This function assigns the value of call number for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data	pointer to the call-data structure
 * @param[in]	pnumber		pointer to the call number string
*/
void _vcui_doc_set_call_number(call_data_t *pcall_data, char *pnumber)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	memset(pcall_data->call_num, 0, sizeof(pcall_data->call_num));
	vcall_engine_util_strcpy(pcall_data->call_num, VC_PHONE_NUMBER_LENGTH_MAX, pnumber);
}

/**
 * This function retrieves the value of the call name (available from contact DB)
 *
 * @return	pointer to the call name string
 * @param[in]	pcall_data	pointer to the call-data structure
*/
char *_vcui_doc_get_call_display_name(call_data_t *pcall_data)
{
	VCUI_RETURN_NULL_IF_FAIL(pcall_data);
	return (char *) pcall_data->call_display;
}

/**
 * This function assigns the value of contact name for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data	pointer to the call-data structure
 * @param[in]	pname		pointer to the call number string
*/
void _vcui_doc_set_call_display_name(call_data_t *pcall_data, char *pname)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	memset(pcall_data->call_display, 0, sizeof(pcall_data->call_display));
	vcall_engine_util_strcpy(pcall_data->call_display, VC_DISPLAY_NAME_LENGTH_MAX, pname);
}

/**
 * This function retrieves the caller-ID file path (caller-ID for split screen)
 *
 * @return	pointer to the caller-ID file path string
 * @param[in]	pcall_data	pointer to the call-data structure
*/
char *_vcui_doc_get_caller_id_file_path(call_data_t *pcall_data)
{
	VCUI_RETURN_NULL_IF_FAIL(pcall_data);
	return (char *) pcall_data->call_file_path;
}

/**
 * This function assigns the value of caller-ID file-path for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data	pointer to the call-data structure
 * @param[in]	pfile_path	pointer to the caller-ID file-path string
*/
void _vcui_doc_set_caller_id_file_path(call_data_t *pcall_data, char *pfile_path)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	memset(pcall_data->call_file_path, 0, sizeof(pcall_data->call_file_path));
	vcall_engine_util_strcpy(pcall_data->call_file_path, VC_IMAGE_PATH_LENGTH_MAX, pfile_path);
}

/**
 * This function retrieves the caller-ID full file path (caller-ID for full screen)
 *
 * @return	pointer to the caller-ID full file path string
 * @param[in]	pcall_data	pointer to the call-data structure
*/
char *_vcui_doc_get_caller_id_full_file_path(call_data_t *pcall_data)
{
	VCUI_RETURN_NULL_IF_FAIL(pcall_data);
	return (char *) pcall_data->call_full_file_path;
}

/**
 * This function assigns the value of caller-ID full file-path for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data		pointer to the call-data structure
 * @param[in]	pfull_file_path	pointer to the caller-ID full file-path string
*/
void _vcui_doc_set_caller_id_full_file_path(call_data_t *pcall_data, char *pfull_file_path)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	memset(pcall_data->call_full_file_path, 0, sizeof(pcall_data->call_full_file_path));
	vcall_engine_util_strcpy(pcall_data->call_full_file_path, VC_IMAGE_PATH_LENGTH_MAX, pfull_file_path);
}

/**
 * This function retrieves the value of the call start time
 *
 * @return	value of the call start time
 * @param[in]	pcall_data	pointer to the call-data structure
*/
int _vcui_doc_get_call_start_time(call_data_t *pcall_data)
{
	VCUI_RETURN_INVALID_IF_FAIL(pcall_data);
	return pcall_data->start_time;
}

/**
 * This function assigns the value of call start time for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data	pointer to the call-data structure
*/
void _vcui_doc_set_call_start_time(call_data_t *pcall_data)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	time(&(pcall_data->start_time));
}

/**
 * This function retrieves the value of the contact index
 *
 * @return	value of the contact index
 * @param[in]	pcall_data	pointer to the call-data structure
*/
int _vcui_doc_get_contact_index(call_data_t *pcall_data)
{
	VCUI_RETURN_INVALID_IF_FAIL(pcall_data);
	return pcall_data->contact_id;
}

/**
 * This function assigns the value of contact index for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data		pointer to the call-data structure
 * @param[in]	contact_index	value of the contact index
*/
void _vcui_doc_set_contact_index(call_data_t *pcall_data, int contact_index)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	pcall_data->contact_id = contact_index;
}

/**
 * This function retrieves the value of the contact phone type
 *
 * @return	value of the contact phone type
 * @param[in]	pcall_data	pointer to the call-data structure
*/
int _vcui_doc_get_contact_phone_type(call_data_t *pcall_data)
{
	VCUI_RETURN_INVALID_IF_FAIL(pcall_data);
	return pcall_data->contact_phone_type;
}

/**
 * This function assigns the value of contact phone type for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data	pointer to the call-data structure
 * @param[in]	phone_type	value of the contact phone type
*/
void _vcui_doc_set_contact_phone_type(call_data_t *pcall_data, int phone_type)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	pcall_data->contact_phone_type = phone_type;
}

/**
 * This function retrieves the value of the remaining days for the contact birthday
 *
 * @return	value of the contact remaining days in birthday
 * @param[in]	pcall_data	pointer to the call-data structure
*/
int _vcui_doc_get_birthday_remaining_days(call_data_t *pcall_data)
{
	VCUI_RETURN_INVALID_IF_FAIL(pcall_data);
	return pcall_data->bday_remaining_days;
}

/**
 * This function assigns the value of birthday remaining days for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data		pointer to the call-data structure
 * @param[in]	bday_rem_days	value of the birthday remaining days
*/
void _vcui_doc_set_birthday_remaining_days(call_data_t *pcall_data, int bday_rem_days)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	pcall_data->bday_remaining_days = bday_rem_days;
}

/**
 * This function retrieves the value of the auto reject status
 *
 * @return	value of the auto reject status
 * @param[in]	pcall_data	pointer to the call-data structure
*/
int _vcui_doc_get_auto_reject_status(call_data_t *pcall_data)
{
	VCUI_RETURN_INVALID_IF_FAIL(pcall_data);
	return pcall_data->brejected;
}

/**
 * This function assigns the value of auto reject status for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data		pointer to the call-data structure
 * @param[in]	bauto_rejected	value of the auto reject status
*/
void _vcui_doc_set_auto_reject_status(call_data_t *pcall_data, gboolean bauto_rejected)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	pcall_data->brejected = bauto_rejected;
}

/**
 * This function retrieves if a call is MO or MT type
 *
 * @return	value of the call type - MO or MT
 * @param[in]	pcall_data	pointer to the call-data structure
*/
int _vcui_doc_get_call_type(call_data_t *pcall_data)
{
	VCUI_RETURN_INVALID_IF_FAIL(pcall_data);
	return pcall_data->call_type;
}

/**
 * This function assigns the value of call type for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data	pointer to the call-data structure
 * @param[in]	call_type	type of call, MO or MT
*/
void _vcui_doc_set_call_type(call_data_t *pcall_data, int call_type)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	pcall_data->call_type = call_type;
}

/**
 * This function retrieves the call status of a particular call data structure (HOLD/UNHOLD)
 *
 * @return	value of the call status, HOLD/UNHOLD
 * @param[in]	pcall_data	pointer to the call-data structure
*/
int _vcui_doc_get_call_status(call_data_t *pcall_data)
{
	VCUI_RETURN_INVALID_IF_FAIL(pcall_data);
	return pcall_data->caller_status;
}

/**
 * This function assigns the value of call status for a given pointer of the call-data structure
 *
 * @return	nothing
 * @param[in]	pcall_data	pointer to the call-data structure
 * @param[in]	call_status	status of call, active/held
*/
void _vcui_doc_set_call_status(call_data_t *pcall_data, int call_status)
{
	VCUI_RETURN_IF_FAIL(pcall_data);
	pcall_data->caller_status = call_status;
}

/**
 * This function retrieves the pointer to the most recent MO(outgoing) call data
 *
 * @return	pointer to the call data structure
 * @param[in]	nothing
*/
call_data_t *_vcui_doc_get_recent_mo_call_data()
{
	return recent_mo;
}

/**
 * This function assigns the pointer of the most recent MO(outgoing) call data
 * to the pointer stored in the vcui-document file (recent_mo)
 *
 * @return	nothing
 * @param[in]	in		pointer to the call-data structure to be copied/assigned
*/
void _vcui_doc_set_recent_mo_call_data(call_data_t *in)
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

/**
 * This function retrieves the pointer to the most recent MT(incoming) call data
 *
 * @return	pointer to the call data structure
 * @param[in]	nothing
*/
call_data_t *_vcui_doc_get_recent_mt_call_data()
{
	return recent_mt;
}

/**
 * This function assigns the pointer of the most recent MT(incoming) call data
 * to the pointer stored in the vcui-document file (recent_mt)
 *
 * @return	nothing
 * @param[in]	in		pointer to the call-data structure to be copied/assigned
*/
void _vcui_doc_set_recent_mt_call_data(call_data_t *in)
{
	CALL_UI_DEBUG("..");
	if (in == NULL) {
		CALL_UI_DEBUG("set recent_mt to null");
	}
	_vcui_doc_set_all_recent(in);
	recent_mt = in;
}

/**
 * This function retrieves the pointer to the most recent call data, either MT(incoming)/MO(outgoing)
 *
 * @return	pointer to the call data structure
 * @param[in]	nothing
*/
call_data_t *_vcui_doc_get_recent_call_data()
{
	CALL_UI_DEBUG("..");
	if (recent_call == NULL) {
		CALL_UI_DEBUG("recent is NULL");
		if (recent_mo != NULL) {
			recent_call = recent_mo;
			CALL_UI_DEBUG("recent is mo");
		} else if (recent_mt != NULL) {
			recent_call = recent_mt;
			CALL_UI_DEBUG("recent is mt");
		}
	}
	return recent_call;
}

/**
 * This function assigns the pointer of the most recent MT(incoming)/MO(outgoing) call data
 * to the pointer stored in the vcui-document file (recent_call)
 *
 * @return	nothing
 * @param[in]	in		pointer to the call-data structure to be copied/assigned
*/
void _vcui_doc_set_all_recent(call_data_t *in)
{
	CALL_UI_DEBUG("..");
	if (in == NULL) {
		CALL_UI_DEBUG("set recent_call to null");
	}
	recent_call = in;
}

/**
 * This function adds the call data structure to the list of call data maintained in the
 * vcui-document file (caller_list link list)
 *
 * @return	nothing
 * @param[in]	in		pointer to the call-data structure to be copied/assigned
*/
void _vcui_doc_add_call_data(call_data_t *in)
{
	if (in == NULL)
		return;
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			return;
		}
	}

	caller_list = eina_list_append(caller_list, (void *)in);
}

/**
 * This function updates the call data structure contained in the list of call data maintained in the
 * vcui-document file (caller_list link list) with the new call-data structure
 *
 * @return	nothing
 * @param[in]	in		pointer to the call-data structure to be updated
*/
void _vcui_doc_update_call_data(call_data_t *in)
{
	if (in == NULL)
		return;
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			*cd = *in;
			return;
		}
	}
}

/**
 * This function checks if the call data pointer data is valid and present in the 'caller_list' list
 *
 * @return	TRUE if the call-data is present and FALSE if there is no such call-data in the list
 * @param[in]	in		pointer to the call-data structure
*/
Eina_Bool _vcui_doc_is_valid_call_data(call_data_t *in)
{
	if (in == NULL)
		return EINA_FALSE;
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			return EINA_TRUE;
		}
	}

	return EINA_FALSE;
}

/**
 * This function removes the call data structure from the list of call data maintained in the
 * vcui-document file (caller_list link list) and frees the call data structure
 *
 * @return	nothing
 * @param[in]	in		pointer to the call-data structure to be added
*/
void _vcui_doc_remove_call_data(call_data_t *in)
{
	if (in == NULL) {
		CALL_UI_DEBUG("Call data is Null");
		return;
	}
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			if (in == _vcui_doc_get_recent_mo_call_data())
				_vcui_doc_set_recent_mo_call_data(NULL);
			if (in == _vcui_doc_get_recent_mt_call_data())
				_vcui_doc_set_recent_mt_call_data(NULL);

			caller_list = eina_list_remove(caller_list, in);
			free(in);
			in = NULL;
			CALL_UI_DEBUG("Call data removed");
			break;
		}

	}

	if (_vcui_doc_get_all_call_data_count() == 0) {
		eina_list_free(caller_list);
		caller_list = NULL;
	}

}

/**
 * This function removes the call data structure from the list of call data maintained in the
 * vcui-document file (caller_list link list)
 *
 * @return	a pointer to the call data structure which is passed
 * @param[in]	in		pointer to the call-data structure to be added
*/
call_data_t *_vcui_doc_remove_call_data_from_list(call_data_t *in)
{
	if (in == NULL) {
		CALL_UI_DEBUG("Call data is Null");
		return NULL;
	}
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd == in) {
			caller_list = eina_list_remove(caller_list, in);
			return in;
		}
	}
	return NULL;
}

/**
 * This function removes all the call data from list and frees the memory
 *
 * @return	nothing
 * @param[in]	void
*/
void _vcui_doc_remove_all_call_data()
{
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			free(cd);
			cd = NULL;
		}
	}
	eina_list_free(caller_list);
	caller_list = NULL;
}

/**
 * This function retrieves the earliest call data based on the call status
 *
 * @return	pointer to the call data structure
 * @param[in]	call_status		value of the call status (hold/unhold)
*/
call_data_t *_vcui_doc_get_call_data_by_call_status(int call_status)
{

	Eina_List *l = NULL;
	call_data_t *fast_cd = NULL;
	call_data_t *cd = NULL;
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

/**
 * This function retrieves the most recent MO call data
 *
 * @return	pointer to the call data structure
 * @param[in]	nothing
*/
call_data_t *_vcui_doc_get_call_data_mo_type()
{

	Eina_List *l = NULL;
	call_data_t *last_cd = NULL;
	call_data_t *cd = NULL;
	int i = 0;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->call_type == CALL_OUTGOING) {
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

/**
 * This function retrieves the first call data in the list
 *
 * @return	pointer to the call data structure
 * @param[in]	nothing
*/
call_data_t *_vcui_doc_get_first_call_data_from_list()
{
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			return cd;
		}
	}
	return NULL;
}

/**
 * This function retrieves the first call data in the list which is of UNHOLD status
 *
 * @return	pointer to the call data structure
 * @param[in]	nothing
*/
call_data_t *_vcui_doc_get_first_call_data_by_unhold_status()
{
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_UNHOLD) {
				return cd;
			}
		}
	}
	return NULL;
}

/**
 * This function retrieves the call data based on the given call handle
 *
 * @return	pointer to the call data structure
 * @param[in]	handle	value of the call handle
*/
call_data_t *_vcui_doc_get_call_data_by_handle(int handle)
{
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->call_handle == handle) {
				return cd;
			}
		}
	}
	return NULL;
}

/**
 * This function retrieves the count of call data which are in HOLD status
 *
 * @return	count of held call data
 * @param[in]	nothing
*/
int _vcui_doc_get_hold_call_data_count()
{
	int i = 0;
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_HOLD) {
				i++;
			}
		}
	}
	CALL_UI_DEBUG("(%d)", i);
	return i;
}

/**
 * This function retrieves the count of call data which are in UNHOLD status
 *
 * @return	count of active call data
 * @param[in]	nothing
*/
int _vcui_doc_get_unhold_call_data_count()
{
	int i = 0;
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_UNHOLD) {
				i++;
			}
		}
	}
	CALL_UI_DEBUG("(%d)", i);
	return i;
}

/**
 * This function retrieves the count of call data which are in NO status (neither hold/unhold)
 *
 * @return	count of 'no status' call data
 * @param[in]	nothing
*/
int _vcui_doc_get_no_status_call_data_count()
{
	int i = 0;
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == NO_STATUS) {
				i++;
			}
		}
	}
	return i;
}

/**
 * This function retrieves the count of all call data in the list
 *
 * @return	count of all call data
 * @param[in]	nothing
*/
int _vcui_doc_get_all_call_data_count()
{
	int i = eina_list_count(caller_list);
	return i;
}

/**
 * This function retrieves the call status of a group (greater than 1 member group)
 *
 * @return	value of the call status (HOLD/UNHOLD)
 * @param[in]	nothing
*/
int _vcui_doc_get_group_call_status()
{
	if (_vcui_doc_get_all_call_data_count() > 1) {
		if (_vcui_doc_get_hold_call_data_count() > 1) {
			return CALL_HOLD;
		} else {
			return CALL_UNHOLD;
		}
	} else {
		return CALL_UNHOLD;
	}
}

/**
 * This function retrieves the pointer to the list containing the call data with HOLD status
 *
 * @return	pointer to the HOLD call data list
 * @param[in]	nothing
*/
Eina_List *_vcui_doc_get_caller_list_with_hold_status()
{
	if (_vcui_doc_get_all_call_data_count() == 0)
		return NULL;
	Eina_List *hold_list = NULL;
	Eina_List *l = NULL;
	call_data_t *cd = NULL;

	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_HOLD) {
				hold_list = eina_list_append(hold_list, cd);
			}
		}
	}
	return hold_list;
}

/**
 * This function retrieves the pointer to the list containing the call data with UNHOLD status
 *
 * @return	pointer to the UNHOLD call data list
 * @param[in]	nothing
*/
Eina_List *_vcui_doc_get_caller_list_with_unhold_status()
{
	if (_vcui_doc_get_all_call_data_count() == 0)
		return NULL;
	Eina_List *unhold_list = NULL;
	Eina_List *l = NULL;
	call_data_t *cd = NULL;

	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			CALL_UI_DEBUG("_vcui_doc_get_caller_list_with_unhold_status");
			if (cd->caller_status == CALL_UNHOLD) {
				CALL_UI_DEBUG("find it");
				unhold_list = eina_list_append(unhold_list, cd);
			}
		}
	}
	return unhold_list;
}

/**
 * This function prints all the call data structure members
 *
 * @return	nothing
 * @param[in]	msg_pos		type of message
*/
void _vcui_doc_print_all_call_data(char *msg_pos)
{
	CALL_UI_DEBUG(" --------%s------------", msg_pos);
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			CALL_UI_DEBUG(" call_handle %d", cd->call_handle);
			CALL_UI_DEBUG(" call_num %s", cd->call_num);
			CALL_UI_DEBUG(" call_display %s", cd->call_display);
			CALL_UI_DEBUG(" call_file_path %s", cd->call_file_path);
			CALL_UI_DEBUG(" call_full_file_path %s", cd->call_full_file_path);
			CALL_UI_DEBUG(" call_time %d", (int)(cd->start_time));
		}
	}
	CALL_UI_DEBUG(" --------------------------");
}

/**
 * This function assigns UNHOLD call status to all the call data in the list
 *
 * @return	nothing
 * @param[in]	nothing
*/
void _vcui_doc_set_all_call_data_to_unhold_status()
{
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_HOLD) {
				cd->caller_status = CALL_UNHOLD;
			}
		}
	}
}

/**
 * This function assigns HOLD call status to all the call data in the list
 *
 * @return	nothing
 * @param[in]	nothing
*/
void _vcui_doc_set_all_call_data_to_hold_status()
{
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
	EINA_LIST_FOREACH(caller_list, l, cd) {
		if (cd != NULL) {
			if (cd->caller_status == CALL_UNHOLD) {
				cd->caller_status = CALL_HOLD;
			}
		}
	}
}

/**
 * This function swaps the HOLD and UNHOLD calls in the call list
 *
 * @return	nothing
 * @param[in]	nothing
*/
void _vcui_doc_swap_all_call_data_status()
{
	Eina_List *l = NULL;
	call_data_t *cd = NULL;
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

