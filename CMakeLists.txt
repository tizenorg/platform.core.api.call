CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(project_prefix "capi")
SET(prefix "/usr")
SET(version "0.0.1")
SET(maintainer "ByungWoo Lee <bw1212.lee@samsung.com>")
SET(description "Telephony Call Framework")
SET(service "telephony")
SET(submodule "call")
SET(dependents "dlog vconf glib-2.0 capi-base-common")
SET(pc_dependents "capi-base-common")
SET(deb_dependents "dlog-dev libvconf-dev libglib2.0-dev capi-base-common-dev")

SET(Services 
        "application"
        "base"
        "content"
        "location"
        "media"
        "messaging"
        "network"
        "social"
        "telephony"
        "system"
   )

#FILE(STRINGS config.cfg configs REGEX "^ *[^#]")
FOREACH(lines ${configs})
#    MESSAGE(${lines})
    IF(${lines} MATCHES "([^=]*)=['\"](.*)['\"]")
        SET(key ${CMAKE_MATCH_1})
        SET(value ${CMAKE_MATCH_2})
#        MESSAGE("${key} -> ${value}")
        SET(${key} "${value}")
    ENDIF()
ENDFOREACH(lines ${configs})

LIST(FIND Services ${service} sfind)

IF( ${sfind} EQUAL -1 )
    MESSAGE(FATAL_ERROR "Service must be one of ")
    FOREACH( s IN ${Services} )
        MESSAGE(FATAL_ERROR "[${s}]")
    ENDFOREACH( s IN ${Services} )
ENDIF( ${sfind} EQUAL -1 )

SET(fw_name "${project_prefix}-${service}-${submodule}")

PROJECT(${fw_name})

SET(CMAKE_INSTALL_PREFIX ${prefix})
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION ${version})

SET(INC_DIR include)
INCLUDE_DIRECTORIES(${INC_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_name} REQUIRED ${dependents})
FOREACH(flag ${${fw_name}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -Wall -Werror")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DTIZEN_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--rpath=/usr/lib")

aux_source_directory(src SOURCES)
ADD_LIBRARY(${fw_name} SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(${fw_name} ${${fw_name}_LDFLAGS})

SET_TARGET_PROPERTIES(${fw_name}
     PROPERTIES
     VERSION ${FULLVER}
     SOVERSION ${MAJORVER}
     CLEAN_DIRECT_OUTPUT 1
)

INSTALL(TARGETS ${fw_name} DESTINATION lib)
INSTALL(
        DIRECTORY ${INC_DIR}/ DESTINATION include/${service}
        FILES_MATCHING
        PATTERN "*_private.h" EXCLUDE
        PATTERN "${INC_DIR}/*.h"
        )

SET(PC_NAME ${fw_name})
SET(PC_REQUIRED ${pc_dependents})
SET(PC_LDFLAGS -l${fw_name})

CONFIGURE_FILE(
    capi-telephony-call.pc.in
    ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc
    @ONLY
)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc DESTINATION lib/pkgconfig)

IF(UNIX)

ADD_CUSTOM_TARGET (distclean @echo cleaning for source distribution)
ADD_CUSTOM_COMMAND(
        DEPENDS clean 
        COMMENT "distribution clean"
        COMMAND find
        ARGS    . 
        -not -name config.cmake -and \(
        -name tester.c -or
        -name Testing -or
        -name CMakeFiles -or
        -name cmake.depends -or
        -name cmake.check_depends -or
        -name CMakeCache.txt -or
        -name cmake.check_cache -or
        -name *.cmake -or
        -name Makefile -or
        -name core -or
        -name core.* -or
        -name gmon.out -or
        -name install_manifest.txt -or
        -name *.pc -or
        -name *~ \)
        | grep -v TC | xargs rm -rf
        TARGET  distclean
        VERBATIM
)

ENDIF(UNIX)
