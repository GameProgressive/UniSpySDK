add_executable(cdkeygen gcdkeygen.c)
target_link_libraries(cdkeygen uscdkey)
set_target_properties(cdkeygen PROPERTIES FOLDER Tests)
