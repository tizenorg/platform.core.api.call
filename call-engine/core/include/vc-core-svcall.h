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


#ifndef __VC_CORE_SVCALL_H_
#define __VC_CORE_SVCALL_H_
#include "vc-core-callagent.h"

#ifdef  _CPHS_DEFINED_

/**
 * This function retrieves cphs information from the Telephony and stores with the engine
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent	Handle to Voicecall Call Agent
 */
gboolean _vc_core_svcall_init_cphs_info(call_vc_callagent_state_t *pcall_agent);

/**
 * This function retrieves status of the given csp service type
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent	Handle to Voicecall Call Agent
 * @param[in]		csp_service	csp service whose status to be retreived
 */
gboolean _vc_core_svcall_cphs_csp_get_status(call_vc_callagent_state_t *pcall_agent, voicecall_cphs_csp_service csp_service);

/**
 * This function retrieves cuurent active line in ALS
 *
 * @return		Returns #voice_call_cphs_alsline_t
 * @param[in]		pcall_agent	Handle to Voicecall Call Agent
 */
voice_call_cphs_alsline_t _vc_core_svcall_get_cphs_als_active_line(call_vc_callagent_state_t *pcall_agent);
#endif

#endif				/*__VC_CORE_SVCALL_H_*/
