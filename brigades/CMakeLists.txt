add_library(usbrigades
    gsbMain.c
    gsbSerialize.c
    gsbServices.c
    gsbUtil.c
    brigades.h
    gsbMain.h
    gsbSerialize.h
    gsbServices.h
    gsbUtil.h
)

target_link_libraries(usbrigades PUBLIC uswebservice)

if (UNISPY_BUILD_TESTS)
    add_subdirectory(brigadesTest)
endif()
