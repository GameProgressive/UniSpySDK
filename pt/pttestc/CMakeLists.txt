add_executable(pttest pttestc.c)
target_link_libraries(pttest uspt)
set_target_properties(pttest PROPERTIES FOLDER Tests)
