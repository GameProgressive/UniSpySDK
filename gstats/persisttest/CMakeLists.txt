add_executable(persisttest persisttest.c)
target_link_libraries(persisttest usstats)
set_target_properties(persisttest PROPERTIES FOLDER Tests)
