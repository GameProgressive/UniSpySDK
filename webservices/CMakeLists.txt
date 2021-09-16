cmake_minimum_required(VERSION 3.0.0)

add_library(rswebservice
    AuthService.c
	RacingService.c
)

target_link_libraries(rswebservice rscommon)

if(RS_BUILD_TESTS)
    # This test appears to be missing some required functions from AuthServiceOpen.h
    # add_executable(authtest authtest/authtest.c)
    # target_link_libraries(authtest rswebservice rshttp)
    # target_compile_definitions(authtest PRIVATE test_main=main)
endif()
