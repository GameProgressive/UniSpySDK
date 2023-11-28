cmake_minimum_required(VERSION 3.16)

project(UniSpySDK VERSION 2.10 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(MFCMacro)

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

# Set default build to be release if no other specified.
if (NOT CMAKE_CONFIGURATION_TYPES AND 
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE
)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Do we want OpenSSL support or integrated GameSpy SSLv3 engine support?
option(UNISPY_OPENSSL "Enable OpenSSL support" ON)
option(UNISPY_HTTPLOG "Enable HTTP logging" OFF)
option(UNISPY_SECURESECONNECT "Enable connecting to GameSpy services with a secure protocol (HTTPS)" ON)
option(UNISPY_ENABLE_FORCE_IP "Enable force ip" OFF)
option(UNISPY_NO_RSA_CHECK "Disable signature RSA check on Web login" OFF)
set(UNISPY_FORCE_IP "localhost" CACHE STRING "Force a custom IP to connect")

# This is GameSpy built-in memory managed. I don't know if it needs to be disabled but a test requires it.
option(UNISPY_MEM_MANAGED "Enable GameSpy built-in memory manager" ON)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(UNISPY_ARCH x64)
else()
    set(UNISPY_ARCH x86)
endif()

if(WIN32 OR "${CMAKE_SYSTEM_NAME}" MATCHES "Windows" OR MINGW OR CYGWIN)
    option(UNISPY_MSVC_STATIC "Use MSVC static runtime" OFF)

    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

    # WinSock1 is deprecated, it's better to use WinSock2
    option(UNISPY_WINSOCK2 "Link against Windows Socket 2 instead of Windows Socket 1." ON)

    if(UNISPY_WINSOCK2)
        add_definitions(-DGSI_WINSOCK2)
        set(UNISPY_WINSOCK_LIB ws2_32)
    else()
        set(UNISPY_WINSOCK_LIB winsock32)
    endif()

    if(UNISPY_MSVC_STATIC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    set(UNISPY_WINDOWS 1)
endif()

if (APPLE OR "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    set(UNISPY_MACOSX 1) # preparation for common refactor

    if ("${CMAKE_SYSTEM_NAME}" MATCHES "iOS")
        set(UNISPY_IPHONE 1) # preparation for common refactor
    endif()
endif()

if (UNIX)
    if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        set(UNISPY_LINUX 1) # preparation for common refactor
    endif()
endif()

# Do we optionally want to build the test executables?
option(UNISPY_BUILD_TESTS "Builds test executables." OFF)

if(UNISPY_DOMAIN_NAME)
    add_definitions(-DGSI_DOMAIN_NAME="${UNISPY_DOMAIN_NAME}")
    message(STATUS "Server name is set to ${UNISPY_DOMAIN_NAME}")
endif()

add_definitions(-DGS_PEER -DRS_FORCE_IP="${RS_FORCE_IP}")

if (UNISPY_ENABLE_FORCE_IP)
    add_definitions(-DUNISPY_FORCE_IP="${UNISPY_FORCE_IP}")
endif()

if(UNISPY_SECURESECONNECT)
    add_definitions(-DUNISPY_USE_HTTPS)
endif()

if (UNISPY_MEM_MANAGED)
    add_definitions(-DGSI_MEM_MANAGED)
endif()

if (UNISPY_NO_RSA_CHECK)
    add_definitions(-DUNISPY_NO_RSA_CERT)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Clang, Apple Clang and GCC have stricter warning requirements.
    add_compile_options(-Wall -Wno-unused-parameter -Wextra)
    
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(UNISPY_COMPILER_CLANG 1)
    else()
        set(UNISPY_COMPILER_GCC 1)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR MSVC)
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
    set(UNISPY_COMPILER_MSVC 1)
endif()

add_subdirectory(brigades)
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
add_subdirectory(Direct2Game)
add_subdirectory(Voice2)
add_subdirectory(sharedDll)
