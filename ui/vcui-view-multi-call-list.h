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


#ifndef _VCUI_VIEW_MULTI_CALL_LIST_
#define _VCUI_VIEW_MULTI_CALL_LIST_

typedef struct {
	Evas_Object *navi_frame;
	Evas_Object *contents;

	Evas_Object *ic;

	Evas_Object *record_btn;

	vcui_app_call_status_t call_status;

	Evas_Object *multibox_gl;

	int	total_members;
} vcui_view_multi_call_list_priv_t;

#endif

voice_call_view_data_t *_vcui_view_multi_call_list_new(vcui_app_call_data_t *ad);
