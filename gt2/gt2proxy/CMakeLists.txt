add_executable(gt2proxy gt2proxy.c)
target_link_libraries(gt2proxy usgt2)
set_target_properties(gt2proxy PROPERTIES FOLDER Tests)
