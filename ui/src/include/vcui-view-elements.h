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


#ifndef _VCUI_VIEW_ELEMENTS_H_
#define _VCUI_VIEW_ELEMENTS_H_


void _vcui_swap_btn_cb(void *data, Evas_Object * obj, void *event_info);
void _vcui_conf_img_cb(void *data, Evas_Object * obj, void *event_info);


#ifdef _VC_CONTACT_OPT_
Evas_Object *_vcui_create_contact_button(void *data);
Evas_Object *_vcui_create_contact_button_disabled(void *data);
#endif /*_VC_CONTACT_OPT_*/

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

Evas_Object *_vcui_show_wallpaper_image(Evas_Object *contents);
void _vcui_delete_contact_image(Evas_Object *contents);
Evas_Object *_vcui_show_contact_image(Evas_Object *contents, Evas_Object *win_main, char *path);
Evas_Object *_vcui_show_default_image(Evas_Object *contents, Evas_Object *win_main, char *path);
Evas_Object *_vcui_show_calling_name_bg(Evas_Object *contents);

Evas_Object *_vcui_create_videocall_button(void *data, char *number);
Evas_Object *_vcui_create_voicecall_button(void *data, char *number);
Evas_Object *_vcui_create_message_button(void *data, char *number);
Evas_Object *_vcui_create_add_to_contacts_button(void *data, char *number);
Evas_Object *_vcui_create_view_contact_button(void *data, int ct_id);

Evas_Object *_vcui_create_quickpanel_mute_button(void *data);
Evas_Object *_vcui_create_quickpanel_unhold_button(void *data);
Evas_Object *_vcui_create_quickpanel_end_button(void *data);

Evas_Object *_vcui_create_hold_swap_button(void *data);
void _vcui_show_caller_info_name(void *data, const char *name, Eina_Bool bhold);
void _vcui_show_caller_info_number(void *data, const char *number, Eina_Bool bhold);
Evas_Object *_vcui_show_caller_info_icon(void *data, Eina_Bool bhold);
Evas_Object *_vcui_show_caller_info_status(void *data, const char *status, Eina_Bool bhold);
Evas_Object *_vcui_show_contact_image_split(Evas_Object *contents, const char *path, const char *full_path, Eina_Bool bhold);
Evas_Object *_vcui_show_call_bg_img(Evas_Object *contents);
Evas_Object *_vcui_show_call_not_saved_bg_img(Evas_Object *contents);
void _vcui_elements_check_keypad_n_hide(void *data);

#endif	/* _VCUI_VIEW_ELEMENTS_H_ */
