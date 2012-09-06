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
#define ASCII_P		80
#define	ASCII_p		112
#define	ASCII_COMMA	44
#define ASCII_W				87
#define	ASCII_w				119
#define	ASCII_SEMI_COLON	59

#define EIGTH_BIT	0x80
#define SEVENTH_BIT 0x40
#define SIXTH_BIT	0x20
#define FIFTH_BIT	0x10
#define FOURTH_BIT	0x08
#define THIRD_BIT	0x04
#define SECOND_BIT	0x02
#define FIRST_BIT	0x01

#define CALLVC_MIN(x, y) ((x) <= (y) ? (x) : (y))
#define ISUSSDDIGIT(X) ((X >= '2') && (X <= '9'))
#define ISDIGIT(X) ((X >= '0') && (X <= '9'))
#define ISSSDIGITINCALL(X) ((X >= '7') && (X <= '9'))
#define IS1CHARUSSDDIGIT(X) ((X >= '1') && (X <= '9'))

#define DATE_FORMAT_1 "EEMMMddyyyy"	/* Thu Aug 23 2001 */
#define TIME_12_TYPE "hma"	/* 7:58 AM */
#define TIME_24_TYPE "HHmm"	/* 19:58 */

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

	CALL_ENG_DEBUG(ENG_DEBUG, "Source phone number - %s", src);
	int i = 0;
	int j = 0;
	int nSrc = 0;

	nSrc = strlen(src);
	CALL_ENG_DEBUG(ENG_DEBUG, "source String len - %d", nSrc);

	for (i = 0; i < nSrc; ++i) {
		switch (src[i]) {
		case '(':
			{
				if (src[i + 1] == '0')
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
		if (j > 40) {
			break;
		}
	}

	dst[j] = '\0';
	CALL_ENG_DEBUG(ENG_DEBUG, "Destination phone number - %s", dst);

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

	pst = pszNum;
	while (*pst != '\0') {
		if (*pst == 'P' || *pst == 'p' || *pst == ',' || *pst == 'W' || *pst == 'w' || *pst == ';') {
			break;
		}
		pst++;
	}

	if (strlen(pst) == 0) {
		if (strlen(pszNum) == 0)
			return FALSE;
		_vc_core_util_strcpy(pBuffer, nBufSize, pszNum);
	} else {
		if (pst == pszNum)
			return FALSE;

		_vc_core_util_strcpy(pBuffer, min((((int)pst - (int)pszNum) + 1), (nBufSize)), pszNum);
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
	pst = pszNum;
	while (*pst != '\0') {
		if (*pst == 'P' || *pst == 'p' || *pst == ',' || *pst == 'W' || *pst == 'w' || *pst == ';') {
			break;
		}
		pst++;
	}
	if (strlen(pst) == 0) {
		if (strlen(pszNum) == 0)
			return FALSE;
		_vc_core_util_strcpy(pBuffer, nBufSize, pszNum);
	} else {
		if (pst == pszNum)
			return FALSE;

		_vc_core_util_strcpy(pBuffer, min((((int)pst - (int)pszNum) + 1), (nBufSize)), pszNum);
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
	VOICECALL_RETURN_VALUE_IF_FAIL((pinput_string != NULL), SS_SI_FORMAT_INVALID);

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

gboolean _vc_core_util_get_mnc(unsigned long *mnc)
{
	int ret = 0;
	int plmn_value = 0;

	VOICECALL_RETURN_FALSE_IF_FAIL((mnc != NULL));

	ret = vconf_get_int(VCONFKEY_TELEPHONY_PLMN, &plmn_value);
	if (0 == ret) {
		char mnc_value[4];
		char plmn_string[10];

		CALL_ENG_DEBUG(ENG_DEBUG, "plmn_value = [%d]", plmn_value);

		memset(plmn_string, 0, sizeof(plmn_string));
		memset(mnc_value, 0, sizeof(mnc_value));

		snprintf(plmn_string, 10, "%d", plmn_value);

		/* 4~6th digits of plmn value constitutes the mnc value */
		_vc_core_util_strcpy(mnc_value, 4, &plmn_string[3]);
		*mnc = (unsigned long)atoi(mnc_value);
	} else {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_get_int failed..[%d]", ret);
		*mnc = CALL_NETWORK_MNC_01;
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "mnc = %ld", *mnc);

	return TRUE;
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
	CALL_ENG_DEBUG(ENG_DEBUG, "pNumber(%s)", pNumber);

	len = strlen(pNumber);

	if (len > 3) {
		if (pNumber[len - 1] == '#') {
			if (pNumber[0] == '*' || pNumber[0] == '#') {
				return TRUE;
			}
		} else {
			/*
			 * '*31#', '#31#' -> launch CISS
			 * '*31#nn..', '#31#nn...' -> launch Voice-call
			 */
			if (strncmp(pNumber, CALL_COMMON_CLI_SHOW_ID, 4) == 0 || strncmp(pNumber, CALL_COMMON_CLI_HIDE_ID, 4) == 0) {
				if (len > 4)
					return FALSE;
				return TRUE;
			}
		}
	}

	if ((len == 2) && (ISUSSDDIGIT(pNumber[0]) && ISDIGIT(pNumber[1]))) {
		CALL_ENG_DEBUG(ENG_DEBUG, "USSD string");
		return TRUE;
	}

	if ((len == 1) && (IS1CHARUSSDDIGIT(pNumber[0]))) {
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
	CALL_ENG_DEBUG(ENG_DEBUG, "number(%s)", number);

	len = strlen(number);
	if (len > 2 || len < 1)
		return FALSE;

	if (number[0] > '6')
		return FALSE;

	if (len == 1) {
		/* 0 ~ 4 */
		if (number[0] >= '0' && number[0] <= '4')
			return TRUE;
	} else {
		/* 11 ~ 17, 21 ~ 27 */
		num_int = atoi(number);

		if (num_int >= 11 && num_int <= 17)
			return TRUE;
		if (num_int >= 21 && num_int <= 27)
			return TRUE;
	}

	return FALSE;
}

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
	VOICECALL_RETURN_FALSE_IF_FAIL(tel_number != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(dtmf_number != NULL);

	memset(dtmf_number, 0, dtmf_buf_len);

	pst = (char *)tel_number;
	while (*pst != '\0') {
#ifdef	WDIAL_SEND_DTMF
		if (*pst == 'P' || *pst == 'p' || *pst == ',' || *pst == 'W' || *pst == 'w' || *pst == ';') {
#else
		if (*pst == 'P' || *pst == 'p' || *pst == ',') {
#endif
			break;
		}
		pst++;
	}

	if (strlen(pst) == 0) {
		return FALSE;
	} else {
		if (pst == tel_number)
			return FALSE;
		strncpy(dtmf_number, pst, CALLVC_MIN((int)strlen(pst), (dtmf_buf_len-1)));
	}
	return TRUE;
}

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
		if (!(IS_DIGIT(pnumber[len - 1]) || (pnumber[len - 1] >= ASCII_A && pnumber[len - 1] <= ASCII_D) || \
				(pnumber[len - 1] == ASCII_STAR || pnumber[len - 1] == ASCII_HASH) || \
				(pnumber[len - 1] == ASCII_P || pnumber[len - 1] == ASCII_p || pnumber[len - 1] == ASCII_COMMA) || \
				(pnumber[len - 1] == ASCII_W || pnumber[len - 1] == ASCII_w || pnumber[len - 1] == ASCII_SEMI_COLON))) {
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

		/* Function Test SLP-Call-0250 fixed.
		 * If user Input "*" or "#" and then make a call,
		 * the device should alert you that the phone number is incorrect.
		 */
		/*if ((strlen(ptel_number) == 1) && ((strcmp(ptel_number, "*") == 0) ||(strcmp(ptel_number, "#") == 0))) */
		/* 1 digit number -> wrong number popup */
		if (strlen(ptel_number) == 1) {
			CALL_ENG_DEBUG(ENG_DEBUG, "It is wrong number.(1 digit number)");
			return FALSE;
		}
		/*end */

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
		/*Not in a zuhause area.. */
		return FALSE;
	} else {
		/*in a zuhause area.. */
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

	CALL_ENG_DEBUG(ENG_DEBUG, "vol_level(%d)", vol_level);
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
	/*int error_code = -1; */

	pcug_info = (voicecall_cug_info_t *) &psetupcall_info->cug_info;

	/*Get the CUG Information from the Settings */
	memset(&psetupcall_info->cug_info, 0, sizeof(voicecall_cug_info_t));
	pcug_info->bcug_used = 0;
}

void _vc_core_util_get_identity_mode(voicecall_setup_info_t *psetupcall_info)
{
	int id_mode = 0;	/*0 - default, 1-show, 2-hide */
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
	char skeleton[128] = { '\0', };
	UChar customSkeleton[64] = { '\0', };
	enum appcore_time_format time_format;

	appcore_get_timeformat(&time_format);

	switch (time_format) {
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

	u_uastrncpy(customSkeleton, skeleton, strlen(skeleton));

	generator = udatpg_open(locale, &status);
	bestPatternCapacity = (int32_t) (sizeof(bestPattern) / sizeof((bestPattern)[0]));

	bestPatternLength = udatpg_getBestPattern(generator, customSkeleton, u_strlen(customSkeleton), bestPattern, bestPatternCapacity, &status);
	u_austrncpy(bestPatternString, bestPattern, 128);

	CALL_ENG_DEBUG(ENG_DEBUG, "BestPattern(%s)", bestPatternString);

	date = (UDate) time * 1000;

	formatter = udat_open(UDAT_IGNORE, UDAT_IGNORE, locale, NULL, -1, bestPattern, -1, &status);

	/* UDAT_IGNORE Passed so that our best pattern is used in generating pattern */
	formattedCapacity = (int32_t) (sizeof(formatted) / sizeof((formatted)[0]));
	formattedLength = udat_format(formatter, date, formatted, formattedCapacity, NULL, &status);
	u_austrncpy(formattedString, formatted, 128);
	udatpg_close(generator);
	udat_close(formatter);

	CALL_ENG_DEBUG(ENG_DEBUG, "DATE & TIME(%s)", formattedString);
	return g_strdup(formattedString);
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

void _vc_core_util_download_test_call(char *result)
{
	int ret;
	FILE *d_call_test = NULL;
	char string[20] = { };

	VOICECALL_RETURN_IF_FAIL(result != NULL);

	/*create string */
	snprintf(string, sizeof(string), "/tmp/%s", result);

	d_call_test = fopen(string, "w");
	if (d_call_test == NULL) {
		CALL_ENG_DEBUG(ENG_DEBUG, "FAIL:fopen(%s)", string);
		return;
	}
	/*ret = fwrite("\0", sizeof(char), 1, d_call_test); */
	/*CALL_ENG_DEBUG(ENG_DEBUG,"Result:fwrite(%d)", ret); */

	ret = fclose(d_call_test);
	CALL_ENG_DEBUG(ENG_DEBUG, "Result:fwrite(%d)", ret);
	if (ret != 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "FAIL:fclose");
		return;
	}

}

