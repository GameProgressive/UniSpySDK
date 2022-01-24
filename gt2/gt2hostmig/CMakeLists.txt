add_executable(gt2hostmig gt2hostmig.c)
target_link_libraries(gt2hostmig usgt2 usqr2)
set_target_properties(gt2hostmig PROPERTIES FOLDER Tests)
