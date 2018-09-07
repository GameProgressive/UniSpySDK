cmake_minimum_required(VERSION 3.0.0)

find_package(Speex)

if(NOT SPEEX_FOUND)
    add_subdirectory(libspeex)
    set(SPEEX_LIBRARIES speex)
endif()

if(WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(DEFAULT_HARDWARE gvDirectSound.c)
elseif(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DEFAULT_HARDWARE gvOSXAudio.c)
endif()

add_library(rsvoice2
    gvCodec.c
    gvCustomDevice.c
    gvDevice.c
    gvFrame.c
    gvMain.c
    gvSource.c
    gvSpeex.c
    gvUtil.c
    ${DEFAULT_HARDWARE}
)

target_link_libraries(rsvoice2 ${SPEEX_LIBRARRIES})
target_include_directories(rsvoice2 PRIVATE $<BUILD_INTERFACE:${SPEEX_INCLUDE_DIRS}>)

if(UNIX AND NOT APPLE)
    target_compile_definitions(rsvoice2 PRIVATE -DGV_NO_DEFAULT_HARDWARE)
endif()