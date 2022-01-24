add_library(ussake
    sakeMain.c
    sakeRequest.c
    sakeRequestMisc.c
    sakeRequestModify.c
    sakeRequestRead.c
    sake.h
    sakeMain.h
    sakeRequest.h
    sakeRequestInternal.h
)

target_link_libraries(ussake PUBLIC ushttp)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(saketest)
endif()
