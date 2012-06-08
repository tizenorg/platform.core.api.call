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


#ifndef _VOICE_CALL_SERVICE_H_
#define _VOICE_CALL_SERVICE_H_

#include "voice-call-core.h"
#include "voice-call-engine-msg.h"

/**
* This struct defines a contact record information for voice-call
*/
typedef struct {
	unsigned int ct_index;						/**<Specifies the index for a record  */
	int phone_type;								/**<Specified the phone type */
	char display_name[VC_DISPLAY_NAME_LENGTH_MAX];					  /**<Specifies the display name character data of a record in the contact table. */
	char caller_id_path[VC_IMAGE_PATH_LENGTH_MAX];					  /**<CTS_IMG_NORMAL><Specifies the caller id path of a record in the contact table. */
	char caller_full_id_path[VC_IMAGE_PATH_LENGTH_MAX];				       /**<CTS_IMG_FULL>*/
	char ring_tone[VC_RINGTONE_PATH_LENGTH_MAX];						/**<Specifies the ring tone character data of a record in the contact table. */
} voicecall_contact_info_t;

/**
 * This function on the loud speaker state
 *
 * @return		gboolean
 * @param[in]		pcall_core		Handle to voicecall core		
 */
gboolean voicecall_service_loudspeaker_on(call_vc_core_state_t *pcall_core);

/**
 * This function off the loud speaker state
 *
 * @return		gboolean
 * @param[in]		pcall_core		Handle to voicecall core		
 */
gboolean voicecall_service_loudspeaker_off(call_vc_core_state_t *pcall_core);

/**
 * This function is mute on
 *
 * @return		gboolean
 * @param[in]		pcall_core		Handle to voicecall core		
 */
gboolean voicecall_service_mute_status_on(call_vc_core_state_t *pcall_core);

/**
 * This function is mute off
 *
 * @return		gboolean
 * @param[in]		pcall_core		Handle to voicecall core		
 */
gboolean voicecall_service_mute_status_off(call_vc_core_state_t *pcall_core);

/**
 * This function set volume level.
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core			Handle to voicecall core		
 * @param[in]		vol_alert_type			volume alert type
 * @param[in]		volume_level			volume level to be set
 */
gboolean voicecall_service_set_volume(call_vc_core_state_t *pcall_core, voicecall_snd_volume_alert_type_t vol_alert_type, int volume_level);
#endif
