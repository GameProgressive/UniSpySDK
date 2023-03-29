MFC_Stage1()

add_executable(track
    LoginDlg.cpp
    track.cpp
    trackDlg.cpp
    LoginDlg.h
    resource.h
    StdAfx.h
    track.h
    trackDlg.h
    track.rc
)

target_link_libraries(track usstats ushttp usgp)
target_precompile_headers(track PRIVATE StdAfx.h)
set_target_properties(track PROPERTIES FOLDER Tests)

MFC_Stage2(track)
