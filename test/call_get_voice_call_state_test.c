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

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_CALL_TEST"
#define DBG_MODE (1)

void convert_state_into_string(call_state_e call_state, char* call_state_string)
{
	switch(call_state)
	{
		case CALL_STATE_IDLE:
			snprintf(call_state_string, 256, "idle");
			break;
		case CALL_STATE_CONNECTING:
			snprintf(call_state_string, 256, "connecting");
			break;
		case CALL_STATE_ACTIVE:
			snprintf(call_state_string, 256, "active");
			break;
		default:
			snprintf(call_state_string, 256, "unknown");
			break;		
	}

}

int main()
{
	int ret = 0;
	call_state_e call_state = CALL_STATE_IDLE;
	char call_state_string[256] = "";
	int ret_value = call_get_voice_call_state(&call_state);

	switch(ret_value)
	{
		case CALL_ERROR_NONE:
			convert_state_into_string(call_state, call_state_string);
			LOGI("[%s] CALL is %s", __FUNCTION__, call_state_string);
			ret = 0;
			break;
		case CALL_ERROR_OPERATION_FAILED:
			LOGI("[%s] CALL_ERROR_OPERATION_FAILED", __FUNCTION__);			
			ret = -1;
			break;
		case CALL_ERROR_INVALID_PARAMETER:
			LOGI("[%s] CALL_ERROR_INVALID_PARAMETER", __FUNCTION__);			
			ret = -1;
			break;                                                                                                                                                
		default:
			LOGI("[%s] Unexpected return value", __FUNCTION__);						
			ret = -1;
			break;
	}

	return ret;
}
