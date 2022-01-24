add_library(uspt
    ptMain.c
    pt.h
)

target_link_libraries(uspt PUBLIC ushttp)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(pttestc)
endif()
