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


#ifndef _VOICE_CALL_DEVICE_H_
#define _VOICE_CALL_DEVICE_H_

#include "voice-call-core.h"
/**
 * This function initialize earjack event.
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core			Handle to voicecall core		
 */
gboolean _voicecall_dvc_earjack_init(call_vc_core_state_t *pcall_core);

/**
 * This function gets the earjack status 
 *
 * @return		void
 * @param[in]		pcall_core		Handle to voicecall core		
 */
void _voicecall_dvc_get_earjack_status(call_vc_core_state_t *pcall_core);

/**
 * This function gets the earjack is connected or not
 *
 * @return		void
 * @param[in]		pcall_core		Handle to voicecall core		
 */
gboolean _voicecall_dvc_get_earjack_connected(void);

/**
 * This function control the lcd status.
 *
 * @return	void
 * @param[in]	state	one of voicecall_lcd_control_t members.
 */
void _voicecall_dvc_control_lcd_state(voicecall_lcd_control_t state);

#endif
