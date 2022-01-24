add_executable(authtest authtest.c)
target_link_libraries(authtest uswebservice)
set_target_properties(authtest PROPERTIES FOLDER Tests)
