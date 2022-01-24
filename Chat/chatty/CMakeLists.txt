MFC_Stage1()

add_executable(chattestmfc
    ChannelListDlg.h
    ChannelModeDlg.h
    chatty.h
    chattyDoc.h
    chattyView.h
    ChildFrm.h
    ConnectDlg.h
    EnterDlg.h
    GetUserInfoDlg.h
    KickReasonDlg.h
    MainFrm.h
    resource.h
    SendRawDlg.h
    SetPasswordDlg.h
    SetTopicDlg.h
    StdAfx.h
    TalkDlg.h
    ChannelListDlg.cpp
    ChannelModeDlg.cpp
    chatty.cpp
    chattyDoc.cpp
    chattyView.cpp
    ChildFrm.cpp
    ConnectDlg.cpp
    EnterDlg.cpp
    GetUserInfoDlg.cpp
    KickReasonDlg.cpp
    MainFrm.cpp
    SendRawDlg.cpp
    SetPasswordDlg.cpp
    SetTopicDlg.cpp
    TalkDlg.cpp
    chatty.rc
)

target_link_libraries(chattestmfc uschat)
target_precompile_headers(chattestmfc PRIVATE StdAfx.h)
set_target_properties(chattestmfc PROPERTIES FOLDER Tests)

MFC_Stage2(chattestmfc)
