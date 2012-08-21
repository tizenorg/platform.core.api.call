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

#ifndef __VC_CORE_CALL_AGENT_H_
#define __VC_CORE_CALL_AGENT_H_

#include <stdbool.h>
#include "vc-core-callmanager.h"
#ifdef _CPHS_DEFINED_
#include "status-interface.h"
#endif

/**< Maximum Telephony Event */
#define CALL_VC_TAPI_CALL_EVENT_MAX_NUM	33
#define CALL_VC_SIMATK_EVENT_MAX_NUM	3	/**< Maximum SIM ATK Event */
#define CALL_VC_TAPI_READY_EVENT_NUM	1	/**< Tapi Service Ready Event */
#define CALL_VC_TAPI_FACTORY_EVENT_NUM	1	/**< Tapi Service FACTORY Event */
#define CALL_VC_TAPI_SUBSCRIPTION_MAX		(CALL_VC_TAPI_CALL_EVENT_MAX_NUM+CALL_VC_TAPI_READY_EVENT_NUM+CALL_VC_SIMATK_EVENT_MAX_NUM+CALL_VC_TAPI_FACTORY_EVENT_NUM)

/**
 * This enumeration provides CA STATEs
 */
typedef enum {
	CALL_VC_CA_STATE_NORMAL,							/**< Normal state */
	CALL_VC_CA_STATE_SPLIT_CALLBOX,						/**< Call split state */
	CALL_VC_CA_STATE_WAIT_SPLIT,						/**< Waiting for call split state */
	CALL_VC_CA_STATE_DROP_CALLBOX,						/**< Call drop state */
	CALL_VC_CA_STATE_WAIT_DROP,							/**< Waiting for drop state */
#ifdef SWAP_SUPPORT
	CALL_VC_CA_STATE_WAIT_SWAP_HOLD_OR_ACTIVATE,		/**< Wait for swapping activce/hold call state */
	CALL_VC_CA_STATE_WAIT_SWAP_HOLD,					/**< Wait for swapping held call state */
	CALL_VC_CA_STATE_WAIT_SWAP_ACTIVE,					/**< Wait for swapping active call state */
#endif
	CALL_VC_CA_STATE_WAIT_SWAP,							/**< Wait for swap state */
	CALL_VC_CA_STATE_WAIT_HOLD,							/**< Wait for hold state */
	CALL_VC_CA_STATE_WAIT_UNHOLD,						/**< Wait for unhold state */
	CALL_VC_CA_STATE_WAIT_JOIN,							/**< Wait for join state */
	CALL_VC_CA_STATE_WAIT_TRANSFER_CNF,					/**< Wait for transfer confirmation state */
	CALL_VC_CA_STATE_WAIT_TRANSFER_CALLEND,				/**< Wait for transfer callend state */
	CALL_VC_CA_STATE_WAIT_RELEASE_ALL_ACTIVECALL,		/**< Wait for release all active call state */
	CALL_VC_CA_STATE_WAIT_RELEASE_ALL_HOLDCALL,			/**< Wait for release all held call state */
	CALL_VC_CA_STATE_SENDMSG_CALLBOX,					/**< Message send callbox active state */
	CALL_VC_CA_STATE_VIEW_CONTACT_DETAIL_CALLBOX,		/**< Contact detail view state */
	CALL_VC_CA_STATE_SAVE_TO_CONTACT_CALLBOX,			/**< Save to contact state */
	CALL_VC_CA_STATE_SS_WAIT_RELEASE_ALL_ACTIVECALL,	/**< Wait for release all active calls */
	CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS,			/**< Wait for release all calls */
	CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SETUP,	/**< Wait for release all calls to setup */
	CALL_VC_CA_STATE_WAIT_RELEASE_ALL_CALLS_TO_SWITCH_TO_VIDEO_CALL,/**< Wait for release all calls to setup videocall */
	CALL_VC_CA_STATE_MAX_NUM							/**< Max value for the state */
} call_vc_ca_state_t;

/**
 * This structure defines voicecall agent data
 */
typedef struct _voicecall_engine_t {
	TapiHandle *tapi_handle;
	call_vc_ca_state_t callagent_state;			/**< Call Agent State */
	voicecall_inout_state_t io_state;		/**< Voicecall Engine IO State */
	call_vc_manager_t call_manager;			/**< Handle for the Call Manager */
	TelSimCardType_t card_type;				/**< SIM Card Type */

	gboolean bdtmf_ring;					/**< dtmf ring? */

	gboolean bis_no_sim;						/**< SIM not available , if TRUE */
	gboolean bdownload_call;					/**< Automated call test after binary download , if TRUE */

	/*No structure in TAPI  14Mar08 */
	/*tapi_call_ccbs_info_t	ccbs_info[CALL_VC_CCBS_NUMBER_MAX]; < call control for busy subscriber info */
	int ccbs_index;												  /**< Index for  ccbs_info*/

	int barring_ind_type;									/* barring ind type */
	/*AOC*/
	float aoc_ppm;												/**< Price per unit value of currency meter */

	/*CPHS Info*/
#ifdef _CPHS_DEFINED_
	status_class_cphs_type cphs_status;				 /**< stores the cphs status information*/
	gboolean bcphs_read_success;								/**< Flag for CPHS read status*/
#endif

	/*Subscription Information */
	unsigned int subscription_id[CALL_VC_TAPI_SUBSCRIPTION_MAX];
	int curr_tapi_path;

	/*Client Information */
	voicecall_cb client_callback;					/**<Callback fucntion to send event to client*/
	void *puser_data;						/**<Stores the User data set during engine initialization*/

} call_vc_callagent_state_t;

/**
 * This function intializes the call agent
 *
 * @return		Pointer to call agent state.
 */
call_vc_callagent_state_t *_vc_core_ca_init_agent();

/**
 * This function changes the in out state of the call agent
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pAgent		Pointer to the call agent structure
 * @param[in]		new_state		The new i/o state that should be set
 * @see			_vc_core_ca_change_agent_state
 */
gboolean _vc_core_ca_change_inout_state(call_vc_callagent_state_t *pAgent, voicecall_inout_state_t new_state);

/**
 * This function changes the in call agent state
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]		pAgent		Pointer to the call agent structure
 * @param[in]		new_state		The new call agent state that should be set
 * @see			_vc_core_ca_change_inout_state
 */
gboolean _vc_core_ca_change_agent_state(call_vc_callagent_state_t *pAgent, call_vc_ca_state_t new_state);

/**
 * This function finalizes the call agent
 *
 * @return		Returns void
 * @param[in]		pcall_agent Pointer to the call agent structure
 */
void _vc_core_ca_finish_agent(call_vc_callagent_state_t *pcall_agent);

/**
 * This function checks if all the call members have terminated or not
 *
 * @return		Returns TRUE if no call members exist, FALSE otherwise
 * @param[in]	pagent		Pointer to the call agent structure
 */
gboolean _vc_core_ca_check_end(call_vc_callagent_state_t *pagent);

/**
 * This function ends all the calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]	pagent	Handle to voicecall engine
 */
gboolean _vc_core_ca_end_all_calls(call_vc_callagent_state_t *pagent);

/**
 * This function ends all the active calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]	pagent	Handle to voicecall engine
 */
gboolean _vc_core_ca_end_active_calls(call_vc_callagent_state_t *pagent);

/**
 * This function ends all the held calls
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]	pagent        Handle to voicecall engine
 */
gboolean _vc_core_ca_end_held_calls(call_vc_callagent_state_t *pagent);

/**
 * This function sends the response to the SAT engine
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]	pagent				Pointer to the call agent structure
 * @param[in]	sat_rqst_resp_type	sat rquest/response type used by the client
 * @param[in]	sat_response_type	response to be sent to sat
 */
gboolean _vc_core_ca_send_sat_response(call_vc_callagent_state_t *pagent, voicecall_engine_sat_rqst_resp_type sat_rqst_Resp_type, call_vc_sat_reponse_type_t sat_response_type);

/**
 * This function intializes the callagent data
 *
 * @return		void
 * @param[in]		pagent					Pointer to the call agent structure
 */
void _vc_core_ca_init_data(call_vc_callagent_state_t *pagent);

/**
 * This function sends response event to the registered client
 *
 * @return		This function returns TRUE on success and FALSE on failure
 * @param[in]	pcall_agent	Pointer to the call agent structure
 * @param[in]	event		response event type
 * @param[in]	param1		param 1 to be passed to the client
 * @param[in]	param2		param 2 to be passed to the client
 * @param[in]	param3		param 3 to be passed to the client
 */
gboolean _vc_core_ca_send_event_to_client(call_vc_callagent_state_t *pcall_agent, int event, int param1, int param2, void *param3);

/**
 * This function checks whether outgoing call is possible
 *
 * @return		This function returns TRUE if outgoing call is possible or else FALSE
 * @param[in]	pcall_agent			Pointer to the call agent structure
 * @param[in]	bemergency_number	TRUE - if outgoing call being made is emergency call or else FALSE
 */
gboolean _vc_core_ca_is_mocall_possible(call_vc_callagent_state_t *pcall_agent, gboolean bemergency_number);

/**
 * This function checks whether private call is possible or not
 *
 * @return		This function returns TRUE if private call is possible or else FALSE
 * @param[in]	pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_ca_is_private_call_possible(call_vc_callagent_state_t *pcall_agent);

/**
 * This function checks whether call transfer is possible
 *
 * @return		This function returns TRUE if transfer is possible or else FALSE
 * @param[in]	pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_ca_is_transfer_call_possible(call_vc_callagent_state_t *pcall_agent);

/**
 * This function checks whether conference call is possible
 *
 * @return		This function returns TRUE if transfer is possible or else FALSE
 * @param[in]	pcall_agent			Pointer to the call agent structure
 */
gboolean _vc_core_ca_is_conf_call_possible(call_vc_callagent_state_t *pcall_agent);

/**
 * This function clears the data of a connected call givenits call handle
 *
 * @return		Returns TRUE on success and FALSE on failure
 * @param[in]	pcall_agent	Handle to voicecall engine
 * @param[in]	call_handle	Call handle of the connected call to be cleared
 */
gboolean _vc_core_ca_clear_connected_call(call_vc_callagent_state_t *pcall_agent, int call_handle);

#endif				/* __VC_CORE_CALL_AGENT_H_ */
