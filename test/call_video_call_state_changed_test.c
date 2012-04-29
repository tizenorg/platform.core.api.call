/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <string.h>
#include <call.h>
#include <dlog.h>
#include <glib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_CALL_TEST"
#define DBG_MODE (1)

static GMainLoop *event_loop;

char* convert_state_to_string(call_state_e call_state)
{
	switch(call_state)
	{
		case CALL_STATE_IDLE:
			return "idle";
		case CALL_STATE_CONNECTING:
			return "connecting";
		case CALL_STATE_ACTIVE:
			return "active";
		default:
			return "unknown";
	}
}

void video_call_state_changed(call_state_e call_state, void* user_data)
{
	LOGI("[%s] Start video_call_state_changed", __FUNCTION__);

	LOGI("[%s] Status of video call: %s", __FUNCTION__, convert_state_to_string(call_state));
	LOGI("[%s] user data: %s", __FUNCTION__, user_data);

	LOGI("[%s] End video_call_state_changed", __FUNCTION__);	
	g_main_loop_quit(event_loop);
}

int main()
{
	if( call_set_video_call_state_changed_cb(video_call_state_changed, 
				"call_video_call_state_changed_test") == CALL_ERROR_NONE )
	{
		LOGI("[%s] Succeeded to set callback function", __FUNCTION__);
	}
	else
	{
		LOGE("[%s] Failed to set callback function", __FUNCTION__);
		return -1;
	}

	LOGI("[%s] If you change the state of video, then callback function will be called", __FUNCTION__);
	event_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(event_loop);

	if( call_unset_video_call_state_changed_cb() == CALL_ERROR_NONE )
	{
		LOGI("[%s] Succeeded to unset callback function", __FUNCTION__);
	}
	else
	{
		LOGE("[%s] Failed to unset callback function", __FUNCTION__);
		return -1;
	}

	return 0;
}
