MFC_Stage1()

add_executable(statstestmfc
    gp_stats.h
    gp_statsDlg.h
    resource.h
    StdAfx.h
    gp_stats.cpp
    gp_statsDlg.cpp
    gp_stats.rc
)

target_link_libraries(statstestmfc usstats usgp ushttp)
target_precompile_headers(statstestmfc PRIVATE StdAfx.h)
set_target_properties(statstestmfc PROPERTIES FOLDER Tests)

MFC_Stage2(statstestmfc)
