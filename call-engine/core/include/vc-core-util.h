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


#ifndef __VC_CORE_UTIL_H_
#define __VC_CORE_UTIL_H_

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "vc-core-error.h"
#include "vc-core-engine-types.h"

#include "TelDefines.h"
#include "TelNetwork.h"
#include "TelSim.h"
#include "TapiCommon.h"
#include "TapiEvent.h"
#include "ITapiCall.h"
#include "ITapiSim.h"
#include "ITapiNetwork.h"
#include "ITapiSound.h"
#include "ITapiSs.h"
#include "ITapiSat.h"

#include "vconf.h"
#include "vconf-keys.h"
#include "contacts-svc.h"
#ifdef CALL_DEBUG_ON_DLOG
#include <dlog.h>
#endif

#define CALL_NETWORK_MCC_UK	0xEA	/*234*/
#define CALL_NETWORK_MCC_IE	0x110	/*272*/
#define CALL_NETWORK_MCC_UKRAINE	0xFF	/*255*/
#define CALL_NETWORK_MCC_SAMSUNG_SUWON_3G	450001	/*450001*/
#define CALL_NETWORK_MCC_ITALY	222

#define IS_DIGIT(value)		((value) >= '0' && (value) <= '9')

#ifdef CALL_DEBUG_ON_DLOG
#define ENG_ERR			LOG_ERROR
#define ENG_WARN		LOG_WARN
#define ENG_DEBUG		LOG_DEBUG
#else
#define ENG_ERR
#define ENG_WARN
#define ENG_DEBUG
#endif

#ifdef CALL_DEBUG_ON_DLOG
#define CALL_ENG_DEBUG(level, frmt, args...)       \
	do {LOG(level, TAG_CALL, "[VC_ENGINE] [%s:%d] "frmt"\n",  __func__, __LINE__, ##args); } while (0)
#define CALL_ENG_KPI(frmt, args...)       \
	do {LOG(LOG_DEBUG, TAG_CALL_LAUNCH, "[VC_KPI] [%s:%d] "frmt"\n",  __func__, __LINE__, ##args); } while (0)
#else
#define CALL_ENG_DEBUG(level, fmt, args...)
#endif

#define VOICECALL_RETURN_IF_FAIL(check_condition)	\
			if (!(check_condition)) return;

#define VOICECALL_RETURN_FALSE_IF_FAIL(check_condition)	\
			if (!(check_condition)) return FALSE;

#define VOICECALL_RETURN_VALUE_IF_FAIL(check_condition, value)	\
			if (!(check_condition)) return value;

#define VOICECALL_RETURN_NULL_IF_FAIL(check_condition)	\
			if (!(check_condition)) return NULL;

#define VOICECALL_RETURN_ZERO_IF_FAIL(check_condition)	\
			if (!(check_condition)) return 0;

#define VOICECALL_RETURN_INVALID_IF_FAIL(check_condition)	\
			if (!(check_condition)) return -1;

#define VOICECALL_RETURN_VALUE_IF_NOT_IN_RANGE(value, min_value, max_value, ret_val)	\
			if ((value < min_value) || (value > max_value)) return ret_val;

#ifndef min
#define min(a, b)    (((a) < (b)) ? (a) : (b))
#endif				/*#ifndef min*/

typedef unsigned int call_vc_handle;

#ifdef TIMER_ENABLED
#define	GET_CURR_TIME()	_vc_core_util_get_curr_time()
#define	PRINT_DIFF_TIME(start, end, message)	_vc_core_util_print_diff_time(start, end, message)
#define	PRINT_CURRENT_TIME(message) _vc_core_util_print_curr_time(message)

#else
#define	GET_CURR_TIME()	0
#define		PRINT_DIFF_TIME(start, end, message)
#define	PRINT_CURRENT_TIME(message)
#endif

/**
 * This enumeration defines SS's SI vaild types
 */
typedef enum {
	SS_SI_FORMAT_INVALID = -1,				/**< SI format is invalid */
	SS_SI_FORMAT_VALID						/**< SI format is valid */
} call_vc_ss_si_format;

/**
 * This enumeration defines the publish types
 */
typedef enum {
	CALL_VC_SAT_RESPONSE						/**< SAT response publish type */
} call_vc_publish_type_t;

/**
 * This enum defines the type of power mode.
 */
typedef enum {
	CALL_VC_POWER_NONE = 0,
	CALL_VC_POWER_GRANT_DIMMING,
	CALL_VC_POWER_GRANT_LCDOFF,
	CALL_VC_POWER_GRANT_SLEEP,
	CALL_VC_POWER_PROHIBIT_DIMMING,
	CALL_VC_POWER_PROHIBIT_LCDOFF,
	CALL_VC_POWER_PROHIBIT_SLEEP,
	CALL_VC_POWER_NORMAL_STATUS
} call_vc_power_mode_t;

/**
 * This function publishes the event of a given publish type
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		src		Telehone number from which call number needd to be parsed
 * @param[out]		dst		Pointer to the buffer to store the parsed number
 */
gboolean _vc_core_util_remove_invalid_chars(const char *src, char *dst);

/**
 * This function publishes the event of a given publish type
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]		pszTelNumber		Telehone number from which call number nned to be extracted
 * @param[out]	pBuffer			Pointer to the buffer to store the extracted number
 * @param[in]		nBufSize			Size of input buffer
 */
gboolean _vc_core_util_extract_call_number(const char *pszTelNumber, char *pBuffer, const int nBufSize);
gboolean _vc_core_util_extract_call_number_without_cli(const char *pszTelNumber, char *pBuffer, const int nBufSize);

#ifdef TIMER_CHECK	/*unused*/
void call_vc_print_elapsed_time(char *string, gulong * tim_out);
void call_vc_print_diff_time(time_t time1, time_t time2);
#endif

/**
 * This function checks whether the given number is emergency number by verifying with sim emergency numbers
 *
 * @return		TRUE if the number is emergency number, FALSE otherwise
 * @param[in]		card_type	simcard type
 * @param[in]		pNumber		number to be verified
 */
gboolean _vc_core_util_check_emergency_number(TelSimCardType_t card_type, char *pNumber, gboolean b_is_no_sim, int *ecc_category);

/**
 * This function checks whether the given number is SS string or not
 *
 * @return		TRUE if the number is SS string, FALSE otherwise
 * @param[in]		pNumber		number to be verified
 */
gboolean _vc_core_util_check_ss_string(const char *pNumber);

/**
 * This function checks whether the given number is SS string or not
 *
 * @return		TRUE if the number is in call SS string, FALSE otherwise
 * @param[in]		pNumber		number to be verified
 */
gboolean _vc_core_util_check_incall_ss_string(const char *number);

/*PDIAL_SEND_DTMF*/
/**
* This function extracts the dtmf number from the given telephone number
*
* @param[in]	  tel_number		source telephone number
* @param[out] dtmf_number		extracted dtmf number
* @param[out] dtmf_buf_len		size of dtmf number buffer
*/
gboolean _vc_core_util_extract_dtmf_number(const char *tel_number, char *dtmf_number, const int dtmf_buf_len);
/*PDIAL_SEND_DTMF*/

/**
 * This function checks whether the given number ia a valid dtmf number or not
 *
 * @return		TRUE if the number can be used for sending dtmf , FALSE otherwise
 * @param[in]		pnumber		number to be verified
 */
gboolean _vc_core_util_isvalid_dtmf_number(char *pnumber);

/**
* This function checks whether the entire dtmf string is valid or not
*
* @return		TRUE if the number can be used for sending dtmf , FALSE otherwise
* @param[in]		pnumber		number to be verified
*/
gboolean _vc_core_util_isvalid_full_dtmf_number(char *pnumber);

/**
* This function checks the validity of the given string for SS string
*
* @return		Returns SS_SI_FORMAT_INVALID - if the string is invalid ss string, SS_SI_FORMAT_VALID - if valid
* @param[in]	pinput_string	string to be verified
*/
call_vc_ss_si_format _vc_core_util_check_si_format(const char *pinput_string);

/**
* This function checks the validity of the given telephone number
*
* @return		TRUE if the given number is valid telephone number, FALSE otherwise
* @param[in]		ptel_number	telephone number to be verified
*/
gboolean _vc_core_util_isvalid_telephone_number(char *ptel_number);

/**
* This function checks the if it is GCF mode or not
*
* @return		TRUE if it is GCF mode, FALSE otherwise
*/
gboolean _vc_core_util_check_gcf_status(void);

/**
* This function checks the if it is in zuhause area or not
*
* @return		TRUE if it is in zuhause area, FALSE otherwise
*/
gboolean _vc_core_util_check_zuhause_status(void);

/**
* This function retreive mcc information from the telephony
*
* @internal
* @return		TRUE on success, FALSE - otherwise
* @param[out]		mcc	mcc information to be retreived
*/
gboolean _vc_core_util_get_mcc(unsigned long *mcc);

gboolean _vc_core_util_set_call_status(int call_status);
gboolean _vc_core_util_check_video_call_status(void);
gboolean _vc_core_util_get_SAP_status();
void _vc_core_util_get_cug_info(voicecall_setup_info_t *psetupcall_info);
void _vc_core_util_get_identity_mode(voicecall_setup_info_t *psetupcall_info);
gboolean _vc_core_util_is_offline_mode(void);
gboolean _vc_core_util_is_pwlock(void);
gboolean _vc_core_util_get_nw_status(int *network_status);
gboolean _vc_core_util_strcpy(char *pbuffer, int buf_count, const char *pstring);

/**
 * This function make the date and time string
 *
 * @return		Returns newly copied string on success or NULL on failure
 * @param[out]	pbuffer		Target Buffer
 * @param[in]	time	time
 */
char *_vc_core_util_get_date_time(time_t time);

/**
 * This function check phone lock status
 *
 * @return		Returns whether phone is locked or not.
 * @param[in]	void
 */
gboolean _vc_core_util_phonelock_status(void);

gboolean _vc_core_util_set_sleep_status(call_vc_power_mode_t type);
gboolean _vc_core_util_get_call_alert_type(int *alert_type);

#ifdef TIMER_ENABLED	/*unused*/
clock_t _vc_core_util_get_curr_time();
void _vc_core_util_print_diff_time(clock_t start, clock_t end, char *message);
void _vc_core_util_print_curr_time(char *message);
#endif

void _vc_core_util_download_test_call(char *result);

gboolean _vc_core_util_set_call_volume(int vol_level);
int _vc_core_util_get_call_volume(void);
#endif				/* __VC_CORE_UTIL_H_ */
