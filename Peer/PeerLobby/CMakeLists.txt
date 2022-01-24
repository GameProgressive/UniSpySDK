MFC_Stage1()

add_executable(PeerLobby
    ConnectPage.h
    CreatePage.h
    GroupPage.h
    LobbyWizard.h
    PeerLobby.h
    resource.h
    SideBarCtrl.h
    StagingPage.h
    StdAfx.h
    TitlePage.h
    ConnectPage.cpp
    CreatePage.cpp
    GroupPage.cpp
    LobbyWizard.cpp
    PeerLobby.cpp
    SideBarCtrl.cpp
    StagingPage.cpp
    TitlePage.cpp
    PeerLobby.rc
)

target_link_libraries(PeerLobby PRIVATE uspeer)
target_precompile_headers(PeerLobby PRIVATE StdAfx.h)
set_target_properties(PeerLobby PROPERTIES FOLDER Tests)

MFC_Stage2(PeerLobby)
