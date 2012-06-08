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


#ifndef __VC_CORE_CCBS_H_
#define __VC_CORE_CCBS_H_

#include "vc-core-util.h"
#include "vc-core-callagent.h"

#ifdef _CCBS_DEFINED_
/**
 * This function initializes the CCBS info
 *
 * @return		Returns TRUE on success and FALSE on failure
 */
gboolean call_vc_init_ccbs_info(call_vc_callagent_state_t *pcall_agent);
#endif

/**
 * This function checks if CCBS info is possible or not
 *
 * @return		Returns TRUE if possible and FALSE if not possible
 */
gboolean call_vc_ccbs_info_possible(call_vc_callagent_state_t *pcall_agent);

/**
 * This function adds one CCBS information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pCcbsInfo	Pointer to the CCBS info structure
 * @see			call_vc_delete_oneccbs_info
 */
/*gboolean call_vc_add_oneccbs_info(call_vc_callagent_state_t *pcall_agent,tapi_call_ccbs_info_t *pCcbsInfo );*/

/**
 * This function deletes one CCBS information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		ccbsIndex	Index of the CCBS info to be deleted
 * @see			call_vc_add_oneccbs_info
 */
gboolean call_vc_delete_oneccbs_info(call_vc_callagent_state_t *pcall_agent, int ccbsIndex);

#endif				/* __VC_CORE_CCBS_H_ */
