cmake_minimum_required(VERSION 3.0.0)

project(retrospy VERSION 2.06 LANGUAGES C)

# Set default build to be release if no other specified.
if (NOT CMAKE_CONFIGURATION_TYPES AND 
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE
)
	set(CMAKE_BUILD_TYPE Release)
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

if(RETROSPY_SERVER_NAME)
    add_definitions(-DGSI_DOMAIN_NAME="${RETROSPY_SERVER_NAME}")
    message(STATUS "RETROSPY server name is set to ${RETROSPY_SERVER_NAME}")
endif()

add_subdirectory(common)
add_subdirectory(Chat)
add_subdirectory(gcdkey)
add_subdirectory(ghttp)
add_subdirectory(GP)
add_subdirectory(gstats)
add_subdirectory(gt2)

add_library(retrospy dllmain.c)
target_link_libraries(retrospy SHARED rscommon rschat rscdkey rshttp rsgp rsstats rsgt2)
