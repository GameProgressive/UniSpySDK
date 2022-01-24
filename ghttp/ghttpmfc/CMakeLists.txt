MFC_Stage1()

add_executable(httptestmfc
    ghttpmfc.cpp
    ghttpmfcDlg.cpp
    ghttpmfc.h
    ghttpmfcDlg.h
    resource.h
    StdAfx.h
    ghttpmfc.rc
)

target_link_libraries(httptestmfc ushttp)
target_precompile_headers(httptestmfc PRIVATE StdAfx.h)
set_target_properties(httptestmfc PROPERTIES FOLDER Tests)

MFC_Stage2(httptestmfc)
