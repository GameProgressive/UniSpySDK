MFC_Stage1()

add_executable(multiTrack
    HostOrJoinDlg.h
    LoginDlg.h
    multiTrack.h
    multiTrackDlg.h
    resource.h
    StdAfx.h
    WaitingDlg.h
    HostOrJoinDlg.cpp
    LoginDlg.cpp
    multiTrack.cpp
    multiTrackDlg.cpp
    WaitingDlg.cpp
    multiTrack.rc
)

target_link_libraries(multiTrack usstats usgt2 ushttp usgp)
target_precompile_headers(multiTrack PRIVATE StdAfx.h)
set_target_properties(multiTrack PROPERTIES FOLDER Tests)

MFC_Stage2(multiTrack)
