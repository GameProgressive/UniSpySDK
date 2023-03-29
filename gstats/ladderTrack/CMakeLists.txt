MFC_Stage1()

add_executable(ladderTrack
    HostOrJoinDlg.cpp
    ladderTrack.cpp
    ladderTrackDlg.cpp
    LoginDlg.cpp
    WaitingDlg.cpp
    ladderTrack.rc
    HostOrJoinDlg.h
    ladderTrack.h
    ladderTrackDlg.h
    LoginDlg.h
    resource.h
    StdAfx.h
    WaitingDlg.h
)

target_link_libraries(ladderTrack usstats usgt2 ushttp usgp)
target_precompile_headers(ladderTrack PRIVATE StdAfx.h)
set_target_properties(ladderTrack PROPERTIES FOLDER Tests)

MFC_Stage2(ladderTrack)
