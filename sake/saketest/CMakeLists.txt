add_executable(saketest saketest.c)
target_link_libraries(saketest usgp ussake)
set_target_properties(saketest PROPERTIES FOLDER Tests)
