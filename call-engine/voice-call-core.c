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


#include <msg.h>

#include "vc-core-util.h"
#include "vc-core-engine-types.h"
#include "vc-core-callagent.h"
#include "vc-core-engine.h"
#include "vc-core-error.h"
#include "voice-call-core.h"
#include "voice-call-dbus.h"
#include "voice-call-engine-msg.h"
#include "voice-call-service.h"
#include "voice-call-device.h"
#include "voice-call-sound.h"

#include "voice-call-bt.h"
#include "vc-core-engine.h"

#include "voice-call-engine.h"

#define MO_REDIAL_COUNT_MAX 10
#define MO_REDIAL_TIMER_INTERVAL_FIRST	3000	/**< MO Redial Timer Interva; - 5 Secs*/
#define MO_REDIAL_TIMER_INTERVAL_SECOND		3000	/**< MO Redial Timer Interva; - 1 Minute*/
#define MO_REDIAL_TIMER_INTERVAL_THIRD	3000	/**< MO Redial Timer Interva; - 3 Minutes*/

#define MINUTE_MINDER_TIMEOUT_VALUE	60000

#define DTMF_PAUSE_TIMER_INTERVAL_FIRST		3000	/*3 Seconds */
#define DTMF_PAUSE_TIMER_INTERVAL_REST			3000	/*3 Seconds */
#define DTMF_PAUSE_TIMER_INTERVAL_GCF_FIRST	800	/*0.8 Second */
#define	DTMF_PAUSE_TIMER_INTERVAL_GCF_REST		3300	/*3 Seconds */

gboolean mo_redial_timer_cb(void *data);

#define SET_PATH_TIMER_VALUE	50
static guint g_set_path_timer_handler = 0;
static gboolean __voicecall_core_set_path_timer_cb(gpointer puser_data);

/* For Debug Information, Call Engine Event name string constant */
char *gszcall_engine_event[VC_ENGINE_EVENT_MAX] = {
	"VC_CALL_INCOM",
	"VC_CALL_OUTGOING",
	"VC_CALL_OUTGOING_ORIG",
	"VC_CALL_OUTGOING_ALERT",
	"VC_CALL_CONNECTED",
	"VC_CALL_NORMAL_END",
	"VC_CALL_INCOM_END",
	"VC_CALL_INCOM_DROPPED",
	"VC_CALL_REJECTED_END",
	"VC_CALL_OUTGOING_END",	/* 10 */

	"VC_CALL_OUTGOING_ABORTED",
	"VC_CALL_DTMF_ACK",
	"VC_CALL_AUTO_REDIAL_CHECK",
	"VC_CALL_ANSWER_CNF",
	"VC_CALL_SS_HELD",
	"VC_CALL_SS_RETREIVED",
	"VC_CALL_SS_SWAP",
	"VC_CALL_SS_SETUP_CONF",
	"VC_CALL_SS_SPLIT_CONF",
	"VC_CALL_SS_TRANSFERRED",	/* 20 */

	"VC_CALL_SS_CONNECT_LINE_IND",
	"VC_CALL_IND_FORWARD",
	"VC_CALL_IND_ACTIVATE",
	"VC_CALL_IND_HOLD",
	"VC_CALL_IND_TRANSFER",
	"VC_CALL_IND_SETUPCONFERENCE",
	"VC_CALL_IND_BARRING",
	"VC_CALL_IND_WAITING",
	"VC_CALL_IND_CUGINFO",
	"VC_CALL_IND_SSNOTIFY",	/* 30 */

	"VC_CALL_IND_CALLINGNAMEINFO",
	"VC_CALL_IND_REDIRECT_CNF",
	"VC_CALL_IND_ACTIVATECCBS_CNF",
	"VC_CALL_IND_ACTIVATECCBS_USERINFO",
	"VC_CALL_IND_AOC",
	"VC_ERROR_OCCURED",
	"VC_ACTION_INCOM_FORCE",
	"VC_ACTION_SAT_REQUEST",
	"VC_ACTION_SAT_RESPONSE",
	"VC_ACTION_CALL_END_HELD_RETREIVED",	/* 40 */

	"VC_ACTION_NO_ACTIVE_TASK",
	"VC_CALL_GET_VOLUME_RESP"
};

static gboolean __vc_core_is_answermode_enabled_from_testmode(void);
static gboolean __vc_core_is_answermode_enabled(void);
static void __voicecall_core_start_auto_answer(call_vc_core_state_t *pcall_core, gboolean isTestMode);
static void __voicecall_core_cancel_auto_answer(call_vc_core_state_t *pcall_core);
/*static void __voicecall_core_check_headset_earjack_status(call_vc_core_state_t *pcall_core);*/
static void __vc_core_set_auto_redial_count(call_vc_core_state_t *pcall_core, int auto_redial_count);
/*static gboolean __voicecall_core_callstatus_set_timer_cb(gpointer puser_data);*/

static int __voicecall_core_get_string_id_by_errorcode(int error_code);
static void __voicecall_core_mocall_reset_engine_state(voicecall_engine_t *pcall_engine);
static gboolean __voicecall_core_is_redial_cuase(int end_cause);

static gboolean __voicecall_core_queue_dtmf_string(call_vc_core_state_t *pcall_core, char *dtmf_string, gboolean bsat_dtmf);
static gboolean __voicecall_core_handle_dtmf_ack(call_vc_core_state_t *pcall_core, gboolean success);

/**
 * This function puts the currently active call on hold
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_hold_call(voicecall_engine_t *pcall_engine)
{

	voicecall_error_t error_code = ERROR_VOICECALL_NONE;

	error_code = _vc_core_engine_hold_call(pcall_engine);

	CALL_ENG_DEBUG(ENG_DEBUG, "Error Code : %d", error_code);
	return (ERROR_VOICECALL_NONE == error_code || ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS == error_code) ? TRUE : FALSE;
}

/**
 * This function retreives the currently held call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_retrieve_call(voicecall_engine_t *pcall_engine)
{
	voicecall_error_t error_code = ERROR_VOICECALL_NONE;

	error_code = _vc_core_engine_retrieve_call(pcall_engine);

	CALL_ENG_DEBUG(ENG_DEBUG, "Error Code : %d", error_code);
	return (ERROR_VOICECALL_NONE == error_code || ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS == error_code) ? TRUE : FALSE;
}

/**
 * This function swaps the currently available active and held calls
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_swap_calls(voicecall_engine_t *pcall_engine)
{
	voicecall_error_t error_code = ERROR_VOICECALL_NONE;

	error_code = _vc_core_engine_swap_calls(pcall_engine);

	CALL_ENG_DEBUG(ENG_DEBUG, "Error Code : %d", error_code);
	return (ERROR_VOICECALL_NONE == error_code || ERROR_VOICECALL_PREVIOUS_REQUEST_IN_PROGRESS == error_code) ? TRUE : FALSE;
}

/**
 * This function clears the MO Call Details
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_clear_mo_call(voicecall_engine_t *pcall_engine)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_finalize_call(pcall_engine, VC_OUTGOING_CALL, -1)) ? TRUE : FALSE;
}

/**
 * This function clears the Connected Call Details
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		call_handle		Handle of the Connected Call to be cleared
 */
inline gboolean voicecall_core_clear_connected_call(voicecall_engine_t *pcall_engine, int call_handle)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_finalize_call(pcall_engine, VC_CONNECTED_CALL, call_handle)) ? TRUE : FALSE;
}

/**
 * This function changes the voicecall engine's state
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		eng_state		Engine State to be changed
 */
inline gboolean voicecall_core_change_engine_state(voicecall_engine_t *pcall_engine, int eng_state)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_change_engine_iostate(pcall_engine, eng_state)) ? TRUE : FALSE;
}

/**
 * This function ends an Outgoing Call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_end_mo_call(voicecall_engine_t *pcall_engine)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_end_call(pcall_engine, VC_END_OUTGOING_CALL)) ? TRUE : FALSE;
}

/**
 * This function retreives the Voicecall Engine's State
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[out]	io_state			Voicecall Engine InOut State
 */
inline gboolean voicecall_core_get_engine_state(voicecall_engine_t *pcall_engine, int *eng_state)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_status_get_engine_iostate(pcall_engine, eng_state)) ? TRUE : FALSE;
}

/**
 * This function checks whether any call exists
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[out]	active_calls		TRUE - If active call exists or FALSE If active call doesn't exists
 * @param[out]	held_calls		TRUE - If held call exists or FALSE If held call doesn't exists
 */
inline gboolean voicecall_core_is_call_exists(voicecall_engine_t *pcall_engine, gboolean *active_calls, gboolean *held_calls)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_status_isexists_any_call(pcall_engine, active_calls, held_calls)) ? TRUE : FALSE;
}

/**
 * This function checks whether incoming call exists or not
 *
 * @return		Returns TRUE if incoming call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_incoming_call_exists(voicecall_engine_t *pcall_engine)
{
	gboolean bmtcall_exists = FALSE;

	_vc_core_engine_status_isexists_call_bytype(pcall_engine, VC_INCOMING_CALL, &bmtcall_exists);
	return bmtcall_exists;
}

/**
 * This function checks whether outgoing call exists or not
 *
 * @return		Returns TRUE if outgoing call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_outgoing_call_exists(voicecall_engine_t *pcall_engine)
{
	gboolean bmocall_exists = FALSE;

	_vc_core_engine_status_isexists_call_bytype(pcall_engine, VC_OUTGOING_CALL, &bmocall_exists);
	return bmocall_exists;
}

/**
 * This function checks whether any connexcted call exists or not
 *
 * @return		Returns TRUE if connected call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_connected_call_exist(voicecall_engine_t *pcall_engine)
{
	gboolean bcall_exists = FALSE;

	_vc_core_engine_status_isexists_call_bytype(pcall_engine, VC_CONNECTED_CALL, &bcall_exists);
	return bcall_exists;
}

/**
 * This function checks whether any connexcted call exists or not in the given group
 *
 * @return		Returns TRUE if connected call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		group_index		Group Index to be searhced
 */
inline gboolean voicecall_core_is_connected_call_exist_in_group(voicecall_engine_t *pcall_engine, int group_index)
{
	gboolean bcall_exists = FALSE;
	voicecall_error_t error_code = ERROR_VOICECALL_NONE;

	if (ERROR_VOICECALL_NONE != _vc_core_engine_group_isexists_connected_call_ingroup(pcall_engine, group_index, &bcall_exists)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Error_code %d", error_code);

	}

	return bcall_exists;
}

/**
 * This function checks whether any call exists
 *
 * @return		Returns TRUE if connected call exists or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_any_call_exists(voicecall_engine_t *pcall_engine)
{
	gboolean bcall_exists = FALSE;

	_vc_core_engine_status_isexists_call_bytype(pcall_engine, VC_INCOMING_CALL, &bcall_exists);
	CALL_ENG_DEBUG(ENG_DEBUG, "Incoming Call = [%d]", bcall_exists);

	if (FALSE == bcall_exists) {
		_vc_core_engine_status_isexists_call_bytype(pcall_engine, VC_OUTGOING_CALL, &bcall_exists);
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Outgoing Call = [%d]", bcall_exists);

	if (FALSE == bcall_exists) {
		_vc_core_engine_status_isexists_call_bytype(pcall_engine, VC_CONNECTED_CALL, &bcall_exists);
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "Connected Call = [%d]", bcall_exists);

	return bcall_exists;
}

/**
 * This function retreives the totally number of availavle calls including connected, MO and MT Calls
 *
 * @return		Returns TRUE if success or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[out]	ptotal_call_member	number of avialble calls
 */
inline gboolean voicecall_core_get_total_call_member(voicecall_engine_t *pcall_engine, int *ptotal_call_member)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_status_get_call_member_count(pcall_engine, ptotal_call_member)) ? TRUE : FALSE;
}

/**
 * This function checks whether voicecall engine's call agent is idle or not
 *
 * @return		Returns TRUE if call agent is idle or FALSE on failure
 * @param[in]		pcall_engine		Handle to voicecall engine
 */
inline gboolean voicecall_core_is_callagent_idle(voicecall_engine_t *pcall_engine)
{
	gboolean bcallagent_idle = FALSE;

	_vc_core_engine_status_is_engine_busy(pcall_engine, &bcallagent_idle);
	CALL_ENG_DEBUG(ENG_DEBUG, "Call Agent Busy State : %d", bcallagent_idle);

	return !bcallagent_idle;
}

/**
* This function checks the current call status and engine status
*
* @return		TRUE, if connected calls available and engine is in idle, FALSE otherwise
 * @param[in]		pcall_engine		Handle to voicecall engine
*/
inline gboolean voicecall_core_is_incall_request_possible(voicecall_engine_t *pcall_engine)
{
	int eng_state = VC_INOUT_STATE_NONE;
	int member_num_0 = 0;
	int member_num_1 = 0;

	_vc_core_engine_group_get_connected_member_count(pcall_engine, 0, &member_num_0);
	_vc_core_engine_group_get_connected_member_count(pcall_engine, 1, &member_num_1);
	voicecall_core_get_engine_state(pcall_engine, &eng_state);
	if (!(voicecall_core_is_callagent_idle(pcall_engine) && ((member_num_1 + member_num_0) > 0)
	      && (eng_state == VC_INOUT_STATE_NONE))) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Engine Busy, Cannot complete Request,mem_0:%d,mem_1:%d,eng_state:%d ", member_num_0, member_num_1, eng_state);
		return FALSE;
	}

	return TRUE;
}

/**
 * This function changes the modem call audio path
 *
 * @return		TRUE sucess, FALSE otherwise
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		audio_path		audio path to be changed
 */
inline gboolean voicecall_core_change_audio_path(voicecall_engine_t *pcall_engine, voicecall_audio_path_t audio_path, gboolean bextra_volume)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_change_audio_path(pcall_engine, audio_path, bextra_volume)) ? TRUE : FALSE;
}

/**
 * This function sets the voice call audio volume for the given audio path type
 *
 * @return		returns TRUE in success , FALSE otherwise
 * @param[in]		pcall_engine		Handle to voicecall engine
 * @param[in]		tapi_snd_path		audio path for the volume to be set
 * @param[in]		vol_level			volume level
 */
inline gboolean voicecall_core_set_audio_volume(voicecall_engine_t *pcall_engine, voicecall_audio_path_t tapi_snd_path, int vol_level)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_set_audio_volume(pcall_engine, tapi_snd_path, (voicecall_audio_volume_t) vol_level)) ? TRUE : FALSE;
}

/**
 * This function retreives the voice call audio volume for the given audio path type
 *
 * @return		returns TRUE in success , FALSE otherwise
 * @param[in]		pcall_engine			Handle to voicecall engine
 * @param[in]		audio_path_type		audio path for the volume to be retreived
 */
inline gboolean voicecall_core_get_audio_volume(voicecall_engine_t *pcall_engine, voicecall_audio_path_t audio_path_type)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_get_audio_volume(pcall_engine, audio_path_type)) ? TRUE : FALSE;
}

/**
 * This function set the voice call audio mute status
 *
 * @return		returns TRUE in success , FALSE otherwise
 * @param[in]		pcall_engine			Handle to voicecall engine
 * @param[in]		bvoice_mute			mute status
 */
inline gboolean voicecall_core_set_audio_mute_status(voicecall_engine_t *pcall_engine, gboolean bvoice_mute)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_set_audio_mute(pcall_engine, bvoice_mute)) ? TRUE : FALSE;
}

/**
 * This function retreives the first active call among the available active calls
 *
 * @return		Returns TRUE if success or FALSE on failure
 * @param[in]		pcall_engine			Handle to voicecall engine
 * @param[out]	pcall_handle		call handle of the active call
 */
inline gboolean voicecall_core_get_zuhause(voicecall_engine_t *pcall_engine, gboolean * bzuhause)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_status_is_zuhause_area(pcall_engine, bzuhause)) ? TRUE : FALSE;
}

/**
 * This function retreives the Voicecall Engine's State
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]	pcall_engine	Handle to voicecall engine
 * @param[in]	call_handle	Call handle of the call for which the call object is retrieved
 * @param[out]	pcall_object	Pointer to the retrived call object info
 */
inline gboolean voicecall_core_get_call_object(voicecall_engine_t *pcall_engine, int call_handle, call_vc_call_objectinfo_t * pcall_object)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_status_get_call_object(pcall_engine, call_handle, pcall_object)) ? TRUE : FALSE;
}

/**
 * This function sends response to sat engine
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]	pcall_engine	Handle to voicecall engine
 * @param[in]		sat_rqst_resp_type sat rqst/resp type to be set by the client
 * @param[in]		sat_response_type sat response type to be sent to sat
 */
inline gboolean voicecall_core_send_sat_response(voicecall_engine_t *pcall_engine, voicecall_engine_sat_rqst_resp_type sat_rqst_resp_type, call_vc_sat_reponse_type_t sat_response_type)
{
	voicecall_error_t error_code = 0;
	error_code = _vc_core_engine_send_sat_response(pcall_engine, sat_rqst_resp_type, sat_response_type);
	CALL_ENG_DEBUG(ENG_DEBUG, "error_code:[%d] ", error_code);
	return (ERROR_VOICECALL_NONE == error_code) ? TRUE : FALSE;
}

/**
 * This function retreives the number of active call members
 *
 * @return		Returns TRUE if success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[out]	pactive_member_num	number of active call members available
 */
inline gboolean voicecall_core_get_active_call_member(call_vc_core_state_t *pcall_core, int *pactive_member_num)
{
	*pactive_member_num = 0;
	return (ERROR_VOICECALL_NONE == _vc_core_engine_status_get_call_member_info(pcall_core->pcall_engine, VC_ACTIVE_CALL, pactive_member_num)) ? TRUE : FALSE;
}

/**
 * This function checks whether possible to make conference call
 *
 * @return		Returns TRUE If Conference call can be made or FALSE if not
 * @param[in]		papp_document	Handle to Application Document
 */
inline gboolean voicecall_core_is_conf_call_possible(call_vc_core_state_t *pcall_core)
{
	gboolean bconf_call = FALSE;

	_vc_core_engine_status_is_conf_call_possible(pcall_core->pcall_engine, &bconf_call);

	return bconf_call;
}

/**
 * This function checks whether possible to transfer call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 */
inline gboolean voicecall_core_is_transfer_call_possible(call_vc_core_state_t *pcall_core)
{
	gboolean btransfer_call = FALSE;

	_vc_core_engine_status_is_transfer_call_possible(pcall_core->pcall_engine, &btransfer_call);

	return btransfer_call;
}

/**
 * This function checks whether the given code is a valid Supplementary Services Code
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		pszInput			Number to be verified
 */
inline gboolean voicecall_core_is_valid_sscode(call_vc_core_state_t *pcall_core, const char *pszInput)
{
	gboolean bsscode = FALSE;

	_vc_core_engine_status_isvalid_ss_code(pcall_core->pcall_engine, pszInput, &bsscode);

	return bsscode;
}

#ifdef _CPHS_DEFINED_
/**
 * This function gets the cphs status from the engine
 *
 * @return		TRUE if queried status is enabled, FALSE otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		csp_service		csp service to be queried
 */
inline gboolean voicecall_core_get_cphs_csp_status(call_vc_core_state_t *pcall_core, voicecall_cphs_csp_service csp_service)
{
	gboolean bcsp_status = FALSE;

	_vc_core_engine_status_get_cphs_csp_status(pcall_core->pcall_engine, csp_service, &bcsp_status);
	return bcsp_status;
}
#endif

/**
 * This function informs the Voicecall Engine that current SS operation has been completed
 *
 * @return		Returns TRUE if all the calls are ended or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
inline gboolean voicecall_core_set_check_ss_on_end(call_vc_core_state_t *pcall_core)
{
	return (ERROR_VOICECALL_NONE == _vc_core_engine_status_set_end_flag(pcall_core->pcall_engine, VC_RETREIVE_CALL_ON_MOCALL_END)) ? TRUE : FALSE;
}

/**
 * This function extracts vaild phone number
 *
 * @return		void
 * @param[in]
 */
inline void voicecall_core_extract_phone_number(const char *source_tel_number, char *phone_number, const int buf_size)
{
	_vc_core_engine_extract_phone_number(source_tel_number, phone_number, buf_size);
}

/************************
*  inline function END
**************************/
void voicecall_core_set_status(call_vc_core_state_t *pcall_core, call_vc_core_flags_t core_flags, gboolean bstatus)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "core flags:[0x%x], Set status:[%d] ", core_flags, bstatus);

	if (CALL_VC_CORE_FLAG_NONE == core_flags) {
		/*Set the document flag to defaults */
		pcall_core->core_status = CALL_VC_CORE_FLAG_NONE;
		return;
	}

	if (TRUE == bstatus) {
		/*Set Flag */
		pcall_core->core_status |= core_flags;
	} else {
		/*Remove bit field only if it is already set/ otherwise ignore it */
		if ((pcall_core->core_status & core_flags) == core_flags) {
			pcall_core->core_status = (pcall_core->core_status ^ core_flags);
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "After SET, core_status:[0x%x]", pcall_core->core_status);

}

gboolean voicecall_core_get_status(call_vc_core_state_t *pcall_core, call_vc_core_flags_t core_flags)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "core_flags:[0x%x]", core_flags);
	CALL_ENG_DEBUG(ENG_DEBUG, "Before Get, core_status:[0x%x]", pcall_core->core_status);

	if ((pcall_core->core_status & core_flags) == core_flags) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Flag [0x%x] is available", core_flags);
		return TRUE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Flag [0x%x] is not available", core_flags);
	return FALSE;
}

static gboolean __voicecall_core_minute_minder(gpointer puser_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)puser_data;

	if ((voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine) == FALSE)
	    && (voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine) == FALSE)
	    && (voicecall_core_is_connected_call_exist(pcall_core->pcall_engine) == TRUE)) {
		voicecall_snd_play_effect_tone(pcall_core->papp_snd, VOICE_CALL_SND_EFFECT_CALL_MINUTE_MINDER);
	}

	return TRUE;
}

static gboolean __voicecall_core_set_path_timer_cb(gpointer puser_data)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	VOICECALL_RETURN_FALSE_IF_FAIL(puser_data != NULL);
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)puser_data;

	voicecall_snd_change_path(pcall_core->papp_snd);

	if (g_set_path_timer_handler > 0) {
		g_source_remove(g_set_path_timer_handler);
		g_set_path_timer_handler = 0;
	}

	return FALSE;
}

static gboolean __voicecall_core_handle_call_end_on_silent_reject(call_vc_core_state_t *pcall_core, int call_handle)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle = %d", call_handle);

	if ((pcall_core->mtcall_silent_reject_handle == call_handle)) {
		/*Call rejected due to lawmo lock */
		if (FALSE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
			/*Connected calls need to be checked, Connected emergency calls may be avaialble */
			voicecall_core_set_to_default(pcall_core);
		} else {
			CALL_ENG_DEBUG(ENG_ERR, "Connected calls available");
			_vc_core_engine_status_dump_call_details(pcall_core->pcall_engine);
		}
		pcall_core->mtcall_silent_reject_handle = -1;
		return TRUE;
	}

	return FALSE;
}

static gboolean __voicecall_core_silent_reject_mt(call_vc_core_state_t *pcall_core, int call_handle)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle = %d", call_handle);

	voicecall_error_t error_code = ERROR_VOICECALL_NONE;

	pcall_core->mtcall_silent_reject_handle = call_handle;

	/*Reject the call */
	error_code = _vc_core_engine_reject_call(pcall_core->pcall_engine, FALSE);

	if (ERROR_VOICECALL_NONE != error_code) {
		CALL_ENG_DEBUG(ENG_ERR, "_vc_core_engine_reject_call Failed, error_code = %d", error_code);
	}

	return TRUE;
}

static void __voicecall_core_processing_mo_cancel(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;
	int total_call_member;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (TRUE == voicecall_snd_is_signal_playing(pcall_core->papp_snd)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Signal is playing, skipping cancel timer");
		return;
	}

	vc_engine_outgoing_end_type event_data;

	/* normal outgong end */
	CALL_ENG_DEBUG(ENG_DEBUG, "It is normal outgong end case.");
	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle:[%d],end_cause_type:[%d]", pcall_core->mo_end_call_handle, pcall_core->mo_end_cause_type);

	memset(&event_data, 0, sizeof(event_data));
	event_data.call_handle = pcall_core->mo_end_call_handle;
	event_data.end_cause_type = pcall_core->mo_end_cause_type;
	vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_END_TO_UI, (void *)&event_data);

	_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_END, pcall_core->mo_end_call_handle, NULL);

	__vc_core_set_auto_redial_count(pcall_core, 0);
	voicecall_core_clear_mo_call(pcall_engine);

	__voicecall_core_mocall_reset_engine_state(pcall_engine);

	/* __vcui_app_view_mo_finish_call() start */
	voicecall_snd_stop_signal(pcall_core->papp_snd);

	voicecall_core_get_total_call_member(pcall_engine, &total_call_member);
	/* If No Connected Calls End the UI */
	if (total_call_member == 0) {
		/*Reset voice call core to default values */
		voicecall_core_set_to_default(pcall_core);
	} else {
		voicecall_snd_change_path(pcall_core->papp_snd);
	}
	/* __vcui_app_view_mo_finish_call() end */

}

static void __voicecall_core_mocall_signal_play_end_cb(gpointer pdata)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)pdata;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (FALSE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No More calls, resetting path");
		_vc_core_util_set_call_status(VCONFKEY_CALL_OFF);
	}

	if (TRUE == voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Updating MO UI on signal end");
		__voicecall_core_processing_mo_cancel(pcall_core);
	}
}

static voicecall_snd_signal_type_t __voicecall_core_get_signal_type_from_endcause(int end_cause)
{
	int signal_type = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "End end_cause_type: %d", end_cause);

	switch (end_cause) {
	case VC_ENDCAUSE_USER_BUSY:
	case VC_ENDCAUSE_USER_DOESNOT_RESPOND:
	case VC_ENDCAUSE_USER_UNAVAILABLE:
	case VC_ENDCAUSE_NO_ANSWER:
		{
			signal_type = VOICE_CALL_SIGNAL_USER_BUSY_TONE;
		}
		break;
	default:
		signal_type = VOICE_CALL_SIGNAL_NONE;
		break;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Signal Type: %d", signal_type);

	return signal_type;
}

static void __voicecall_core_handle_normal_end(call_vc_core_state_t *pcall_core, int call_handle, voice_call_end_cause_type_t end_cause)
{
	int total_call_member = 0;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	/*Set the callstatus to OFF before processing the End Event Animation */
	if (TRUE == voicecall_core_is_connected_call_exist(pcall_engine)) {
		_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_ACTIVE);
	} else if ((TRUE == voicecall_core_is_incoming_call_exists(pcall_engine)) || (TRUE == voicecall_core_is_outgoing_call_exists(pcall_engine))) {
		_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_CONNECTING);
	} else {
		/*Reset the Path Actual path must be closed when modem path closed!! */
		_vc_core_util_set_call_status(VCONFKEY_CALL_OFF);
	}

	if (FALSE == voicecall_snd_play_effect_tone(pcall_core->papp_snd, VOICE_CALL_SND_EFFECT_CALL_DISCONNECT)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Effect tone not played, check and play busy tone");
	}
	if ((FALSE == voicecall_core_is_connected_call_exist(pcall_engine))
	    && (TRUE == voicecall_core_is_incoming_call_exists(pcall_engine))
	    && (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_SPEAKER)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Incoming Call: TRUE,Connected Call:FALSE, Speaker: TRUE. So change path to normal");
		voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER_EARJACK);
		voicecall_snd_change_path(pcall_core->papp_snd);
	}

	voicecall_core_clear_connected_call(pcall_engine, call_handle);

	voicecall_core_get_total_call_member(pcall_engine, &total_call_member);
	if (0 == total_call_member) {
		voicecall_core_set_to_default(pcall_core);
	}

}

static gboolean __voicecall_core_handle_rejected_call_end(call_vc_core_state_t *pcall_core, int call_handle)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	voicecall_snd_stop_alert(pcall_core->papp_snd);

	/*Send Incoming call End Event to Blue Tooth */
	_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_END, call_handle, NULL);

	if (FALSE == voicecall_core_is_connected_call_exist(pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No More Calls");
		voicecall_core_set_to_default(pcall_core);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Setting Path for Voicecall");
		/*Change Path to Call, when the incomging call is cancelled. */
		voicecall_snd_change_path(pcall_core->papp_snd);
		_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_ACTIVE);
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Reject Call End Processed");
	return TRUE;
}

static void __voicecall_core_handle_outgoingcall_end(call_vc_core_state_t *pcall_core, int call_handle, voice_call_end_cause_type_t end_cause_type)
{
	gboolean bsignal_play = FALSE;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "call_handle=:%d,end_cause=%d", call_handle, end_cause_type);
	pcall_core->mo_end_call_handle = call_handle;
	pcall_core->mo_end_cause_type = end_cause_type;

	if (FALSE == voicecall_core_is_connected_call_exist(pcall_engine)) {
		voicecall_snd_signal_type_t end_signal_type = VOICE_CALL_SIGNAL_NONE;
		/*Play Signal Tone only when the connected calls are not exists */
		end_signal_type = __voicecall_core_get_signal_type_from_endcause(end_cause_type);

		if (end_signal_type != VOICE_CALL_SIGNAL_NONE) {
			voicecall_snd_set_signal_type(pcall_core->papp_snd, end_signal_type);
			voicecall_snd_play_signal(pcall_core->papp_snd, __voicecall_core_mocall_signal_play_end_cb, pcall_core);
			bsignal_play = TRUE;

			/* signal tone play case : just end string updated */
			{
				vc_engine_outgoing_end_signal_play_type event_data;

				/* normal outgong end */
				CALL_ENG_DEBUG(ENG_DEBUG, "It is normal outgong end case.");
				CALL_ENG_DEBUG(ENG_DEBUG, "call_handle:[%d],end_cause_type:[%d]", call_handle, pcall_core->mo_end_cause_type);

				memset(&event_data, 0, sizeof(event_data));
				event_data.call_handle = call_handle;
				event_data.end_cause_type = pcall_core->mo_end_cause_type;
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_END_SIGNAL_PLAY_TO_UI, (void *)&event_data);
			}

		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Connected call exists, not playing signal tone");
	}
	if (TRUE == voicecall_core_is_connected_call_exist(pcall_engine)) {
		/* Set phonestatus value */
		_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_ACTIVE);
	} else if (FALSE == bsignal_play) {
		/*Reset the Path Actual path must be closed when modem path closed!! */
		_vc_core_util_set_call_status(VCONFKEY_CALL_OFF);
	} else {
		/* othing to do. */
	}
	__voicecall_core_processing_mo_cancel(pcall_core);

}

static gboolean __voicecall_core_handle_incoming_call_end(call_vc_core_state_t *pcall_core, int call_handle)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	voicecall_snd_stop_alert(pcall_core->papp_snd);

	/*Send Incoming call End Event to Blue Tooth */
	_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_END, call_handle, NULL);

	if (FALSE == voicecall_core_is_connected_call_exist(pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "No More Calls");
		voicecall_core_set_to_default(pcall_core);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Setting Path for Voicecall");
		/*Change Path to Call, when the incomging call is cancelled. */
		voicecall_snd_change_path(pcall_core->papp_snd);
		_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_ACTIVE);
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Incoming Call End Processed");
	return TRUE;
}

static gboolean voicecall_core_cb(int event, int param1, int param2, void *param3, void *puser_data)
{
	CALL_ENG_DEBUG(ENG_WARN, " Engine Event: %s(%d)", gszcall_engine_event[event], event);

	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)puser_data;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_VC_DUMP_CALLDETAILS(&pcall_engine->call_manager);

	switch (event) {
	case VC_CALL_INCOM:
		{
			int call_handle = param1;
			char tel_number[VC_PHONE_NUMBER_LENGTH_MAX];
			gboolean bauto_reject = FALSE;
			gboolean bauto_reject_unknown = FALSE;
			gboolean restricted = FALSE;
			gboolean bcalling_namemode = FALSE;
			gboolean brejected_number = FALSE;
			vc_engine_incoming_type event_data;

			memset(&event_data, 0, sizeof(event_data));
			event_data.bday_remaining_days = -1;

			CALL_ENG_DEBUG(ENG_DEBUG, "tel_number:[%s]", (char *)param3);
			_vc_core_util_strcpy(tel_number, sizeof(tel_number), (char *)param3);

			/*Changing the path to headset/phone will be decided by user accept action.
			   This will apply for second incoming call, if the first call was accpeted by BT Headset
			   and the path was changed to BT headset, then the path will be in headset for second call by default
			   So reset the headset flag for second call so the path will not
			   be automatically changed to headset, it will be decided by user action.
			   If this requirement fits only for first incoming call then comment this fix. */
			/* vcui_app_snd_set_status(papp_document->papp_snd, VCUI_APP_AUDIO_HEADSET, FALSE); */

			/*Will be set based on user action */
			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, FALSE);

			if (_vc_core_util_check_video_call_status() == TRUE) {
				/*Check for Lawmo Lock */
				if (TRUE == __voicecall_core_silent_reject_mt(pcall_core, call_handle)) {
					CALL_ENG_DEBUG(ENG_DEBUG, "Call rejected due to silent reject");
					return TRUE;
				}
			}

			/* Check for Restricted Mode */
			_vc_core_engine_status_isrestricted_call(pcall_engine, call_handle, &restricted);
			_vc_core_engine_status_get_calling_namemode(pcall_engine, call_handle, &bcalling_namemode);
			if (TRUE == bcalling_namemode) {
				call_vc_call_objectinfo_t call_object;

				voicecall_core_get_call_object(pcall_engine, call_handle, &call_object);
				CALL_ENG_DEBUG(ENG_DEBUG, "call_object.calling_name:[%s]", call_object.calling_name);

				_vc_core_util_strcpy(event_data.call_name, sizeof(event_data.call_name), call_object.calling_name);
			} else if (TRUE == restricted) {
				call_vc_call_objectinfo_t call_object;

				voicecall_core_get_call_object(pcall_engine, call_handle, &call_object);
				CALL_ENG_DEBUG(ENG_DEBUG, "call_object.name_mode:[%s]", call_object.name_mode);

				event_data.brestricted = TRUE;
				if (call_object.name_mode == CALL_VC_NAME_MODE_PAYPHONE) {
					event_data.bpayphone = TRUE;
				} else {
					event_data.bpayphone = FALSE;

				}
				event_data.contact_index = -1;
				event_data.phone_type = -1;
			} else {
				voicecall_contact_info_t ct_info;
				memset(&ct_info, 0, sizeof(ct_info));
				ct_info.ct_index = -1;

				voicecall_service_contact_info_by_number(tel_number, &ct_info);

				_vc_core_util_strcpy(event_data.call_name, sizeof(event_data.call_name), ct_info.display_name);
				_vc_core_util_strcpy(event_data.call_file_path, sizeof(event_data.call_file_path), ct_info.caller_id_path);
				_vc_core_util_strcpy(event_data.call_full_file_path, sizeof(event_data.call_full_file_path), ct_info.caller_full_id_path);
				event_data.contact_index = ct_info.ct_index;
				event_data.phone_type = ct_info.phone_type;
				event_data.bday_remaining_days = ct_info.bday_remaining_days;

				voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
				char ringtone_path[VOICE_CALL_SND_RINGTONE_PATH_LEN] = { 0, };
				memset(papp_snd->ring_tone, 0, VOICE_CALL_SND_RINGTONE_PATH_LEN);
				if (TRUE == g_file_test(ct_info.ring_tone, G_FILE_TEST_EXISTS)) {
					snprintf(ringtone_path, sizeof(ringtone_path), "file://%s", ct_info.ring_tone);
					_vc_core_util_strcpy(papp_snd->ring_tone, VOICE_CALL_SND_RINGTONE_PATH_LEN, ringtone_path);
					CALL_ENG_DEBUG(ENG_DEBUG, "From Contact Ringtone: %s", papp_snd->ring_tone);
				} else {
					/*Get Ringtone File From Settings */
					CALL_ENG_DEBUG(ENG_DEBUG, "Invalid Ringtone from Contact: %s", ct_info.ring_tone);
				}
			}

			/* send to ui */
			event_data.call_handle = call_handle;
			event_data.brejected = brejected_number;
			_vc_core_util_strcpy(event_data.call_num, sizeof(event_data.call_num), tel_number);

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_INCOM_TO_UI, (void *)&event_data);

			{
				/* in case of rejected number, sound & callstatus is not processed */
				if (!brejected_number) {
					gboolean benabledTestMode = FALSE;
					voicecall_snd_register_cm(pcall_core->papp_snd);

					/*Send Incoming Call Event to Blue Tooth */
					_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_INCOM, call_handle, tel_number);

					_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_CONNECTING);

					benabledTestMode = __vc_core_is_answermode_enabled_from_testmode();
					if ((TRUE == __vc_core_is_answermode_enabled()) || (TRUE == benabledTestMode)) {
						CALL_ENG_DEBUG(ENG_DEBUG, "auto answer mode is enabled.");
						__voicecall_core_start_auto_answer(pcall_core, benabledTestMode);
					}

					CALL_ENG_DEBUG(ENG_DEBUG, "Preparing Sound ");
					voicecall_snd_prepare_alert(pcall_core->papp_snd, call_handle);
					if (FALSE == voicecall_core_is_connected_call_exist(pcall_core->pcall_engine)) {
						CALL_ENG_DEBUG(ENG_DEBUG, "Changing MM Path just before playing the ring tone");
						sound_manager_call_session_set_mode(pcall_core->papp_snd->psnd_session, SOUND_CALL_SESSION_MODE_RINGTONE);
					} else {
						CALL_ENG_DEBUG(ENG_DEBUG, "2nd MT call alert.");
					}
					voicecall_snd_play_alert(pcall_core->papp_snd);
				}
			}
		}
		break;

	case VC_CALL_OUTGOING:
		{
			int call_handle = param1;
			vc_engine_common_with_handle_type event_data;
			memset(&event_data, 0, sizeof(event_data));

			CALL_ENG_DEBUG(ENG_DEBUG, "MO Call Start: Call handle %d", call_handle);
		}
		break;

	case VC_CALL_OUTGOING_ORIG:
		{
			int call_handle = param1;
			vc_engine_outgoing_orig_type event_data;
			call_vc_call_objectinfo_t callobject_info;

			memset(&event_data, 0, sizeof(event_data));
			event_data.call_handle = call_handle;
			if (pcall_core->call_setup_info.call_type == VC_CALL_ORIG_TYPE_EMERGENCY) {
				event_data.bemergency = TRUE;
			} else {
				event_data.bemergency = FALSE;
			}

			CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d, bemergency:[%d]", event_data.call_handle, event_data.bemergency);
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_ORIG_TO_UI, (void *)&event_data);

			g_set_path_timer_handler = g_timeout_add(SET_PATH_TIMER_VALUE, __voicecall_core_set_path_timer_cb, pcall_core);

			/*Send Event to Blue Tooth */
			voicecall_core_get_call_object(pcall_engine, call_handle, &callobject_info);
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_ORIG, call_handle, callobject_info.tel_number);
		}
		break;

	case VC_CALL_OUTGOING_ALERT:
		{
			int call_handle = param1;
			vc_engine_common_with_handle_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			event_data.call_handle = call_handle;

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_ALERT_TO_UI, (void *)&event_data);

			/*Play Connected Effect Tone */
			CALL_ENG_KPI("voicecall_snd_play_effect_tone start");
			if (FALSE == voicecall_snd_play_effect_tone(pcall_core->papp_snd, VOICE_CALL_SND_EFFECT_CALL_CONNECT)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "voicecall_snd_play_effect_tone return value is FALSE");
				voicecall_snd_change_path(pcall_core->papp_snd);
			}
			CALL_ENG_KPI("voicecall_snd_play_effect_tone done");

			CALL_ENG_KPI("_vc_bt_send_response_to_bt start");
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_REMOTE_RINGING, call_handle, NULL);
			CALL_ENG_KPI("_vc_bt_send_response_to_bt done");
		}
		break;

	case VC_CALL_ANSWER_CNF:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Answer confirm");
			if (_vc_core_cm_get_call_member_count(&(pcall_engine->call_manager)) == 1) {
				CALL_ENG_DEBUG(ENG_DEBUG, "single call state");
				voicecall_snd_change_path(pcall_core->papp_snd);
			}
		}
		break;

	case VC_CALL_CONNECTED:
		{
			int call_handle = param1;
			vc_engine_connected_type event_data;
			int bstatus = FALSE;

			memset(&event_data, 0, sizeof(event_data));

			CALL_ENG_DEBUG(ENG_DEBUG, "Connected Call Handle : %d", call_handle);

			event_data.call_handle = call_handle;
			event_data.bt_status = (int)_vc_bt_get_bt_status();

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_CONNECTED_TO_UI, (void *)&event_data);
			voicecall_snd_stop_alert(pcall_core->papp_snd);	/* To stop alert in case of call accept by AT command */
			voicecall_snd_change_path(pcall_core->papp_snd);

			/* Set phone-status value */
			_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_ACTIVE);

			/* check minute minder */
			if (vconf_get_bool(VCONFKEY_CISSAPPL_MINUTE_MINDER_BOOL, &bstatus)) {
				CALL_ENG_DEBUG(ENG_ERR, "vconf_get_bool failed.");
			}
			if (bstatus) {
				if (pcall_core->minute_minder_timer == 0) {
					pcall_core->minute_minder_timer = g_timeout_add(MINUTE_MINDER_TIMEOUT_VALUE, __voicecall_core_minute_minder, pcall_core);
				}
			}

			/*Send Event to Blue Tooth */
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_CONNECT, call_handle, NULL);

			/*Call is accepted, reset the flag */
			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, FALSE);


		}
		break;

	case VC_CALL_NORMAL_END:
		{
			int call_handle = param1;
			voice_call_end_cause_type_t end_cause = param2;
			vc_engine_normal_end_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			CALL_ENG_DEBUG(ENG_DEBUG, "Normal End Call Handle : %d,End Cause=%d", call_handle, end_cause);

			event_data.call_handle = call_handle;
			event_data.end_cause_type = end_cause;

			__voicecall_core_handle_normal_end(pcall_core, call_handle, end_cause);

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_NORMAL_END_TO_UI, (void *)&event_data);

			/*Send Event to Blue Tooth */
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_END, call_handle, NULL);
		}
		break;

	case VC_CALL_INCOM_END:
		{
			int call_handle = param1;
			vc_engine_common_with_handle_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			CALL_ENG_DEBUG(ENG_DEBUG, "Incoming call End Call Handle: %d", call_handle);

			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, FALSE);

			if (TRUE == __voicecall_core_handle_call_end_on_silent_reject(pcall_core, call_handle)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Call end processed for silent reject:");
				return TRUE;
			}

			event_data.call_handle = call_handle;

			__voicecall_core_handle_incoming_call_end(pcall_core, call_handle);

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_INCOM_END_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_INCOM_DROPPED:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Check it. Not used");
		}
		break;

	case VC_CALL_REJECTED_END:
		{
			int call_handle = param1;
			vc_engine_common_with_handle_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			CALL_ENG_DEBUG(ENG_DEBUG, "Rejected call End Call Handle: %d", call_handle);

			event_data.call_handle = call_handle;

			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, FALSE);

			__voicecall_core_handle_rejected_call_end(pcall_core, call_handle);

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_REJECTED_END_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_OUTGOING_END:
		{
			int call_handle = param1;
			int end_cause_type = param2;
			int bauto_redial = -1;

			CALL_ENG_DEBUG(ENG_DEBUG, "end cause type :[%d]", end_cause_type);
			CALL_ENG_DEBUG(ENG_DEBUG, "bauto_redial:[%d]", bauto_redial);

			if ((TRUE == bauto_redial) && (FALSE == voicecall_core_is_connected_call_exist(pcall_engine))
			    && (FALSE == voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_MOCALL_END_BY_USER))
			    && (TRUE == __voicecall_core_is_redial_cuase(end_cause_type))) {
				/* auto redial */
				CALL_ENG_DEBUG(ENG_DEBUG, "It is auto redial case.");

				_vc_core_util_set_call_status(VCONFKEY_CALL_OFF);

				vc_engine_outgoing_end_type event_data;

				memset(&event_data, 0, sizeof(event_data));
				event_data.call_handle = call_handle;
				event_data.end_cause_type = end_cause_type;
				event_data.bauto_redial = TRUE;
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_END_TO_UI, (void *)&event_data);
			} else {
				__voicecall_core_handle_outgoingcall_end(pcall_core, call_handle, end_cause_type);
			}
		}
		break;

	case VC_CALL_OUTGOING_ABORTED:
		{
			vc_engine_common_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_ABORTED_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_DTMF_ACK:
		{
			gboolean bsuccess = param1;
			CALL_ENG_DEBUG(ENG_DEBUG, "bsuccess:[%d]", bsuccess);
			__voicecall_core_handle_dtmf_ack(pcall_core, bsuccess);
		}
		break;

	case VC_CALL_AUTO_REDIAL_CHECK:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Check it. Not used");
		}
		break;

	case VC_CALL_SS_HELD:
		{
			int call_handle = param1;
			vc_engine_common_type event_data;

			/*  held popup shold not be displayed on outgoing popup */
			memset(&event_data, 0, sizeof(event_data));

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_SS_HELD_TO_UI, (void *)&event_data);

			/*Send Event to Blue Tooth */
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_HOLD, call_handle, NULL);
		}
		break;

	case VC_CALL_SS_RETREIVED:
		{
			int call_handle = param1;
			vc_engine_common_type event_data;

			/*  held popup shold not be displayed on outgoing popup */
			memset(&event_data, 0, sizeof(event_data));
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_SS_RETREIVED_TO_UI, (void *)&event_data);

			/*Send Event to Blue Tooth */
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_RETRIEVE, call_handle, NULL);
		}
		break;

	case VC_CALL_SS_SWAP:
		{
			vc_engine_common_type event_data;

			int call_handle = param1;

			memset(&event_data, 0, sizeof(event_data));

			/* Show Call Swapped Message Box */
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_SS_SWAP_TO_UI, (void *)&event_data);

			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_SWAPPED, call_handle, NULL);
		}
		break;

	case VC_CALL_SS_SETUP_CONF:
		{
			vc_engine_common_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			/* Show Call Joined Message Box */
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_SS_SETUP_CONF_TO_UI, (void *)&event_data);

			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_JOINED, 0, NULL);
		}
		break;

	case VC_CALL_SS_SPLIT_CONF:
		{
			vc_engine_common_with_handle_type event_data;
			int call_handle = param1;
			CALL_ENG_DEBUG(ENG_DEBUG, "The handle to be split is %d", call_handle);

			memset(&event_data, 0, sizeof(event_data));
			event_data.call_handle = call_handle;

			/* Show Private Call Message Box */
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_SS_SPLIT_CONF_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_SS_TRANSFERRED:
		{
			vc_engine_common_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_SS_TRANSFERRED_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_SS_CONNECT_LINE_IND:
		{
			int call_handle = param1;
			char *pconnected_number = (char *)param3;
			vc_engine_msg_box_type event_data;

			CALL_ENG_DEBUG(ENG_DEBUG, "call_handle : [%d]", call_handle);

			/* check whether Call Info for recevice Call Handle exists or not. */

			if ((pconnected_number != NULL) && (strlen(pconnected_number) > 0)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "connected line ind : [%s]", pconnected_number);

				memset(&event_data, 0, sizeof(event_data));

				event_data.string_id = IDS_CALL_POP_CALL_IS_DIVERTED;
				_vc_core_util_strcpy(event_data.diverted_num, sizeof(event_data.diverted_num), pconnected_number);
			} else {
				CALL_ENG_DEBUG(ENG_DEBUG, "No conneccted info");
			}

		}
		break;

	case VC_CALL_IND_FORWARD:
		{
			vc_engine_ind_forward_type event_data;
			int fwd_type = param1;

			memset(&event_data, 0, sizeof(event_data));
			event_data.fwd_type = fwd_type;

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_IND_FORWARD_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_IND_ACTIVATE:
		{
			vc_engine_common_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_IND_ACTIVATE_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_IND_HOLD:
		{
			vc_engine_common_type event_data;
			int call_handle = param1;

			memset(&event_data, 0, sizeof(event_data));

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_IND_HOLD_TO_UI, (void *)&event_data);

			/*Send Event to Blue Tooth */
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_HOLD, call_handle, NULL);
		}
		break;

	case VC_CALL_IND_TRANSFER:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Not used.");
		}
		break;

	case VC_CALL_IND_SETUPCONFERENCE:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Not used.");
		}
		break;

	case VC_CALL_IND_BARRING:
		{
			vc_engine_common_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_IND_BARRING_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_IND_WAITING:
		{
			vc_engine_common_type event_data;

			memset(&event_data, 0, sizeof(event_data));

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_IND_WAITING_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_IND_CUGINFO:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Not used.");
		}
		break;

	case VC_CALL_IND_SSNOTIFY:
		{
			vc_engine_ind_ssnotify_type event_data;
			int ss_type = param1;

			memset(&event_data, 0, sizeof(event_data));
			event_data.ss_type = ss_type;

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_IND_SSNOTIFY_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_IND_CALLINGNAMEINFO:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Not used.");
		}
		break;
	case VC_CALL_IND_ACTIVATECCBS_CNF:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Not used.");
		}
		break;

	case VC_CALL_IND_ACTIVATECCBS_USERINFO:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Not used.");
		}
		break;

	case VC_CALL_IND_AOC:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Not yet.");
		}
		break;

	case VC_ERROR_OCCURED:
		{
			int error_code = param1;

			CALL_ENG_DEBUG(ENG_DEBUG, "error code:[%d]", error_code);
			switch (error_code) {
			case ERROR_VOICECALL_INVALID_DTMF_CHAR:
			case ERROR_VOICECALL_DTMF_FAILED:
				{
					vc_engine_error_occured_type event_data;

					memset(&event_data, 0, sizeof(event_data));
					event_data.error_code = error_code;

					vcall_engine_send_event_to_client(VC_ENGINE_MSG_ERROR_OCCURED_TO_UI, (void *)&event_data);
				}
				break;
			default:
				{
					vc_engine_msg_box_type event_data;
					int string_id = -1;

					string_id = __voicecall_core_get_string_id_by_errorcode(error_code);

					memset(&event_data, 0, sizeof(event_data));
					event_data.string_id = __voicecall_core_get_string_id_by_errorcode(error_code);

					vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
				}
				break;
			}
		}
		break;

	case VC_ACTION_INCOM_FORCE:
		{
			int call_handle = param1;
			vc_engine_common_with_handle_type event_data;

			CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle = %d", call_handle);

			memset(&event_data, 0, sizeof(event_data));
			event_data.call_handle = call_handle;

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_ACTION_INCOM_FORCE_TO_UI, (void *)&event_data);
		}
		break;

	case VC_ACTION_SAT_REQUEST:
		{
			if (SAT_RQST_SETUP_CALL == param1) {
				voicecall_sat_callinfo_t *psat_callinfo = (voicecall_sat_callinfo_t *) param3;
				vc_engine_outgoing_type event_data;

				if (psat_callinfo == NULL) {
					CALL_ENG_DEBUG(ENG_ERR, "psat_callinfo is NULL..");
					assert(psat_callinfo != NULL);
				} else {
					CALL_ENG_DEBUG(ENG_DEBUG, "VC_ACTION_SAT_REQUEST is received by Voice call.");
					pcall_core->call_setup_info.call_setup_by = VC_CALL_SETUP_BY_SAT;
					pcall_core->call_setup_info.call_type = VC_CALL_ORIG_TYPE_SAT;

					/*Phone Number */
					_vc_core_util_strcpy(pcall_core->call_setup_info.source_tel_number, VC_PHONE_NUMBER_LENGTH_MAX, psat_callinfo->call_number);
					_vc_core_engine_extract_phone_number(pcall_core->call_setup_info.source_tel_number, pcall_core->call_setup_info.tel_number, VC_PHONE_NUMBER_LENGTH_MAX);

					/*Name */
					CALL_ENG_DEBUG(ENG_ERR, "psat_callinfo->disp_text:[%s]", psat_callinfo->disp_text);

					memset(&event_data, 0, sizeof(event_data));
					_vc_core_util_strcpy(event_data.call_num, sizeof(event_data.call_num), pcall_core->call_setup_info.tel_number);
					event_data.contact_index = -1;
					event_data.phone_type = -1;
					event_data.bday_remaining_days = -1;
					_vc_core_util_strcpy(event_data.call_num, sizeof(event_data.call_num), psat_callinfo->disp_text);

					vcall_engine_send_event_to_client(VC_ENGINE_MSG_OUTGOING_TO_UI, (void *)&event_data);

					/*Get Icon Information */
					if (TRUE == psat_callinfo->bicon_present) {	/*bicon_present is TRUE when SAT icon info available and when GCF is enabled */
						CALL_ENG_DEBUG(ENG_ERR, "SAT icon available.");
					}

					/* Prepare and Make Call with the Give Information */
					if (FALSE == voicecall_core_setup_call(pcall_core, FALSE)) {
						/*Send Response to SAT Engine */
						voicecall_core_send_sat_response(pcall_core->pcall_engine, SAT_RQST_SETUP_CALL, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
					}
				}
			} else if (SAT_RQST_SEND_DTMF == param1) {
				voicecall_sat_callinfo_t *psat_callinfo = (voicecall_sat_callinfo_t *) param3;

				CALL_ENG_DEBUG(ENG_DEBUG, "SAT Send DTMF Number: %s, hidden: %d", psat_callinfo->call_number, psat_callinfo->bsat_hidden);
				__voicecall_core_queue_dtmf_string(pcall_core, psat_callinfo->call_number, TRUE);
			} else {
				CALL_ENG_DEBUG(ENG_ERR, "Invalid SAT Request Type: %d", param2);
			}
		}
		break;

	case VC_ACTION_SAT_RESPONSE:
		{
			if (SAT_RESP_SETUP_CALL == param1) {
				voicecall_sat_callinfo_t *psat_call_info = (voicecall_sat_callinfo_t *) param3;

				CALL_ENG_DEBUG(ENG_DEBUG, "sat_mo_call_ctrl_res = %d", psat_call_info->sat_mo_call_ctrl_res);

				if (CALL_NOT_ALLOWED == psat_call_info->sat_mo_call_ctrl_res) {
					vc_engine_msg_box_type event_data;

					memset(&event_data, 0, sizeof(event_data));
					event_data.string_id = IDS_CALL_POP_CALLNOTCALLOWED;
					vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
				} else if (CALL_ALLOWED_WITH_MOD == psat_call_info->sat_mo_call_ctrl_res) {
					CALL_ENG_DEBUG(ENG_DEBUG, "psat_call_info->call_number = [%s]", psat_call_info->call_number);
					CALL_ENG_DEBUG(ENG_DEBUG, "psat_call_info->disp_text = [%s]", psat_call_info->disp_text);
#ifdef _OLD_SAT_
					mocall_index = vcui_app_doc_get_mocall_index(papp_document);

					/*Update MO CALL Display Data with the SAT modified info */
					if (mocall_index != -1) {
						vcui_app_call_display_data_t display_data;
						vcui_app_call_display_data_t new_display_data;
						vcui_app_view_mo_state_t *pmo_state = NULL;

						if (FALSE == vcui_app_cdm_get_display_object_byindex(&papp_document->call_display_manager, mocall_index, &display_data)) {
							return TRUE;
						}

						vcui_app_cdm_clear_display_object(&new_display_data);

						/*Copy all must parameters required for MO Call */
						new_display_data.call_handle = display_data.call_handle;
						new_display_data.call_index = display_data.call_index;
						new_display_data.call_type = display_data.call_type;
						new_display_data.start_time = display_data.start_time;
						new_display_data.used = display_data.used;

						if (strlen(psat_call_info->call_number) > 0) {
							vcui_app_util_strcpy(new_display_data.source_tel_number, sizeof(new_display_data.source_tel_number), psat_call_info->call_number);
							_vc_core_engine_extract_phone_number(new_display_data.source_tel_number, new_display_data.tel_number, VC_PHONE_NUMBER_LENGTH_MAX);
						}

						if (strlen(psat_call_info->disp_text) > 0) {
							vcui_app_util_strcpy(new_display_data.name, sizeof(new_display_data.name), psat_call_info->disp_text);
						}

						/*todo, Check whether contact search need to be done for the SAT modified number */

						/*Set the newly modified data to the CDM */
						vcui_app_cdm_set_display_object(pdisplay_mgr, &new_display_data);

						/*Update the MO View */
						pmo_state = (vcui_app_view_mo_state_t *) calloc(1, sizeof(vcui_app_view_mo_state_t));
						pmo_state->mo_call_state = VCUI_MO_CALL_STATE_UPDATE;

						/* Update MO Call Screen View to update the connection status */
						dv_view_manager_update_view(dv_document_get_view_manager(DV_DOCUMENT(papp_document)), VCUI_APP_VIEWID_MO_VIEW, VCUI_APP_UPD_MO_SET_STATE, pmo_state);

					} else {
						VCUI_DEBUG(VCUI_LOG_ERR, "Invalid Mo Call Index: %d", mocall_index);
					}
#endif
				} else if (CALL_CHANGED_TO_SS == psat_call_info->sat_mo_call_ctrl_res) {
					/*Issue notification to Launch SS */
				}
			}
		}
		break;

	case VC_ACTION_CALL_END_HELD_RETREIVED:
		{
			int call_handle = param1;
			vc_engine_common_with_handle_type event_data;

			CALL_ENG_DEBUG(ENG_DEBUG, "call handle:[%d]", call_handle);

			memset(&event_data, 0, sizeof(event_data));

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_ACTION_CALL_END_HELD_RETREIVED_TO_UI, (void *)&event_data);
		}
		break;

	case VC_ACTION_NO_ACTIVE_TASK:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Not yet.");
		}
		break;

	case VC_CALL_GET_VOLUME_RESP:
		{
			vc_engine_vol_resp_type event_data;

			CALL_ENG_DEBUG(ENG_DEBUG, "Current Voicecall (TAPI)Volume Type %d, Current Volume Level: %d", param1, param2);

			memset(&event_data, 0, sizeof(event_data));
			event_data.vol_alert_type = VOICE_CALL_VOL_TYPE_VOICE;
			event_data.vol_level = param2;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_GET_VOLUME_RESP_TO_UI, (void *)&event_data);
		}
		break;

	case VC_CALL_NOTI_WBAMR:
		{
			vc_engine_wbamr_status_type event_data;

			CALL_ENG_DEBUG(ENG_DEBUG, "Current WBAmr status %d", param1);

			memset(&event_data, 0, sizeof(event_data));
			event_data.bstatus = param1;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_NOTI_WBAMR_TO_UI, (void *)&event_data);
		}
		break;

	default:
		CALL_ENG_DEBUG(ENG_DEBUG, " Engine Event ID : %d not handled", event);
		break;
	}

	CALL_VC_DUMP_CALLDETAILS(&pcall_engine->call_manager);

	CALL_ENG_DEBUG(ENG_DEBUG, " Ended.");

	return TRUE;
}

/**
 * This function converts Error Code to string id.
 *
 * @return		string id
 * @param[in]		error_code		error code to be used to display the message content
 */
static int __voicecall_core_get_string_id_by_errorcode(int error_code)
{
	int string_id = -1;
	switch (error_code) {
	case ERROR_VOICECALL_DTMF_FAILED:
		string_id = IDS_CALL_POP_DTMFSENDING_FAIL;
		break;

	case ERROR_VOICECALL_CALL_NOT_ALLOWED:
		string_id = IDS_CALL_POP_CALLNOTCALLOWED;
		break;

	case ERROR_VOICECALL_CALL_IMPOSSIBLE_NOSIM_NOEMERGNUM:
		string_id = IDS_CALL_POP_SOS_CALL_ONLY_IN_NO_SIM_MODE;
		break;

	case ERROR_VOICECALL_EMERGENCY_CALLS_ONLY:
		string_id = IDS_CALL_POP_CALLING_EMERG_ONLY;
		break;

	case ERROR_VOICECALL_PHONE_NOT_INITIALIZED:
		string_id = IDS_CALL_POP_PHONE_NOT_INITIALISED;
		break;

	case ERROR_VOICECALL_ANSWER_FAILED:
		string_id = IDS_CALL_POP_CALLFAILED;
		break;

	case ERROR_VOICECALL_HOLD_REJECTED:
	case ERROR_VOICECALL_HOLD_FAILED:
		string_id = IDS_CALL_POP_HOLD_FAILED;
		break;

	case ERROR_VOICECALL_ACTIVATE_REJECTED:
	case ERROR_VOICECALL_RETREIVE_FAILED:
		string_id = IDS_CALL_POP_UNABLE_TO_RETRIEVE;
		break;

	case ERROR_VOICECALL_SWAP_REJECTED:
	case ERROR_VOICECALL_SWAP_FAILED:
		string_id = IDS_CALL_POP_SWAP_FAILED;
		break;

	case ERROR_VOICECALL_SPLIT_CONF_FAILED:
		string_id = IDS_CALL_POP_SPLIT_FAILED;
		break;

	case ERROR_VOICECALL_SETUP_CONF_FAILED:
		string_id = IDS_CALL_POP_JOIN_FAILED;
		break;

	case ERROR_VOICECALL_TRANSFER_FAILED:
		string_id = IDS_CALL_POP_TRANSFER_FAILED;
		break;

	case ERROR_VOICECALL_SWAP_NOT_SUPPORTED:
		string_id = IDS_CALL_POP_SWAP_NOT_SUPPORTED;
		break;

	case ERROR_VOICECALL_HOLD_NOT_SUPPORTED:
		string_id = IDS_CALL_POP_HOLD_NOT_SUPPORTED;
		break;

	case ERROR_VOICECALL_RETREIVE_NOT_SUPPORTED:
		string_id = IDS_CALL_POP_UNHOLD_NOT_SUPPORTED;
		break;

	case ERROR_VOICECALL_SETUP_CONF_NOT_SUPPORTED:
		string_id = IDS_CALL_POP_JOIN_NOT_SUPPORTED;
		break;

	case ERROR_VOICECALL_SPLIT_CONF_NOT_SUPPORTED:
		string_id = IDS_CALL_POP_SPLIT_NOT_SUPPORTED;
		break;
	case ERROR_VOICECALL_TRANSFER_NOT_SUPPORTED:
		string_id = IDS_CALL_POP_TRANSFER_NOT_SUPPORTED;
		break;

	case ERROR_VOICECALL_INCOMPLETE:
		string_id = IDS_CALL_POP_INCOMPLETE;
		break;

	case ERROR_VOICECALL_UNAVAILABLE:
		string_id = IDS_CALL_POP_UNAVAILABLE;
		break;

	case ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED:
		string_id = IDS_CALL_POP_CALLFAILED;
		break;

	case ERROR_VOICECALL_INVALID_CALL_TYPE:
	case ERROR_VOICECALL_INVALID_TELEPHONE_NUMBER:
		string_id = IDS_CALL_POP_CAUSE_WRONG_NUMBER;
		break;

	default:
		CALL_ENG_DEBUG(ENG_DEBUG, " Invalid Error Code: %x", error_code);
		string_id = IDS_CALL_POP_CALLFAILED;
		break;
	}
	return string_id;
}

gboolean voicecall_core_set_to_default(call_vc_core_state_t *pcall_core)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "");
	if (pcall_core->pcall_engine == NULL)
		return FALSE;

	/*Set Engine states to default */
	_vc_core_engine_set_to_default_values(pcall_core->pcall_engine);

	/*Initialize MO Call Setup Info */
	pcall_core->call_setup_info.mo_call_index = VC_TAPI_INVALID_CALLHANDLE;
	pcall_core->call_setup_info.call_setup_by = VC_CALL_SETUP_BY_NORMAL;

	pcall_core->call_setup_info.mo_call_index = VC_TAPI_INVALID_CALLHANDLE;
	pcall_core->call_setup_info.call_setup_by = VC_CALL_SETUP_BY_NORMAL;
	memset(pcall_core->call_setup_info.tel_number, 0, sizeof(pcall_core->call_setup_info.tel_number));

	pcall_core->mo_redial_timer = -1;
	__vc_core_set_auto_redial_count(pcall_core, 0);

	_vc_core_util_set_call_status(VCONFKEY_CALL_OFF);
	voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_NONE, TRUE);

	pcall_core->bt_connected = _vc_bt_get_bt_status();
	if (FALSE == pcall_core->bt_connected) {
		voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER_EARJACK);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "BT connected, Not changing the sound status");
	}

	/* sound reset */
	voicecall_snd_unregister_cm(pcall_core->papp_snd);

	if (vconf_set_int(VCONFKEY_FACTORY_CALL_CONNECT_STATE, VCONFKEY_FACTORY_CALL_DISCONNECTED)) {
		CALL_ENG_DEBUG(ENG_ERR, "vconf_set_int failed.");
	}

	return TRUE;
}

static void __voicecall_core_mocall_reset_engine_state(voicecall_engine_t *pcall_engine)
{
	int eng_state = VC_INOUT_STATE_NONE;

	voicecall_core_get_engine_state(pcall_engine, &eng_state);

	CALL_ENG_DEBUG(ENG_DEBUG, "current engine state is: %d", eng_state);

	if ((eng_state > VC_INOUT_STATE_OUTGOING_START) && (eng_state < VC_INOUT_STATE_OUTGOING_END)) {
		voicecall_core_change_engine_state(pcall_engine, VC_INOUT_STATE_NONE);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Engine state is already changed, current engine state is: %d", eng_state);
		CALL_VC_DUMP_CALLDETAILS(&pcall_engine->call_manager);
	}
}

/**
 * This function initialize voicecall core
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		pcallback_func	callback function
 */
int voicecall_core_init(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = NULL;

	if (ERROR_VOICECALL_NONE != _vc_core_engine_init(&pcall_engine, (voicecall_cb) voicecall_core_cb, pcall_core)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Voicecall Engine Init Failed");
		return FALSE;
	}

	if (FALSE == voicecall_snd_init(pcall_core, &pcall_core->papp_snd)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "voicecall_snd_init() failed");
		return FALSE;
	}
	/*Store Voicecall Engine Handle */
	pcall_core->pcall_engine = pcall_engine;

	_vc_bt_status_init(pcall_core);

	_voicecall_dvc_earjack_init(pcall_core);

	voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_NONE, TRUE);
	return TRUE;
}

/**
 * This function prepares a voice call with the given data
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		bemergency		emergency call or not from dialer
 * @param[in]
 */
gboolean voicecall_core_setup_call(call_vc_core_state_t *pcall_core, gboolean bemergency)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;
	voicecall_setup_info_t setupcall_info = { 0, };
	voicecall_error_t error_code = -1;
	gboolean bemergency_call = FALSE;
	gboolean bmocall_exists = FALSE;
	gboolean bmtcall_exists = FALSE;
	int nw_status = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SOS_CALL_ONLY, bemergency);

	_vc_core_engine_status_isexists_call_bytype(pcall_engine, VC_OUTGOING_CALL, &bmocall_exists);
	if (TRUE == bmocall_exists) {
		CALL_ENG_DEBUG(ENG_DEBUG, "MO call is in progress...");
		voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
		return FALSE;
	}

	/*Ignore the MO Call, if already an Incoming call is in progress, MT Call is given high priority */
	_vc_core_engine_status_isexists_call_bytype(pcall_engine, VC_INCOMING_CALL, &bmtcall_exists);
	if (TRUE == bmtcall_exists) {
		CALL_ENG_DEBUG(ENG_DEBUG, "MT call is in progress");
		voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
		return FALSE;
	}

	if (TRUE == _vc_core_util_check_video_call_status()) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Voice call is not allowed during video call...");
		voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
		vc_engine_msg_box_type event_data;

		memset(&event_data, 0, sizeof(event_data));
		event_data.string_id = IDS_CALL_POP_VOICE_CALL_IS_NOT_ALLOWED_DURING_VIDEO_CALL;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);

		return FALSE;
	}

	if (TRUE == _vc_core_util_get_SAP_status()) {
		CALL_ENG_DEBUG(ENG_DEBUG, "SAP is on");
		voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
		vc_engine_msg_box_type event_data;

		memset(&event_data, 0, sizeof(event_data));
		event_data.string_id = IDS_CALL_POP_UNAVAILABLE;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);

		return FALSE;
	}

	/*Prpare a call with the Voicecall Engine */
	setupcall_info.call_type = pcall_core->call_setup_info.call_type;
	_vc_core_util_strcpy(setupcall_info.source_tel_number, sizeof(setupcall_info.source_tel_number), pcall_core->call_setup_info.source_tel_number);
	_vc_core_util_strcpy(setupcall_info.tel_number, sizeof(setupcall_info.tel_number), pcall_core->call_setup_info.tel_number);

	/*Get CUG Details */
	_vc_core_util_get_cug_info(&setupcall_info);

	/*Get Identity Mode */
	_vc_core_util_get_identity_mode(&setupcall_info);

	CALL_ENG_DEBUG(ENG_DEBUG, "identity_mode = [%d], tel_number = [%s]", setupcall_info.identity_mode, setupcall_info.tel_number);

	error_code = _vc_core_engine_prepare_call(pcall_engine, &setupcall_info);
	if (ERROR_VOICECALL_NONE != error_code) {
		CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_prepare_call failed, error code: %d", error_code);
		_vc_core_engine_finalize_call(pcall_engine, VC_OUTGOING_CALL, -1);
		voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
		vc_engine_msg_box_type event_data;

		memset(&event_data, 0, sizeof(event_data));
		event_data.string_id = __voicecall_core_get_string_id_by_errorcode(error_code);
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
		return FALSE;
	}
	pcall_core->call_setup_info.call_type = setupcall_info.call_type;
	CALL_ENG_DEBUG(ENG_DEBUG, "call_type:[%d]", pcall_core->call_setup_info.call_type);

	pcall_core->call_setup_info.mo_call_index = setupcall_info.mo_call_index;
	if (TRUE == _vc_core_util_is_offline_mode()) {
		_vc_core_engine_finalize_call(pcall_engine, VC_OUTGOING_CALL, -1);
		voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);

		vc_engine_msg_box_type event_data;

		memset(&event_data, 0, sizeof(event_data));
		event_data.string_id = IDS_CALL_POP_CHANGEOFFLINEMODETOCALL;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
		return FALSE;
	}

	/* Check for Following Conditions , only if not emergency Number */
	CALL_ENG_DEBUG(ENG_DEBUG, "mo_call_index = [%d]", setupcall_info.mo_call_index);
	error_code = _vc_core_engine_status_check_emergency_byindex(pcall_engine, setupcall_info.mo_call_index, &bemergency_call);
	if (ERROR_VOICECALL_NONE != error_code) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Emergency Check Error code: %d", error_code);
	}

	/*Check PwLock */
	if (TRUE == _vc_core_util_is_pwlock()) {
		CALL_ENG_DEBUG(ENG_DEBUG, "PwLock is enabled.");
		if (bemergency_call == FALSE) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Only emergency call is possible.");
			_vc_core_engine_finalize_call(pcall_engine, VC_OUTGOING_CALL, -1);
			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);

			vc_engine_msg_box_type event_data;
			memset(&event_data, 0, sizeof(event_data));
			event_data.string_id = IDS_CALL_POP_CALLING_EMERG_ONLY;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);

			return FALSE;
		}
	}

	/*Check for the following cases, if the call is not emergency call */
	if (FALSE == bemergency_call) {
		vc_engine_msg_box_type event_data;

		memset(&event_data, 0, sizeof(event_data));
		/* Check for NW Status and Emergency Mode */
		if (FALSE == _vc_core_util_get_nw_status(&nw_status)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "Can't get a network status...");
			_vc_core_engine_finalize_call(pcall_engine, VC_OUTGOING_CALL, -1);

			event_data.string_id = IDS_CALL_POP_CALLNOTCALLOWED;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);

			voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
			return FALSE;
		} else {
			if ((VCONFKEY_TELEPHONY_SVCTYPE_NONE == nw_status) || (VCONFKEY_TELEPHONY_SVCTYPE_NOSVC == nw_status) || (VCONFKEY_TELEPHONY_SVCTYPE_SEARCH == nw_status)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "No Service: Call not Allowed");
				_vc_core_engine_finalize_call(pcall_engine, VC_OUTGOING_CALL, -1);
				event_data.string_id = IDS_CALL_POP_CALLNOTCALLOWED;
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);

				voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
				return FALSE;
			} else if ((VCONFKEY_TELEPHONY_SVCTYPE_EMERGENCY == nw_status)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Emergency mode: Emergency call only...");
				_vc_core_engine_finalize_call(pcall_engine, VC_OUTGOING_CALL, -1);
				event_data.string_id = IDS_CALL_POP_CALLING_EMERG_ONLY;
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);

				voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_SETUPCALL_FAIL, TRUE);
				return FALSE;
			} else {
				/* ok. */
			}

		}

		/*Check for voicemail number if it is not an emergency call */
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "MO Call Setup OVer");

	if (_vc_bt_get_bt_status() == TRUE) {
		voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_BT);
	}

	voicecall_core_make_call(pcall_core);

	return TRUE;
}

/**
 * This function makes the actual voicecall prepared by the #voicecall_core_setup_call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_make_call(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	voicecall_error_t error_code = ERROR_VOICECALL_NONE;
	int call_handle = -1;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	/*Make Actual Call with Voicecall Engine */
	error_code = _vc_core_engine_make_call(pcall_engine, pcall_core->call_setup_info.mo_call_index, &call_handle);
	if (ERROR_VOICECALL_NONE != error_code) {
		vc_engine_msg_box_type event_data;

		memset(&event_data, 0, sizeof(event_data));

		_vc_core_engine_finalize_call(pcall_engine, VC_OUTGOING_CALL, -1);

		CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_make_call Failed: Error_Code: %d..", error_code);

		switch (error_code) {
		case ERROR_VOICECALL_TAPI_CAUSE_CALL_FAILED:
			event_data.string_id = IDS_CALL_POP_CALLFAILED;
			break;

		case ERROR_VOICECALL_CALL_IMPOSSIBLE_NOSIM_NOEMERGNUM:
			event_data.string_id = IDS_CALL_POP_CALLFAILED;
			break;

		default:
			event_data.string_id = IDS_CALL_BODY_CALLENDED;
			break;
		}
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);

		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "Call Handle After Setup Call %d.", call_handle);

	voicecall_snd_register_cm(pcall_core->papp_snd);

	/* Set phonestatus value */
	_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_CONNECTING);

	return TRUE;
}

/**
 * This function processed sat setup call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]
*/
gboolean voicecall_core_process_sat_setup_call(vcall_engine_sat_setup_call_info_t *sat_setup_call_info)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "");
	TelSatSetupCallIndCallData_t *SatSetupCallIndCallInfo = NULL;

	SatSetupCallIndCallInfo = (TelSatSetupCallIndCallData_t *) calloc(1, sizeof(TelSatSetupCallIndCallData_t));
	if (NULL == SatSetupCallIndCallInfo)
		return FALSE;

	SatSetupCallIndCallInfo->commandId = sat_setup_call_info->command_id;
	SatSetupCallIndCallInfo->calltype = sat_setup_call_info->command_qualifier;
	SatSetupCallIndCallInfo->dispText.stringLen = strlen(sat_setup_call_info->disp_text);
	memcpy(SatSetupCallIndCallInfo->dispText.string, sat_setup_call_info->disp_text, strlen(sat_setup_call_info->disp_text));

	SatSetupCallIndCallInfo->callNumber.stringLen = strlen(sat_setup_call_info->call_num);
	memcpy(SatSetupCallIndCallInfo->callNumber.string, sat_setup_call_info->call_num, strlen(sat_setup_call_info->call_num));

	SatSetupCallIndCallInfo->duration = sat_setup_call_info->duration;

	_vc_core_engine_handle_sat_events_cb(SatSetupCallIndCallInfo, NULL);

	if (SatSetupCallIndCallInfo) {
		free(SatSetupCallIndCallInfo);
		SatSetupCallIndCallInfo = NULL;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "End..");

	return TRUE;
}

/**
 * This function processed incoming call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]
*/
gboolean voicecall_core_process_incoming_call(call_vc_core_incoming_info_t *incoming_call_info)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "");
	TelCallIncomingCallInfo_t *IncomingCallInfo = NULL;

	CALL_ENG_DEBUG(ENG_DEBUG, " number is : [%s]", incoming_call_info->call_num);

	IncomingCallInfo = (TelCallIncomingCallInfo_t *) malloc(sizeof(TelCallIncomingCallInfo_t));
	if (NULL == IncomingCallInfo)
		return FALSE;
	memset(IncomingCallInfo, 0, sizeof(IncomingCallInfo));

	IncomingCallInfo->CallHandle = incoming_call_info->call_handle;
	IncomingCallInfo->CallType = incoming_call_info->call_type;
	IncomingCallInfo->CliPresentationIndicator = incoming_call_info->cli_presentation_indicator;
	_vc_core_util_strcpy(IncomingCallInfo->szCallingPartyNumber, sizeof(IncomingCallInfo->szCallingPartyNumber), incoming_call_info->call_num);
	IncomingCallInfo->CallingNameInfo.NameMode = incoming_call_info->calling_name_mode;
	_vc_core_util_strcpy(IncomingCallInfo->CallingNameInfo.szNameData, sizeof(IncomingCallInfo->CallingNameInfo.szNameData), incoming_call_info->calling_name);
	_vc_core_util_strcpy(IncomingCallInfo->RedirectInfo.szRedirectedNumber, sizeof(IncomingCallInfo->RedirectInfo.szRedirectedNumber), incoming_call_info->redirected_number);
	_vc_core_util_strcpy(IncomingCallInfo->RedirectInfo.szRedirectSubAddress, sizeof(IncomingCallInfo->RedirectInfo.szRedirectSubAddress), incoming_call_info->redirected_sub_address);
	IncomingCallInfo->CliCause = incoming_call_info->cli_cause;
	IncomingCallInfo->fwded = incoming_call_info->bfwded;
	IncomingCallInfo->ActiveLine = incoming_call_info->active_line;

	_vc_core_engine_handle_incoming_tapi_events(IncomingCallInfo, NULL);

	if (IncomingCallInfo) {
		free(IncomingCallInfo);
		IncomingCallInfo = NULL;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "End..");

	return TRUE;
}

/**
 * This function answers an incoming call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]
*/
gboolean voicecall_core_answer_call(call_vc_core_state_t *pcall_core, gboolean auto_accept)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	gboolean active_calls = FALSE;
	gboolean held_calls = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	/*First Stop the Incoming alert */
	voicecall_snd_stop_alert(pcall_core->papp_snd);

	__voicecall_core_cancel_auto_answer(pcall_core);

	_vc_core_engine_status_isexists_any_call(pcall_engine, &active_calls, &held_calls);
	CALL_ENG_DEBUG(ENG_DEBUG, "active_calls=%d, held_calls=%d", active_calls, held_calls);

	if (TRUE == active_calls && TRUE == held_calls) {
		/* Both Active and held calls available, so show Accept Call Choice Box */
		vc_engine_accept_choice_box_type event_data;

		memset(&event_data, 0, sizeof(event_data));
		event_data.choice = VC_CALL_ACCEPT_2;

		vcall_engine_send_event_to_client(VC_ENGINE_MSG_ACCEPT_CHOICE_BOX_TO_UI, (void *)&event_data);

		return TRUE;
	} else if (TRUE == active_calls) {
		/*If Auto Accpet is FALSE, show popup for manual accept */
		if (FALSE == auto_accept) {
			/* Active  calls available, so show Accept Call Choice Box */
			vc_engine_accept_choice_box_type event_data;

			memset(&event_data, 0, sizeof(event_data));
			event_data.choice = VC_CALL_ACCEPT_1;

			vcall_engine_send_event_to_client(VC_ENGINE_MSG_ACCEPT_CHOICE_BOX_TO_UI, (void *)&event_data);
		} else {
			voicecall_core_answer_call_bytype(pcall_core, VC_ANSWER_HOLD_ACTIVE_AND_ACCEPT);
		}
		return TRUE;
	} else if (TRUE == held_calls) {
		/* vcui_app_view_mtcall_destroy(papp_document); */
	}

	/* Normal Call Scenario */
	voicecall_core_answer_call_bytype(pcall_core, VC_ANSWER_NORMAL);

	return TRUE;
}

/**
 * This function answers an incoming call  according to the given type
 *
 * @return		Returns TRUE -if answer is sucess, FALSE - otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_answer_call_bytype(call_vc_core_state_t *pcall_core, voicecall_answer_type_t answer_type)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	voicecall_error_t error_code = ERROR_VOICECALL_NONE;

	CALL_ENG_DEBUG(ENG_DEBUG, "answer type: %d", answer_type);

	/*First Stop the Incoming alert */
	voicecall_snd_stop_alert(pcall_core->papp_snd);
	__voicecall_core_cancel_auto_answer(pcall_core);

	error_code = _vc_core_engine_answer_call(pcall_engine, answer_type);

	if (ERROR_VOICECALL_NONE != error_code) {
		CALL_ENG_DEBUG(ENG_ERR, "_vc_core_engine_answer_call Failed : %d", error_code);
		return FALSE;
	}

	return TRUE;
}

/**
* This function rejects an incoming call
*
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pcall_core		Handle to voicecall core
* @param[in]		bUDUB			TRUE - set UDUB, FALSE - reject call
*/
gboolean voicecall_core_reject_mt(call_vc_core_state_t *pcall_core, gboolean bUDUB)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;
	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;

	voicecall_error_t error_code = ERROR_VOICECALL_NONE;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	/*Incoming call rejected, reset the accept by flag */
	voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_ACCEPT_BY_BT, FALSE);

	/* Stop Incmoing Call Alert */
	voicecall_snd_stop_alert(papp_snd);

	__voicecall_core_cancel_auto_answer(pcall_core);

	CALL_ENG_DEBUG(ENG_DEBUG, "bUDUB = %d", bUDUB);
	error_code = _vc_core_engine_reject_call(pcall_engine, bUDUB);

	if (ERROR_VOICECALL_NONE != error_code) {
		CALL_ENG_DEBUG(ENG_ERR, "_vc_core_engine_reject_call Failed, error_code = %ud", error_code);
		return FALSE;
	}

	return TRUE;

}

/**
 * This function ends the call by state
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_end_call(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	return (ERROR_VOICECALL_NONE == _vc_core_engine_end_call(pcall_engine, VC_END_ACTIVE_OR_HELD_CALLS)) ? TRUE : FALSE;
}

/**
 * This function ends the call corresponding to the given call handle
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		call_handle		handle of the call to be ended
 */
gboolean voicecall_core_end_call_by_handle(call_vc_core_state_t *pcall_core, int call_handle)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	return (ERROR_VOICECALL_NONE == _vc_core_engine_end_call_byhandle(pcall_engine, call_handle)) ? TRUE : FALSE;
}

/**
 * This function ends all available calls
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_end_all_calls(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	return (ERROR_VOICECALL_NONE == _vc_core_engine_end_call(pcall_engine, VC_END_ALL_CALLS)) ? TRUE : FALSE;
}

/**
 * This function ends all available active calls
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_end_all_active_calls(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	return (ERROR_VOICECALL_NONE == _vc_core_engine_end_call(pcall_engine, VC_END_ALL_ACTIVE_CALLS)) ? TRUE : FALSE;
}

/**
 * This function ends all available held calls
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_end_all_held_calls(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	return (ERROR_VOICECALL_NONE == _vc_core_engine_end_call(pcall_engine, VC_END_ALL_HELD_CALLS)) ? TRUE : FALSE;
}

/**
 * This function cancel outgoing call
 *
 * @return		Returns TRUE -if answer is sucess, FALSE - otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_cancel_call(call_vc_core_state_t *pcall_core)
{
	int io_state = 0;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	voicecall_call_type_bysetup_t call_setup_by = VC_CALL_SETUP_BY_NORMAL;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	/*Get Call Setup by Information */
	call_setup_by = pcall_core->call_setup_info.call_setup_by;

	_vc_core_engine_status_get_engine_iostate(pcall_engine, &io_state);

	switch (io_state) {
	case VC_INOUT_STATE_OUTGOING_WAIT_HOLD:
		{
			int call_handle = -1;

			if (VC_CALL_SETUP_BY_SAT == call_setup_by) {
				_vc_core_engine_send_sat_response(pcall_engine, SAT_RQST_SETUP_CALL, CALL_VC_ME_CLEAR_DOWN_BEFORE_CONN);
			}

			/* Clear the MO Call, since the call is not dialed yet */
			_vc_core_engine_status_get_call_handle_bytype(pcall_engine, VC_OUTGOING_CALL, &call_handle);
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_END, call_handle, NULL);

			__vc_core_set_auto_redial_count(pcall_core, 0);
			voicecall_core_clear_mo_call(pcall_engine);

			_vc_core_engine_change_engine_iostate(pcall_engine, VC_INOUT_STATE_OUTGOING_ABORTED);
		}
		return TRUE;

	case VC_INOUT_STATE_OUTGOING_WAIT_ORIG:
	case VC_INOUT_STATE_OUTGOING_WAIT_ALERT:
	case VC_INOUT_STATE_OUTGOING_WAIT_CONNECTED:
		{
			/*To retrieve the held call automatically once the mo call is ended, if held is call is available */
			_vc_core_engine_status_set_end_flag(pcall_engine, VC_RETREIVE_CALL_ON_MOCALL_END);

			/* release the call , since it is dialed and waiting for connecting */
			if (FALSE == voicecall_core_end_mo_call(pcall_engine)) {
				CALL_ENG_DEBUG(ENG_DEBUG, "MO Call Release Failed");
			} else {
				voicecall_core_set_status(pcall_core, CALL_VC_CORE_FLAG_MOCALL_END_BY_USER, TRUE);
				_vc_core_engine_change_engine_iostate(pcall_engine, VC_INOUT_STATE_OUTGOING_WAIT_RELEASE);
			}
		}
		return TRUE;

	case VC_INOUT_STATE_OUTGOING_WAIT_RELEASE:
		{
			/*Call already released */
			CALL_ENG_DEBUG(ENG_DEBUG, "MO Call has been released already");
		}
		return TRUE;

	case VC_INOUT_STATE_OUTGOING_SHOW_REDIALCAUSE:
	case VC_INOUT_STATE_OUTGOING_ABORTED:
	case VC_INOUT_STATE_OUTGOING_WAIT_REDIAL:
		{
			int call_handle = -1;

			if (VC_CALL_SETUP_BY_SAT == call_setup_by) {
				_vc_core_engine_send_sat_response(pcall_engine, SAT_RQST_SETUP_CALL, CALL_VC_NETWORK_UNABLE_TO_PROCESS_COMMAND);
			}

			/*Stop Signal Play */
			voicecall_snd_stop_signal(pcall_core->papp_snd);

			__voicecall_core_mocall_reset_engine_state(pcall_engine);

			_vc_core_engine_status_get_call_handle_bytype(pcall_engine, VC_OUTGOING_CALL, &call_handle);
			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_END, call_handle, NULL);

			__vc_core_set_auto_redial_count(pcall_core, 0);
			voicecall_core_clear_mo_call(pcall_engine);

		}
		return TRUE;

	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Action not defined for this IO State: %d", io_state);
		break;
	}

	return FALSE;
}

/**
 * This function process hold/retrive/swap conntected call
 *
 * @return		Returns TRUE -if answer is sucess, FALSE - otherwise
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_process_hold_call(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	gboolean active_calls = FALSE;
	gboolean held_calls = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	if (TRUE == voicecall_core_get_status(pcall_core, CALL_VC_CORE_FLAG_SOS_CALL_ONLY)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "SOS Call... Ignore this button...");
		return TRUE;
	}

	if (FALSE == voicecall_core_is_incall_request_possible(pcall_engine)) {
		return TRUE;
	}

	voicecall_core_is_call_exists(pcall_engine, &active_calls, &held_calls);
	CALL_ENG_DEBUG(ENG_DEBUG, "active calls: %d, held calls: %d", active_calls, held_calls);

	if (active_calls && held_calls) {

		/*Both Calls available, swap the calls */
		if (FALSE == voicecall_core_swap_calls(pcall_engine)) {
			return FALSE;
		}
	} else if (active_calls) {
		/*Only activa call available, hold the call */
		if (FALSE == voicecall_core_hold_call(pcall_engine)) {
			return FALSE;
		}
	} else if (held_calls) {
		/*Only Held call available, retrieve the call */
		if (FALSE == voicecall_core_retrieve_call(pcall_engine)) {
			return FALSE;
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "no call exists!");
		return FALSE;
	}

	return TRUE;
}

/**
 * This function sets up a conference call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_setup_conference(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	return (ERROR_VOICECALL_NONE == _vc_core_engine_setup_conference(pcall_engine)) ? TRUE : FALSE;
}

/**
 * This function splits the call corressponding to the given call handle and makes a private call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		call_handle		Call to be splitted from the conference call
 */
gboolean voicecall_core_make_private_call(call_vc_core_state_t *pcall_core, int call_handle)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	return (ERROR_VOICECALL_NONE == _vc_core_engine_private_call(pcall_engine, call_handle)) ? TRUE : FALSE;
}

/**
 * This function transfers the call from active call to the held call
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_core_transfer_calls(call_vc_core_state_t *pcall_core)
{
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	return (ERROR_VOICECALL_NONE == _vc_core_engine_transfer_calls(pcall_engine)) ? TRUE : FALSE;
}

/**
 * This function sends a dtmf string
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		dtmf_string		dtmf string to be sent
 */
gboolean voicecall_core_send_dtmf(call_vc_core_state_t *pcall_core, char *dtmf_string)
{
	voicecall_error_t error_code = ERROR_VOICECALL_NONE;
	CALL_ENG_DEBUG(ENG_DEBUG, "dtmf string: %s", dtmf_string);

	pcall_core->bdtmf_queue = FALSE;
	error_code = _vc_core_engine_send_dtmf(pcall_core->pcall_engine, dtmf_string);
	return (ERROR_VOICECALL_NONE == error_code) ? TRUE : FALSE;
}

/**
* This function stops sound alert in case of reject with msg
*
* @return		Returns TRUE on success or FALSE on failure
* @param[in]		pcall_core		Handle to voicecall core
*/
gboolean voicecall_core_stop_alert(call_vc_core_state_t *pcall_core)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "");

	/* Stop Incmoing Call Alert */
	voicecall_snd_stop_alert(pcall_core->papp_snd);

	return TRUE;
}

inline gboolean voicecall_core_get_mo_call_handle(call_vc_core_state_t *pcall_core, int *pcall_handle)
{
	*pcall_handle = -1;
	return (ERROR_VOICECALL_NONE == _vc_core_engine_status_get_call_handle_bytype(pcall_core->pcall_engine, VC_OUTGOING_CALL, pcall_handle)) ? TRUE : FALSE;
}

inline int voicecall_core_get_auto_redial_count(call_vc_core_state_t *pcall_core)
{
	return pcall_core->auto_redial_count;
}

void __vc_core_set_auto_redial_count(call_vc_core_state_t *pcall_core, int auto_redial_count)
{
	pcall_core->auto_redial_count = auto_redial_count;
}

gboolean voicecall_core_start_redial(call_vc_core_state_t *pcall_core, int manual_redial)
{
	int auto_redial_status = FALSE;
	int redial_count = 1;
	int call_handle = -1;
	int total_call_member = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "");
	CALL_ENG_DEBUG(ENG_DEBUG, "auto_redial_status:[%d]", auto_redial_status);

	redial_count = voicecall_core_get_auto_redial_count(pcall_core);

	redial_count++;

	/*
	   bmanual_redial == TRUE : Redial is made by User, No need to check the auto redial status and count
	   auto_redial_status == 1: Auto Redial for GCF case, auto redial count must be checked
	 */
	if ((1 == manual_redial) || ((auto_redial_status == 1) && (redial_count < MO_REDIAL_COUNT_MAX))) {
		CALL_ENG_DEBUG(ENG_DEBUG, "redial_count = %d", redial_count);

		/*Start Redial Timer */
		if (1 == redial_count) {
			CALL_ENG_DEBUG(ENG_DEBUG, "MO_REDIAL_TIMER_INTERVAL_FIRST");
			pcall_core->mo_redial_timer = g_timeout_add(MO_REDIAL_TIMER_INTERVAL_FIRST, mo_redial_timer_cb, pcall_core);
		} else if ((redial_count > 1) && (redial_count < 5)) {
			CALL_ENG_DEBUG(ENG_DEBUG, "MO_REDIAL_TIMER_INTERVAL_SECOND");
			pcall_core->mo_redial_timer = g_timeout_add(MO_REDIAL_TIMER_INTERVAL_SECOND, mo_redial_timer_cb, pcall_core);
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "MO_REDIAL_TIMER_INTERVAL_THIRD");
			pcall_core->mo_redial_timer = g_timeout_add(MO_REDIAL_TIMER_INTERVAL_THIRD, mo_redial_timer_cb, pcall_core);
		}

		voicecall_core_change_engine_state(pcall_core->pcall_engine, VC_INOUT_STATE_OUTGOING_WAIT_REDIAL);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "You dont need to redial close the MO Call Things");

		/* __vcui_app_view_mo_canceltimer_cb() */
		_vc_core_engine_status_get_call_handle_bytype(pcall_core->pcall_engine, VC_OUTGOING_CALL, &call_handle);
		_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_END, call_handle, NULL);

		__vc_core_set_auto_redial_count(pcall_core, 0);
		voicecall_core_clear_mo_call(pcall_core->pcall_engine);

		__voicecall_core_mocall_reset_engine_state(pcall_core->pcall_engine);

		voicecall_core_get_total_call_member(pcall_core->pcall_engine, &total_call_member);

		voicecall_snd_stop_signal(pcall_core->papp_snd);

		/* If No Connected Calls End the UI */
		if (total_call_member == 0) {
			/*Reset voice call core to default values */
			voicecall_core_set_to_default(pcall_core);
		} else {
			voicecall_snd_change_path(pcall_core->papp_snd);
		}

	}

	return TRUE;
}

inline gboolean voicecall_core_prepare_redial(call_vc_core_state_t *pcall_core, int call_handle)
{
	voicecall_error_t error_code = ERROR_VOICECALL_NONE;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	error_code = _vc_core_engine_prepare_redial(pcall_core->pcall_engine, call_handle);

	CALL_ENG_DEBUG(ENG_DEBUG, "_vc_core_engine_prepare_redial returned : %d", error_code);

	return (ERROR_VOICECALL_NONE == error_code) ? TRUE : FALSE;
}

gboolean mo_redial_timer_cb(void *data)
{
	int call_handle = -1;
	int redial_count;

	CALL_ENG_DEBUG(ENG_DEBUG, "mo_redial_timer_cb");
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)data;
	voicecall_core_get_mo_call_handle(pcall_core, &call_handle);
	CALL_ENG_DEBUG(ENG_DEBUG, "call handle is %d", call_handle);

	redial_count = voicecall_core_get_auto_redial_count(pcall_core);
	redial_count++;
	__vc_core_set_auto_redial_count(pcall_core, redial_count);

	CALL_ENG_DEBUG(ENG_DEBUG, "redial_count:[%d]", redial_count);

	voicecall_core_prepare_redial(pcall_core, call_handle);
	CALL_ENG_DEBUG(ENG_DEBUG, "voicecall_core_prepare_redial done.");

	voicecall_core_make_call(pcall_core);
	return FALSE;
}

gboolean voicecall_core_stop_redial(call_vc_core_state_t *pcall_core)
{
	int call_handle = -1, total_call_member = -1;

	CALL_ENG_DEBUG(ENG_DEBUG, "coming inside voicecall_core_stop_redial");

	_vc_core_util_set_call_status(VCONFKEY_CALL_OFF);

	/*Cancel the Redial Timer */
	if (pcall_core->mo_redial_timer != -1) {
		CALL_ENG_DEBUG(ENG_DEBUG, "mo_redial_timer removing..");
		g_source_remove(pcall_core->mo_redial_timer);
		pcall_core->mo_redial_timer = -1;
	}

	/* __vcui_app_view_mo_canceltimer_cb() */
	_vc_core_engine_status_get_call_handle_bytype(pcall_core->pcall_engine, VC_OUTGOING_CALL, &call_handle);
	_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_CALL_END, call_handle, NULL);

	__vc_core_set_auto_redial_count(pcall_core, 0);
	voicecall_core_clear_mo_call(pcall_core->pcall_engine);

	__voicecall_core_mocall_reset_engine_state(pcall_core->pcall_engine);

	voicecall_core_get_total_call_member(pcall_core->pcall_engine, &total_call_member);

	voicecall_snd_stop_signal(pcall_core->papp_snd);

	/* If No Connected Calls End the UI */
	if (total_call_member == 0) {
		/*Reset voice call core to default values */
		voicecall_core_set_to_default(pcall_core);
	} else {
		voicecall_snd_change_path(pcall_core->papp_snd);
	}
	return TRUE;

}

static gboolean __voicecall_core_auto_answer_timer_cb(gpointer puser_data)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)puser_data;

	if (pcall_core->auto_answer_timer > 0) {
		g_source_remove(pcall_core->auto_answer_timer);
		pcall_core->auto_answer_timer = 0;
	}

	/*Check for Incoming call and then answer the call */
	if (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Call Answered");
		voicecall_core_answer_call(pcall_core, TRUE);
	}

	return FALSE;
}

#if 0
static gboolean __voicecall_core_callstatus_set_timer_cb(gpointer puser_data)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)puser_data;

	if (pcall_core->callstatus_timer > 0) {
		g_source_remove(pcall_core->callstatus_timer);
		pcall_core->callstatus_timer = 0;
	}

	_vc_core_util_set_call_status(VCONFKEY_CALL_VOICE_CONNECTING);

	return FALSE;
}
#endif

static gboolean __voicecall_core_auto_answer_idle_cb(gpointer puser_data)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)puser_data;
	int engine_state = 0;
	int auto_answer_time = 0, ret = -1;
	unsigned long auto_answer_time_interval = 0;

	_vc_core_engine_status_get_engine_iostate(pcall_core->pcall_engine, &engine_state);
	CALL_ENG_DEBUG(ENG_DEBUG, "eng_state : %d", engine_state);
	if (engine_state != VC_INOUT_STATE_INCOME_BOX) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Engine State not in Income Box, Current State: %d", engine_state);
		return FALSE;
	}
	/* Read the time interval from gconf and set the redial timer  */
	ret = vconf_get_int(VCONFKEY_CISSAPPL_ANSWERING_MODE_TIME_INT, &auto_answer_time);
	if (0 == ret) {
		auto_answer_time_interval = auto_answer_time * 1000;
		CALL_ENG_DEBUG(ENG_DEBUG, "The time interval is : %ld", auto_answer_time_interval);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "vconf_get_int FAILED");
		return FALSE;
	}

	pcall_core->auto_answer_timer = g_timeout_add(auto_answer_time_interval, __voicecall_core_auto_answer_timer_cb, pcall_core);
	return FALSE;
}

/**
* This function checks whether given answer mode is enabled or not
*
* @return		returns TRUE if given answer mode type is enabled in the settings or FALSE otherwise
*
*/
static gboolean __vc_core_is_answermode_enabled_from_testmode(void)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "..");
	int benabled = -1;
	int ret = -1;

	ret = vconf_get_int(VCONFKEY_TESTMODE_AUTO_ANSWER, &benabled);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "benabled = %d", benabled);
		if (TRUE == benabled)
			return TRUE;
		else
			return FALSE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "vconf_get_int FAILED");
		return FALSE;
	}
}

/**
* This function checks whether given answer mode is enabled or not
*
* @return		returns TRUE if given answer mode type is enabled in the settings or FALSE otherwise
*
*/
static gboolean __vc_core_is_answermode_enabled(void)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "coming inside voicecall_is_answermode_enabled");
	int answer_mode_enabled = -1;
	int ret = -1;

	ret = vconf_get_int(VCONFKEY_CISSAPPL_ANSWERING_MODE_INT, &answer_mode_enabled);
	if (0 == ret) {
		CALL_ENG_DEBUG(ENG_DEBUG, "answer_mode_enabled = %d", answer_mode_enabled);
		if (2 == answer_mode_enabled)	/* here 2 is auto answer mode is enabled */
			return TRUE;
		else
			return FALSE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "vconf_get_int FAILED");
		return FALSE;
	}

}

/**
* This function processes auto answer request
*
* @return		Returns void
* @param[in]		pcall_core		Handle to voicecall core
*/
static void __voicecall_core_start_auto_answer(call_vc_core_state_t *pcall_core, gboolean isTestMode)
{
	gboolean earjack_connected = FALSE;
	gboolean headset_connected = FALSE;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	if (TRUE == isTestMode) {
		CALL_ENG_DEBUG(ENG_DEBUG, "In case Of Testmode, always auto answer enabled");

		g_idle_add(__voicecall_core_auto_answer_idle_cb, pcall_core);
		return;
	}
	earjack_connected = _voicecall_dvc_get_earjack_connected();

	if (TRUE == earjack_connected) {
		CALL_ENG_DEBUG(ENG_DEBUG, "earjack is connected");
	}
	headset_connected = _vc_bt_get_bt_status();

	if (TRUE == headset_connected) {
		CALL_ENG_DEBUG(ENG_DEBUG, "headset is connected");
	}

	if (TRUE == earjack_connected || TRUE == headset_connected) {
		g_idle_add(__voicecall_core_auto_answer_idle_cb, pcall_core);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Without earjack or headset, skip auto answer ");
		return;
	}
}

/**
* This function cancels the auto answering timer
*
* @return		void
* @param[in]		pcall_core		Handle to voicecall core
*/
static void __voicecall_core_cancel_auto_answer(call_vc_core_state_t *pcall_core)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "coming inside __voicecall_cancel_auto_answer");
	if (pcall_core->auto_answer_timer > 0) {
		g_source_remove(pcall_core->auto_answer_timer);
		pcall_core->auto_answer_timer = 0;
	}
}

#if 0
/**
* This function checks BT headset and Earjack status
*
* @return		void
* @param[in]		pcall_core		Handle to voicecall core
*/
static void __voicecall_core_check_headset_earjack_status(call_vc_core_state_t *pcall_core)
{
	gboolean bt_connected = FALSE;

	bt_connected = _vc_bt_get_bt_status();
	CALL_ENG_DEBUG(ENG_DEBUG, "Bt connected =%d", bt_connected);

	pcall_core->bt_connected = bt_connected;

	CALL_ENG_DEBUG(ENG_DEBUG, "Update the earjack status");
	_voicecall_dvc_get_earjack_status(pcall_core);
}
#endif

/**
 * This function parses the in call supplementary services string and returns the in call ss to be used
 *
 * @return		Returns in call ss state #vcui_app_incall_ss_state_t
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		number			number to be parsed
 */
call_vc_core_incall_ss_state_t voicecall_core_parse_incall_ss_string(call_vc_core_state_t *pcall_core, char *number)
{
	call_vc_core_incall_ss_state_t ss_value = CALL_VC_CORE_SS_USSD;

	if (strlen(number) == 1) {
		switch (number[0]) {
		case '0':
			ss_value = CALL_VC_CORE_SS_0;
			break;
		case '1':
			ss_value = CALL_VC_CORE_SS_1;
			break;
		case '2':
			ss_value = CALL_VC_CORE_SS_2;
			break;
		case '3':
			ss_value = CALL_VC_CORE_SS_3;
			break;
		case '4':
			ss_value = CALL_VC_CORE_SS_4;
			break;
		default:
			ss_value = CALL_VC_CORE_SS_USSD;
			break;
		}
	} else if (strlen(number) == 2) {
		if ((number[0] == '1') && (number[1] > '0') && (number[1] < '8')) {
			pcall_core->selected_call_id_in_ss = atoi(number + 1);
			ss_value = CALL_VC_CORE_SS_1X;
		}

		if ((number[0] == '2') && (number[1] > '0') && (number[1] < '8')) {
			pcall_core->selected_call_id_in_ss = atoi(number + 1);
			ss_value = CALL_VC_CORE_SS_2X;
		}
	}
	CALL_ENG_DEBUG(ENG_DEBUG, "ss parsed value: %d", ss_value);
	return ss_value;
}

/**
 * This function starts the supplementary services while on call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		incall_ss_state		state of the In Call Supplementary Service
 */
gboolean voicecall_core_start_incall_ss(call_vc_core_state_t *pcall_core, call_vc_core_incall_ss_state_t incall_ss_state)
{
	gboolean active_calls = FALSE, held_calls = FALSE;

	_vc_core_engine_status_isexists_any_call(pcall_core->pcall_engine, &active_calls, &held_calls);

	CALL_ENG_DEBUG(ENG_DEBUG, "ss state = %d", incall_ss_state);
	vc_engine_msg_box_type event_data;
	memset(&event_data, 0, sizeof(event_data));
	event_data.string_id = IDS_CALL_POP_OPERATION_REFUSED;

	/*Cancel DTMF Sending if any and close the dtmf ui */
	/* vcui_app_doc_cancel_dtmf_queue(papp_document); sathwick TBD */

	switch (incall_ss_state) {
		/* Releases all held calls or Set UDUB(User Determined User Busy) for a waiting call */
	case CALL_VC_CORE_SS_0:
		{
			/* if an incoming call is activated, reject the incoming all  */
			if (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
				voicecall_core_reject_mt(pcall_core, TRUE);
				return TRUE;
			} else if (held_calls) {
				voicecall_core_end_all_held_calls(pcall_core);
			} else {
				CALL_ENG_DEBUG(ENG_DEBUG, "There are no held calls to do the processing");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
			}
		}
		break;
	case CALL_VC_CORE_SS_1:
		{
			if (voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
				/* Accept incoming call */
				voicecall_core_answer_call_bytype(pcall_core, VC_ANSWER_RELEASE_ACTIVE_AND_ACCEPT);
				return TRUE;
			} else if (voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine)) {
				/* This fucntion checks for held calls to be retreived on response of ending call */
				voicecall_core_set_check_ss_on_end(pcall_core);
				voicecall_core_end_mo_call(pcall_core->pcall_engine);
			} else if (active_calls) {
				voicecall_core_end_all_active_calls(pcall_core);
				voicecall_core_set_check_ss_on_end(pcall_core);
			} else if (held_calls) {
				_vc_core_engine_retrieve_call(pcall_core->pcall_engine);
			} else {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
			}
		}
		break;
	case CALL_VC_CORE_SS_1X:
		{
			if (voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
			} else if (active_calls) {

				if (!(ERROR_VOICECALL_NONE == _vc_core_engine_end_call_bycallId(pcall_core->pcall_engine, pcall_core->selected_call_id_in_ss))) {

					CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
					vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
				}
			} else {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
			}
		}
		break;
	case CALL_VC_CORE_SS_2:
		{

			if (voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {
				if (active_calls && held_calls) {

					CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
					vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
				} else {
#ifdef _CPHS_DEFINED_		/* Not used currently */
					if (TRUE == active_calls && (voicecall_core_get_cphs_csp_status(pcall_core->pcall_engine, VC_CPHS_CSP_HOLD))) {

						CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
						vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
						break;
					}
#endif
					voicecall_core_answer_call(pcall_core, TRUE);
				}
			} else if (voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine)) {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
			} else if (active_calls && held_calls) {
				_vc_core_engine_swap_calls(pcall_core->pcall_engine);
			} else if (active_calls) {
				_vc_core_engine_hold_call(pcall_core->pcall_engine);
			} else if (held_calls) {
				_vc_core_engine_retrieve_call(pcall_core->pcall_engine);
			} else {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
			}
		}
		break;
	case CALL_VC_CORE_SS_2X:
		{
			if (voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine) || voicecall_core_is_outgoing_call_exists(pcall_core->pcall_engine)) {

				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
			} else if (TRUE == active_calls && FALSE == held_calls) {
				int active_members = 0;

				voicecall_core_get_active_call_member(pcall_core, &active_members);

				if (!(active_members > 1 && (ERROR_VOICECALL_NONE == _vc_core_engine_private_call_by_callid(pcall_core->pcall_engine, pcall_core->selected_call_id_in_ss)))) {

					CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
					vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
				}
			} else {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
			}
		}
		break;
	case CALL_VC_CORE_SS_3:
		{
			if (TRUE == voicecall_core_is_incoming_call_exists(pcall_core->pcall_engine)) {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
				return TRUE;
			}

			if (TRUE == voicecall_core_is_conf_call_possible(pcall_core)) {
				_vc_core_engine_setup_conference(pcall_core->pcall_engine);
			} else {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
			}
		}
		break;
	case CALL_VC_CORE_SS_4:
		if (TRUE == voicecall_core_is_transfer_call_possible(pcall_core)) {
			if (FALSE == voicecall_core_transfer_calls(pcall_core)) {

				CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
			}
		} else {

			CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
		}
		break;
	default:
		CALL_ENG_DEBUG(ENG_DEBUG, "Invalid SS State");
		CALL_ENG_DEBUG(ENG_DEBUG, "Need to show a popup to the user ");
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
	}
	return TRUE;
}

/**
 * This function processed the supplementary services while on call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 * @param[in]		pdialled_number		dial number
 */
void voicecall_core_process_incall_ss(call_vc_core_state_t *pcall_core, char *pdialled_number)
{
	call_vc_core_incall_ss_state_t incall_ss_state = CALL_VC_CORE_SS_NONE;

	CALL_ENG_DEBUG(ENG_DEBUG, " ..");

	incall_ss_state = voicecall_core_parse_incall_ss_string(pcall_core, pdialled_number);

	if (incall_ss_state != CALL_VC_CORE_SS_USSD) {
		voicecall_core_start_incall_ss(pcall_core, incall_ss_state);
	} else {

		CALL_ENG_DEBUG(ENG_DEBUG, "Involves CISS functionality so need for us to handle and will be handled by CISS");
		vc_engine_msg_box_type event_data;
		memset(&event_data, 0, sizeof(event_data));
		event_data.string_id = IDS_CALL_POP_OPERATION_REFUSED;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_MESSAGE_BOX_TO_UI, (void *)&event_data);
	}
}

#ifdef	PDIAL_SEND_DTMF
gboolean voicecall_core_send_phone_number_dtmf(gpointer puser_data)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)vcall_engine_get_core_state();
	int call_handle = (int)puser_data;
	char dtmf_number[VC_PHONE_NUMBER_LENGTH_MAX];
	call_vc_call_objectinfo_t obj_info = { 0, };

	CALL_ENG_DEBUG(ENG_DEBUG, "inside ...");

	if (TRUE == _vc_core_cm_get_call_object(&pcall_core->pcall_engine->call_manager, call_handle, &obj_info)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Source telephone number - %s", obj_info.source_tel_number);
		if (TRUE == _vc_core_util_extract_dtmf_number(obj_info.source_tel_number, dtmf_number, sizeof(dtmf_number))) {
			CALL_ENG_DEBUG(ENG_DEBUG, "DTMF number - %s", dtmf_number);
			if (VC_CALL_ORIG_TYPE_SAT == pcall_core->call_setup_info.call_type) {
				__voicecall_core_queue_dtmf_string(pcall_core, dtmf_number, TRUE);
			} else {
				__voicecall_core_queue_dtmf_string(pcall_core, dtmf_number, FALSE);
			}
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "Call Info not available for call handle: %d", call_handle);
	}

	return FALSE;
}

/**
 * This function cancels the dtmf queue
 *
 * @return		TRUE - sucess, FALSE otherwise
 * @param[in]		papp_document		Handle to Application Document
 */
gboolean voicecall_core_cancel_dtmf_queue(call_vc_core_state_t *pcall_core)
{
	voicecall_dtmf_info_t *pdtmf_info = (voicecall_dtmf_info_t *)&pcall_core->dtmf_info;

	CALL_ENG_DEBUG(ENG_DEBUG, "Start:pcall_core(%p)", pcall_core);

	if (FALSE == pdtmf_info->bdtmf_queue) {
		return TRUE;
	}

	/*Remove Pauser Timer */
	if (pdtmf_info->dtmf_pause_timer > 0) {
		g_source_remove(pdtmf_info->dtmf_pause_timer);
		pdtmf_info->dtmf_pause_timer = -1;
	}

	/*Reset the Status Flags */
	pdtmf_info->bdtmf_queue = FALSE;
	pdtmf_info->dtmf_index = 0;
	pdtmf_info->bdtmf_wait = FALSE;
	memset(pdtmf_info->dtmf_number, 0, sizeof(pdtmf_info->dtmf_number));
	if (TRUE == pdtmf_info->bsat_dtmf) {
		voicecall_core_send_sat_response(pcall_core->pcall_engine, SAT_RQST_SEND_DTMF, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
	}
	pdtmf_info->bsat_dtmf = FALSE;
	pdtmf_info->bsat_hidden = FALSE;
	CALL_ENG_DEBUG(ENG_DEBUG, "End");
	return TRUE;
}

static gboolean __voicecall_core_dtmf_pause_timer_cb(gpointer puser_data)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)puser_data;

	__voicecall_core_handle_dtmf_ack(pcall_core, TRUE);

	/*Always return FALSE, so that it won't be called again */
	return FALSE;
}

gboolean __voicecall_core_send_dtmf_idle_cb(gpointer pdata)
{
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)pdata;
	voicecall_dtmf_info_t *pdtmf_info = (voicecall_dtmf_info_t *)&pcall_core->dtmf_info;
	voicecall_error_t vc_error;
	vc_engine_dtmf_ack_type event_data;
	char dtmf_string[2];

	dtmf_string[0] = pdtmf_info->dtmf_number[pdtmf_info->dtmf_index];
	dtmf_string[1] = '\0';

	CALL_ENG_DEBUG(ENG_DEBUG, "inside ...");

	vc_error = _vc_core_engine_send_dtmf(pcall_core->pcall_engine, dtmf_string);

	if (vc_error == ERROR_VOICECALL_INVALID_DTMF_CHAR) {
		CALL_ENG_DEBUG(ENG_DEBUG, "ERROR_VOICECALL_INVALID_DTMF_CHAR");
	} else if (ERROR_VOICECALL_NONE != vc_error) {
		voicecall_core_cancel_dtmf_queue(pcall_core);
		memset(&event_data, 0, sizeof(event_data));
		event_data.bstatus = FALSE;
		event_data.string_id = IDS_CALL_POP_DTMFSENDING_FAIL;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);

		if (TRUE == pdtmf_info->bsat_dtmf) {
			voicecall_core_send_sat_response(pcall_core->pcall_engine, SAT_RQST_SEND_DTMF, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
		}

		if (!((TRUE == pdtmf_info->bsat_dtmf) && (TRUE == pdtmf_info->bsat_hidden))) {
			memset(&event_data, 0, sizeof(event_data));
			event_data.bstatus = FALSE;
			event_data.string_id = IDS_CALL_POP_DTMFSENDING_FAIL;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);
		}
	}

	return FALSE;
}

static gboolean __voicecall_core_handle_dtmf_ack(call_vc_core_state_t *pcall_core, gboolean success)
{
	voicecall_dtmf_info_t *pdtmf_info = (voicecall_dtmf_info_t *)&pcall_core->dtmf_info;
	vc_engine_dtmf_ack_type event_data;
	CALL_ENG_DEBUG(ENG_DEBUG, "...");

	if (FALSE == pdtmf_info->bdtmf_queue) {
		CALL_ENG_DEBUG(ENG_DEBUG, "DTMF Queue Canceled, do nothing");
		return TRUE;
	}

	if (TRUE == success) {
		char dtmf_string[2];
		pdtmf_info->dtmf_index++;

		dtmf_string[0] = pdtmf_info->dtmf_number[pdtmf_info->dtmf_index];
		dtmf_string[1] = '\0';

		CALL_ENG_DEBUG(ENG_DEBUG, "Current dtmf_index: %d,dtmf_max_length=%d", pdtmf_info->dtmf_index, pdtmf_info->dtmf_max_length);
		CALL_ENG_DEBUG(ENG_DEBUG, "Current DTMF String: %s", &pdtmf_info->dtmf_number[pdtmf_info->dtmf_index]);

		/*Find the End of the queue */
		if (pdtmf_info->dtmf_index >= pdtmf_info->dtmf_max_length) {

			CALL_ENG_DEBUG(ENG_DEBUG, "Updating DTMF Progress before destroying");

			if (!((TRUE == pdtmf_info->bsat_dtmf) && (TRUE == pdtmf_info->bsat_hidden))) {
				memset(&event_data, 0, sizeof(event_data));
				event_data.bstatus = FALSE;	/*check it*/
				event_data.string_id = IDS_CALL_POP_DTMF_SENT; /*check it*/
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);
			}

			if (TRUE == pdtmf_info->bsat_dtmf) {
				voicecall_core_send_sat_response(pcall_core->pcall_engine, SAT_RQST_SEND_DTMF, CALL_VC_ME_RET_SUCCESS);
			}
		} else {
			if (0 == strcasecmp(dtmf_string, "p") || 0 == strcmp(dtmf_string, ",")) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Pause on the dtmf string");

				memset(&event_data, 0, sizeof(event_data));
				event_data.bstatus = TRUE;
				event_data.string_id = IDS_CALL_POP_SENDING;
				snprintf(event_data.display_string, sizeof(event_data.display_string), "%s", &pdtmf_info->dtmf_number[pdtmf_info->dtmf_index]);
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);

				int dtmf_interval = 0;

				if (FALSE == _vc_core_util_check_gcf_status()) {
					dtmf_interval = (pdtmf_info->dtmf_index == 0) ? DTMF_PAUSE_TIMER_INTERVAL_FIRST : DTMF_PAUSE_TIMER_INTERVAL_REST;
				} else {
#ifdef GCONF_GCF_SAT_TEST
					int start_interval = DTMF_PAUSE_TIMER_INTERVAL_GCF_FIRST;
					int rest_interval = DTMF_PAUSE_TIMER_INTERVAL_GCF_REST;
					{
						GConfClient *client = NULL;
						client = gconf_client_get_default();
						if (NULL == client) {
							CALL_ENG_DEBUG(ENG_DEBUG, "gconf_client_get_default failed..");
						} else {

							start_interval = gconf_client_get_int(client, PATH_DTMF_INTERVAL_GCF_FIRST, NULL);

							rest_interval = gconf_client_get_int(client, PATH_DTMF_INTERVAL_GCF_REST, NULL);
							g_object_unref(client);
						}

					}
					CALL_ENG_DEBUG(ENG_DEBUG, "start_interval:%d, rest_interval = %d", start_interval, rest_interval);
					dtmf_interval = (pdtmf_info->dtmf_index == 0) ? start_interval : rest_interval;
#else
					dtmf_interval = (pdtmf_info->dtmf_index == 0) ? DTMF_PAUSE_TIMER_INTERVAL_GCF_FIRST : DTMF_PAUSE_TIMER_INTERVAL_GCF_REST;
#endif
				}
				CALL_ENG_DEBUG(ENG_DEBUG, "dtmf_interval:%d", dtmf_interval);
				pdtmf_info->dtmf_pause_timer = g_timeout_add(dtmf_interval, __voicecall_core_dtmf_pause_timer_cb, pcall_core);
			} else if (0 == strcasecmp(dtmf_string, "w") || 0 == strcmp(dtmf_string, ";")) {
				CALL_ENG_DEBUG(ENG_DEBUG, "Wait on the dtmf string");
				memset(&event_data, 0, sizeof(event_data));
				event_data.bstatus = TRUE;
				event_data.string_id = IDS_CALL_POP_UNAVAILABLE;	/*assign ID when string is added*/
				snprintf(event_data.display_string, sizeof(event_data.display_string), "%s", &pdtmf_info->dtmf_number[pdtmf_info->dtmf_index]);
				vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);
			} else {
				g_idle_add_full(G_PRIORITY_HIGH_IDLE + 25, __voicecall_core_send_dtmf_idle_cb, pcall_core, NULL);
			}
		}
	} else {
		voicecall_core_cancel_dtmf_queue(pcall_core);

		memset(&event_data, 0, sizeof(event_data));
		event_data.bstatus = FALSE;
		event_data.string_id = IDS_CALL_POP_DTMFSENDING_FAIL;
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);
	}

	return TRUE;
}

/**
 * This function queues the dtmf digits one by one from the given dtmf string
 *
 * @return		TRUE - sucess, FALSE otherwise
 * @param[in]		papp_document		Handle to Application Document
 * @param[in]		dtmf_string			dtmf string to be queued
 */
static gboolean __voicecall_core_queue_dtmf_string(call_vc_core_state_t *pcall_core, char *dtmf_string, gboolean bsat_dtmf)
{
	voicecall_dtmf_info_t *pdtmf_info = (voicecall_dtmf_info_t *)&pcall_core->dtmf_info;
	gboolean bhidden_mode = FALSE;
	char dtmf_digit[2];
	vc_engine_dtmf_ack_type event_data;

	CALL_ENG_DEBUG(ENG_DEBUG, "bsat_dtmf = %d", bsat_dtmf);

	if (TRUE == bsat_dtmf) {
		/*Always get the status from the engine */
		_vc_core_engine_get_sat_dtmf_hidden_mode(pcall_core->pcall_engine, &bhidden_mode);
		CALL_ENG_DEBUG(ENG_DEBUG, "SAT Hidden Mode : %d", bhidden_mode);
	}

	/*Check for the validity of the DTMF String */
	if (FALSE == _vc_core_util_isvalid_full_dtmf_number(dtmf_string)) {
		if (TRUE == bsat_dtmf) {
			voicecall_core_send_sat_response(pcall_core->pcall_engine, SAT_RQST_SEND_DTMF, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
		}

		if (!((TRUE == bsat_dtmf) && (TRUE == bhidden_mode))) {
			memset(&event_data, 0, sizeof(event_data));
			event_data.bstatus = FALSE;
			event_data.string_id = IDS_CALL_POP_INVALID_DTMF;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);
		}
		return FALSE;
	}

	/*Reset DTMF Values */
	pdtmf_info->bdtmf_queue = FALSE;
	pdtmf_info->dtmf_index = 0;
	pdtmf_info->bsat_dtmf = bsat_dtmf;

	if (TRUE == bsat_dtmf) {
		/*Initialize */
		pdtmf_info->bsat_hidden = bhidden_mode;
	}

	/*It takes only 40 characters from the source, rest of the characters are ignored*/
	memset(pdtmf_info->dtmf_number, 0, sizeof(pdtmf_info->dtmf_number));
	strncpy(pdtmf_info->dtmf_number, dtmf_string, min((sizeof(pdtmf_info->dtmf_number) - 1), strlen(dtmf_string)));
	pdtmf_info->dtmf_max_length = strlen(pdtmf_info->dtmf_number);
	CALL_ENG_DEBUG(ENG_DEBUG, "Dtmf Number:%s ,dtmf_max_length:%d", pdtmf_info->dtmf_number, pdtmf_info->dtmf_max_length);

	dtmf_digit[0] = pdtmf_info->dtmf_number[pdtmf_info->dtmf_index];
	dtmf_digit[1] = '\0';

	/*Send DTMF */
	if (0 == strcasecmp(dtmf_digit, "p") || 0 == strcmp(dtmf_digit, ",")) {
		int dtmf_interval = 0;

		if (FALSE == _vc_core_util_check_gcf_status()) {
			dtmf_interval = (pdtmf_info->dtmf_index == 0) ? DTMF_PAUSE_TIMER_INTERVAL_FIRST : DTMF_PAUSE_TIMER_INTERVAL_REST;
		} else {
#ifdef GCONF_GCF_SAT_TEST
			int start_interval = DTMF_PAUSE_TIMER_INTERVAL_GCF_FIRST;
			int rest_interval = DTMF_PAUSE_TIMER_INTERVAL_GCF_REST;
			{

				GConfClient *client = NULL;
				client = gconf_client_get_default();
				if (NULL == client) {
					CALL_ENG_DEBUG(ENG_DEBUG, "gconf_client_get_default failed..");
				} else {

					start_interval = gconf_client_get_int(client, PATH_DTMF_INTERVAL_GCF_FIRST, NULL);

					rest_interval = gconf_client_get_int(client, PATH_DTMF_INTERVAL_GCF_REST, NULL);
					g_object_unref(client);
				}

			}
			CALL_ENG_DEBUG(ENG_DEBUG, "start_interval:%d, rest_interval = %d", start_interval, rest_interval);
			dtmf_interval = (pdtmf_info->dtmf_index == 0) ? start_interval : rest_interval;
#else
			dtmf_interval = (pdtmf_info->dtmf_index == 0) ? DTMF_PAUSE_TIMER_INTERVAL_GCF_FIRST : DTMF_PAUSE_TIMER_INTERVAL_GCF_REST;
#endif
			CALL_ENG_DEBUG(ENG_DEBUG, "updated dtmf_interval:%d", dtmf_interval);
		}
		pdtmf_info->dtmf_pause_timer = g_timeout_add(dtmf_interval, __voicecall_core_dtmf_pause_timer_cb, pcall_core);
	} else if (0 == strcasecmp(dtmf_digit, "w") || 0 == strcmp(dtmf_digit, ";")) {
		/* enable wait flag for dtmf sending */
		pdtmf_info->bdtmf_wait = TRUE;
	} else if ((ERROR_VOICECALL_NONE != _vc_core_engine_send_dtmf(pcall_core->pcall_engine, dtmf_digit))) {
		if (!((TRUE == pdtmf_info->bsat_dtmf) && (TRUE == pdtmf_info->bsat_hidden))) {
			memset(&event_data, 0, sizeof(event_data));
			event_data.bstatus = FALSE;
			event_data.string_id = IDS_CALL_POP_DTMFSENDING_FAIL;
			vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);
		}
		if (TRUE == pdtmf_info->bsat_dtmf) {
			voicecall_core_send_sat_response(pcall_core->pcall_engine, SAT_RQST_SEND_DTMF, CALL_VC_ME_UNABLE_TO_PROCESS_COMMAND);
		}
		return FALSE;
	}
	pdtmf_info->bdtmf_queue = TRUE;

	/*Create Progressbar popup */
	if (!((TRUE == pdtmf_info->bsat_dtmf) && (TRUE == pdtmf_info->bsat_hidden))) {
		memset(&event_data, 0, sizeof(event_data));
		event_data.bstatus = TRUE;
		CALL_ENG_DEBUG(ENG_DEBUG, "pdtmf_info->bdtmf_wait [%d]", pdtmf_info->bdtmf_wait);
		if (pdtmf_info->bdtmf_wait) {
			event_data.string_id = IDS_CALL_POP_UNAVAILABLE;
		} else {
			event_data.string_id = IDS_CALL_POP_SENDING;
		}
		_vc_core_util_strcpy(event_data.display_string, VC_PHONE_NUMBER_LENGTH_MAX, pdtmf_info->dtmf_number);
		vcall_engine_send_event_to_client(VC_ENGINE_MSG_DTMF_ACK_TO_UI, (void *)&event_data);
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "SAT DTMF Hidden Mode, not showing UI");
	}
	return TRUE;
}
#endif

gboolean voicecall_core_change_sound_path(call_vc_core_state_t *pcall_core, voicecall_snd_audio_type_t sound_path)
{
	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	int ret = FALSE;
	int total_call_member = -1;

	CALL_ENG_DEBUG(ENG_DEBUG, "sound_path:[%d]", sound_path);

	voicecall_core_get_total_call_member(pcall_core->pcall_engine, &total_call_member);
	if (total_call_member == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "There are not active calls hence it should not work");
		return FALSE;
	}

	switch (sound_path) {
	case VOICE_CALL_AUDIO_SPEAKER:
		{
			if (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_BT) {
				/*_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_PHONE, -1, NULL);*/
				_vc_bt_request_switch_headset_path(pcall_core, FALSE);
			}
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_SPEAKER);
			ret = TRUE;
		}
		break;

	case VOICE_CALL_AUDIO_HEADSET:
		{
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_BT);
/*			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_HEADSET, -1, NULL);*/
			_vc_bt_request_switch_headset_path(pcall_core, TRUE);

			ret = TRUE;
		}
		break;

	case VOICE_CALL_AUDIO_RECEIVER_EARJACK:
		{
			if (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_BT) {
				/*_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_PHONE, -1, NULL);*/
				_vc_bt_request_switch_headset_path(pcall_core, FALSE);
			}
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER_EARJACK);
			ret = TRUE;
		}
		break;

	default:
		{
			CALL_ENG_DEBUG(ENG_DEBUG, "Unknown type!!");
			ret = FALSE;
		}
		break;
	}

	/* Change Audio Path according to the current status */
	voicecall_snd_change_path(papp_snd);

	return ret;
}

gboolean voicecall_core_get_sound_path(call_vc_core_state_t *pcall_core, int *sound_path)
{
	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	switch (voicecall_snd_get_path_status(papp_snd)) {
	case VOICE_CALL_SND_PATH_SPEAKER:
		{
			*sound_path = VOICE_CALL_AUDIO_SPEAKER;
		}
		break;

	case VOICE_CALL_SND_PATH_BT:
		{
			*sound_path = VOICE_CALL_AUDIO_HEADSET;
		}
		break;

	case VOICE_CALL_SND_PATH_RECEIVER_EARJACK:
	default:
		{
			*sound_path = VOICE_CALL_AUDIO_RECEIVER_EARJACK;
		}
		break;
	}

	return TRUE;
}

static gboolean __voicecall_core_is_redial_cuase(int end_cause)
{
	CALL_ENG_DEBUG(ENG_DEBUG, "end_cause(%d)", end_cause);

	switch (end_cause) {
	case VC_ENDCAUSE_CALL_BARRED:
	case VC_ENDCAUSE_NO_SERVICE:
	case VC_ENDCAUSE_USER_UNAVAILABLE:
	case VC_ENDCAUSE_INVALID_NUMBER_FORMAT:
	case VC_ENDCAUSE_NUMBER_CHANGED:
	case VC_ENDCAUSE_NO_CREDIT:
	case VC_ENDCAUSE_UNASSIGNED_NUMBER:
		return FALSE;

	case VC_ENDCAUSE_CALL_ENDED:
	case VC_ENDCAUSE_CALL_DISCONNECTED:
	case VC_ENDCAUSE_NO_ANSWER:
	case VC_ENDCAUSE_NW_BUSY:
	case VC_ENDCAUSE_CALL_SERVICE_NOT_ALLOWED:
	case VC_ENDCAUSE_NW_FAILED:
	case VC_ENDCAUSE_REJECTED:
	case VC_ENDCAUSE_USER_BUSY:
	case VC_ENDCAUSE_WRONG_GROUP:
	case VC_ENDCAUSE_CALL_NOT_ALLOWED:
	case VC_ENDCAUSE_CALL_FAILED:
	case VC_ENDCAUSE_NO_USER_RESPONDING:
	case VC_ENDCAUSE_USER_ALERTING_NO_ANSWER:
	case VC_ENDCAUSE_SERVICE_TEMP_UNAVAILABLE:
	case VC_ENDCAUSE_USER_DOESNOT_RESPOND:
	case VC_ENDCAUSE_IMEI_REJECTED:
	case VC_ENDCAUSE_TAPI_ERROR:
	default:
		return TRUE;
	}
}

/**
 * This function processed the supplementary services while on call
 *
 * @return		Returns TRUE If transfer call can be made or FALSE if not
 * @param[in]		pcall_core		Handle to voicecall core
 */
void voicecall_core_process_dtmf_send_status(call_vc_core_state_t *pcall_core, gboolean bsuccess)
{
	CALL_ENG_DEBUG(ENG_DEBUG, " ..");
	VOICECALL_RETURN_IF_FAIL(pcall_core);
	__voicecall_core_handle_dtmf_ack(pcall_core, bsuccess);
}

