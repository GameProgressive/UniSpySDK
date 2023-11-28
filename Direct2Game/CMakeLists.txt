add_library(usd2g
    d2gDeserialize.c
    d2gDeserialize.h
    d2gDownloads.c
    d2gDownloads.h
    d2gMain.c
    d2gMain.h
    d2gServices.c
    d2gServices.h
    d2gUtil.c
    d2gUtil.h
    Direct2Game.h
)

target_link_libraries(usd2g PUBLIC ushttp uswebservice)

target_compile_definitions(usd2g PRIVATE -DGHTTP_EXTENDEDERROR)

if (UNISPY_BUILD_TESTS)
    add_subdirectory(Direct2GameTest)
endif()
