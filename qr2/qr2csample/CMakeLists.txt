add_executable(qr2csample qr2csample.c)
target_link_libraries(qr2csample PRIVATE usqr2)
set_target_properties(qr2csample PROPERTIES FOLDER Tests)
