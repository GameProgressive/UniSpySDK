find_package(Speex)

if(NOT SPEEX_FOUND)
    add_subdirectory(libspeex)
    set_target_properties(speex PROPERTIES FOLDER ThirdParty)
    set(SPEEX_LIBRARIES speex)
endif()

# Find libgsm for the voice2bench program.
# Appears to only be needed for win32 or psp builds of voice2bench
find_package(Gsm)

if(NOT GSM_FOUND)
    add_subdirectory(libgsm)
    set(GSM_LIBRARIES gsm)
    set_target_properties(gsm PROPERTIES FOLDER ThirdParty)
endif()


if(UNISPY_WINDOWS)
    set(DEFAULT_HARDWARE gvDirectSound.c gvDirectSound.h)
elseif(UNISPY_MACOSX)
    set(DEFAULT_HARDWARE gvOSXAudio.c gvOSXAudio.h)
endif()

add_library(usvoice2
    gvCodec.c
    gvCustomDevice.c
    gvDevice.c
    gvFrame.c
    gvMain.c
    gvSource.c
    gvSpeex.c
    gvUtil.c
    gv.h
    gvCodec.h
    gvCustomDevice.h
    gvDevice.h
    gvFrame.h
    gvGSM.h
    gvMain.h

    # PS2
    #gvLogitechPS2Codecs.h
    #gvLogitechPS2Codecs.c
    #gvPS2Audio.h
    #gvPS2Eyetoy.h
    #gvPS2Headset.h
    #gvPS2Spu2.h
    #gvPS2Audio.c
    #gvPS2Eyetoy.c
    #gvPS2Headset.c
    #gvPS2Spu2.c

    # PS3
    #gvPS3Audio.h
    #gvPS3Headset.h
    #gvPS3Audio.c
    #gvPS3Headset.c

    # PSP
    #gvPSPAudio.h
    #gvPSPAudio.c

    gvSource.h
    gvSpeex.h
    gvSpeexSpu.h
    gvUtil.h
    ${DEFAULT_HARDWARE}
)

target_link_libraries(usvoice2 PUBLIC ${SPEEX_LIBRARIES} ${GSM_LIBRARIES})
target_include_directories(usvoice2 PRIVATE $<BUILD_INTERFACE:${SPEEX_INCLUDE_DIRS}>)

if(UNIX AND NOT UNISPY_MACOSX)
    target_compile_definitions(usvoice2 PRIVATE -DGV_NO_DEFAULT_HARDWARE)
endif()

target_link_libraries(usvoice2 PUBLIC uscommon)

if(UNISPY_BUILD_TESTS)
    if(UNISPY_WINDOWS)
        find_package(DirectX)
    
        if(DirectX_DSOUND_INCLUDE_FOUND)
            set(UNISPY_VOICE_PLATFORM_INCLUDES ${UNISPY_VOICE_PLATFORM_INCLUDES} ${DirectX_DSOUND_INCLUDE_DIR} PARENT_SCOPE)
            set(UNISPY_VOICE_PLATFORM_LIBRARIES ${UNISPY_VOICE_PLATFORM_LIBRARIES} ${DirectX_DSOUND_LIBRARY} PARENT_SCOPE)
        endif()
    endif()

    # If its not UNIX or it is UNIX and is APPLE.
    if(UNISPY_WINDOWS OR UNISPY_MACOSX)
        # Can't get this test to link correctly so disabling for now.
        #add_subdirectory(voicetest)
        add_subdirectory(voice2bench)

        if (UNISPY_WINDOWS)
            add_subdirectory(Voice2BuddyMFC)
        endif()
    endif()
endif()
