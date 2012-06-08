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


#ifndef _VOICE_CALL_UI_VIEW_DOCUMENT
#define _VOICE_CALL_UI_VIEW_DOCUMENT

typedef struct _call_data_t {
	unsigned char call_handle;
	char call_num[VC_PHONE_NUMBER_LENGTH_MAX];
	char call_display[VC_DISPLAY_NAME_LENGTH_MAX];
	char call_file_path[VC_IMAGE_PATH_LENGTH_MAX];
	char call_full_file_path[VC_IMAGE_PATH_LENGTH_MAX];
	int caller_status;
	time_t start_time;

	int mo_mt_status;

	int contact_id;
	int contact_phone_type;
	gboolean bno_end_show;	/* multi list end */
} call_data_t;

int _vcui_doc_get_count_hold();
int _vcui_doc_get_count_unhold();
int _vcui_doc_get_count_nostatus();

int _vcui_doc_is_call_data(call_data_t *in);
void _vcui_doc_add_call_data(call_data_t *in);
void _vcui_doc_update_call_data(call_data_t *in);
void _vcui_doc_remove_call_data(call_data_t *in);
void _vcui_doc_remove_all_data();
call_data_t *_vcui_doc_remove_call_data_only_list(call_data_t *in);

call_data_t *_vcui_doc_get_call_handle(int handle);

int _vcui_doc_get_count();

Eina_List *_vcui_doc_get_hold_caller();
Eina_List *_vcui_doc_get_unhold_caller();
Eina_List *_vcui_doc_get_caller();

call_data_t *_vcui_doc_get_last_status(int call_status);
call_data_t *_vcui_doc_get_last_type_mo();

void _vcui_doc_caller_list_init();
call_data_t *_vcui_doc_get_first();

call_data_t *_vcui_doc_get_first_hold();
call_data_t *_vcui_doc_get_first_unhold();

void _vcui_doc_recent_init();
call_data_t *_vcui_doc_get_recent_mo();
call_data_t *_vcui_doc_get_recent_mt();
call_data_t *_vcui_doc_get_all_recent();
void _vcui_doc_set_all_recent(call_data_t *in);
void _vcui_doc_set_mo_recent(call_data_t *in);
void _vcui_doc_set_mt_recent(call_data_t *in);

void _vcui_doc_set_unhold_all();
void _vcui_doc_set_hold_all();
void _vcui_doc_set_swap_all();

int _vcui_doc_get_show_callstatus();

void _vcui_doc_all_print(char *);
void _vcui_doc_all_print_address();

int get_status_backhide();
int get_status_delete();
void add_status(int in);
int get_status_all();
#endif

