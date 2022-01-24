add_library(ussc
    sciInterface.c
    sciMain.c
    sciReport.c
    sciSerialize.c
    sciWebServices.c
    sc.h
    sci.h
    sciInterface.h
    sciReport.h
    sciSerialize.h
)

target_link_libraries(ussc PUBLIC uswebservice)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(sctest)
    add_subdirectory(sctestmatchless)

    if (UNISPY_WINDOWS)
        add_subdirectory(scRaceSample)
    endif()
endif()
