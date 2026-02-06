# 整个工程的开始配置文件
cmake_minimum_required(VERSION 3.10)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -W -Wall -D_X64 -D_DEBUG -D_LIB -g -O0 -Wno-unused-parameter")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -W -Wall -D_X64 -D_LIB -fPIC -D_RELEASE -O2 -rdynamic -Wno-unused-parameter")
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2  -Wall")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2  -Wall")
ELSE()
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    IF(${CMAKE_BUILD_TYPE} STREQUAL "debug")
        SET(CMAKE_BUILD_TYPE "Debug")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0  -Wall")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0  -Wall")
    ELSEIF(${CMAKE_BUILD_TYPE} STREQUAL "release")
        SET(CMAKE_BUILD_TYPE "Release")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2  -Wall")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2  -Wall")
    ELSE()
    ENDIF()
ENDIF()

if (${CMAKE_BUILD_TYPE} STREQUAL "debug")
	MESSAGE("###### Project Debug Build ######")
	if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
		ADD_DEFINITIONS(-std=c++11 -Wall -O0 -g -fPIC -DWITH_OPENSSL -rdynamic)
	elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
		ADD_DEFINITIONS(-DVSK_PLUGIN_WIN_EXPORTS -Wall -D_DEBUG -DWITH_OPENSSL)
	else()
		message("current platform unkonw")
	endif()
else()
	MESSAGE("###### Project Release Build ######")
	if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
		ADD_DEFINITIONS(-std=c++11 -Wall -fPIC -DWITH_OPENSSL -rdynamic)
	elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
		ADD_DEFINITIONS(-DVSK_PLUGIN_WIN_EXPORTS -Wall -DWITH_OPENSSL)
	else()
		message("current platform unkonw")
	endif()
endif()

MESSAGE("CMAKE_HOST_SYSTEM_NAME:" ${CMAKE_HOST_SYSTEM_NAME})
MESSAGE("BUILD TYPE:" ${CMAKE_BUILD_TYPE})
MESSAGE("CMAKE_CXX_FLAGS:" ${CMAKE_CXX_FLAGS})
MESSAGE("CMAKE_C_FLAGS:" ${CMAKE_C_FLAGS})

project(tools)

MESSAGE(STATUS "PROJECT_SOURCE_DIR " ${PROJECT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR} SRCS)

#file(GLOB INC_PATH ${PROJECT_SOURCE_DIR}/../../../tools/SelfLog.cpp)
#list(APPEND INCS ${INC_PATH})

# get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
# foreach(dir ${dirs})
#   message(STATUS "dir='${dir}'")
# endforeach()

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
	SET(LIBS ${LIBS}
		pthread
		# rt
		# m
		# dl
	)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
	message(".....current platform window")
	SET(LIBS ${LIBS}
		ws2_32
		Iphlpapi
	)
else()
	message("current platform unkonw")
endif()

add_library(tools STATIC ${INCS} ${SRCS})
#add_library(vsk_arm_nvr_plugin_shard SHARED ${SRCS})

# add_executable(vsk_arm_nvr_plugin main.cpp)
target_link_libraries(tools ${LIBS})

# target_link_libraries(${LIB_FILE_NAME} 
# 	"-Wl,--whole-archive" #之后的库使用--whole-archive选项
# 	libOpensslEncryptLib.a
# 	libnvrShare.a
# 	libClientSdk.a	
# 	libnvrCommon.a
# 	#libuuid.a
# 	"-Wl,--no-whole-archive" #之后的库不使用--whole-archive选项
# 	#libliv555.a 
# 	libbreakpad_client.a
# 	libvkframe.a
# 	libvktool.a
# 	libprotocol.a
# 	#libmysqlclient.a
# 	libcrypto.a
# 	) 