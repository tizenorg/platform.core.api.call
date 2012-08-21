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


#ifndef _VCUI_VIEW_COMMON_H_
#define _VCUI_VIEW_COMMON_H_

#include <time.h>

#define TIME_BUF_LEN (16)
#define MIN_SEC (60)
#define HOUR_MIN (60)
#define DAY_HOUR (24)

#define TB_W (480)
#define TB_H (90)

typedef struct _vcui_view_common_t {
	Ecore_Timer *tm;
	Ecore_Timer *tm_end;
	Ecore_Timer *tm_end_dialing;
	Ecore_Timer *tm_end_force;

	int bredial;

	char sec, min, hour;
	int timer_flag;
	unsigned int call_time;

	int time_end_flag;
	int time_count;
	int end_type;

	time_t current_call_time;
	time_t start_call_time;
} vcui_view_common_t;

void _vcui_view_common_set_each_time(time_t starttime);

void _vcui_view_common_timer_text_init();

void _vcui_view_common_set_text_time(char *time_dur);
void _vcui_view_common_init();

void _vcui_view_common_call_end_show(time_t start_time, int end_type);
void _vcui_view_common_call_end_show_dialing(int end_type, int bredial);
void _vcui_view_common_call_end_timer_reset(void);

void _vcui_view_common_timer_destroy();
void _vcui_view_common_timer_end_destroy();
void _vcui_view_common_timer_end_dialing_destroy();
void _vcui_view_common_timer_redial_reset();

int _vcui_view_common_call_terminate_or_view_change(void);

void _vcui_view_common_show_noid_image(Evas_Object *layout);
void _vcui_view_common_update_mute_btn(void);

#endif
