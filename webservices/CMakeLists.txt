add_library(uswebservice
    AuthService.c
	RacingService.c
    AuthService.h
    RacingService.h
)

target_link_libraries(uswebservice PUBLIC ushttp)

if(UNISPY_BUILD_TESTS)
    # This test appears to be missing some required functions from AuthServiceOpen.h
    # add_subdirectory(authtest)
endif()
