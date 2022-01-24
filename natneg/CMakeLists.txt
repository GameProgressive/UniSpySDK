add_library(usnatneg
    NATify.c
    natneg.c
    NATify.h
    natneg.h
    nninternal.h
)

target_link_libraries(usnatneg PUBLIC uscommon)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(simpletest)
endif()
