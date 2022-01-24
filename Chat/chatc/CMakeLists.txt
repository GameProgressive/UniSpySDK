add_executable(chattest chatc.c)
target_link_libraries(chattest uschat)
set_target_properties(chattest PROPERTIES FOLDER Tests)
