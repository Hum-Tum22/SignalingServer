cmake_minimum_required(VERSION 3.22.1)

PROJECT(GB28181App)


set(CMAKE_VERBOSE_MAKEFILE ON)
add_definitions(-std=c++11)

IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF()
MESSAGE("BUILD TYPE:" ${CMAKE_BUILD_TYPE})

#SET(CMAKE_C_COMPLITER "GCC")


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/SipServer)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/stateAgents)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/resip/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/libcurl/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/rapidjson)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/device)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/localMediaServer)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/SqlDbManager)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/tools)
if (USE_MYSQL)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../resip/contrib/MySQLConnectorC/include)
endif()
if (USE_SSL)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../resip/contrib/MySQLConnectorC/include)
endif()

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/SipServer/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/device/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/localMediaServer/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/SqlDbManager/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/tools/ SRC_LIST)

if (USE_SSL)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/stateAgents/ SRC_LIST)
else()
FILE(GLOB_RECURSE SRC_CPP_SOURCES ${PROJECT_SOURCE_DIR}/stateAgents/PresenceSubscriptionHandler.cxx)
SET(SRC_LIST ${SRC_LIST} ${SRC_CPP_SOURCES})
endif()
#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/stateAgents/PresenceSubscriptionHandler.cxx SRC_LIST)
#AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/stateAgents/ SRC_LIST)




link_directories(${PROJECT_SOURCE_DIR}/lib/resip/lib)
link_directories(${PROJECT_SOURCE_DIR}/lib/libcurl/lib)

link_libraries(libdum.a)
link_libraries(libresip.a)
link_libraries(libresipares.a)
link_libraries(librutil.a)
link_libraries(libcurl.a)

SET(LIBS
	-Wl,--start-group
	libdum.a
	libresip.a
	libresipares.a
	librutil.a
	libcurl.a
	-Wl,--end-group
)
if (USE_MYSQL)
SET(LIBS ${LIBS}
	libmysql.a
)
endif()

if (USE_SSL)
SET(LIBS ${LIBS}
	libcrypto.a
	libssl.a
)
endif()


#link_libraries(z)
#link_libraries(ssl)
#link_libraries(pthread)
#link_libraries(rt)
#link_libraries(m)
#link_libraries(dl)
#link_libraries(stdc++)
#link_libraries("libAICore_Face_SDK.so")
#link_libraries("libfacenet.so")
#link_libraries("libgomp.so")

#ADD_LIBRARY(DecoderLib STATIC ${DECODERLIB_SOURCES})

#add_library(GB28181 SHARED ${DECODERLIB_SOURCES})
add_executable(GB28181App ${SRC_LIST})
TARGET_LINK_LIBRARIES(GB28181App ${LIBS})