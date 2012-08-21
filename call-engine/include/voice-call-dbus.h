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


#ifndef _VOICE_CALL_DBUS_H_
#define _VOICE_CALL_DBUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "voice-call-bt.h"

/* prototype */
void vc_engine_on_dbus_send_response_to_bt(connectivity_bt_ag_param_info_t bt_resp_info);
void vc_engine_on_dbus_send_connect_to_bt(void);
int vc_engine_dbus_receiver_setup();

#ifdef __cplusplus
}
#endif

#endif

