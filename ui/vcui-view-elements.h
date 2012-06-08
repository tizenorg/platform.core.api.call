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


#ifndef _VOICE_CALL_UI_CALLBACK_
#define _VOICE_CALL_UI_CALLBACK_

void _vcui_swap_btn_cb(void *data, Evas_Object * obj, void *event_info);
void _vcui_conf_img_cb(void *data, Evas_Object * obj, void *event_info);

Evas_Object *_vcui_create_top_left_button(void *data);
Evas_Object *_vcui_create_top_left_button_disabled(void *data);
Evas_Object *_vcui_create_top_middle_button(void *data);
Evas_Object *_vcui_create_top_middle_button_disabled(void *data);
Evas_Object *_vcui_create_top_right_button(void *data);
Evas_Object *_vcui_create_top_right_button_disabled(void *data);
Evas_Object *_vcui_create_bottom_left_button(void *data);
Evas_Object *_vcui_create_bottom_left_button_disabled(void *data);
Evas_Object *_vcui_create_bottom_middle_button(void *data);
Evas_Object *_vcui_create_bottom_middle_button_disabled(void *data);
Evas_Object *_vcui_create_bottom_right_button(void *data);
Evas_Object *_vcui_create_bottom_right_button_disabled(void *data);
Evas_Object *_vcui_create_button_bigend(void *data);
Evas_Object *_vcui_create_button_bigend_disabled(void *data);

Evas_Object *_vcui_create_conf_list_button_hold(void *data);

Evas_Object *_vcui_create_button_accept(void *data);
Evas_Object *_vcui_create_button_reject(void *data);
Evas_Object *_vcui_create_button_second_incoming_reject(void *data, char *text, char *part_name);
Evas_Object *_vcui_create_button_second_incoming_hold_and_accept(void *data, char *text);
Evas_Object *_vcui_create_button_second_incoming_end_and_accept(void *data, char *text);
Evas_Object *_vcui_create_button_second_incoming_end_active_and_accept(void *data, char *text);
Evas_Object *_vcui_create_button_second_incoming_end_hold_and_accept(void *data, char *text);
Evas_Object *_vcui_create_button_second_incoming_end_all_and_accept(void *data, char *text);

Evas_Object *_vcui_show_wallpaper_image(Evas_Object *contents);
void _vcui_delete_contact_image(Evas_Object *contents);
Evas_Object *_vcui_show_contact_image(Evas_Object *contents, Evas_Object *win_main, char *path);
Evas_Object *_vcui_show_default_image(Evas_Object *contents, Evas_Object *win_main, char *path);
void _vcui_set_full_image(Evas_Object *contents, Evas_Object *win_main, char *img_path);
Evas_Object *_vcui_show_calling_name_bg(Evas_Object *contents); 
#endif	/* _VOICE_CALL_UI_CALLBACK_ */
