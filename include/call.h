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


#ifndef __TIZEN_TELEPHONY_CALL_H__
#define __TIZEN_TELEPHONY_CALL_H__

/**
 * @file call.h
 * @brief This file contains call APIs and related enumeration.
 */

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_TELEPHONY_CALL_MODULE
 * @{
 */
 
/**
 * @brief Enumeration for call error.
 */
typedef enum
{
	CALL_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
	CALL_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	CALL_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */	
	CALL_ERROR_OPERATION_FAILED = TIZEN_ERROR_TELEPHONY_CLASS | 0x1000, /**< Operation failed */	
} call_error_e;

/**
 * @brief Enumeration of the call state.
 */
typedef enum
{
    CALL_STATE_IDLE, /**< No activity. */
    CALL_STATE_CONNECTING, /**< Ringing, dialing or new call is coming when another call is already active. */
    CALL_STATE_ACTIVE, /**< A call is active. */
} call_state_e;

/**
 * @brief Called when call state changes.
 * @param [in] call_state The call state
 * @param [in] user_data The user data passed from the callback registration function
 * @pre This callback function is invoked if you register this function using either call_set_voice_call_state_changed_cb() or call_set_video_call_state_changed_cb().
 * @see call_set_voice_call_state_changed_cb()
 * @see call_unset_voice_call_state_changed_cb() 
 * @see call_set_video_call_state_changed_cb()
 * @see call_unset_video_call_state_changed_cb() 
 */
typedef void(* call_state_changed_cb)(call_state_e call_state, void *user_data);


/**
 * @brief Gets the state of voice call.
 * @details Determines if the voice call is connecting, active, or idle. 
 *
 * @remarks When you are dialing a number or a new voice call is ringing, the state of the voice call is #CALL_STATE_CONNECTING. 
 * When a new voice call is connecting while another voice call is already active, 
 * the state of the voice call is #CALL_STATE_CONNECTING as well.
 *
 * @param[out] call_state The current state of the voice call 
 * @return 0 on success, otherwise a negative error value.
 * @retval #CALL_ERROR_NONE Successful
 * @retval #CALL_ERROR_OPERATION_FAILED Operation failed
 * @retval #CALL_ERROR_INVALID_PARAMETER Invalid parameter
 */
int call_get_voice_call_state(call_state_e *call_state);

/**
 * @brief Gets the state of video call.
 * @details Determines if the video call is connecting, active, or idle.  
 * @remarks When you are dialing a number or a new video call is ringing, the state of the video call is #CALL_STATE_CONNECTING. 
 * If a video call is active, then the state of any other call cannot be #CALL_STATE_CONNECTING and #CALL_STATE_ACTIVE.
 *
 * @param[out] call_state The current state of the video call
 * @return 0 on success, otherwise a negative error value.
 * @retval #CALL_ERROR_NONE Successful
 * @retval #CALL_ERROR_OPERATION_FAILED Operation failed
 * @retval #CALL_ERROR_INVALID_PARAMETER Invalid parameter
 */
int call_get_video_call_state(call_state_e *call_state);

/**
 * @brief Registers a callback function to be invoked when the voice call state changes.
 *
 * @param [in] callback The callback function to register
 * @param [in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #CALL_ERROR_NONE Successful
 * @retval #CALL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #CALL_ERROR_OPERATION_FAILED Operation failed
 * @post call_state_changed_cb() will be invoked. 
 * @see  call_state_changed_cb()
 * @see  call_unset_voice_call_state_changed_cb()
 *
 */
int call_set_voice_call_state_changed_cb(call_state_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * 
 * @return 0 on success, otherwise a negative error value.
 * @retval #CALL_ERROR_NONE Successful
 * @retval #CALL_ERROR_OPERATION_FAILED Operation failed
 * @see	call_set_voice_call_state_changed_cb()
 *
 */
int call_unset_voice_call_state_changed_cb();

/**
 * @brief Registers a callback function to be invoked when the video call state changes.
 *
 * @param [in] callback The callback function to register
 * @param [in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #CALL_ERROR_NONE Successful
 * @retval #CALL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #CALL_ERROR_OPERATION_FAILED Operation failed
 * @post call_state_changed_cb() will be invoked. 
 * @see call_state_changed_cb()
 * @see call_unset_video_call_state_changed_cb()
 *
 */
int call_set_video_call_state_changed_cb(call_state_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * 
 * @return 0 on success, otherwise a negative error value.
 * @retval #CALL_ERROR_NONE Successful
 * @retval #CALL_ERROR_OPERATION_FAILED Operation failed 
 * @see	call_set_video_call_state_changed_cb()
 *
 */
int call_unset_video_call_state_changed_cb();

					
#ifdef __cplusplus
}
#endif


#endif // __TIZEN_TELEPHONY_CALL_H__

/**
 * @}
 */
