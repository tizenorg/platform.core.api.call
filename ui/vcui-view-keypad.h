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


#ifndef _VCUI_VIEW_KEYPAD_
#define _VCUI_VIEW_KEYPAD_

#ifndef STRLEN
#define STRLEN(str) (((str) == NULL) ? 0: strlen(str))
#endif

#define CURSOR_BEGIN 	-1
#define CURSOR_END 	 	-2

#define MAX_DIAL_NUMBER_FONT_SIZE 		70
#define KEYPAD_STR_DEFINE_OPEN_SIZE	73
#define KEYPAD_STR_DEFINE_CLOSE_SIZE	43
#define KEYPAD_ENTRY_DISP_DATA_SIZE	1024
#define KEYPAD_ENTRY_SET_DATA_SIZE	(KEYPAD_STR_DEFINE_OPEN_SIZE + KEYPAD_STR_DEFINE_CLOSE_SIZE + KEYPAD_ENTRY_DISP_DATA_SIZE)

typedef struct {
	Evas_Object *contents;
	Evas_Object *ic;
	call_data_t *now_data;

	Evas_Object *entry;
	int data_len;
	char entry_disp_data[KEYPAD_ENTRY_DISP_DATA_SIZE+1];
} vcui_view_keypad_priv_t;

#endif

voice_call_view_data_t *_vcui_view_keypad_new(vcui_app_call_data_t *ad);
