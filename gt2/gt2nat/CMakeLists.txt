add_executable(gt2nat gt2nat.c)
target_link_libraries(gt2nat usgt2 usqr2)
set_target_properties(gt2nat PROPERTIES FOLDER Tests)
