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


#include "vcui-application.h"
#include "vcui-view-keypad.h"
#include "vcui-view-common.h"

typedef struct _keypad_data_t {
	Evas_Object *parent_ly;
	Evas_Object *keypad_ly;
	Evas_Object *separator_ly;
	Evas_Object *entry;
	int data_len;
	char entry_disp_data[KEYPAD_ENTRY_DISP_DATA_SIZE+1];
	Eina_Bool bkeypad_show;
}keypad_data_t;

static keypad_data_t *gkeypad_data;

static void __vcui_keypad_create_separator_layout(void *data);
static void __vcui_keypad_create_entry(void *data);

static Evas_Object *__vcui_keypad_create_contents(void *data, char *grp_name)
{
	if (data == NULL) {
		CALL_UI_DEBUG("ERROR");
		return NULL;
	}
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;

	Evas_Object *eo;

	/* load edje */
	eo = _vcui_load_edj(vd->app_data->win_main, EDJ_NAME, grp_name);
	if (eo == NULL)
		return NULL;

	return eo;
}

static keypad_data_t *__vcui_keypad_memory_alloc()
{
	CALL_UI_DEBUG("..");

	VCUI_RETURN_VALUE_IF_FAIL(gkeypad_data == NULL, gkeypad_data)

	CALL_UI_DEBUG("..");
	gkeypad_data = (keypad_data_t *) calloc(1, sizeof(keypad_data_t));
	if (gkeypad_data == NULL) {
		CALL_UI_DEBUG("keydata structure not allocated");
		return NULL;
	}
	memset(gkeypad_data, 0x00, sizeof(keypad_data_t));
	gkeypad_data->bkeypad_show = EINA_FALSE;

	return gkeypad_data;
}

void _vcui_keypad_create_layout(void *data, Evas_Object *parent_ly)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	keypad_data_t *pkeypad_data = NULL;

	pkeypad_data = __vcui_keypad_memory_alloc();
	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	if (!pkeypad_data->keypad_ly) {
		CALL_UI_DEBUG("..");
		pkeypad_data->keypad_ly = __vcui_keypad_create_contents(vd, GRP_KEYPAD);
		elm_object_part_content_set(parent_ly, "keypad_region", pkeypad_data->keypad_ly);
	}
	CALL_UI_DEBUG("keypad_ly :%p", pkeypad_data->keypad_ly);

	__vcui_keypad_create_separator_layout(vd);

	__vcui_keypad_create_entry(vd);
}

static void __vcui_keypad_create_separator_layout(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	if (!pkeypad_data->separator_ly) {
		CALL_UI_DEBUG("..");
		pkeypad_data->separator_ly = __vcui_keypad_create_contents(vd, GRP_KEYPAD_SEP_LAYOUT);
		elm_object_part_content_set(pkeypad_data->keypad_ly, "separator/bg", pkeypad_data->separator_ly);
	}
}

Eina_Bool _vcui_keypad_get_show_status(void)
{
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_FALSE_IF_FAIL(pkeypad_data != NULL)

	CALL_UI_DEBUG("Get show status(%d)", pkeypad_data->bkeypad_show);
	return pkeypad_data->bkeypad_show;
}

void _vcui_keypad_set_show_status(Eina_Bool bkeypad_status)
{
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	CALL_UI_DEBUG("Set show status(%d)", bkeypad_status);
	pkeypad_data->bkeypad_show = bkeypad_status;
}

static void __vcui_keypad_on_key_down(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	CALL_UI_DEBUG("source(%s)", source);

	char before_dest[KEYPAD_STR_DEFINE_OPEN_SIZE + KEYPAD_ENTRY_DISP_DATA_SIZE + 1] = { 0, };
	char *sub_buffer_pointer = NULL;
	char entry_dest[KEYPAD_ENTRY_SET_DATA_SIZE + 1] = { 0, };
	char *keypad_source;
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	if (NULL == vd) {
		CALL_UI_DEBUG("data is null");
		return;
	}

	if (strlen(source) >= KEYPAD_ENTRY_DISP_DATA_SIZE) {
		CALL_UI_DEBUG("strlen(source) >= DATA_BUF_SIZE(%d)", KEYPAD_ENTRY_DISP_DATA_SIZE);
		return;
	}

	if (strcmp(source, "star") == 0) {
		keypad_source = "*";
	} else if (strcmp(source, "sharp") == 0) {
		keypad_source = "#";
	} else {
		keypad_source = (char *)source;
	}

	char dtmf_number[2];
	dtmf_number[0] = keypad_source[0];
	dtmf_number[1] = '\0';
	vcall_engine_send_dtmf_number(dtmf_number);

	if (strlen(pkeypad_data->entry_disp_data) == KEYPAD_ENTRY_DISP_DATA_SIZE) {
		sub_buffer_pointer = &pkeypad_data->entry_disp_data[1];

		snprintf(pkeypad_data->entry_disp_data, sizeof(pkeypad_data->entry_disp_data), "%s", sub_buffer_pointer);
		CALL_UI_DEBUG("pkeypad_data->entry_disp_data after change [%s]", pkeypad_data->entry_disp_data);
	}

	pkeypad_data->entry_disp_data[pkeypad_data->data_len] = keypad_source[0];
	if (pkeypad_data->data_len < (KEYPAD_ENTRY_DISP_DATA_SIZE - 1)) {
		pkeypad_data->data_len++;
	}

	snprintf(before_dest, sizeof(before_dest), "<font_size=%d><color=#FFFFFF><shadow_color=#000000><style=outline_shadow>%s", MAX_DIAL_NUMBER_FONT_SIZE, pkeypad_data->entry_disp_data);

	snprintf(entry_dest, sizeof(entry_dest), "%s</style></shadow_color></color></font_size>", before_dest);

	CALL_UI_DEBUG("entry_dest [%s]", pkeypad_data->entry_disp_data);
	elm_entry_entry_set(pkeypad_data->entry, entry_dest);
	elm_entry_cursor_end_set(pkeypad_data->entry);
}

static Evas_Object *__vcui_keypad_create_single_line_scrolled_entry(void *content)
{
	Evas_Object *en;

	if (content == NULL) {
		CALL_UI_DEBUG("content is NULL!");
		return NULL;
	}

	en = elm_entry_add(content);
	elm_entry_scrollable_set(en, EINA_TRUE);

	elm_entry_select_all(en);
	elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	elm_entry_bounce_set(en, EINA_FALSE, EINA_FALSE);
	elm_entry_line_wrap_set(en, ELM_WRAP_WORD);
	elm_entry_input_panel_enabled_set(en, EINA_FALSE);
	elm_entry_single_line_set(en, EINA_TRUE);
	elm_entry_cursor_end_set(en);
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(en);

	return en;
}

static void __vcui_keypad_create_entry(void *data)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *)data;
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	if (!pkeypad_data->entry) {
		CALL_UI_DEBUG("..");
		pkeypad_data->entry = __vcui_keypad_create_single_line_scrolled_entry(pkeypad_data->keypad_ly);
		memset(pkeypad_data->entry_disp_data, 0x0, sizeof(pkeypad_data->entry_disp_data));
		pkeypad_data->data_len = 0;

		edje_object_signal_callback_add(_EDJ(pkeypad_data->keypad_ly), "pad_down", "*", __vcui_keypad_on_key_down, vd);

		edje_object_part_swallow(_EDJ(pkeypad_data->keypad_ly), "textblock/textarea", pkeypad_data->entry);
	}
}

void _vcui_keypad_delete_layout(Evas_Object *parent_ly)
{
	CALL_UI_DEBUG("..");
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	if (pkeypad_data->separator_ly) {
		CALL_UI_DEBUG("..");
		edje_object_part_unswallow(_EDJ(pkeypad_data->keypad_ly), pkeypad_data->separator_ly);
		evas_object_del(pkeypad_data->separator_ly);
		pkeypad_data->separator_ly = NULL;
	}

	if (pkeypad_data->entry) {
		CALL_UI_DEBUG("..");
		edje_object_part_unswallow(_EDJ(pkeypad_data->keypad_ly), pkeypad_data->entry);
		evas_object_del(pkeypad_data->entry);
		pkeypad_data->entry = NULL;
	}

	if (pkeypad_data->keypad_ly) {
		CALL_UI_DEBUG("..");
		edje_object_part_unswallow(_EDJ(parent_ly), pkeypad_data->keypad_ly);
		evas_object_del(pkeypad_data->keypad_ly);
		pkeypad_data->keypad_ly = NULL;
	}

	pkeypad_data->parent_ly = NULL;

	free(pkeypad_data);
	pkeypad_data = NULL;
	gkeypad_data = NULL;
}

void _vcui_keypad_delete_separator_layout(void)
{
	CALL_UI_DEBUG("..");
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	if (pkeypad_data->separator_ly) {
		edje_object_part_unswallow(_EDJ(pkeypad_data->keypad_ly), pkeypad_data->separator_ly);
		evas_object_del(pkeypad_data->separator_ly);
		pkeypad_data->separator_ly = NULL;
	}
}

static void __vcui_keypad_hide_btn_effect_done(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;
	keypad_data_t *pkeypad_data = gkeypad_data;
	Evas_Object *parent_ly = NULL;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	/*Reset the entry area and text*/
	memset(pkeypad_data->entry_disp_data, '\0', KEYPAD_ENTRY_DISP_DATA_SIZE+1);
	pkeypad_data->data_len = 0;
	elm_entry_entry_set(pkeypad_data->entry, "");
	elm_entry_cursor_end_set(pkeypad_data->entry);

	parent_ly = pkeypad_data->parent_ly;
	VCUI_RETURN_IF_FAIL(parent_ly != NULL)

	/*Hide keypad layout - ON view*/
	edje_object_signal_emit(_EDJ(parent_ly), "HIDE", "KEYPAD_BTN");
	/*Show caller info area*/
	edje_object_signal_emit(_EDJ(parent_ly), "SHOW", "CALL_AREA");


	if (ad->badd_call_clicked) {
		_vcui_doc_launch_phoneui_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data));
		ad->badd_call_clicked = EINA_FALSE;
	} else if (ad->bcontact_clicked) {
		_vcui_doc_launch_contact_list_ug(NULL, NULL, vd->layout, NULL, NULL, NULL, &(vd->app_data->ugs_array_data));
		ad->bcontact_clicked = EINA_FALSE;
	}
	ad->beffect_show = EINA_FALSE;
}

void _vcui_keypad_show_hide_effect(void *data, Evas_Object *parent_ly)
{
	CALL_UI_DEBUG("..");
	voice_call_view_data_t *vd = (voice_call_view_data_t *) data;
	vcui_app_call_data_t *ad = vd->app_data;
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	ad->beffect_show = EINA_TRUE;

	edje_object_signal_emit(_EDJ(pkeypad_data->keypad_ly), "HIDE", "KEYPADBTN");
	pkeypad_data->parent_ly = parent_ly;
	edje_object_signal_callback_add(_EDJ(pkeypad_data->keypad_ly), "DONE", "HIDEKEYPAD", __vcui_keypad_hide_btn_effect_done, vd);
}

void _vcui_keypad_show_layout(void *data)
{
	CALL_UI_DEBUG("..");
	keypad_data_t *pkeypad_data = gkeypad_data;

	VCUI_RETURN_IF_FAIL(pkeypad_data != NULL)

	edje_object_signal_emit(_EDJ(pkeypad_data->keypad_ly), "SHOW", "KEYPADBTN");
}
