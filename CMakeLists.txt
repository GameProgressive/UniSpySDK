cmake_minimum_required(VERSION 3.0.0)

project(UniSpySDK VERSION 2.06 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Set default build to be release if no other specified.
if (NOT CMAKE_CONFIGURATION_TYPES AND 
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE
)
	set(CMAKE_BUILD_TYPE Release)
endif()

# Do we want OpenSSL support or integrated GameSpy SSLv3 engine support?
option(RS_OPENSSL "Enable OpenSSL support" ON)
option(RS_HTTPLOG "Enable HTTP logging" OFF)
option(RS_SECURESECONNECT "Enable connecting to UniSpy services with a secure protocol (HTTPS)" ON)

# This is GameSpy built-in memory managed. I don't know if it needs to be disabled but a test requires it.
#option(RS_MEM_MANAGED "Enable GameSpy built-in memory manager" ON)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

	# WinSock1 is deprecated, it's better to use WinSock2
    option(RS_WINSOCK2 "Link against Windows Socket 2 instead of Windows Socket 1." ON)
	
	set(RS_ACTUAL_PLATFORM "win32")
elseif(APPLE OR "${CMAKE_SYSTEM}" MATCHES "Darwin")
	set(RS_ACTUAL_PLATFORM "macosx")
elseif(UNIX AND NOT APPLE)
	set(RS_ACTUAL_PLATFORM "linux")
endif()

if (NOT DEFINED RS_PLATFORM)
	if (NOT DEFINED RS_ACTUAL_PLATFORM)
		message(FATAL_ERROR "Please define a platform to build the code")
	endif()
	
	set(RS_PLATFORM ${RS_ACTUAL_PLATFORM})
	message(STATUS "Building platform ${RS_PLATFORM}")
endif()

if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    set(RS_64BIT 1)
endif()

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(RS_64BIT 1)
endif()

if(CMAKE_CL_64)
   set(RS_64BIT 1)
endif()

# Do we optionally want to build the test executables?
option(RS_BUILD_TESTS "Builds test executables." OFF)

if(RS_SERVER_NAME)
    message(STATUS "UniSpy server name is set to ${RS_SERVER_NAME}")
endif()

if (NOT DEFINED RS_SERVER_NAME)
	set(RS_SERVER_NAME "gamespy.com")
endif()

add_definitions(-DGS_PEER)

#if (RS_MEM_MANAGED)
add_definitions(-DGSI_MEM_MANAGED)
#endif()

configure_file(common/rsCommon.h.in ${CMAKE_BINARY_DIR}/generated/rsCommon.h)
include_directories(${CMAKE_BINARY_DIR}/generated)

#add_subdirectory(brigades)
add_subdirectory(common) # Most of the common stuff gets built into this.
add_subdirectory(Chat)
add_subdirectory(natneg)
add_subdirectory(qr2) # depends on natneg
add_subdirectory(gcdkey) # tests depend on qr2
add_subdirectory(ghttp) # depends on common includes gsSoap here instead of common as gsSoap relies on ghttp.
add_subdirectory(gt2) # depends on common includes UDPEngine here instead of common as UDPEngine relies on gt2.
add_subdirectory(GP) # tests depend on gt2 for the UDPEngine
add_subdirectory(gstats) # depends on common
add_subdirectory(pinger) # depends on common for DArray
add_subdirectory(serverbrowsing) # depends on qr2
add_subdirectory(Peer) # depends on qr2, Chat, pinger, serverbrowsing
add_subdirectory(pt) # depends on ghttp
add_subdirectory(sake) # depends on common and ghttp. test relies on GP and gt2 for UDPEngine
add_subdirectory(webservices)
add_subdirectory(sc) # depends on common webservices ghttp
add_subdirectory(Voice2)
add_subdirectory(RetroSpy)

