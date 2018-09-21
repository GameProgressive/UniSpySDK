cmake_minimum_required(VERSION 3.0.0)

add_library(rssc
    sciInterface.c
    sciMain.c
    sciReport.c
    sciSerialize.c
    sciWebServices.c
)

target_link_libraries(rssc rscommon rswebservice rshttp)

if(RS_BUILD_TESTS)
    add_executable(sctest sctest/sctest.c)
    target_link_libraries(sctest rssc)
    target_compile_definitions(sctest PRIVATE test_main=main)
    add_executable(sctestmatchless sctestmatchless/sctestmatchless.c)
    target_link_libraries(sctestmatchless rssc)
    target_compile_definitions(sctestmatchless PRIVATE test_main=main)
endif()
