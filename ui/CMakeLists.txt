CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(UI_SRCS 
	../call-engine/core/vc-core-engine.c
	../call-engine/core/vc-core-callmanager.c
	../call-engine/core/vc-core-engine-status.c 
	../call-engine/core/vc-core-tapi-evnt.c
	../call-engine/core/vc-core-util.c
	../call-engine/core/vc-core-callagent.c
	../call-engine/core/vc-core-ccbs.c
	../call-engine/core/vc-core-engine-group.c 
	../call-engine/core/vc-core-svcall.c
	../call-engine/core/vc-core-tapi-rqst.c
	../call-engine/core/vc-core-ecc.c
	../call-engine/voice-call-core.c
	../call-engine/voice-call-sound.c
	../call-engine/voice-call-service.c
	../call-engine/voice-call-device.c
	../call-engine/voice-call-bt.c
	../call-engine/voice-call-engine.c
	../call-engine/voice-call-dbus.c
	./src/vcui-application.c
	./src/vcui-app-window.c
	./src/vcui-view-choice.c
	./src/vcui-document.c
	./src/vcui-view-dialing.c
	./src/vcui-view-incoming-lock.c
	./src/vcui-view-keypad.c
	./src/vcui-view-elements.c
	./src/vcui-view-common.c
	./src/vcui-view-single-call.c
	./src/vcui-view-multi-call-split.c
	./src/vcui-view-multi-call-conf.c
	./src/vcui-view-multi-call-list.c
	./src/vcui-view-popup.c
	./src/vcui-doc-launch.c
	./src/vcui-view-callend.c
	./src/vcui-view-quickpanel.c	#_QUICKPANNEL_SUPPORT_
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ../call-engine/include ../call-engine/core/include ./src/include)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "${PREFIX}/data")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "${RESDIR}/icons/default/small")
SET(EDJDIR "${RESDIR}/edje")
SET(IMGDIR "${RESDIR}/images")
SET(MEDIADIR "${RESDIR}/media")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")


# for LOG
ADD_DEFINITIONS("-DCALL_DEBUG_ON_DLOG")

# for Engine
ADD_DEFINITIONS("-DCALL_VC_SEND_SAT_RESPONSE_ORIG")
ADD_DEFINITIONS("-DRELEASE_ALL_AND_ACCEPT_SUPPORT")
#ADD_DEFINITIONS("-DVC_WITHOUT_SWAP_CNF")

# for SOUND

# for debuging			
#ADD_DEFINITIONS("-DVOICECALL_KPI")
#ADD_DEFINITIONS("-DCALLDETAILS_DUMP")

#for UI
#ADD_DEFINITIONS("-DSEND_END_TO_UG")
ADD_DEFINITIONS("-DPDIAL_SEND_DTMF")
ADD_DEFINITIONS("-D_QUICKPANNEL_SUPPORT_")
ADD_DEFINITIONS("-DSIGNAL_HANDLER")

INCLUDE(FindPkgConfig)
pkg_check_modules(ui_pkgs REQUIRED
	appcore-efl
	vconf
	utilX
	alsa
	pmapi	
	sensor
	dlog
	aul
	contacts-service
	ui-gadget-1
	tapi 
	contacts-service 
	capi-media-sound-manager
	mm-sound
	mm-camcorder
	mm-player
	mm-session 
	pmapi 
	devman_haptic 
	msg-service 
	ecore
	ecore-x
	ecore-input
	edje
	eina
	evas
	elementary
	icu-i18n
	appsvc
	notification
	minicontrol-provider
	dbus-1
	dbus-glib-1
	capi-system-info
)


SET(EXTRA_CFLAGS "")
FOREACH(flag ${ui_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${COMMON_C_FLAGS} -fpie ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")
ADD_DEFINITIONS("-DDATADIR=\"${DATADIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DICONDIR=\"${ICONDIR}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-DIMGDIR=\"${IMGDIR}\"")
ADD_DEFINITIONS("-DMEDIADIR=\"${MEDIADIR}\"")

ADD_EXECUTABLE(voice-call-ui ${UI_SRCS})
TARGET_LINK_LIBRARIES(voice-call-ui "-pie" "-ldl" ${ui_pkgs_LDFLAGS})
message(${ui_pkgs_LDFLAGS})

ADD_CUSTOM_TARGET(voice-call-ui.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/images ${SHARED_FLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/call_edc/voice-call-ui.edc ${CMAKE_CURRENT_SOURCE_DIR}/voice-call-ui.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/call_edc/voice-call-ui.edc
)
ADD_DEPENDENCIES(voice-call-ui voice-call-ui.edj)

ADD_CUSTOM_TARGET(call_theme.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/themes
		${CMAKE_CURRENT_SOURCE_DIR}/themes/call_theme.edc ${CMAKE_CURRENT_SOURCE_DIR}/themes/call_theme.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/themes/call_theme.edc
)
ADD_DEPENDENCIES(voice-call-ui voice-call-ui.edj call_theme.edj)

#install binary & edj files
INSTALL(TARGETS voice-call-ui DESTINATION ${BINDIR})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/voice-call-ui.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/themes/call_theme.edj DESTINATION ${EDJDIR})

# install ini file ( not used)
#INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/voice-call-ui.ini DESTINATION share/process-info)

# install manifest file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.call.xml DESTINATION /opt/share/packages)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.call.png DESTINATION /opt/share/icons/default/small)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images/hd/icon/ DESTINATION ${IMGDIR} FILES_MATCHING PATTERN "*.png")
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images/hd/landscape/ DESTINATION ${IMGDIR} FILES_MATCHING PATTERN "*.png")
INSTALL(DIRECTORY ../call-engine/sounds/ DESTINATION ${MEDIADIR} FILES_MATCHING PATTERN "*.wav")

# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${DATADIR})

# i18n
ADD_SUBDIRECTORY(po)
