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
#include <stdlib.h>
#include <string.h>
#include <call.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <dlog.h>
#include <glib.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CAPI_TELEPHONY_CALL"

typedef struct _call_cb_data
{
	call_state_e previous_state;  // previous call state
	const void* state_changed_cb;  // call_state_changed_cb
	void* user_data;  // user data
} call_cb_data;

static bool is_registered = false;  // whether vconf_notify_key_changed is registered or not
static call_cb_data voice_call_cb = {CALL_STATE_IDLE, NULL, NULL};
static call_cb_data video_call_cb = {CALL_STATE_IDLE, NULL, NULL};

 
// Internal Macros
#define CALL_CHECK_INPUT_PARAMETER(arg) \
	if( arg == NULL ) \
	{ \
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, CALL_ERROR_INVALID_PARAMETER); \
		return CALL_ERROR_INVALID_PARAMETER; \
	}


// Callback function adapter
static void __call_state_changed_cb_adapter(keynode_t *node, void* user_data)
{
	call_state_e current_call_state = CALL_STATE_IDLE;

	// Check whether the state of voice call is changed
	if( voice_call_cb.state_changed_cb != NULL )
	{
		if( call_get_voice_call_state(&current_call_state) == CALL_ERROR_NONE )
		{
			if( voice_call_cb.previous_state != current_call_state )
			{
				((call_state_changed_cb)(voice_call_cb.state_changed_cb))(current_call_state, voice_call_cb.user_data);
				voice_call_cb.previous_state = current_call_state;
			}
		}
	}

	// Check whether the state of video call is changed
	if( video_call_cb.state_changed_cb != NULL )
	{
		if( call_get_video_call_state(&current_call_state) == CALL_ERROR_NONE )
		{
			if( video_call_cb.previous_state != current_call_state )
			{
				((call_state_changed_cb)(video_call_cb.state_changed_cb))(current_call_state, video_call_cb.user_data);
				video_call_cb.previous_state = current_call_state;
			}
		}
	}
}


int call_get_voice_call_state(call_state_e *call_state)
{
	int vconf_value = 0;

	CALL_CHECK_INPUT_PARAMETER(call_state);

	if( vconf_get_int(VCONFKEY_CALL_STATE, &vconf_value) != 0 )
	{
		LOGE("[%s] OPERATION_FAILED(0x%08x) : failed to get call state", __FUNCTION__, CALL_ERROR_OPERATION_FAILED);
		return CALL_ERROR_OPERATION_FAILED;
	}

	if( vconf_value == VCONFKEY_CALL_VOICE_CONNECTING )
	{
		LOGI("[%s] The state of voice call is CALL_STATE_CONNECTING", __FUNCTION__);
		*call_state = CALL_STATE_CONNECTING;
	}
	else if( vconf_value == VCONFKEY_CALL_VOICE_ACTIVE )
	{
		LOGI("[%s] The state of voice call is CALL_STATE_ACTIVE", __FUNCTION__);
		*call_state = CALL_STATE_ACTIVE;
	}
	else
	{
		LOGI("[%s] The state of voice call is CALL_STATE_IDLE", __FUNCTION__);
		*call_state = CALL_STATE_IDLE;
	}

	return CALL_ERROR_NONE;
}

int call_get_video_call_state(call_state_e *call_state)
{
	int vconf_value = 0;

	CALL_CHECK_INPUT_PARAMETER(call_state);

	if( vconf_get_int(VCONFKEY_CALL_STATE, &vconf_value) != 0 )
	{
		LOGE("[%s] OPERATION_FAILED(0x%08x) : failed to get call state", __FUNCTION__, CALL_ERROR_OPERATION_FAILED);
		return CALL_ERROR_OPERATION_FAILED;
	}

	if( vconf_value == VCONFKEY_CALL_VIDEO_CONNECTING )
	{
		LOGI("[%s] The state of video call is CALL_STATE_CONNECTING", __FUNCTION__);
		*call_state = CALL_STATE_CONNECTING;
	}
	else if( vconf_value == VCONFKEY_CALL_VIDEO_ACTIVE )
	{
		LOGI("[%s] The state of video call is CALL_STATE_ACTIVE", __FUNCTION__);
		*call_state = CALL_STATE_ACTIVE;
	}
	else
	{
		LOGI("[%s] The state of video call is CALL_STATE_IDLE", __FUNCTION__);
		*call_state = CALL_STATE_IDLE;
	}

	return CALL_ERROR_NONE;
}

int call_set_voice_call_state_changed_cb(call_state_changed_cb callback, void* user_data)
{
	call_state_e voice_call_state = CALL_STATE_IDLE;	

	CALL_CHECK_INPUT_PARAMETER(callback);
	
	if( call_get_voice_call_state(&voice_call_state) != CALL_ERROR_NONE )
	{
		LOGE("[%s] OPERATION_FAILED(0x%08x) : fail to get the current state of voice call",
				__FUNCTION__, CALL_ERROR_OPERATION_FAILED);
		return CALL_ERROR_OPERATION_FAILED;
	}

	if( is_registered == false ) 
	{
		if( vconf_notify_key_changed(VCONFKEY_CALL_STATE, (vconf_callback_fn)__call_state_changed_cb_adapter, NULL) != 0 )
		{
			LOGE("[%s] OPERATION_FAILED(0x%08x) : fail to register callback function", __FUNCTION__, CALL_ERROR_OPERATION_FAILED);
			return CALL_ERROR_OPERATION_FAILED;			
		}

		is_registered = true;			
	}

	voice_call_cb.previous_state = voice_call_state;
	voice_call_cb.state_changed_cb = callback;
	voice_call_cb.user_data = user_data;

	return CALL_ERROR_NONE;
}

int call_unset_voice_call_state_changed_cb()
{
	if( video_call_cb.state_changed_cb == NULL && is_registered == true )
	{
		if( vconf_ignore_key_changed(VCONFKEY_CALL_STATE, (vconf_callback_fn)__call_state_changed_cb_adapter) != 0 )
		{
			LOGE("[%s] OPERATION_FAILED(0x%08x) : fail to unregister callback function", __FUNCTION__, CALL_ERROR_OPERATION_FAILED);
			return CALL_ERROR_OPERATION_FAILED;
		}

		is_registered = false;	
	}

	voice_call_cb.previous_state = CALL_STATE_IDLE;	
	voice_call_cb.state_changed_cb = NULL;
	voice_call_cb.user_data = NULL;

	return CALL_ERROR_NONE;
}

int call_set_video_call_state_changed_cb(call_state_changed_cb callback, void* user_data)
{
	call_state_e video_call_state = CALL_STATE_IDLE;	

	CALL_CHECK_INPUT_PARAMETER(callback);

	if( call_get_video_call_state(&video_call_state) != CALL_ERROR_NONE )
	{
		LOGE("[%s] OPERATION_FAILED(0x%08x) : fail to get the current state of video call",
				__FUNCTION__, CALL_ERROR_OPERATION_FAILED);
		return CALL_ERROR_OPERATION_FAILED;
	}

	if( is_registered == false ) 
	{
		if( vconf_notify_key_changed(VCONFKEY_CALL_STATE, (vconf_callback_fn)__call_state_changed_cb_adapter, NULL) != 0 )
		{
			LOGE("[%s] OPERATION_FAILED(0x%08x) : fail to register callback function", __FUNCTION__, CALL_ERROR_OPERATION_FAILED);
			return CALL_ERROR_OPERATION_FAILED;			
		}

		is_registered = true;			
	}

	video_call_cb.previous_state = video_call_state;
	video_call_cb.state_changed_cb = callback;
	video_call_cb.user_data = user_data;

	return CALL_ERROR_NONE;
}

int call_unset_video_call_state_changed_cb()
{
	if( voice_call_cb.state_changed_cb == NULL && is_registered == true )
	{
		if( vconf_ignore_key_changed(VCONFKEY_CALL_STATE, (vconf_callback_fn)__call_state_changed_cb_adapter) != 0 )
		{
			LOGE("[%s] OPERATION_FAILED(0x%08x) : fail to unregister callback function", __FUNCTION__, CALL_ERROR_OPERATION_FAILED);
			return CALL_ERROR_OPERATION_FAILED;
		}

		is_registered = false;	
	}

	video_call_cb.previous_state = CALL_STATE_IDLE;	
	video_call_cb.state_changed_cb = NULL;
	video_call_cb.user_data = NULL;

	return CALL_ERROR_NONE;
}
