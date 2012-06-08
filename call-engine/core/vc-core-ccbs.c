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


#ifdef _CCBS_DEFINED_

#include <string.h>
#include <assert.h>

#include "vc-core-ccbs.h"
#include "vc-core-util.h"
#include "vc-core-callagent.h"

/**
 * This function initializes the CCBS info
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent	Handle to the voicecall agent
 */
gboolean call_vc_init_ccbs_info(call_vc_callagent_state_t *pcall_agent)
{
	pcall_agent->ccbs_index = 0;
	memset(&pcall_agent->ccbs_info, 0, sizeof(tapi_call_ccbs_info_t)*CALL_VC_CCBS_NUMBER_MAX);
	return TRUE;
}

/**
 * This function checks if CCBS info is possible or not
 *
 * @return		Returns TRUE if possible and FALSE if not possible
 * @param[in]		pcall_agent	Handle to the voicecall agent
 */
gboolean call_vc_ccbs_info_possible(call_vc_callagent_state_t *pcall_agent)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (pcall_agent->ccbs_index < 0 || pcall_agent->ccbs_index >= CALL_VC_CCBS_NUMBER_MAX) {
		return FALSE;
	}
	return TRUE;
}

/**
 * This function adds one CCBS information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent	Handle to the voicecall agent
 * @param[in]		pCcbsInfo	Pointer to the CCBS info structure
 * @see			call_vc_delete_oneccbs_info
 */
gboolean call_vc_add_oneccbs_info(call_vc_callagent_state_t *pcall_agent, tapi_call_ccbs_info_t *pCcbsInfo)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pCcbsInfo != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (!(pcall_agent->ccbs_index >= 0 && pcall_agent->ccbs_index < CALL_VC_CCBS_NUMBER_MAX)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No Free Index to add ccbs_index  = %d", pcall_agent->ccbs_index);
		return FALSE;
	}
	memcpy(&pcall_agent->ccbs_info[pcall_agent->ccbs_index++], pCcbsInfo, sizeof(tapi_call_ccbs_info_t));

	return TRUE;
}

/**
 * This function deletes one CCBS information
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent	Handle to the voicecall agent
 * @param[in]		ccbsIndex	Index of the CCBS info to be deleted
 * @see			call_vc_add_oneccbs_info
 */
gboolean call_vc_delete_oneccbs_info(call_vc_callagent_state_t *pcall_agent, int ccbsIndex)
{
	int i = 0;
	int j = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	for (i = 0; i < pcall_agent->ccbs_index; i++) {
		if (ccbsIndex == pcall_agent->ccbs_info[i].ccbs_info.index) {
			for (j = i; j < pcall_agent->ccbs_index - 1; j++) {
				memcpy(&pcall_agent->ccbs_info[j], &pcall_agent->ccbs_info[j + 1], sizeof(tapi_call_ccbs_info_t));
			}

			/*Remove the Last Index data*/
			memset(&pcall_agent->ccbs_info[j], 0, sizeof(tapi_call_ccbs_info_t));
			pcall_agent->ccbs_index--;
			break;
		}
	}
	if (i == pcall_agent->ccbs_index)
		return FALSE;

	return TRUE;
}
#endif
