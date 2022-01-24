MFC_Stage1()

add_executable(ScRaceSample
    atlas_Competition_Race_Sample_App_v1.h
    atlas_sc_race_v1.h
    HostOrJoinDlg.h
    LoginDlg.h
    resource.h
    ScRaceSample.h
    ScRaceSampleDlg.h
    StdAfx.h
    WaitingDlg.h
    HostOrJoinDlg.cpp
    LoginDlg.cpp
    ScRaceSample.cpp
    ScRaceSampleDlg.cpp
    WaitingDlg.cpp
    ScRaceSample.rc
)

target_link_libraries(ScRaceSample ussc usgt2 usgp)
target_precompile_headers(ScRaceSample PRIVATE StdAfx.h)
set_target_properties(ScRaceSample PROPERTIES FOLDER Tests)

MFC_Stage2(ScRaceSample)
