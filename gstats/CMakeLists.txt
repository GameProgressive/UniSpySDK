add_library(usstats
    gbucket.c
    gstats.c
    gbucket.h
    gpersist.h
    gstats.h
)

target_link_libraries(usstats PUBLIC uscommon)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(statstest)
    add_subdirectory(persisttest)

    if (UNISPY_WINDOWS)
        add_subdirectory(track)
        add_subdirectory(gp_stats)
        add_subdirectory(multiTrack)
        add_subdirectory(ladderTrack)
    endif()
endif()
