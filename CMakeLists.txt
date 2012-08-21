CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(media-service C)
SET(VERSION_MAJOR 1)
SET(VERSION "${VERSION_MAJOR}.0.0")

SET(MEDIASERVICE-LIB "media-service")
SET(MEDIASERVICE-HASH-LIB "media-svc-hash")
SET(MEDIASERVICE-PLUGIN-LIB "media-svc-plugin")
SET(MEDIACONTENT-PLUGIN-LIB "media-content-plugin")
SET(SRCS 
	src/visual/visual-svc-db.c
	src/visual/visual-svc-db-util.c
	src/visual/visual-svc-util.c
	src/visual/media-svc-api.c
	src/visual/media-svc-thumb.c   
    
	src/visual/IfegDecodeAGIF.c
	src/visual/media-img-codec.c
	src/visual/media-img-codec-osal.c
	src/visual/media-img-codec-parser.c

	src/visual/minfo-item.c	
	src/visual/minfo-tag.c
	src/visual/minfo-cluster.c
	src/visual//minfo-bookmark.c
	src/visual/minfo-streaming.c
	src/visual/minfo-meta.c
	src/visual/minfo-api.c

	src/audio/audio-svc-music-table.c
	src/audio/audio-svc-playlist-table.c
	src/audio/audio-svc-ta.c
	src/audio/audio-svc-utils.c
	src/audio/audio-svc-db-utils.c
	src/audio/audio-svc.c

	uuid/clear.c
	uuid/compare.c
	uuid/copy.c
	uuid/gen_uuid.c
	uuid/isnull.c
	uuid/pack.c
	uuid/parse.c
	uuid/unpack.c
	uuid/unparse.c
	uuid/uuid_time.c

	src/common/media-svc.c
	src/common/media-svc-media.c
	src/common/media-svc-album.c
	src/common/media-svc-media-folder.c
	src/common/media-svc-db-utils.c
	src/common/media-svc-util.c
	src/common/media-svc-debug.c
	)

SET(HASH_SRCS
	md5/md5.c
	md5/media-svc-hash.c
	)

SET(PLUGIN_SRCS
	plugin/media-svc-plugin.c
	)
SET(CONTENTPLUGIN_SRCS
	plugin/media-content-plugin.c
	)

SET(TEST-SRCS test/test_media_info.c)
SET(TEST-AUDIO-SRCS test/audio/audio_svc_test.c)
SET(TEST-VISUAL-SRCS test/test_visual.c)

SET(VENDOR "samsung")
SET(PACKAGE ${PROJECT_NAME})
SET(PKGNAME "com.${VENDOR}.${PACKAGE}")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(DATADIR "${PREFIX}/share")
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/include/visual ${CMAKE_SOURCE_DIR}/src/include/audio ${CMAKE_SOURCE_DIR}/src/include/common ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/uuid ${CMAKE_SOURCE_DIR}/md5)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED 
glib-2.0 gobject-2.0 libpng libpng12 gio-2.0 gio-unix-2.0 
dlog sqlite3 db-util mm-common  mm-fileinfo drm-client aul media-thumbnail
libexif gthread-2.0 vconf ecore-evas evas mmutil-imgp mmutil-jpeg dbus-1)



FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fPIC")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DVENDOR=\"${VENDOR}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DPACKAGE_NAME=\"${PKGNAME}\"")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=64")
#ADD_DEFINITIONS("-D_PERFORMANCE_CHECK_")
#ADD_DEFINITIONS("-D_USE_LOG_FILE_")
#ADD_DEFINITIONS("-D_USE_DBUS_")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--hash-style=both")
CONFIGURE_FILE(libmedia-service.pc.in libmedia-service.pc @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libmedia-service.pc DESTINATION lib/pkgconfig)

LINK_DIRECTORIES(lib)

ADD_LIBRARY(${MEDIASERVICE-HASH-LIB} SHARED ${HASH_SRCS})
SET_TARGET_PROPERTIES(${MEDIASERVICE-HASH-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${MEDIASERVICE-HASH-LIB} PROPERTIES VERSION ${VERSION})
ADD_LIBRARY(${MEDIASERVICE-LIB} SHARED ${SRCS}) 
TARGET_LINK_LIBRARIES(${MEDIASERVICE-LIB} ${pkgs_LDFLAGS} ${MEDIASERVICE-HASH-LIB} z Xrender X11 xcb expat Xau Xdmcp)
SET_TARGET_PROPERTIES(${MEDIASERVICE-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${MEDIASERVICE-LIB} PROPERTIES VERSION ${VERSION})
ADD_LIBRARY(${MEDIASERVICE-PLUGIN-LIB} SHARED ${PLUGIN_SRCS})
TARGET_LINK_LIBRARIES(${MEDIASERVICE-PLUGIN-LIB} ${MEDIASERVICE-LIB})
SET_TARGET_PROPERTIES(${MEDIASERVICE-PLUGIN-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${MEDIASERVICE-PLUGIN-LIB} PROPERTIES VERSION ${VERSION})
ADD_LIBRARY(${MEDIACONTENT-PLUGIN-LIB} SHARED ${CONTENTPLUGIN_SRCS})
TARGET_LINK_LIBRARIES(${MEDIACONTENT-PLUGIN-LIB} ${MEDIASERVICE-LIB})
SET_TARGET_PROPERTIES(${MEDIACONTENT-PLUGIN-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${MEDIACONTENT-PLUGIN-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${MEDIASERVICE-LIB} LIBRARY DESTINATION lib)
INSTALL(TARGETS ${MEDIASERVICE-HASH-LIB} LIBRARY DESTINATION lib)
INSTALL(TARGETS ${MEDIASERVICE-PLUGIN-LIB} LIBRARY DESTINATION lib)
INSTALL(TARGETS ${MEDIACONTENT-PLUGIN-LIB} LIBRARY DESTINATION lib)

#ADD_EXECUTABLE( media_service_test ${TEST-SRCS})
#TARGET_LINK_LIBRARIES(media_service_test ${pkgs_LDFLAGS} ${MEDIASERVICE-LIB} )
#INSTALL(TARGETS media_service_test DESTINATION ${BINDIR})

#ADD_EXECUTABLE( visual_test ${TEST-VISUAL-SRCS})
#TARGET_LINK_LIBRARIES(visual_test ${pkgs_LDFLAGS} ${MEDIASERVICE-LIB} )
#INSTALL(TARGETS visual_test DESTINATION ${BINDIR})

#ADD_EXECUTABLE( audio_test ${TEST-AUDIO-SRCS})
#TARGET_LINK_LIBRARIES(audio_test ${pkgs_LDFLAGS} ${MEDIASERVICE-LIB} )
#INSTALL(TARGETS audio_test DESTINATION ${BINDIR})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/visual-svc-error.h DESTINATION include/media-service)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/visual-svc.h DESTINATION include/media-service/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/visual-svc-types.h DESTINATION include/media-service/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/audio-svc-error.h DESTINATION include/media-service)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/audio-svc.h DESTINATION include/media-service/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/audio-svc-types.h DESTINATION include/media-service/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/media-svc.h DESTINATION include/media-service/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/media-svc-error.h DESTINATION include/media-service/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/media-svc-types.h DESTINATION include/media-service/)

#INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/image/thumb_default.png DESTINATION /opt/data/gallery-beat)

