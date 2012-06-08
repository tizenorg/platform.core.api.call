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


#ifndef	_VCUI_VIEW_CHOICE_H_
#define	_VCUI_VIEW_CHOICE_H_

void _vcui_view_change(vcui_app_call_view_id_t view_id, int param1, void *param2, void *param3);
void _vcui_view_auto_change();
void _vcui_view_update();
void _vcui_view_all_hide();
void _vcui_view_destroy(vcui_app_call_view_id_t view_id);
 
#endif	/*_VCUI_VIEW_CHOICE_H_*/
