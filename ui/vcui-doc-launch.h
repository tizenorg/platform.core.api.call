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


#include "ui-gadget.h"

#ifndef _VCUI_DOC_LAUNCH_
#define _VCUI_DOC_LAUNCH_

#define PKGNAME_CONTACT_UG	"contacts-list-efl"
#define PKGNAME_DIALER_UG	"phoneui-efl"

typedef enum {
	VCUI_UG_TYPE_NOE = 0,
	VCUI_UG_TYPE_CONTACT_LIST,
	VCUI_UG_TYPE_ADD_CALL,
	VCUI_UG_TYPE_ADD_TO_CONTACTS,
	VCUI_UG_TYPE_MAX
} vcui_ug_type;

struct vcui_ugs_array {
	int ug_count;
	int last_ug_type;
	Eina_List *ug_lists;
};

struct vcui_ug_priv_data {
	void (*on_start_callback) (void *);
	void (*on_destroy_callback) (void *);
	void *destroy_callback_param;
	void *need_navi;
	void *need_parent;
	void *need_layout;
	void *need_ug;
	void *need_ug_lists;
};

void _vcui_doc_launch_contact_list_ug(void *parent_ui_gadget, void *navi, void *parent, void (*on_start_callback) (void *), void (*on_destroy_callback) (void *), void *callback_param, void *ugs_array_data);
void _vcui_doc_launch_phoneui_ug(void *parent_ui_gadget, void *navi, void *parent, void (*on_start_callback) (void *), void (*on_destroy_callback) (void *), void *callback_param, void *ugs_array_data);
void _vcui_doc_launch_add_to_contacts_ug(void *parent_ui_gadget, void *navi, void *parent, void (*on_start_callback) (void *), void (*on_destroy_callback) (void *), void *callback_param, void *ugs_array_data, void *data);

void _vcui_doc_launch_msg_composer(void *data, char *number);
void _vcui_doc_launch_destroy_ug_all(void *ugs_array_data);

#endif
