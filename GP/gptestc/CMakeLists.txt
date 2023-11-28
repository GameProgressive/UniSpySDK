add_executable(gptest gptestc.c)
target_link_libraries(gptest PRIVATE usgp)
set_target_properties(gptest PROPERTIES FOLDER Tests)
