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


#include "voice-call-service.h"
#include "voice-call-dbus.h"
#include "vc-core-util.h"
#include "voice-call-core.h"
#include "voice-call-sound.h"
#include "voice-call-bt.h"

/*
 *  If the birthday is Today then it will return 0
 *  If the birthday is within 1 week gap then it will return between 1 to 7
 *  In all other cases it will return -1
 */
static int __voicecall_service_get_bday_remaining_days(CTSstruct *contact)
{
	int diff = -1;
	GSList *event_list = NULL;
	GSList *cursor = NULL;

	contacts_svc_struct_get_list(contact, CTS_CF_EVENT_LIST, &event_list);
	for (cursor = event_list; cursor; cursor = cursor->next) {
		if (CTS_EVENT_TYPE_BIRTH == contacts_svc_value_get_int(cursor->data, CTS_EVENT_VAL_TYPE_INT)) {
			int bday_date;
			time_t t_now;
			time_t t_bday;
			struct tm *timeinfo;
			bday_date = contacts_svc_value_get_int(cursor->data, CTS_EVENT_VAL_DATE_INT);
			time(&t_now);
			timeinfo = localtime(&t_now);
			timeinfo->tm_sec = timeinfo->tm_min = 59;
			timeinfo->tm_hour = 23;
			timeinfo->tm_mday = bday_date % 100;
			timeinfo->tm_mon = (bday_date%10000)/100 - 1;
			CALL_ENG_DEBUG(ENG_DEBUG, "bday date = %d, month = %d", timeinfo->tm_mday, timeinfo->tm_mon);
			t_bday = mktime(timeinfo);
			if (t_now < t_bday) {
				diff = (int)((t_bday - t_now) / (24 * 60 * 60));
				if (diff < 0 || diff > 7) {
					diff = -1;	/* All the other days which dont fall in one week gap is invalid */
				}
			} else {
				CALL_ENG_DEBUG(ENG_DEBUG, "No need to show bday icon, becauae bday is not falling in 1 week gap");
			}

			break;
		}
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "diff = %d", diff);
	return diff;
}

/**
 * This function gets  contact info from phone number
 *
 * @return		gboolean
 * @param[in]		phonenumber		phone number
 * @param[out]	ct_info			contact info
 */
gboolean voicecall_service_contact_info_by_number(char *phonenumber, voicecall_contact_info_t * ct_info)
{
	CALL_ENG_KPI("voicecall_service_contact_info_by_number start");
	int index = 0;
	int ret = -1;
	CTSstruct *contact = NULL;
	CTSvalue *name = NULL, *base = NULL;
	const char *first = NULL, *last = NULL, *display = NULL;
	GSList *get_list, *cursor;

	VOICECALL_RETURN_FALSE_IF_FAIL(phonenumber != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(ct_info != NULL);

	CALL_ENG_DEBUG(ENG_DEBUG, "num:[%s],index:[%d]..", phonenumber, ct_info->ct_index);

	contacts_svc_connect();

	if (ct_info->ct_index == -1) {
		index = contacts_svc_find_contact_by(CTS_FIND_BY_NUMBER, phonenumber);
		if (index < CTS_SUCCESS) {
			CALL_ENG_DEBUG(ENG_DEBUG, "ret:[%d]..No found record", ret);
			ct_info->ct_index = -1;
			ct_info->phone_type = -1;
			ct_info->bday_remaining_days = -1;
			return FALSE;
		} else
			ct_info->ct_index = index;
	}

	ret = contacts_svc_get_contact(ct_info->ct_index, &contact);

	if (ret < 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "ret:[%d]..No found record", ret);
		ct_info->ct_index = -1;
		ct_info->phone_type = -1;
		return FALSE;
	}
	/* If it comes to here, it means we have found a contact. */
	contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &name);
	first = contacts_svc_value_get_str(name, CTS_NAME_VAL_FIRST_STR);
	last = contacts_svc_value_get_str(name, CTS_NAME_VAL_LAST_STR);
	display = contacts_svc_value_get_str(name, CTS_NAME_VAL_DISPLAY_STR);

	if (display != NULL) {
		snprintf(ct_info->display_name, sizeof(ct_info->display_name), "%s", display);
	} else {
		if (first != NULL && last != NULL) {
			if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
				snprintf(ct_info->display_name, sizeof(ct_info->display_name), "%s %s", first, last);
			else
				snprintf(ct_info->display_name, sizeof(ct_info->display_name), "%s %s", last, first);
		} else if (first == NULL && last != NULL) {
			snprintf(ct_info->display_name, sizeof(ct_info->display_name), "%s", last);
		} else if (first != NULL && last == NULL) {
			snprintf(ct_info->display_name, sizeof(ct_info->display_name), "%s", first);
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "Error!. first & last name is NULL!!");
			snprintf(ct_info->display_name, sizeof(ct_info->display_name), "%s", "no name");
		}
	}
	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &get_list);
	cursor = get_list;
	while (cursor) {
		if (strcmp(contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR), phonenumber) == 0) {
			ct_info->phone_type = contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT);
		}
		cursor = cursor->next;
		if (cursor == NULL)
			break;
	}

	contacts_svc_struct_get_value(contact, CTS_CF_BASE_INFO_VALUE, &base);

	_vc_core_util_strcpy(ct_info->caller_id_path, sizeof(ct_info->caller_id_path), contacts_svc_value_get_str(base, CTS_BASE_VAL_IMG_PATH_STR));

	char *full_path = NULL;
	contacts_svc_get_image(CTS_IMG_FULL, ct_info->ct_index, &full_path);
	if (full_path != NULL) {
		_vc_core_util_strcpy(ct_info->caller_full_id_path, sizeof(ct_info->caller_full_id_path), full_path);
	}
	free(full_path);

	_vc_core_util_strcpy(ct_info->ring_tone, sizeof(ct_info->ring_tone), contacts_svc_value_get_str(base, CTS_BASE_VAL_RINGTONE_PATH_STR));

	ct_info->bday_remaining_days = __voicecall_service_get_bday_remaining_days(contact);

	CALL_ENG_DEBUG(ENG_DEBUG, "contact index:[%d]", ct_info->ct_index);
	CALL_ENG_DEBUG(ENG_DEBUG, "phone_type:[%d]", ct_info->phone_type);
	CALL_ENG_DEBUG(ENG_DEBUG, "bday_remaining_days :[%d]", ct_info->bday_remaining_days);
	CALL_ENG_DEBUG(ENG_DEBUG, "display name:[%s]", ct_info->display_name);
	CALL_ENG_DEBUG(ENG_DEBUG, "img path:[%s]", ct_info->caller_id_path);
	CALL_ENG_DEBUG(ENG_DEBUG, "full img path:[%s]", ct_info->caller_full_id_path);
	CALL_ENG_DEBUG(ENG_DEBUG, "ringtone path:[%s]", ct_info->ring_tone);

	contacts_svc_struct_free(contact);
	contacts_svc_disconnect();
	CALL_ENG_DEBUG(ENG_DEBUG, "Ended!!");
	CALL_ENG_KPI("voicecall_service_contact_info_by_number done");
	return TRUE;
}

/**
 * This function gets  contact info from contact id
 *
 * @return		gboolean
 * @param[in]		phonenumber		phone number
 * @param[in]		contact_id		contact id
 * @param[in]		storage_type		storage type
 * @param[out]	ct_info			contact info
 */
gboolean voicecall_service_contact_info_by_contact_id(char *phonenumber, int contact_id, voicecall_contact_info_t * ct_info)
{
#ifdef _CT_SEARCH_BY_ID_
	VOICECALL_RETURN_FALSE_IF_FAIL(phonenumber != NULL);
	VOICECALL_RETURN_FALSE_IF_FAIL(ct_info != NULL);

	int bfound = FALSE;
	int ret;
	contact_t record;
	int index = 0;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	memset(&record, 0, sizeof(contact_t));

	if (contact_db_service_connect() != CONTACT_OPERATION_SUCCESS) {
		CALL_ENG_DEBUG(ENG_DEBUG, "contact_db_service_connect error");
		return FALSE;
	}

	ret = contact_db_service_contact_get(CONTACT_DB_GET_RECORD_BY_KEY, contact_id, storage_type, &record);

	if (ret == CONTACT_OPERATION_SUCCESS) {
		bfound = TRUE;
	} else {
		bfound = FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, " bfound = [%d]", bfound);
	if (bfound) {
		if (record.contact.display_name != NULL)
			_vc_core_util_strcpy(ct_info->display_name, sizeof(ct_info->display_name), record.contact.display_name);
		if (record.contact.media_data.ringtone_path != NULL)
			_vc_core_util_strcpy(ct_info->ring_tone, sizeof(ct_info->ring_tone), record.contact.media_data.ringtone_path);

		ct_info->ct_index = contact_id;
		ct_info->storage_type = storage_type;

		CALL_ENG_DEBUG(ENG_DEBUG, "phone count:[%d]", record.phone_count);
		for (index = 0; index < record.phone_count; index++) {
			if (strcmp(record.phone_info[index].number, phonenumber) == 0) {
				ct_info->phone_type = record.phone_info[index].type;
			}
		}
		CALL_ENG_DEBUG(ENG_DEBUG, "***  found  ***");
		CALL_ENG_DEBUG(ENG_DEBUG, "name:[%s] vs [%s]", ct_info->display_name, record.contact.display_name);
		CALL_ENG_DEBUG(ENG_DEBUG, "ringtone path:[%s] vs [%s]", ct_info->ring_tone, record.contact.media_data.ringtone_path);
	} else {
		ct_info->ct_index = contact_id;
		ct_info->storage_type = storage_type;
		ct_info->phone_type = -1;

		CALL_ENG_DEBUG(ENG_DEBUG, "not found. name:[%s], path:[%s]", ct_info->display_name, ct_info->caller_id_path);
	}

	contact_db_service_contact_free(&record);
	if (contact_db_service_disconnect() != CONTACT_OPERATION_SUCCESS) {
		CALL_ENG_DEBUG(ENG_DEBUG, "contact_db_service_disconnect error");
		return FALSE;
	}
#endif
	return TRUE;
}

/**
 * This function on the loud speaker state
 *
 * @return		gboolean
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_service_loudspeaker_on(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	gboolean ret = FALSE;
	int total_call_member = -1;

	CALL_ENG_DEBUG(ENG_DEBUG, "");

	voicecall_core_get_total_call_member(pcall_core->pcall_engine, &total_call_member);
	if (total_call_member == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "There are not active calls hence it should not work");
		return FALSE;
	}

	/* Toggle the LoudSpeaker Status */
	if (voicecall_snd_get_path_status(pcall_core->papp_snd) != VOICE_CALL_SND_PATH_SPEAKER) {
		if (TRUE == _vc_bt_is_bt_connected(pcall_core) &&
		(voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_BT)) {
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_SPEAKER);
/*			_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_PHONE, -1, NULL);*/
			_vc_bt_request_switch_headset_path(pcall_core, FALSE);
		} else {
			voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_SPEAKER);
			voicecall_snd_change_path(papp_snd);
		}
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "loudspeacker is already on.");
		ret = FALSE;
	}
	return ret;
}

/**
 * This function off the loud speaker state
 *
 * @return		gboolean
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_service_loudspeaker_off(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	gboolean ret = FALSE;
	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	int total_call_member = -1;
	voicecall_core_get_total_call_member(pcall_core->pcall_engine, &total_call_member);
	if (total_call_member == 0) {
		CALL_ENG_DEBUG(ENG_DEBUG, "There are not active calls hence it should not work");
		return FALSE;
	}

	CALL_ENG_DEBUG(ENG_DEBUG, "");

		if (voicecall_snd_get_path_status(pcall_core->papp_snd) == VOICE_CALL_SND_PATH_SPEAKER) {
			if (TRUE == _vc_bt_is_bt_connected(pcall_core)) {
				voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_BT);
				/*_vc_bt_send_response_to_bt(pcall_core, BT_AG_RES_SWITCH_TO_HEADSET, -1, NULL);*/
				_vc_bt_request_switch_headset_path(pcall_core, TRUE);
			} else {
				CALL_ENG_DEBUG(ENG_DEBUG, "Set VOICE_CALL_SND_PATH_RECEIVER_EARJACK");
				voicecall_snd_set_path_status(pcall_core->papp_snd, VOICE_CALL_SND_PATH_RECEIVER_EARJACK);
				voicecall_snd_change_path(papp_snd);
			}
		} else {
			CALL_ENG_DEBUG(ENG_DEBUG, "loudspeacker is already on.");
			ret = FALSE;
		}
	return ret;
}

/**
 * This function is mute on
 *
 * @return		void
 * @param[in]		pcall_core		Handle to voicecall core
 */

gboolean voicecall_service_mute_status_on(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	gboolean bactive_calls = FALSE;
	gboolean bheld_calls = FALSE;

	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	voicecall_core_is_call_exists(pcall_engine, &bactive_calls, &bheld_calls);

	if (FALSE == bactive_calls && TRUE == bheld_calls) {
		CALL_ENG_DEBUG(ENG_DEBUG, "nothing to do.");
		/*Mute button should not be handled if only held calls exists */
		return TRUE;
	}

	if (FALSE == papp_snd->bmute_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Setting Voice Audio Mute Status On.");
		voicecall_core_set_audio_mute_status(pcall_engine, TRUE);
		papp_snd->bmute_status = TRUE;
		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "mute status is already on.");
		return FALSE;
	}

}

/**
 * This function is mute off
 *
 * @return		void
 * @param[in]		pcall_core		Handle to voicecall core
 */
gboolean voicecall_service_mute_status_off(call_vc_core_state_t *pcall_core)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	gboolean bactive_calls = FALSE;
	gboolean bheld_calls = FALSE;

	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;
	voicecall_engine_t *pcall_engine = pcall_core->pcall_engine;

	CALL_ENG_DEBUG(ENG_DEBUG, "..");

	voicecall_core_is_call_exists(pcall_engine, &bactive_calls, &bheld_calls);

	if (FALSE == bactive_calls && TRUE == bheld_calls) {
		CALL_ENG_DEBUG(ENG_DEBUG, "nothing to do.");
		/*Mute button should not be handled if only held calls exists */
		return TRUE;
	}

	if (TRUE == papp_snd->bmute_status) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Setting Voice Audio Mute Status Off.");
		voicecall_core_set_audio_mute_status(pcall_engine, FALSE);
		papp_snd->bmute_status = FALSE;
		return TRUE;
	} else {
		CALL_ENG_DEBUG(ENG_DEBUG, "mute status is already off.");
		return FALSE;
	}

}

/**
 * This function set volume level.
 *
 * @return		Returns TRUE on success or FALSE on failure
 * @param[in]		pcall_core			Handle to voicecall core
 * @param[in]		vol_alert_type			volume alert type
 * @param[in]		volume_level			volume level to be set
 */
gboolean voicecall_service_set_volume(call_vc_core_state_t *pcall_core, voicecall_snd_volume_alert_type_t vol_alert_type, int volume_level)
{
	VOICECALL_RETURN_FALSE_IF_FAIL(pcall_core != NULL);

	voicecall_snd_mgr_t *papp_snd = pcall_core->papp_snd;

	voicecall_snd_set_volume(papp_snd, vol_alert_type, volume_level);

	return TRUE;
}

