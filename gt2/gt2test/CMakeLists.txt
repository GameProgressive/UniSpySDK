MFC_Stage1()

add_executable(gt2testmfc
    gt2test.h
    gt2testDlg.h
    resource.h
    StdAfx.h
    gt2test.rc
    gt2test.cpp
    gt2testDlg.cpp
)

target_link_libraries(gt2testmfc usgt2)
target_precompile_headers(gt2testmfc PRIVATE StdAfx.h)
set_target_properties(gt2testmfc PROPERTIES FOLDER Tests)

MFC_Stage2(gt2testmfc)
