MFC_Stage1()

add_executable(PeerTestMFC
    PeerTest.h
    PeerTestDlg.h
    resource.h
    StdAfx.h
    PeerTest.cpp
    PeerTestDlg.cpp
    PeerTest.rc
)

target_link_libraries(PeerTestMFC PRIVATE uspeer)
target_precompile_headers(PeerTestMFC PRIVATE StdAfx.h)
set_target_properties(PeerTestMFC PROPERTIES FOLDER Tests)

MFC_Stage2(PeerTestMFC)
