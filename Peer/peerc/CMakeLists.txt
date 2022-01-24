add_executable(peertest peerc.c)
target_link_libraries(peertest PRIVATE uspeer)
set_target_properties(peertest PROPERTIES FOLDER Tests)
