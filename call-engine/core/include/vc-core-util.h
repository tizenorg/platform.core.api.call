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

#ifdef DRM_USED
#include "drm-service-types.h"
#include "drm-service.h"
#endif

#include <tapi_common.h>
#include <TapiUtility.h>
#include <ITapiCall.h>
#include <ITapiSat.h>
#include <ITapiSim.h>

#include "vconf.h"
#include "vconf-keys.h"
#include "contacts-svc.h"
#ifdef CALL_DEBUG_ON_DLOG
#include <dlog.h>
#endif

/* Mcc */
#define CALL_NETWORK_MCC_UK				234		/*UK = 234*/
#define CALL_NETWORK_MCC_IRELAND		272		/*Ireland = 272*/
#define CALL_NETWORK_MCC_UAE			424		/*UAE = 424*/
#define CALL_NETWORK_MCC_GHANA			620		/*Ghana = 620*/
#define CALL_NETWORK_MCC_ISRAEL			425		/*Israel = 425*/
#define CALL_NETWORK_MCC_CROATIA		219		/*Croatia = 219*/
#define CALL_NETWORK_MCC_SERBIA			220		/*Serbia = 220*/
#define CALL_NETWORK_MCC_RUSSIA			250		/*Russia = 250*/

#define CALL_NETWORK_MCC_TAIWAN			466		/*Taiwan = 466*/
#define CALL_NETWORK_MCC_HONGKONG		454		/*Hongkong = 454*/
#define CALL_NETWORK_MCC_MALAYSIA       502		/*MALAYSIA = 502*/
#define CALL_NETWORK_MCC_AUSTRALIA		505		/*Australia = 505*/
#define CALL_NETWORK_MCC_NEWZEALAND		530		/*NewZealand = 530*/

#define CALL_NETWORK_MCC_USA			310		/*USA = 310*/
#define CALL_NETWORK_MCC_CANADA			302		/*Canada = 302*/
#define CALL_NETWORK_MCC_BRASIL			724		/*Brasil = 724*/
#define CALL_NETWORK_MCC_MEXICO			334		/*Mexico = 334*/
#define CALL_NETWORK_MCC_URGUAY			748		/*Urguay = 748*/
#define CALL_NETWORK_MCC_COLOMBIA		732		/*Colombia = 732*/
#define CALL_NETWORK_MCC_CHILE			730		/*Chile = 730*/
#define CALL_NETWORK_MCC_PERU			716		/*Peru = 716*/
#define CALL_NETWORK_MCC_VENEZUELA		734		/*Venezuela = 734*/
#define CALL_NETWORK_MCC_GUATEMALA		704		/*Guatemala = 704*/
#define CALL_NETWORK_MCC_ELSALVADOR		706		/*El Salvador = 706*/
#define CALL_NETWORK_MCC_NICARAGUA		710		/*Nicaragua = 710*/
#define CALL_NETWORK_MCC_PANAMA			714		/*Panama = 714*/

#define CALL_NETWORK_MCC_JAPAN			440		/*Japan = 440*/
#define CALL_NETWORK_MCC_KOREA			450		/*Korae = 450*/
#define CALL_NETWORK_MCC_CHINA			460		/*China = 460*/

#define CALL_NETWORK_MCC_FRANCE			208		/*France = 208*/
#define CALL_NETWORK_MCC_PORTUGAL		268		/*Protugal = 268*/
#define CALL_NETWORK_MCC_ROMANIA		226		/*Romania = 226*/
#define CALL_NETWORK_MCC_DE				262		/*DE = 262(0x106)*/

#define CALL_NETWORK_MCC_TEST_USA		0x001	/*Test MMC*/

/* MNC */
#define CALL_NETWORK_MNC_01			1		/*MNC 01*/
#define CALL_NETWORK_MNC_02			2		/*MNC 02*/
#define CALL_NETWORK_MNC_03			3		/*MNC 03*/
#define CALL_NETWORK_MNC_04			4		/*MNC 04*/
#define CALL_NETWORK_MNC_05			5		/*MNC 05*/
#define CALL_NETWORK_MNC_06			6		/*MNC 06*/
#define CALL_NETWORK_MNC_11			11		/*MNC 11	for +Movil in Panama*/
#define CALL_NETWORK_MNC_20			20		/*MNC 20	for TELCEL in Mexico*/
#define CALL_NETWORK_MNC_123		123		/*MNC 123	for Movistar in Colombia*/
#define CALL_NETWORK_MNC_103		103		/*MNC 103	for TIGO in Colombia*/
#define CALL_NETWORK_MNC_111		111		/*MNC 111	for TIGO in Colombia*/
#define CALL_NETWORK_MNC_30			30		/*MNC 30	for Movistar in Nicaragua*/
#define CALL_NETWORK_MNC_300		300		/*MNC 30	for Movistar in Nicaragua*/
#define CALL_NETWORK_MNC_TEST_USA	0x01	/*Test MNC*/

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

/**
* This function extracts the dtmf number from the given telephone number
*
* @param[in]	  tel_number		source telephone number
* @param[out] dtmf_number		extracted dtmf number
* @param[out] dtmf_buf_len		size of dtmf number buffer
*/
gboolean _vc_core_util_extract_dtmf_number(const char *tel_number, char *dtmf_number, const int dtmf_buf_len);

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

/**
* This function retreive mnc information from the telephony
*
* @internal
* @return		TRUE on success, FALSE - otherwise
* @param[out]		mnc	mnc information to be retreived
*/
gboolean _vc_core_util_get_mnc(unsigned long *mnc);


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

void _vc_core_util_download_test_call(char *result);

gboolean _vc_core_util_set_call_volume(int vol_level);
int _vc_core_util_get_call_volume(void);
#endif				/* __VC_CORE_UTIL_H_ */
