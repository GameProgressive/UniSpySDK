cmake_minimum_required(VERSION 3.0.0)

add_library(rsstats
    gbucket.c
    gstats.c
)

target_link_libraries(rsstats rscommon)

if(RS_BUILD_TESTS)
    add_executable(statstest statstest/statstest.c)
    target_link_libraries(statstest rsstats)
    target_compile_definitions(statstest PRIVATE test_main=main)
    add_executable(persisttest persisttest/persisttest.c)
    target_link_libraries(persisttest rsstats)
    target_compile_definitions(persisttest PRIVATE test_main=main)
endif()
