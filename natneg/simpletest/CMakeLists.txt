add_executable(natnegtest simpletest.c)
target_link_libraries(natnegtest usnatneg)
set_target_properties(natnegtest PROPERTIES FOLDER Tests)
