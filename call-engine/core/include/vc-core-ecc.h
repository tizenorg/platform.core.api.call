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

#ifndef __VC_CORE_ECC_H_
#define __VC_CORE_ECC_H_

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include "vc-core-util.h"

#ifdef CALL_DEBUG_ON_DLOG
#include <dlog.h>
#endif

#define CALL_ECC_MAX_COUNT_DEFAULT_NO_SIM	7
#define CALL_ECC_MAX_COUNT_DEFAULT			2
#define CALL_ECC_MAX_COUNT_3					3
#define CALL_ECC_MAX_COUNT_4					4
#define CALL_ECC_MAX_COUNT_5					5
#define CALL_ECC_MAX_COUNT_6					6
#define CALL_ECC_MAX_COUNT_7					7
#define CALL_ECC_MAX_COUNT_8					8
#define CALL_ECC_MAX_COUNT_9					9
#define CALL_ECC_MAX_COUNT_10					10

gboolean _vc_core_ecc_check_emergency_number(TapiHandle *tapi_handle, TelSimCardType_t card_type, char *pNumber, gboolean b_is_no_sim, int *ecc_category);
#endif	/* __VC_CORE_ECC_H_ */
