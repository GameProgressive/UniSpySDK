add_executable(sbtest sbctest.c)
target_link_libraries(sbtest usserverbrowsing)
set_target_properties(sbtest PROPERTIES FOLDER Tests)
