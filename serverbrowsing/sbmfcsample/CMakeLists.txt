MFC_Stage1()

add_executable(sbmfcsample 
    resource.h
    sbmfcsample.cpp
    sbmfcsample.h
    sbmfcsample.rc
    sbmfcsampleDlg.cpp
    sbmfcsampleDlg.h
    StdAfx.h
)
target_link_libraries(sbmfcsample usserverbrowsing)
target_precompile_headers(sbmfcsample PRIVATE StdAfx.h)
set_target_properties(sbmfcsample PROPERTIES FOLDER Tests)

MFC_Stage2(sbmfcsample)
