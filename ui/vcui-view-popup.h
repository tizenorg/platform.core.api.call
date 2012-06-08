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


#ifndef _VOICE_CALL_UI_POPUP_VIEW_
#define _VOICE_CALL_UI_POPUP_VIEW_

void _vcui_view_popup_unload(Evas_Object *popup_eo);
void _vcui_view_popup_load(char *popup_msg, double time_out, int bterminate);
void _vcui_view_popup_replace(char *popup_msg, double time_out, int bterminate);

void _vcui_view_popup_vol_ringtone(int vol_level);
void _vcui_view_popup_vol_voice(int vol_level);
void _vcui_view_popup_vol_bt(int vol_level);

void _vcui_view_popup_load_reject_call(char *name, char *number, int end_app);
void _vcui_view_popup_load_endcall_time(call_data_t *cd);
void _vcui_view_popup_load_redial(void);

void _vcui_view_popup_load_with_delay(char *popup_msg, double delay_time);

void _vcui_view_popup_load_sending_dtmf(char *status_string, char *dtmf_num);
void _vcui_view_popup_unload_progress(vcui_app_call_data_t *ad);
void _vcui_view_popup_load_progress(char *display_string);

void _vcui_view_popup_load_snd_path(void);

void _vcui_view_create_ticker_noti(char *ticker_msg);

#endif
