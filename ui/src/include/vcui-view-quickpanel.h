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


#ifndef __VCUI_VIEW_QUICKPANEL_H_
#define __VCUI_VIEW_QUICKPANEL_H_

voice_call_view_data_t *_vcui_view_qp_new(vcui_app_call_data_t * ad);
void _vcui_view_qp_update_text_status(voice_call_view_data_t *vd, char *txt_status);
void _vc_ui_view_qp_set_call_timer(Evas_Object *qp_layout, char *pcall_timer);
void _vcui_view_qp_install_window(voice_call_view_data_t *vd);
void _vcui_view_qp_uninstall_window(voice_call_view_data_t *vd);

#endif				/*__VCUI_VIEW_QUICKPANEL_H_*/
