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


#ifndef _VOICE_CALL_UI_INCALL_MULTI_VIEW_SPLIT_
#define _VOICE_CALL_UI_INCALL_MULTI_VIEW_SPLIT_

typedef struct {
	Evas_Object *contents;

	Evas_Object *ic_incall;
	Evas_Object *ic_onhold;

	Evas_Object *record_btn;

	call_data_t *now_data_hold;
	call_data_t *now_data_unhold;

} incall_multi_view_split_priv_t;

voice_call_view_data_t *_vcui_view_multi_call_split_new(vcui_app_call_data_t *ad);
#endif

