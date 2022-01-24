add_executable(gt2test gt2testc.c)
target_link_libraries(gt2test usgt2)
set_target_properties(gt2test PROPERTIES FOLDER Tests)
