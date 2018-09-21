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

target_link_libraries(rsvoice2 ${SPEEX_LIBRARIES})
target_include_directories(rsvoice2 PRIVATE $<BUILD_INTERFACE:${SPEEX_INCLUDE_DIRS}>)

if(UNIX AND NOT APPLE)
    target_compile_definitions(rsvoice2 PRIVATE -DGV_NO_DEFAULT_HARDWARE)
endif()

target_link_libraries(rsvoice2 rscommon)

if(RS_BUILD_TESTS)
    if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
        find_package(DirectX)
    
        if(DirectX_DSOUND_INCLUDE_FOUND)
            set(PLATFORM_INCLUDES ${PLATFORM_INCLUDES} ${DirectX_DSOUND_INCLUDE_DIR})
            set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} ${DirectX_DSOUND_LIBRARY})
        endif()
    endif()

    add_executable(voicetest Voice2Test/Voice2Test.c)
    target_link_libraries(voicetest rsvoice2 ${PLATFORM_LIBRARIES})
    target_include_directories(voicetest BEFORE PRIVATE ${PLATFORM_INCLUDES})
    target_compile_definitions(voicetest PRIVATE test_main=main GV_CUSTOM_SOURCE_TYPE=SOCKADDR_IN)
endif()
