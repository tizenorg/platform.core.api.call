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


#ifndef _VOICE_CALL_UI_INCOMING_LOCK_VIEW_
#define _VOICE_CALL_UI_INCOMING_LOCK_VIEW_

typedef struct {
	Evas_Object *contents;
	Evas_Object *ic;

	Evas_Object *lock_accept;
	int accept_start_x;
	int accept_cur_x;

	Evas_Object *lock_reject;
	int reject_start_x;
	int reject_cur_x;

	call_data_t *now_data;

} incoming_lock_view_priv_t;

voice_call_view_data_t *_vcui_view_incoming_lock_new(vcui_app_call_data_t *ad);

#endif
