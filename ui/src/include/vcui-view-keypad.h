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


#ifndef _VCUI_VIEW_KEYPAD_H_
#define _VCUI_VIEW_KEYPAD_H_

#ifndef STRLEN
#define STRLEN(str)	(((str) == NULL) ? 0 : strlen(str))
#endif

#define CURSOR_BEGIN	-1
#define CURSOR_END		-2

#define MAX_DIAL_NUMBER_FONT_SIZE		70
#define KEYPAD_STR_DEFINE_OPEN_SIZE		73
#define KEYPAD_STR_DEFINE_CLOSE_SIZE	43
#define KEYPAD_ENTRY_DISP_DATA_SIZE		1024
#define KEYPAD_ENTRY_SET_DATA_SIZE	(KEYPAD_STR_DEFINE_OPEN_SIZE + KEYPAD_STR_DEFINE_CLOSE_SIZE + KEYPAD_ENTRY_DISP_DATA_SIZE)


void _vcui_keypad_create_layout(void *data, Evas_Object *parent_ly);
Eina_Bool _vcui_keypad_get_show_status(void);
void _vcui_keypad_set_show_status(Eina_Bool bkeypad_status);
void _vcui_keypad_show_hide_effect(void *data, Evas_Object *parent_ly);
void _vcui_keypad_delete_layout(Evas_Object *parent_ly);
void _vcui_keypad_show_layout(void *data);

#endif
