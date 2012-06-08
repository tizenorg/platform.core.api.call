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


#ifndef _VCUI_VIEW_SINGLE_CALL_
#define _VCUI_VIEW_SINGLE_CALL_
	 
typedef struct {
	Evas_Object *contents;
	Evas_Object *ic;
	Evas_Object *record_btn;
	call_data_t *now_data;
} incall_one_view_priv_t;

voice_call_view_data_t *_vc_ui_view_single_call_new(vcui_app_call_data_t *ad);
#endif
