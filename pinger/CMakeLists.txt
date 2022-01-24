add_library(uspinger
    pingerMain.c
    pinger.h
)

target_link_libraries(uspinger PUBLIC uscommon)

if (UNISPY_BUILD_TESTS)
    add_subdirectory(pingTest)
endif()
