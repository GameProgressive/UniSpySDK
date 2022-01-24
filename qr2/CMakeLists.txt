add_library(usqr2
    qr2.c
    qr2regkeys.c
    qr2.h
    qr2regkeys.h
)

target_link_libraries(usqr2 PUBLIC usnatneg)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(qr2csample)
endif()
