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


#ifndef _VCUI_VIEW_MULTI_CALL_CONF_H_
#define _VCUI_VIEW_MULTI_CALL_CONF_H_


voice_call_view_data_t *_vcui_view_multi_call_conf_new(vcui_app_call_data_t *ad);
int	_vcui_view_multi_call_conf_check_valid_eo(voice_call_view_data_t *vd);
Evas_Object *_vcui_view_multi_call_conf_get_main_layout(voice_call_view_data_t *vd);
int	_vcui_view_multi_call_conf_get_call_status(voice_call_view_data_t *vd);
void _vcui_view_multi_call_conf_set_call_timer(voice_call_view_data_t *vd, char *pcall_dur);
Evas_Object *_vcui_view_multi_call_conf_get_button_layout(voice_call_view_data_t *vd);
Evas_Object *_vcui_view_multi_call_conf_get_cid_layout(voice_call_view_data_t *vd);
int	_vcui_view_multi_call_conf_get_total_members(voice_call_view_data_t *vd);
Evas_Object *_vc_ui_view_multi_call_conf_get_caller_info(voice_call_view_data_t *vd);

#endif	/*_VCUI_VIEW_MULTI_CALL_CONF_H_*/

