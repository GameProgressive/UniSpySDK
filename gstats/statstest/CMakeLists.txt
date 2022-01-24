add_executable(statstest statstest.c)
target_link_libraries(statstest usstats)
set_target_properties(statstest PROPERTIES FOLDER Tests)
