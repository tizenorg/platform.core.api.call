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


#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "voice-call-dbus.h"
#include "voice-call-engine.h"

#include <aul.h>

/*  Voice Call  <-- BT */
#define DBUS_BT_MATCH_RULE 			"type='signal',path='/org/projectx/bluetooth_event',interface='User.Bluetooth.AG'"
#define DBUS_BT_OBJECT_REQUEST  		"/org/projectx/bluetooth_event"
#define DBUS_BT_INTERFACE_REQUEST	"User.Bluetooth.AG"
#define DBUS_BT_METHOD_REQUEST 		"Request"

/*  Voice Call  --> BT */
#define DBUS_BT_SERVICE				"org.projectx.bluetooth"
#define DBUS_BT_OBJECT_RESPONSE  		"/org/projectx/btcall_event"
#define DBUS_BT_INTERFACE_RESPONSE 	"User.Bluetooth.AG"
#define DBUS_BT_METHOD_RESPONSE		"Response"
//#define DBUS_BT_METHOD_CONNECT                "Connect"

#define BT_PKG          "org.tizen.bluetooth"

static DBusGConnection *gconnection = NULL;

typedef struct _dbus_dest_t {
	char *service;
	char *object_path;
	char *interface;
	char *method;
} dbus_dest_t;

static int vc_engine_send_via_dbus(DBusGConnection * conn, dbus_dest_t * dest, int first_arg_type, ...)
{
	DBusMessage *msg;
	DBusMessageIter iter;
	va_list list;
	int type;
	dbus_bool_t ret;

	dbus_int32_t val_int;
	char *val_str;

	CALL_ENG_DEBUG(ENG_DEBUG, "path:%s, interface:%s, method:%s \n", dest->object_path, dest->interface, dest->method);
	msg = dbus_message_new_signal(dest->object_path, dest->interface, dest->method);
	if (msg == NULL) {
		CALL_ENG_DEBUG(ENG_DEBUG, "dbus_message_new_signal failed.\n");
		return VC_ERROR;
	}
	dbus_message_set_destination(msg, dest->service);

	dbus_message_iter_init_append(msg, &iter);

	type = first_arg_type;

	va_start(list, first_arg_type);
	while (type != DBUS_TYPE_INVALID) {
		switch (type) {
		case DBUS_TYPE_INT32:
			val_int = *(int *)(va_arg(list, dbus_int32_t));
			dbus_message_iter_append_basic(&iter, type, &val_int);
			break;

		case DBUS_TYPE_STRING:
			val_str = va_arg(list, char *);
			dbus_message_iter_append_basic(&iter, type, &val_str);
			break;
		}
		type = va_arg(list, int);
	}
	va_end(list);

	ret = dbus_connection_send(dbus_g_connection_get_connection(conn), msg, NULL);
	dbus_connection_flush(dbus_g_connection_get_connection(conn));
	dbus_message_unref(msg);

	if (ret != TRUE)
		return VC_ERROR;

	return VC_NO_ERROR;
}

void vc_engine_on_dbus_send_connect_to_bt(void)
{
	bundle *kb;
	kb = bundle_create();
	bundle_add(kb, "launch-type", "call");
	aul_launch_app(BT_PKG, kb);
	bundle_free(kb);
	CALL_ENG_DEBUG(ENG_DEBUG, "End..");
}

void vc_engine_on_dbus_send_response_to_bt(connectivity_bt_ag_param_info_t bt_resp_info)
{
	dbus_dest_t bt_dbus_dest = {
		DBUS_BT_SERVICE,
		DBUS_BT_OBJECT_RESPONSE,
		DBUS_BT_INTERFACE_RESPONSE,
		DBUS_BT_METHOD_RESPONSE
	};
	CALL_ENG_DEBUG(ENG_DEBUG, "..\n");

	vc_engine_send_via_dbus(gconnection, &bt_dbus_dest, 
			DBUS_TYPE_INT32, &bt_resp_info.param1,
			DBUS_TYPE_INT32, &bt_resp_info.param2,
			DBUS_TYPE_INT32, &bt_resp_info.param3,
			DBUS_TYPE_STRING, bt_resp_info.param4,
			DBUS_TYPE_INVALID); 	
}

/* Handle all bluetooth relative signal */
static void vc_engine_on_dbus_parsing_bt(void *user_data, DBusMessage * message)
{
	DBusMessageIter iter;
	connectivity_bt_ag_param_info_t bt_event_info = { 0, };
	call_vc_core_state_t *pcall_core = (call_vc_core_state_t *)vcall_engine_get_core_state();

	if (dbus_message_iter_init(message, &iter))
		dbus_message_iter_get_basic(&iter, &bt_event_info.param1);

	if (dbus_message_iter_next(&iter))
		dbus_message_iter_get_basic(&iter, &bt_event_info.param2);

	if (dbus_message_iter_next(&iter))
		dbus_message_iter_get_basic(&iter, &bt_event_info.param3);

	if (dbus_message_iter_next(&iter))
		dbus_message_iter_get_basic(&iter, &bt_event_info.param4);

	CALL_ENG_DEBUG(ENG_DEBUG,"param1:[%d], param2[%d], param3[%d], param4[%s] \n",
			bt_event_info.param1, bt_event_info.param2, bt_event_info.param3, bt_event_info.param4);

	_vc_bt_handle_bt_events(pcall_core, &bt_event_info);
}

/* Handle all dbus signal */
static DBusHandlerResult vc_engine_on_dbus_receive(DBusConnection * connection, DBusMessage * message, void *user_data)
{
	int type;

	const char *interface_name = dbus_message_get_interface(message);
	const char *method_name = dbus_message_get_member(message);
	const char *object_path = dbus_message_get_path(message);

	type = dbus_message_get_type(message);
	if (type != DBUS_MESSAGE_TYPE_SIGNAL) {
		/* 
		 * INVALID: 0
		 * METHOD_CALL: 1
		 * METHOD_CALL_RETURN: 2
		 * ERROR: 3
		 * SIGNAL: 4 
		 */
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	if (object_path == NULL) {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
	if (interface_name == NULL) {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
	if (method_name == NULL) {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	/* Check BT Signal */
	if ((strcmp(object_path, DBUS_BT_OBJECT_REQUEST) == 0) && (strcmp(interface_name, DBUS_BT_INTERFACE_REQUEST) == 0)) {
		CALL_ENG_DEBUG(ENG_DEBUG, "received DBus BT signal!\n");
		if (strcmp(method_name, DBUS_BT_METHOD_REQUEST) == 0) {
			CALL_ENG_DEBUG(ENG_DEBUG, "BT Method :[Request] \n");
			vc_engine_on_dbus_parsing_bt(user_data, message);
			return DBUS_HANDLER_RESULT_HANDLED;
		}
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int vc_engine_dbus_receiver_setup()
{
	GError *error = NULL;
	DBusError derror;
	int ret;

	// connectio to dbus-daemon.    
	gconnection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (!gconnection) {
		printf("Could not get connection: %s\n", error->message);
		return FALSE;
	}
	
	dbus_error_init(&derror);

	dbus_bus_add_match(dbus_g_connection_get_connection(gconnection), DBUS_BT_MATCH_RULE, &derror);
	if (dbus_error_is_set(&derror))	// failure
	{
		CALL_ENG_DEBUG(ENG_DEBUG, "Failed to dbus_bus_add_match(%s): %s\n", DBUS_BT_MATCH_RULE, derror.message);
		dbus_error_free(&derror);
		return FALSE;
	}
	// register event filter to handle received dbus-message.   
	ret = dbus_connection_add_filter(dbus_g_connection_get_connection(gconnection), vc_engine_on_dbus_receive, NULL, NULL);
	if (ret != TRUE) {
		CALL_ENG_DEBUG(ENG_DEBUG, "Failed to dbus_connection_add_filter");
		return FALSE;
	}

	return TRUE;
}

