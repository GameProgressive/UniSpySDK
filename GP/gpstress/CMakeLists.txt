add_executable(gpstress gpstress.c)
target_link_libraries(gpstress usgp)
set_target_properties(gpstress PROPERTIES FOLDER Tests)
