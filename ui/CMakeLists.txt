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
	../call-engine/voice-call-core.c
	../call-engine/voice-call-sound.c
	../call-engine/voice-call-service.c
	../call-engine/voice-call-device.c
	../call-engine/voice-call-bt.c
	../call-engine/voice-call-engine.c
	../call-engine/voice-call-dbus.c	
	vcui-application.c
	vcui-app-window.c
	vcui-view-choice.c
	vcui-engine-interface.c
	vcui-document.c 
	vcui-view-dialing.c 
	vcui-view-incoming.c 
	vcui-view-incoming-lock.c
	vcui-view-keypad.c
	vcui-view-elements.c
	vcui-view-common.c 
	vcui-view-single-call.c
	vcui-view-multi-call-split.c
	vcui-view-multi-call-conf.c
	vcui-view-multi-call-list.c
	vcui-view-popup.c
	vcui-doc-launch.c
	vcui-view-callend.c
 )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ../call-engine/include ../call-engine/core/include)

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
ADD_DEFINITIONS("-DSWAP_SUPPORT")
ADD_DEFINITIONS("-DCALL_VC_SEND_SAT_RESPONSE_ORIG")
ADD_DEFINITIONS("-DRELEASE_ALL_AND_ACCEPT_SUPPORT")
ADD_DEFINITIONS("-DVC_ENG_FDN_SUPPORT")

# for debuging			
#ADD_DEFINITIONS("-DTIMER_ENABLED")
#ADD_DEFINITIONS("-DVOICECALL_KPI")
#ADD_DEFINITIONS("-DCALLDETAILS_DUMP")

#for UI
ADD_DEFINITIONS("-DEND_KEY_PROCESSING")
ADD_DEFINITIONS("-DPDIAL_SEND_DTMF")
#ADD_DEFINITIONS("-DBG")
ADD_DEFINITIONS("-D_NEW_SND_")
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
	ui-gadget
	tapi 
	contacts-service 
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
TARGET_LINK_LIBRARIES(voice-call-ui "-pie" ${ui_pkgs_LDFLAGS})
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

# install desktop file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.call.desktop.in ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.call.desktop)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.call.desktop DESTINATION /opt/share/applications)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images/hd/icon/ DESTINATION ${IMGDIR} FILES_MATCHING PATTERN "*.png")
INSTALL(DIRECTORY ../call-engine/sounds/ DESTINATION ${MEDIADIR} FILES_MATCHING PATTERN "*.wav")

# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${DATADIR})

# i18n
ADD_SUBDIRECTORY(po)
