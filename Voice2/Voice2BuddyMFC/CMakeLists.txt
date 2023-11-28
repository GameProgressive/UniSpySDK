MFC_Stage1()

add_executable(Voice2BuddyMFC
    LoginDlg.h
    resource.h
    SetupDlg.h
    StdAfx.h
    Voice2BuddyMFC.h
    Voice2BuddyMFCDlg.h
    VoiceSessionDlg.h
    LoginDlg.cpp
    SetupDlg.cpp
    Voice2BuddyMFC.cpp
    Voice2BuddyMFCDlg.cpp
    VoiceSessionDlg.cpp
    Voice2BuddyMFC.rc
)

target_link_libraries(Voice2BuddyMFC PRIVATE usgp usnatneg usvoice2 ${UNISPY_VOICE_PLATFORM_LIBRARIES})
target_include_directories(Voice2BuddyMFC BEFORE PRIVATE ${UNISPY_VOICE_PLATFORM_INCLUDES})
target_precompile_headers(Voice2BuddyMFC PRIVATE StdAfx.h)
set_target_properties(Voice2BuddyMFC PROPERTIES FOLDER Tests)
target_link_directories(Voice2BuddyMFC PRIVATE ${UNISPY_VOICE_PLATFORM_LIBDIRS})

MFC_Stage2(Voice2BuddyMFC)
