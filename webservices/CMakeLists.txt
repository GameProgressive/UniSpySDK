add_library(uswebservice
    AuthService.c
	RacingService.c
    AuthService.h
    RacingService.h
)

target_link_libraries(uswebservice PUBLIC ushttp)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(authtest)
endif()
