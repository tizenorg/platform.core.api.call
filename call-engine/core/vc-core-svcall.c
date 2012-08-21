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

#ifdef _CPHS_DEFINED_

#include <stdbool.h>
#include <assert.h>
#include "vc-core-engine.h"
#include "vc-core-svcall.h"
#include "vc-core-util.h"
#include "tapi-inc.h"
#include "status-interface.h"

/*Local Function Decleration*/
/**
* This function checks whether csp is enabled in the cphs group
*
* @internal
* @return		TRUE if customer service profile is enabled -FALSE otherwise
* @param[in]		pcall_agent	Handle to Voicecall Call Agent
*/
static gboolean __call_vcsv_cphs_is_csp_enabled(call_vc_callagent_state_t *pcall_agent);

/**
* This function returns the csp index of the given csp service from the cphs cspprofile
*
* @internal
* @return		index to the specified csp service
* @param[in]		pcphs_csp_profile	Handle to csp profile entry table
* @param[in]		csp_group_max_count	maximumm service entry
* @param[in]		csp_service	csp service to be found
*/
static int __call_vcsv_cphs_find_csp_index(TelSimCphsCustomerServiceProfileEntry_t *pcphs_csp_profile, int csp_group_max_count, TelSimCphsCustomerServiceGroup_t csp_service);

/**
 * This function retrieves cphs information from the Telephony and stores with the engine
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pcall_agent	Handle to Voicecall Call Agent
 */
gboolean _vc_core_svcall_init_cphs_info(call_vc_callagent_state_t *pcall_agent)
{
	gboolean ret_val = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	memset(&pcall_agent->cphs_status, 0, sizeof(status_class_cphs_type));

	/*Read CPHS Info */
#ifdef _CPHS_DEFINED_
	ret_val = nps_get_cphs_status(&pcall_agent->cphs_status);
#endif

	if (FALSE == ret_val) {
		CALL_ENG_DEBUG(ENG_DEBUG, "nps_get_cphs_status failed or CPHS feature not enabled");
		pcall_agent->bcphs_read_success = FALSE;
		return FALSE;
	}

	pcall_agent->bcphs_read_success = TRUE;

	return TRUE;
}

/**
 * This function retrieves status of the given csp service type
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		csp_service	csp service whose status to be retreived
 */
gboolean _vc_core_svcall_cphs_csp_get_status(call_vc_callagent_state_t *pcall_agent, voicecall_cphs_csp_service csp_service)
{
	status_class_cphs_type *pcphs_status_info = (status_class_cphs_type *) &pcall_agent->cphs_status;
	int csp_index = -1;

	if (FALSE == __call_vcsv_cphs_is_csp_enabled(pcall_agent)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "CPHS CSP Not enabled");

		if (VC_CPHS_CSP_ALS == csp_service) {
			return FALSE;
		} else {
			/*If CPHS is not supported, then by default return TRUE for the client to have default action */
			return TRUE;
		}
	}

	switch (csp_service) {
	case VC_CPHS_CSP_ALS:	/*CPHS Teleservices */
		{
			csp_index = __call_vcsv_cphs_find_csp_index(pcphs_status_info->csp.serviceProfileEntry, TAPI_SIM_CPHS_CUSTOMER_SERVICE_PROFILE_ENTRY_COUNT_MAX, TAPI_SIM_CPHS_CSP_SERVICE_GROUP_CPHS_TELESERVICES);
		}
		break;
	case VC_CPHS_CSP_HOLD:	/*Call Completion Services */
	case VC_CPHS_CSP_CW:
	case VC_CPHS_CSP_CBS:
	case VC_CPHS_CSP_UUS:
		{
			csp_index = __call_vcsv_cphs_find_csp_index(pcphs_status_info->csp.serviceProfileEntry, TAPI_SIM_CPHS_CUSTOMER_SERVICE_PROFILE_ENTRY_COUNT_MAX, TAPI_SIM_CPHS_CSP_SERVICE_GROUP_CALL_COMPLETION);
		}
		break;
	case VC_CPHS_CSP_CT:	/*Call Offering Services */
	case VC_CPHS_CSP_CFU:
	case VC_CPHS_CSP_CFB:
	case VC_CPHS_CSP_CFNRY:
	case VC_CPHS_CSP_CFNRC:
		{
			csp_index = __call_vcsv_cphs_find_csp_index(pcphs_status_info->csp.serviceProfileEntry, TAPI_SIM_CPHS_CUSTOMER_SERVICE_PROFILE_ENTRY_COUNT_MAX, TAPI_SIM_CPHS_CSP_SERVICE_GROUP_CALL_OFFERING);
		}
		break;
	case VC_CPHS_CSP_MPTY:	/*Other Supplementary Services */
	case VC_CPHS_CSP_CUG:
	case VC_CPHS_CSP_AOC:
	case VC_CPHS_CSP_PREFCUG:
	case VC_CPHS_CSP_CUGOA:
		{
			csp_index = __call_vcsv_cphs_find_csp_index(pcphs_status_info->csp.serviceProfileEntry, TAPI_SIM_CPHS_CUSTOMER_SERVICE_PROFILE_ENTRY_COUNT_MAX, TAPI_SIM_CPHS_CSP_SERVICE_GROUP_OTHER_SUPP_SERVICES);
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "No Actions defined for service type: %d", csp_service);
		break;
	}

	if (-1 == csp_index) {
		CALL_ENG_DEBUG(ENG_DEBUG, "csp_index failed for csp service: %d", csp_service);
		return FALSE;
	}

	switch (csp_service) {
	case VC_CPHS_CSP_ALS:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.cphsTeleservices.bAlternativeLineService;
		}
		break;
	case VC_CPHS_CSP_HOLD:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callComplete.bCallHold;
		}
		break;
	case VC_CPHS_CSP_CW:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callComplete.bCallWaiting;
		}
		break;
	case VC_CPHS_CSP_CBS:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callComplete.bCompletionOfCallToBusySubscriber;
		}
		break;
	case VC_CPHS_CSP_UUS:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callComplete.bUserUserSignalling;
		}
		break;
	case VC_CPHS_CSP_CFU:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callOffering.bCallForwardingUnconditional;
		}
		break;
	case VC_CPHS_CSP_CFB:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callOffering.bCallForwardingOnUserBusy;
		}
		break;
	case VC_CPHS_CSP_CFNRY:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callOffering.bCallForwardingOnNoReply;
		}
		break;
	case VC_CPHS_CSP_CFNRC:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callOffering.bCallForwardingOnUserNotReachable;
		}
		break;
	case VC_CPHS_CSP_CT:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.callOffering.bCallTransfer;
		}
		break;
	case VC_CPHS_CSP_MPTY:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.otherSuppServices.bMultiPartyService;
		}
		break;
	case VC_CPHS_CSP_CUG:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.otherSuppServices.bClosedUserGroup;
		}
		break;
	case VC_CPHS_CSP_AOC:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.otherSuppServices.bAdviceOfCharge;
		}
		break;
	case VC_CPHS_CSP_PREFCUG:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.otherSuppServices.bPreferentialClosedUserGroup;
		}
		break;
	case VC_CPHS_CSP_CUGOA:
		{
			return pcphs_status_info->csp.serviceProfileEntry[csp_index].u.otherSuppServices.bPreferentialClosedUserGroup;
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Action not defined for csp service: %d", csp_service);
		return FALSE;
	}

	return FALSE;
}

voice_call_cphs_alsline_t _vc_core_svcall_get_cphs_als_active_line(call_vc_callagent_state_t *pcall_agent)
{
	status_class_cphs_type *pcphs_status_info = (status_class_cphs_type *) &pcall_agent->cphs_status;

	if (FALSE == __call_vcsv_cphs_is_csp_enabled(pcall_agent)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "CPHS CSP Not enabled");
		return VC_CALL_CPHS_ALS_LINE1;
	}

	if (TAPI_SIM_DYNAMIC_FLAGS_LINE1 == pcphs_status_info->dflagsinfo.dynamicFlags) {
		return VC_CALL_CPHS_ALS_LINE1;
	} else if (TAPI_SIM_DYNAMIC_FLAGS_LINE2 == pcphs_status_info->dflagsinfo.dynamicFlags) {
		return VC_CALL_CPHS_ALS_LINE2;
	}

	return VC_CALL_CPHS_ALS_LINE1;

}

/*Local Function Decleration*/
static gboolean __call_vcsv_cphs_is_csp_enabled(call_vc_callagent_state_t *pcall_agent)
{
	status_class_cphs_type *pnps_cphs_status = NULL;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	if (pcall_agent->bcphs_read_success == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "CSP Status not read");
		return FALSE;
	}

	pnps_cphs_status = &pcall_agent->cphs_status;

	if (FALSE == pnps_cphs_status->cphsinfo.CphsServiceTable.bCustomerServiceProfile) {
		return TRUE;
	}

	return FALSE;
}

static int __call_vcsv_cphs_find_csp_index(TelSimCphsCustomerServiceProfileEntry_t *pcphs_csp_profile, int csp_group_max_count, TelSimCphsCustomerServiceGroup_t csp_service)
{
	int index = 0;
	CALL_ENG_DEBUG(ENG_DEBUG, "");

	for (index = 0; index < csp_group_max_count; index++) {
		if (pcphs_csp_profile[index].customerServiceGroup == csp_service) {
			return index;
		}
	}

	return VC_ERROR;
}
#endif				/*_CPHS_DEFINED_*/
