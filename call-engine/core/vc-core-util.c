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


#include <stdbool.h>
#include <assert.h>
#include "vc-core-engine.h"
#include "vc-core-util.h"
#include "vc-core-callagent.h"
#include "vc-core-callmanager.h"

#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <pmapi.h>

#include <unicode/uloc.h>
#include <unicode/udat.h>
#include <unicode/udatpg.h>
#include <unicode/ustring.h>

#include "appcore-common.h"

#define CALL_VODAFONEUK_CLI_SHOW_ID "1470"
#define CALL_COMMON_CLI_SHOW_ID "*31#"
#define CALL_COMMON_CLI_HIDE_ID "#31#"

#define ASCII_0		48
#define ASCII_9		57
#define ASCII_A		65
#define ASCII_D		68
#define ASCII_STAR	42
#define ASCII_HASH	35
#define ASCII_P		80	/*PDIAL_SEND_DTMF*/
#define	ASCII_p		112	/*PDIAL_SEND_DTMF*/
#define	ASCII_COMMA	44	/*PDIAL_SEND_DTMF*/

#define EIGTH_BIT	0x80
#define SEVENTH_BIT 0x40
#define SIXTH_BIT	0x20
#define FIFTH_BIT	0x10
#define FOURTH_BIT	0x08
#define THIRD_BIT	0x04
#define SECOND_BIT	0x02
#define FIRST_BIT	0x01

#define CALLVC_MIN(x, y) ((x) <= (y) ? (x) : (y))	/*PDIAL_SEND_DTMF*/
#define ISUSSDDIGIT(X) ((X >= '2') && (X <= '9'))
#define ISDIGIT(X) ((X >= '0') && (X <= '9'))
#define ISSSDIGITINCALL(X) ((X >= '7') && (X <= '9'))
#define IS1CHARUSSDDIGIT(X) ((X >= '1') && (X <= '9'))

#define DATE_FORMAT_1 "EEMMMddyyyy" /* Thu Aug 23 2001 */
#define TIME_12_TYPE "hma"  /* 7:58 AM */
#define TIME_24_TYPE "HHmm" /* 19:58 */

typedef enum _call_vc_emergency_number_type_t {
	CALL_VC_NO_SIM_EMERGENCY,
	CALL_VC_UK_EMERGENCY,
	CALL_VC_UKRAINE_EMERGENCY,
	CALL_VC_DEFAULT_EMERGENCY,
	CALL_VC_EMERGENCY_TYPE_MAX
} call_vc_emergency_number_type_t;


#define CALL_VC_EMERGENCY_NUMBER_LEN_MAX	3

/**************************************************************************************************************************
#define CALL_VC_KOREA_EMERGENCY_NUMBERS_CNT				3
static char gcall_vc_korea_emergency_numbers[CALL_VC_KOREA_EMERGENCY_NUMBERS_CNT][CALL_VC_EMERGENCY_NUMBER_LEN_MAX + 1] = {
	"112",
	"911",
	"119"
};
**************************************************************************************************************************/

#define CALL_VC_DEFAULT_EMERGENCY_NUMBERS_CNT				2
static char gcall_vc_emergency_numbers[CALL_VC_DEFAULT_EMERGENCY_NUMBERS_CNT][CALL_VC_EMERGENCY_NUMBER_LEN_MAX + 1] = {
	"112",
	"911",
};

#define CALL_VC_UK_EMERGENCY_NUMBERS_CNT		3
static char gcall_vc_uk_emergency_numbers[CALL_VC_UK_EMERGENCY_NUMBERS_CNT][CALL_VC_EMERGENCY_NUMBER_LEN_MAX + 1] = {
	"112",
	"911",
	"999",
};

#define CALL_VC_UKAINE_EMERGENCY_NUMBERS_CNT		5
static char gcall_vc_ukaine_emergency_numbers[CALL_VC_UKAINE_EMERGENCY_NUMBERS_CNT][CALL_VC_EMERGENCY_NUMBER_LEN_MAX + 1] = {
	"112",
	"911",
	"01",
	"02",
	"03"
};

#define CALL_VC_ITALY_EMERGENCY_NUMBERS_CNT		6
static char gcall_vc_italy_emergency_numbers[CALL_VC_ITALY_EMERGENCY_NUMBERS_CNT][CALL_VC_EMERGENCY_NUMBER_LEN_MAX + 1] = {
	"112",
	"911",
	"999",
	"110",
	"118",
	"115"
};

#define CALL_VC_NO_SIM_DEFAULT_EMERGENCY_NUMBERS_CNT		8
static char gcall_vc_nosim_emergency_numbers[CALL_VC_NO_SIM_DEFAULT_EMERGENCY_NUMBERS_CNT][CALL_VC_EMERGENCY_NUMBER_LEN_MAX + 1] = {
	"000",
	"08",
	"112",
	"110",
	"118",
	"119",
	"911",
	"999"
};

/**
* This function checks the given number against the default emergency numbers list
*
* @internal
* @return		TRUE if given number is found in the emergency number list, FALSE otherwise
* @param[out]	pNumber	number to be verified
*/
static gboolean __vc_core_util_check_default_emergency_number(char *pNumber);

/**
 * This function publishes the event of a given publish type
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		src		Telehone number from which call number needd to be parsed
 * @param[out]		dst		Pointer to the buffer to store the parsed number
 */
gboolean _vc_core_util_remove_invalid_chars(const char *src, char *dst)
{
	VOICECALL_RETURN_FALSE_IF_FAIL((src != NULL));
	VOICECALL_RETURN_FALSE_IF_FAIL((dst != NULL));

	CALL_ENG_DEBUG(ENG_DEBUG, "Source phone number - %s\n",src);
	int i = 0;
	int j = 0;
	int nSrc = 0;

	nSrc = strlen(src);
	CALL_ENG_DEBUG(ENG_DEBUG, , "source String len - %d\n", nSrc);

	for(i = 0; i < nSrc; ++i)
	{
		switch(src[i])
		{
			case '(':
			{
				if(src[i+1]== '0')
					++i;
				break;
			}

			case ')':
			case '-':
			case ' ':
			case '/':
			{
				break;
			}

			default:
			{
				dst[j++] = src[i];
				break;
			}
		}
		if(j>40)
		{
			break;
		}
	}

	dst[j] = '\0';
	CALL_ENG_DEBUG(ENG_DEBUG, , "Destination phone number - %s\n",dst);

	return TRUE;
}

/**
 * This function publishes the event of a given publish type
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pszTelNumber		Telehone number from which call number nned to be extracted
 * @param[out]	pBuffer			Pointer to the buffer to store the extracted number
 * @param[in]		nBufSize			Size of input buffer
 */
gboolean _vc_core_util_extract_call_number(const char *pszTelNumber, char *pBuffer, const int nBufSize)
{
	char *pszNum = NULL;
	char *pst = NULL;

	VOICECALL_RETURN_FALSE_IF_FAIL((pBuffer != NULL));
	VOICECALL_RETURN_FALSE_IF_FAIL((pszTelNumber != NULL));

	CALL_ENG_DEBUG(ENG_DEBUG, "number = %s..", pszTelNumber);

	memset(pBuffer, 0, nBufSize);

	if ((strncmp(pszTelNumber, CALL_COMMON_CLI_SHOW_ID, 4) == 0) || (strncmp(pszTelNumber, CALL_COMMON_CLI_HIDE_ID, 4) == 0))
		pszNum = (char *)pszTelNumber + 4;
	else
		pszNum = (char *)pszTelNumber;

	pst = strchr(pszNum, 'P');

	if (pst == NULL)
		pst = strchr(pszNum, 'p');

	if (pst == NULL)
		pst = strchr(pszNum, ',');	/*browser request*/

	if (pst == NULL) {
		if (strlen(pszNum) == 0)
			return FALSE;
		_vc_core_util_strcpy(pBuffer, nBufSize, pszNum);
	} else {
		if (pst == pszNum)
			return FALSE;

		_vc_core_util_strcpy(pBuffer, min((((int)pst - (int)pszNum)+1), (nBufSize)), pszNum);
	}
	return TRUE;
}

gboolean _vc_core_util_extract_call_number_without_cli(const char *pszTelNumber, char *pBuffer, const int nBufSize)
{
	char *pszNum = NULL;
	char *pst = NULL;

	VOICECALL_RETURN_FALSE_IF_FAIL((pszTelNumber != NULL));
	VOICECALL_RETURN_FALSE_IF_FAIL((pBuffer != NULL));

	CALL_ENG_DEBUG(ENG_DEBUG, "number = %s..", pszTelNumber);

	memset(pBuffer, 0, nBufSize);
	pszNum = (char *)pszTelNumber;
	pst = strchr(pszNum, 'P');

	if (pst == NULL)
		pst = strchr(pszNum, 'p');

	if (pst == NULL)
		pst = strchr(pszNum, ',');	/*browser request*/

	if (pst == NULL) {
		if (strlen(pszNum) == 0)
			return FALSE;
		_vc_core_util_strcpy(pBuffer, nBufSize, pszNum);
	} else {
		if (pst == pszNum)
			return FALSE;

		_vc_core_util_strcpy(pBuffer, min((((int)pst - (int)pszNum)+1), (nBufSize)), pszNum);
	}
	return TRUE;
}

/**
* This function checks the validity of the given string for SS string
 *
* @return		Returns SS_SI_FORMAT_INVALID - if the string is invalid ss string, SS_SI_FORMAT_VALID - if valid
* @param[in]	pinput_string	string to be verified
 */
call_vc_ss_si_format _vc_core_util_check_si_format(const char *pinput_string)
{
	int index = 0, pos = 1, cnt = 0;
	VOICECALL_RETURN_VALUE_IF_FAIL((pinput_string != NULL),SS_SI_FORMAT_INVALID);

	if ((pinput_string[1] == '*') || (pinput_string[1] == '#'))
		pos = 2;

	index = pos;

	while (cnt < 1) {
		if (pinput_string[index++] == '*')
			cnt++;

		if (index >= strlen(pinput_string + pos)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "valid ss format...");
			return SS_SI_FORMAT_VALID;
		}
	}

	return SS_SI_FORMAT_VALID;
}

gboolean _vc_core_util_get_mcc(unsigned long *mcc)
{
	int ret = 0;
	int plmn_value = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL((mcc != NULL));

	ret = vconf_get_int(VCONFKEY_TELEPHONY_PLMN, &plmn_value);
	if (0 == ret) {
		char mcc_value[4];
		char plmn_string[10];

		CALL_ENG_DEBUG(ENG_DEBUG, "plmn_value = [%d]", plmn_value);

		memset(plmn_string, 0, sizeof(plmn_string));
		memset(mcc_value, 0, sizeof(mcc_value));

		snprintf(plmn_string, 10, "%d", plmn_value);

		/*First 3 digits of plmn value constitutes the mcc value */
		_vc_core_util_strcpy(mcc_value, 4, plmn_string);
		*mcc = (unsigned long)atoi(mcc_value);
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
		*mcc = CALL_NETWORK_MCC_UK;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "mcc = %ld", *mcc);

	return TRUE;
}

static gboolean __vc_core_util_check_default_emergency_number(char *pNumber)
{
	int i = 0;
	unsigned long mcc = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL((pNumber != NULL));

	_vc_core_util_get_mcc(&mcc);

	if (mcc == CALL_NETWORK_MCC_UK) {
		/*112, 911,999*/
		for (i = 0; i < CALL_VC_UK_EMERGENCY_NUMBERS_CNT; i++) {
			if (!strcmp(pNumber, gcall_vc_uk_emergency_numbers[i]))
				return TRUE;
		}
	} else if (mcc == CALL_NETWORK_MCC_UKRAINE) {
		/*112, 911, 01, 02, 03*/
		for (i = 0; i < CALL_VC_UKAINE_EMERGENCY_NUMBERS_CNT; i++) {
			if (!strcmp(pNumber, gcall_vc_ukaine_emergency_numbers[i]))
				return TRUE;
		}
	} else if (mcc == CALL_NETWORK_MCC_ITALY) {
		/*112, 911, 999, 110, 118, 115*/
		for (i = 0; i < CALL_VC_ITALY_EMERGENCY_NUMBERS_CNT; i++) {
			if (!strcmp(pNumber, gcall_vc_italy_emergency_numbers[i]))
				return TRUE;
		}
	} else {
		/*112, 911*/
		for (i = 0; i < CALL_VC_DEFAULT_EMERGENCY_NUMBERS_CNT; i++) {
			if (!strcmp(pNumber, gcall_vc_emergency_numbers[i]))
				return TRUE;
		}
	}
	return FALSE;
}

/**
 * This function checks whether the given number is emergency number by verifying with sim emergency numbers
 *
 * @return		TRUE if the number is emergency number, FALSE otherwise
 * @param[in]		card_type	simcard type
 * @param[in]		pNumber		number to be verified
 */
gboolean _vc_core_util_check_emergency_number(TelSimCardType_t card_type, char *pNumber, gboolean b_is_no_sim, int *ecc_category)
{
	int i = 0;
	TapiResult_t tapi_err = TAPI_API_SUCCESS;

	VOICECALL_RETURN_FALSE_IF_FAIL(pNumber != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(ecc_category != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "pNumber = %s", pNumber);
	*ecc_category = 0;

	/*if(call_vc_util_get_sim_mode() == CALL_VC_SIM_INSERTED)*/
	if (b_is_no_sim == FALSE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "We have a SIM");
		switch (card_type) {
		case TAPI_SIM_CARD_TYPE_GSM:
			{
				TelSimEccData_t sim_ecc_info;	/* used to get data for the Ecc information for 2G and 3G. */
				int necc_record_count = 0;	/*Used to count number of Emergency records */

				CALL_ENG_DEBUG(ENG_DEBUG, "[SimCardType=SIM_CARD_TYPE_GSM]");
				memset(&sim_ecc_info, 0x00, sizeof(TelSimEccData_t));

				/*TAPI api Compliance */
				/*To get Emergency data of 2G */
				tapi_err = tel_get_sim_ecc(&sim_ecc_info, &necc_record_count);

				if (TAPI_API_SUCCESS != tapi_err) {
					CALL_ENG_DEBUG(ENG_DEBUG, "tapi_sim_get_ecc_info failed, tapi_err=%d", tapi_err);
					return FALSE;
				}

				if (necc_record_count == 0) {
					CALL_ENG_DEBUG(ENG_DEBUG, "eccinfo is NOT exist. Check default");

					/*SIM card inserted but no ecc infomation. Then check the default ECC number.*/
					return __vc_core_util_check_default_emergency_number(pNumber);
				} else {
					/*Check the availablity of Emergency number in the ECCInfo*/
					CALL_ENG_DEBUG(ENG_DEBUG, "ecc1=%s", sim_ecc_info.EccInfo.szEcc1);
					if (strcmp(pNumber, sim_ecc_info.EccInfo.szEcc1) == 0)	/*ecc1*/
						return TRUE;

					CALL_ENG_DEBUG(ENG_DEBUG, "ecc2=%s", sim_ecc_info.EccInfo.szEcc2);
					if (strcmp(pNumber, sim_ecc_info.EccInfo.szEcc2) == 0)	/*ecc2*/
						return TRUE;

					CALL_ENG_DEBUG(ENG_DEBUG, "ecc3=%s", sim_ecc_info.EccInfo.szEcc3);
					if (strcmp(pNumber, sim_ecc_info.EccInfo.szEcc3) == 0)	/*ecc3*/
						return TRUE;

					CALL_ENG_DEBUG(ENG_DEBUG, "ecc4=%s", sim_ecc_info.EccInfo.szEcc4);
					if (strcmp(pNumber, sim_ecc_info.EccInfo.szEcc4) == 0)	/*ecc4*/
						return TRUE;

					CALL_ENG_DEBUG(ENG_DEBUG, "ecc5=%s", sim_ecc_info.EccInfo.szEcc5);
					if (strcmp(pNumber, sim_ecc_info.EccInfo.szEcc5) == 0)	/*ecc5*/
						return TRUE;

					CALL_ENG_DEBUG(ENG_DEBUG, "No match & check default emergency number...");

					return __vc_core_util_check_default_emergency_number(pNumber);
				}
			}
			break;
		case TAPI_SIM_CARD_TYPE_USIM:
			{
				TelSimEccData_t sim_usim_ecc_info;	/* used to get data for the Ecc information for both 2G and 3G */
				int nuecc_rec_count = 0;

				CALL_ENG_DEBUG(ENG_DEBUG, "SimCardType=SIM_CARD_TYPE_USIM");

				memset(&sim_usim_ecc_info, 0x00, sizeof(TelSimEccData_t));

				/*Synchronous function used to get ECC data */
				tapi_err = tel_get_sim_ecc(&sim_usim_ecc_info, &nuecc_rec_count);

				if (TAPI_API_SUCCESS != tapi_err) {
					CALL_ENG_DEBUG(ENG_DEBUG, "tapi_sim_get_usim_ecc_info failed, tapi_err=%d", tapi_err);
					return FALSE;
				}

				if (nuecc_rec_count == 0) {
					CALL_ENG_DEBUG(ENG_DEBUG, "ueccInfo is NOT exists...");

					/*SIM card inserted but no ecc infomation. Then check the default ECC number.*/
					return __vc_core_util_check_default_emergency_number(pNumber);
				} else {
					CALL_ENG_DEBUG(ENG_DEBUG, "ueccInfo exists...");

					/*Check in USIM Emergency Numbers*/
					for (i = 0; i < nuecc_rec_count; i++) {
						CALL_ENG_DEBUG(ENG_DEBUG, "[ecc=%s, category:[%d]]", sim_usim_ecc_info.UeccInfo[i].szEcc, sim_usim_ecc_info.UeccInfo[i].EccEmergencyServiceInfo);
						if (!strcmp(pNumber, sim_usim_ecc_info.UeccInfo[i].szEcc)) {
							*ecc_category = sim_usim_ecc_info.UeccInfo[i].EccEmergencyServiceInfo;
							CALL_ENG_DEBUG(ENG_DEBUG, "uecc matched!!");
							return TRUE;
						}
					}

					CALL_ENG_DEBUG(ENG_DEBUG, "No match & check default emergency number...");
					return __vc_core_util_check_default_emergency_number(pNumber);
				}
			}
			break;

		case TAPI_SIM_CARD_TYPE_UNKNOWN:
		default:
			{
				return __vc_core_util_check_default_emergency_number(pNumber);
			}
			break;
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "No SIM!");
		for (i = 0; i < CALL_VC_NO_SIM_DEFAULT_EMERGENCY_NUMBERS_CNT; i++) {
			if (!strcmp(pNumber, gcall_vc_nosim_emergency_numbers[i]))
				return TRUE;
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "No emegency number...");
	return FALSE;
}

/**
 * This function checks whether the given number is SS string or not
 *
 * @return		TRUE if the number is SS string, FALSE otherwise
 * @param[in]		pNumber		number to be verified
 */
gboolean _vc_core_util_check_ss_string(const char *pNumber)
{
   int len;
   VOICECALL_RETURN_FALSE_IF_FAIL(pNumber != NULL);
   CALL_ENG_DEBUG(ENG_DEBUG, "pNumber(%s)",pNumber);

   len = strlen(pNumber);

   if(len > 3) {
	  if(pNumber[len-1] == '#') {
		 if(pNumber[0] == '*' || pNumber[0] == '#') {
			return TRUE;
		 }
	  }
	  else {
		 /*
		  * '*31#', '#31#' -> launch CISS
		  * '*31#nn..', '#31#nn...' -> launch Voice-call
		  */
		 if(strncmp (pNumber, CALL_COMMON_CLI_SHOW_ID, 4) == 0
			   || strncmp (pNumber, CALL_COMMON_CLI_HIDE_ID, 4) == 0) {
			if(len > 4)
			   return FALSE;
			return TRUE;
		 }
	  }
   }

   if((len == 2) && (ISUSSDDIGIT( pNumber[0] ) && ISDIGIT( pNumber[1] ))) {
	   CALL_ENG_DEBUG(ENG_DEBUG, "USSD string");
	   return TRUE;   
   }

	if((len == 1) && (IS1CHARUSSDDIGIT(pNumber[0]))) {
		CALL_ENG_DEBUG(ENG_DEBUG, "1 character USSD string");
		return TRUE;   
	}

   return FALSE;
}

/**
 * This function checks whether the given number is in CALL SS string or not
 *
 * @return		TRUE if the number is in call SS string, FALSE otherwise
 * @param[in]		pNumber		number to be verified
 */
gboolean _vc_core_util_check_incall_ss_string(const char *number)
{
   int len;
   int num_int;
   VOICECALL_RETURN_FALSE_IF_FAIL(number != NULL);
   CALL_ENG_DEBUG(ENG_DEBUG, "number(%s)",number);

   len = strlen(number);
   if(len > 2 || len < 1)
	  return FALSE;

   if(number[0] > '6')
	  return FALSE;

   if(len == 1) {
	  /* 0 ~ 4 */
	  if(number[0] >= '0' && number[0] <= '4')
		 return TRUE;
   }
   else {
	  /* 11 ~ 17, 21 ~ 27 */
	  num_int = atoi(number);

	  if(num_int >= 11 && num_int <= 17)
		 return TRUE;
	  if(num_int >= 21 && num_int <= 27)
		 return TRUE;
   }

   return FALSE;
}



/*PDIAL_SEND_DTMF*/
/**
* This function extracts the dtmf number from the given telephone number
*
* @param[in]	  tel_number		source telephone number
* @param[out] dtmf_number		extracted dtmf number
* @param[out] dtmf_buf_len		size of dtmf number buffer
*/
gboolean _vc_core_util_extract_dtmf_number(const char *tel_number, char *dtmf_number, const int dtmf_buf_len)
{
	char *pst;
	VOICECALL_RETURN_FALSE_IF_FAIL(tel_number!= NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(dtmf_number != NULL);

	memset(dtmf_number, 0, dtmf_buf_len);

	pst = strchr(tel_number, 'P');
	if (pst == NULL) {
		pst = strchr(tel_number, 'p');
	}

	if (pst == NULL) {
		pst = strchr(tel_number, ',');	/*Considering "," as Pause charcter - Browser Request */
	}

	if (pst == NULL) {
		return FALSE;
	} else {
		_vc_core_util_strcpy(dtmf_number, CALLVC_MIN((int)strlen(pst), (dtmf_buf_len - 1)),  pst + 1);
	}
	return TRUE;
}

/*PDIAL_SEND_DTMF*/

/**
* This function checks whether the given number ia a valid dtmf number or not
*
* @return		TRUE if the number can be used for sending dtmf , FALSE otherwise
* @param[in]		pnumber		number to be verified
*/
gboolean _vc_core_util_isvalid_dtmf_number(char *pnumber)
{
	int len = 0;
	VOICECALL_RETURN_FALSE_IF_FAIL(pnumber != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_util_isvalid_dtmf_number.. entering");

	if (NULL == pnumber || (len = strlen(pnumber)) <= 0) {
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Length = %d", len);
	while (len > 0) {
		/*PDIAL_SEND_DTMF*/
		if (IS_DIGIT(pnumber[len - 1]) || (pnumber[len - 1] >= ASCII_A && pnumber[len - 1] <= ASCII_D) || (pnumber[len - 1] == ASCII_STAR || pnumber[len - 1] == ASCII_HASH)) {
			return TRUE;
		}
		len--;
	}
	return FALSE;
}

/**
* This function checks whether the given number ia a valid dtmf number or not
*
* @return		TRUE if the number can be used for sending dtmf , FALSE otherwise
* @param[in]		pnumber		number to be verified
*/
gboolean _vc_core_util_isvalid_full_dtmf_number(char *pnumber)
{
	int len = 0;
	VOICECALL_RETURN_FALSE_IF_FAIL(pnumber != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_util_isvalid_dtmf_number.. entering");

	if (NULL == pnumber || (len = strlen(pnumber)) <= 0) {
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Length = %d", len);
	while (len > 0) {
		/*PDIAL_SEND_DTMF*/
		if (!(IS_DIGIT(pnumber[len - 1]) || (pnumber[len - 1] >= ASCII_A && pnumber[len - 1] <= ASCII_D) || (pnumber[len - 1] == ASCII_STAR || pnumber[len - 1] == ASCII_HASH) || (pnumber[len - 1] == ASCII_P || pnumber[len - 1] == ASCII_p || pnumber[len - 1] == ASCII_COMMA))) {
			CALL_ENG_DEBUG(ENG_DEBUG, "invalid character encountered...");
			return FALSE;
		}
		len--;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Fully valid DTMF string..");
	return TRUE;
}

/**
* This function checks the validity of the given telephone number
*
* @return		TRUE if the given number is valid telephone number, FALSE otherwise
* @param[in]		ptel_number	telephone number to be verified
*/
gboolean _vc_core_util_isvalid_telephone_number(char *ptel_number)
{
	int len = 0;
	int i = 0;
	VOICECALL_RETURN_FALSE_IF_FAIL(ptel_number != NULL);

	if (ptel_number != NULL) {
		char call_number[VC_PHONE_NUMBER_LENGTH_MAX] = { 0, };

		/*
		 * If user Input "*" or "#" and then make a call,
		 * the device should alert you that the phone number is incorrect.
		 */
		/* if ((strlen(ptel_number) == 1) && ((strcmp(ptel_number, "*") == 0) ||(strcmp(ptel_number, "#") == 0))) */
		/* 1 digit number -> wrong number popup */
		if (strlen(ptel_number) == 1) {
			CALL_ENG_DEBUG(ENG_DEBUG, "It is wrong number.(1 digit number)");
			return FALSE;
		}
		/*end*/

		/*To avoid checking *31# and #31# */
		if (_vc_core_util_extract_call_number(ptel_number, call_number, sizeof(call_number)) == FALSE) {
			CALL_ENG_DEBUG(ENG_DEBUG, "No proper number = %s", ptel_number);
			return FALSE;
		}

		len = strlen(call_number);
		for (i = 0; i < len; i++) {
			/*'+' should be allowed only as first digit of the dialling number */
			if (i >= 1 && call_number[i] == '+') {
				return FALSE;
			}

			if (!IS_DIGIT(call_number[i]) && call_number[i] != '+' && call_number[i] != ASCII_STAR && call_number[i] != ASCII_HASH && call_number[i] != 'P' && call_number[i] != 'p' && call_number[i] != ',') {
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

gboolean _vc_core_util_check_gcf_status(void)
{
	gboolean bgcf_status = FALSE;
	int ret = -1;

	ret = vconf_get_int(VCONFKEY_ADMIN_GCF_TEST, &bgcf_status);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "bgcf_status = [%d]", bgcf_status);
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
	}

	return bgcf_status;
}

gboolean _vc_core_util_check_zuhause_status(void)
{
	int isZuhauseArea = 0;
	int ret = -1;

	ret = vconf_get_int(VCONFKEY_TELEPHONY_ZONE_ZUHAUSE, &isZuhauseArea);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "isZuhauseArea = [%d]", isZuhauseArea);
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
	}

	if (isZuhauseArea == 0) {
		/*Not in a zuhause area..*/
		return FALSE;
	} else {
		/*in a zuhause area..*/
		return TRUE;
	}
}

/*********************************************************************/
gboolean _vc_core_util_set_call_status(int call_status)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "call_status = [%d]", call_status);

	if (vconf_set_int(VCONFKEY_CALL_STATE, call_status)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "vconf_set_int failed.");
		return FALSE;
	}
	return TRUE;
}

gboolean _vc_core_util_set_call_volume(int vol_level)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "vol_level = [%d]", vol_level);

	if (vconf_set_int(VCONFKEY_CALL_VOLUME_LEVEL, vol_level)) {
		CALL_ENG_DEBUG(ENG_WARN, "vconf_set_int failed.");
		return FALSE;
	}
	return TRUE;
}

int _vc_core_util_get_call_volume(void)
{
	int vol_level = 0;

	if (vconf_get_int(VCONFKEY_CALL_VOLUME_LEVEL, &vol_level)) {
		CALL_ENG_DEBUG(ENG_WARN, "vconf_set_int failed.");
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "vol_level(%d)",vol_level);
	return vol_level;
}

gboolean _vc_core_util_check_video_call_status(void)
{
	int call_status = 0;
	int ret = -1;

	ret = vconf_get_int(VCONFKEY_CALL_STATE, &call_status);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "call_status = [%d]", call_status);
		if (call_status == VCONFKEY_CALL_VIDEO_CONNECTING || call_status == VCONFKEY_CALL_VIDEO_ACTIVE) {
			return TRUE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
	}
	return FALSE;
}

gboolean _vc_core_util_get_SAP_status()
{
	int bt_status = VCONFKEY_BT_DEVICE_NONE;
	gboolean ret = FALSE;

	ret = vconf_get_int(VCONFKEY_BT_DEVICE, &bt_status);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "bt_status = [0x%x]", bt_status);
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
	}

	if (bt_status == VCONFKEY_BT_DEVICE_SAP_CONNECTED) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void _vc_core_util_get_cug_info(voicecall_setup_info_t *psetupcall_info)
{
	VOICECALL_RETURN_IF_FAIL(psetupcall_info != NULL);
	voicecall_cug_info_t *pcug_info = NULL;
	/*int error_code = -1;*/

	pcug_info = (voicecall_cug_info_t *) &psetupcall_info->cug_info;

	/*Get the CUG Information from the Settings */
	memset(&psetupcall_info->cug_info, 0, sizeof(voicecall_cug_info_t));
	pcug_info->bcug_used = 0;
}

void _vc_core_util_get_identity_mode(voicecall_setup_info_t *psetupcall_info)
{
	int id_mode = 0;	/*0 - default, 1-show, 2-hide*/
	int ret = -1;
	VOICECALL_RETURN_IF_FAIL(psetupcall_info != NULL);

	ret = vconf_get_int(VCONFKEY_CISSAPPL_SHOW_MY_NUMBER_INT, &id_mode);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "id_mode = [%d]", id_mode);
		psetupcall_info->identity_mode = id_mode;
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
		psetupcall_info->identity_mode = 0;
	}
}

gboolean _vc_core_util_is_offline_mode(void)
{
	gboolean bstatus = -1;

	if (vconf_get_bool(VCONFKEY_SETAPPL_FLIGHT_MODE_BOOL, &bstatus)) {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_bool failed.");
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "bstatus:[%d]", bstatus);

	return bstatus;
}

gboolean _vc_core_util_is_pwlock(void)
{
	int pwlock_state = -1;

	if (vconf_get_int(VCONFKEY_PWLOCK_STATE, &pwlock_state)) {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed.");
		return FALSE;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "pwlock_state:[%d]", pwlock_state);
	if ((pwlock_state == VCONFKEY_PWLOCK_BOOTING_LOCK) || (pwlock_state == VCONFKEY_PWLOCK_RUNNING_LOCK)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean _vc_core_util_get_nw_status(int *network_status)
{
	int svc_type = -1;
	gboolean ret = FALSE;
	VOICECALL_RETURN_FALSE_IF_FAIL(network_status != NULL);

	ret = vconf_get_int(VCONFKEY_TELEPHONY_SVCTYPE, &svc_type);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "svc_type = [%d]", svc_type);
		*network_status = svc_type;
		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
		return FALSE;
	}
}

/* Function Definitions */
/**
 * This function copies the source string to the target string according to the size of the destination string
 *
 * @return		Returns newly copied string on success or NULL on failure
 * @param[out]	pbuffer		Target Buffer
 * @param[in]		buf_count	Size of Target Buffer
 * @param[in]		pstring		Source String
 */
gboolean _vc_core_util_strcpy(char *pbuffer, int buf_count, const char *pstring)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pbuffer != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(pstring != NULL);

	if (buf_count == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "buf_count is zero!!");
		return FALSE;
	}

#ifdef _NO_USE_STRCPY_
	if ((buf_count - 1) >= (int)strlen(pstring)) {
		strcpy(pbuffer, pstring);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "pbuffer size is smaller than pstring!!");
		strncpy(pbuffer, pstring, (buf_count - 1));
		pbuffer[buf_count - 1] = '\0';
	}
#else
	CALL_ENG_DEBUG(ENG_DEBUG, "pbuffer size(%d), pstring size(%d)", buf_count, strlen(pstring));
	strncpy(pbuffer, pstring, (buf_count - 1));
	pbuffer[buf_count - 1] = '\0';
#endif
	return TRUE;
}

/**
 * This function make the date and time string
 *
 * @return		Returns newly copied string on success or NULL on failure
 * @param[out]	pbuffer		Target Buffer
 * @param[in]		time	time
 */
char *_vc_core_util_get_date_time(time_t time)
{
	int ret = 0;
	UDate date;
	const char *locale = uloc_getDefault();
	UErrorCode status = U_ZERO_ERROR;
	UDateTimePatternGenerator *generator;
	UDateFormat *formatter;
	UChar bestPattern[64] = { '\0', };
	UChar formatted[64] = { '\0', };
	char bestPatternString[128] = { '\0', };
	char formattedString[128] = { '\0', };
	int32_t bestPatternCapacity, formattedCapacity;
	int32_t bestPatternLength, formattedLength;
	char skeleton[128]={ '\0', };
	UChar customSkeleton[64] = { '\0', };
	enum appcore_time_format time_format;

	appcore_get_timeformat(&time_format);

  	switch(time_format) {
	case APPCORE_TIME_FORMAT_12:
		snprintf(skeleton, sizeof(skeleton), "%s%s", DATE_FORMAT_1, TIME_12_TYPE);
		break;
	case APPCORE_TIME_FORMAT_24:
		snprintf(skeleton, sizeof(skeleton), "%s%s", DATE_FORMAT_1, TIME_24_TYPE);
		break;
	case APPCORE_TIME_FORMAT_UNKNOWN:
	default:
		snprintf(skeleton, sizeof(skeleton), "%s%s", DATE_FORMAT_1, TIME_24_TYPE);
		break;
  	}

	u_uastrncpy(customSkeleton, skeleton,strlen(skeleton));

	generator = udatpg_open(locale, &status);
	bestPatternCapacity = (int32_t)(sizeof(bestPattern)/sizeof((bestPattern)[0]));

	bestPatternLength = udatpg_getBestPattern(generator, customSkeleton, u_strlen(customSkeleton), bestPattern,   bestPatternCapacity, &status);
	u_austrncpy(bestPatternString, bestPattern, 128);

	CALL_ENG_DEBUG(ENG_DEBUG, "BestPattern(%s)", bestPatternString);

	date = (UDate) time * 1000;

	formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, locale, NULL, -1, bestPattern, -1, &status);

	/* UDAT_IGNORE Passed so that our best pattern is used in generating pattern */
	formattedCapacity = (int32_t)(sizeof(formatted)/sizeof((formatted)[0]));
	formattedLength = udat_format(formatter, date, formatted, formattedCapacity, NULL, &status);
	u_austrncpy(formattedString, formatted, 128);
	udatpg_close(generator);
	udat_close(formatter);

	CALL_ENG_DEBUG(ENG_DEBUG, "DATE & TIME(%s)", formattedString);
	return g_strdup(formattedString);
}


gboolean _vc_core_util_set_sleep_status(call_vc_power_mode_t type)
{
	return FALSE;		/*it will be processed with sensor in ui*/
}

gboolean _vc_core_util_get_call_alert_type(int *alert_type)	/*VCONFKEY_SETAPPL_PROFILE_CURRENT_CALL_ALERT_TYPE_INT*/
{
	int tmp_alert_type = -1;
	gboolean ret = FALSE;
	char *vconf_key = NULL;
	VOICECALL_RETURN_FALSE_IF_FAIL(alert_type != NULL);

	vconf_key = vconf_get_str("db/setting/cur_profile");
	strncat(vconf_key, "/call_alert_type", strlen("/call_alert_type"));
	ret = vconf_get_int(vconf_key, &tmp_alert_type);
	CALL_ENG_DEBUG(ENG_DEBUG, "vconf_key = [%s]", vconf_key);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "alert_type = [%d]", tmp_alert_type);	/*SETTING_CALL_ALERT_TYPE_MELODY,...*/
		*alert_type = tmp_alert_type;
		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
		return FALSE;
	}
}

gboolean _vc_core_util_phonelock_status(void)
{
	gboolean b_phonelock = FALSE;
	if (!vconf_get_bool(VCONFKEY_SETAPPL_STATE_POWER_ON_LOCK_BOOL, &b_phonelock)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "b_phonelock:[%d]", b_phonelock);
		return b_phonelock;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "get VCONFKEY_SETAPPL_STATE_POWER_ON_LOCK_BOOL failed..");
		return FALSE;
	}
}

#ifdef TIMER_ENABLED
clock_t _vc_core_util_get_curr_time()
{
	struct timeval tv;
	long cur_time;
	gettimeofday(&tv, NULL);

	cur_time = (tv.tv_sec * 1000) + tv.tv_usec / 1000;
	return cur_time;
}

void _vc_core_util_print_diff_time(clock_t start, clock_t end, char *message)
{
	long elapsed_time = 0;
	elapsed_time = (end - start);
	VOICECALL_RETURN_IF_FAIL(message != NULL);
	fprintf(stderr, "\n[VoiceCall TIMER][%s]:\n[%s]\n\tstart time: %ld; end time: %ld; total time: %ld milli seconds\n", __func__, message, start, end, elapsed_time);
}

void _vc_core_util_print_curr_time(char *message)
{
	clock_t curr_time = _vc_core_util_get_curr_time();
	VOICECALL_RETURN_IF_FAIL(message != NULL);
	fprintf(stderr, "\n[VoiceCall TIMER][%s]:\n[%s]\n\t time: [%ld] milli seconds\n", __func__, message, curr_time);
}
#endif

void _vc_core_util_download_test_call(char *result)
{
	int ret;
	FILE *d_call_test = NULL;
	char string[20] = { };

	VOICECALL_RETURN_IF_FAIL(result != NULL);

	/*create string*/
	snprintf(string, sizeof(string), "/tmp/%s", result);

	d_call_test = fopen(string, "w");
	if (d_call_test == NULL) {
		CALL_ENG_DEBUG(ENG_DEBUG, "FAIL:fopen(%s)", string);
		return;
	}
	/*ret = fwrite("\0", sizeof(char), 1, d_call_test);*/
	/*CALL_ENG_DEBUG(ENG_DEBUG,"Result:fwrite(%d)", ret);*/

	ret = fclose(d_call_test);
	CALL_ENG_DEBUG(ENG_DEBUG, "Result:fwrite(%d)", ret);
	if (ret != 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "FAIL:fclose\n");
		return;
	}

}
