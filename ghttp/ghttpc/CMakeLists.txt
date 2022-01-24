add_executable(httptest ghttpc.c)
target_link_libraries(httptest ushttp)
set_target_properties(httptest PROPERTIES FOLDER Tests)
