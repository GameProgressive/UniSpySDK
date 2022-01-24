add_executable(sctest sctest.c)
target_link_libraries(sctest ussc)
set_target_properties(sctest PROPERTIES FOLDER Tests)
