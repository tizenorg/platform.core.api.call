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


#ifndef __VCUI_APP_WINDOW_H_
#define __VCUI_APP_WINDOW_H_

#include "vcui-application.h"

Evas_Object *_vcui_app_win_create_main(vcui_app_call_data_t *ad, const char *name);
void _vcui_app_win_key_grab(vcui_app_call_data_t *ad);
void _vcui_app_win_set_noti_type(int bwin_noti);

#endif				/* __VCUI_MAIN_H_ */
