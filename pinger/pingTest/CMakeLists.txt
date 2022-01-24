add_executable(pingTest pingTest.c)
target_link_libraries(pingTest uspinger)
set_target_properties(pingTest PROPERTIES FOLDER Tests)
