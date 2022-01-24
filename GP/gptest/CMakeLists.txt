MFC_Stage1()

add_executable(gptestmfc
    gptest.cpp
    gptestDlg.cpp
    gptest.h
    gptestDlg.h
    resource.h
    StdAfx.h
    gptest.rc
)

target_link_libraries(gptestmfc usgp)
target_precompile_headers(gptestmfc PRIVATE StdAfx.h)
set_target_properties(gptestmfc PROPERTIES FOLDER Tests)

MFC_Stage2(gptestmfc)
