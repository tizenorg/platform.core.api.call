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

#include "vc-core-ecc.h"
#include "vc-core-util.h"

/**
 * This function checks whether the given number is emergency number by verifying with no sim emergency numbers
 *
 * @return		TRUE if the number is emergency number, FALSE otherwise
 * @param[in]		pNumber		number to be verified
 */
static gboolean __vc_core_ecc_check_emergency_number_nosim(const char *pNumber);

static char *gcall_vc_ecc_numbers_default[] = {
	"112",
	"911"
};

/**
 * This function checks whether the given number is emergency number by verifying with sim emergency numbers
 *
 * @return		TRUE if the number is emergency number, FALSE otherwise
 * @param[in]		card_type	simcard type
 * @param[in]		pNumber		number to be verified
 */
gboolean _vc_core_ecc_check_emergency_number(TapiHandle *tapi_handle, TelSimCardType_t card_type, char *pNumber, gboolean b_is_no_sim, int *ecc_category)
{
	int i = 0;
	int ecc_count = 0;
	char **p_ecc_numbers = NULL;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;
	unsigned long mcc = 0;	/* for checking Emergency number for each country */
	unsigned long mnc = 0;	/* for checking Emergency number for each operator */

	VOICECALL_RETURN_FALSE_IF_FAIL(pNumber != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(ecc_category != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "pNumber = %s", pNumber);
	*ecc_category = 0;

	if (b_is_no_sim == TRUE) {
		return __vc_core_ecc_check_emergency_number_nosim(pNumber);
	}

	_vc_core_util_get_mcc(&mcc);
	_vc_core_util_get_mnc(&mnc);

	switch (card_type) {
	case TAPI_SIM_CARD_TYPE_GSM:
		{
			TelSimEccList_t sim_ecc_list;	/* used to get data for the Ecc information for 2G and 3G. */

			CALL_ENG_DEBUG(ENG_DEBUG, "[SimCardType=SIM_CARD_TYPE_GSM]");
			memset(&sim_ecc_list, 0x00, sizeof(TelSimEccList_t));

			/*TAPI api Compliance */
			/*To get Emergency data of 2G */
			tapi_err = tel_get_sim_ecc(tapi_handle, &sim_ecc_list);

			if (TAPI_API_SUCCESS != tapi_err) {
				CALL_ENG_DEBUG(ENG_DEBUG, "tapi_sim_get_ecc_info failed, tapi_err=%d", tapi_err);
				return FALSE;
			}

			if (sim_ecc_list.ecc_count > 0) {
				CALL_ENG_DEBUG(ENG_DEBUG, "GSM pNumber ecc1(%s) ecc2(%s) ecc3(%s) ecc4(%s) ecc5(%s)",
					       sim_ecc_list.list[0].number, sim_ecc_list.list[1].number, sim_ecc_list.list[2].number, sim_ecc_list.list[3].number, sim_ecc_list.list[4].number);

				if ((strcmp(pNumber, sim_ecc_list.list[0].number) == 0)
					|| (strcmp(pNumber, sim_ecc_list.list[1].number) == 0)
					|| (strcmp(pNumber, sim_ecc_list.list[2].number) == 0)
					|| (strcmp(pNumber, sim_ecc_list.list[3].number) == 0)
					|| (strcmp(pNumber, sim_ecc_list.list[4].number) == 0)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_check_emergency_number: return TRUE");
					return TRUE;
				}
			}
		}
		break;
	case TAPI_SIM_CARD_TYPE_USIM:
		{
			TelSimEccList_t usim_ecc_list;			/* used to get data for the Ecc information for both 2G and 3G */

			CALL_ENG_DEBUG(ENG_DEBUG, "SimCardType=SIM_CARD_TYPE_USIM");
			memset(&usim_ecc_list, 0x00, sizeof(TelSimEccList_t));

			/*Synchronous function used to get ECC data */
			tapi_err = tel_get_sim_ecc(tapi_handle, &usim_ecc_list);

			if (TAPI_API_SUCCESS != tapi_err) {
				CALL_ENG_DEBUG(ENG_DEBUG, "tapi_sim_get_usim_ecc_info failed, tapi_err=%d", tapi_err);
				return FALSE;
			}

			if (usim_ecc_list.ecc_count > 0) {
				for (i = 0; i < usim_ecc_list.ecc_count; i++) {
					CALL_ENG_DEBUG(ENG_DEBUG, "[ecc=%s, category:[%d]]", usim_ecc_list.list[i].number, usim_ecc_list.list[i].category);
					if (!strcmp(pNumber, usim_ecc_list.list[i].number)) {
						*ecc_category = usim_ecc_list.list[i].category;
						CALL_ENG_DEBUG(ENG_DEBUG, "uecc matched!!");
						return TRUE;
					}
				}
			}
		}
		break;
	case TAPI_SIM_CARD_TYPE_RUIM:
		CALL_ENG_DEBUG(ENG_DEBUG, "SimCardType=TAPI_SIM_CARD_TYPE_RUIM");
		break;
	case TAPI_SIM_CARD_TYPE_IMS:
		CALL_ENG_DEBUG(ENG_DEBUG, "SimCardType=TAPI_SIM_CARD_TYPE_IMS");
		break;
	case TAPI_SIM_CARD_TYPE_UNKNOWN:
		CALL_ENG_DEBUG(ENG_DEBUG, "SimCardType=SIM_CARD_TYPE_UNKNOWN");
		break;
	}

	/* There is no ecc number in the SIM card. */
	CALL_ENG_DEBUG(ENG_DEBUG, "mcc : %ld", mcc);

	switch (mcc) {
	default:
		ecc_count = CALL_ECC_MAX_COUNT_DEFAULT;
		p_ecc_numbers = gcall_vc_ecc_numbers_default;	/*112, 911 */
		break;
	}

	VOICECALL_RETURN_FALSE_IF_FAIL(ecc_count > 0);
	VOICECALL_RETURN_FALSE_IF_FAIL(p_ecc_numbers != NULL);

	for (i = 0; i < ecc_count; i++) {
		if (!strcmp(pNumber, p_ecc_numbers[i])) {
			CALL_ENG_DEBUG(ENG_DEBUG, "pNumber (%s), p_ecc_numbers[%d] (%s)", pNumber, i, p_ecc_numbers[i]);
			return TRUE;
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "No emegency number...");
	return FALSE;
}

/* No SIM - Default*/
static char *gcall_vc_ecc_numbers_nosim_default[] = {
	"000",
	"112",
	"110",
	"118",
	"119",
	"911",
	"999"
};

static gboolean __vc_core_ecc_check_emergency_number_nosim(const char *pNumber)
{
	int i = 0;
	int ecc_count = 0;
	char **p_nosim_ecc_numbers = NULL;
	unsigned long mcc = 0;	/* for checking Emergency number for each country */
	unsigned long mnc = 0;	/* for checking Emergency number for each operator */

	VOICECALL_RETURN_FALSE_IF_FAIL(pNumber != NULL);

	if (strlen(pNumber) == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "pNumber's length is ZERO so return FALSE");
		return FALSE;
	}

	_vc_core_util_get_mcc(&mcc);
	_vc_core_util_get_mnc(&mnc);

	CALL_ENG_DEBUG(ENG_DEBUG, "mcc : %ld", mcc);

	switch (mcc) {
	default:
		ecc_count = CALL_ECC_MAX_COUNT_DEFAULT_NO_SIM;
		p_nosim_ecc_numbers = gcall_vc_ecc_numbers_nosim_default;	/*3GPP SPecs = 000, 08, 112, 100, 118, 119, 911 */
		break;
	}

	VOICECALL_RETURN_FALSE_IF_FAIL(ecc_count > 0);
	VOICECALL_RETURN_FALSE_IF_FAIL(p_nosim_ecc_numbers != NULL);

	for (i = 0; i < ecc_count; i++) {
		if (!strcmp(pNumber, p_nosim_ecc_numbers[i])) {
			CALL_ENG_DEBUG(ENG_DEBUG, "pNumber (%s), p_nosim_ecc_numbers[%d] (%s)", pNumber, i, p_nosim_ecc_numbers[i]);
			return TRUE;
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "No emegency number...");

	return FALSE;
}
