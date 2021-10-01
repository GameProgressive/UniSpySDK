cmake_minimum_required(VERSION 3.0.0)

project(retrospy VERSION 2.06 LANGUAGES C CXX)

# Only interpret if() arguments as variables or keywords when unquoted.
# CMake version 3.21.1+ warns when the policy is not set and uses OLD behavior.
cmake_policy(SET CMP0054 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

message(STATUS "CMake version: ${CMAKE_VERSION}.")
message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}.")

# Git.
find_package(Git)
if (Git_FOUND)
    message(STATUS "Git version ${GIT_VERSION_STRING} found at '${GIT_EXECUTABLE}'.")

    # Get the current working branch.
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE RS_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Get the latest commit hash.
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE RS_GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Get the subject of the commit.
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" log -1 --format=%s
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE RS_GIT_COMMIT_SUBJECT
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    message(STATUS
        "Git using branch '${RS_GIT_BRANCH}', commit ${RS_GIT_COMMIT_HASH}/'${RS_GIT_COMMIT_SUBJECT}'.")
endif()

message(STATUS "${PROJECT_NAME} c compiler ${CMAKE_C_COMPILER_ID} at ${CMAKE_C_COMPILER}.")
message(STATUS "${PROJECT_NAME} cxx compiler ${CMAKE_CXX_COMPILER_ID} at ${CMAKE_CXX_COMPILER}.")

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
option(RS_SECURESECONNECT "Enable connecting to RetroSpy services with a secure protocol (HTTPS)" ON)

# This is GameSpy built-in memory managed. I don't know if it needs to be disabled but a test requires it.
#option(RS_MEM_MANAGED "Enable GameSpy built-in memory manager" ON)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

    # WinSock1 is deprecated, it's better to use WinSock2
    option(RS_WINSOCK2 "Link against Windows Socket 2 instead of Windows Socket 1." ON)
endif()

# Do we optionally want to build the test executables?
option(RS_BUILD_TESTS "Builds test executables." OFF)

if(RETROSPY_SERVER_NAME)
    add_definitions(-DGSI_DOMAIN_NAME="${RETROSPY_SERVER_NAME}")
    message(STATUS "RETROSPY server name is set to ${RETROSPY_SERVER_NAME}")
endif()

add_definitions(-DGS_PEER)

if(RS_WINSOCK2)
    add_definitions(-DGSI_WINSOCK2)
endif()

if(RS_SECURESECONNECT)
    add_definitions(-DRS_HTTPS)
endif()

#if (RS_MEM_MANAGED)
add_definitions(-DGSI_MEM_MANAGED)
#endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Clang, Apple Clang and GCC have stricter warning requirements.
    add_compile_options(-Wall -Wno-unused-parameter)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # At least Ninja doesn't remove the /W3 flag when we add /W4|/Wall one, which
    # leads to compilation warnings.  Remove /W3 entirely, as /W4|/Wall be used.
    foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if (${flag_var} MATCHES "/W3")
        string(REGEX REPLACE "/W3" "" ${flag_var} "${${flag_var}}")
      endif()
    endforeach()

    # Use almost max warning level (/W3 used by default).
    # Exclude C4100 (unreferenced formal parameter).
    add_compile_options("/W4;/wd4100")
endif()

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
