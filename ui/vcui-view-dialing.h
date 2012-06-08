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


#ifndef _VCUI_VIEW_DIALING_
#define _VCUI_VIEW_DIALING_

typedef struct {
	Evas_Object *contents;
	Evas_Object *ic;
	call_data_t *now_data;
} vcui_view_dialing_priv_t;



voice_call_view_data_t * _vcui_view_dialing_new(vcui_app_call_data_t * ad);

void _vcui_view_dialing_draw_txt_ended(voice_call_view_data_t *vd, int end_type);
void _vcui_view_dialing_draw_txt_connecting(voice_call_view_data_t *vd);
void _vcui_view_dialing_draw_txt_dialing(voice_call_view_data_t *vd);

#endif
