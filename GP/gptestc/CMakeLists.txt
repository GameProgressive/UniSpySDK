add_executable(gptest gptestc.c)
target_link_libraries(gptest usgp)
set_target_properties(gptest PROPERTIES FOLDER Tests)
