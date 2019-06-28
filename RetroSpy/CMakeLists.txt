cmake_minimum_required(VERSION 3.0.0)

add_library(RetroSpy dllmain.c)

target_link_libraries(RetroSpy SHARED
    #rsbrigades
    rscommon
    rschat
    rscdkey
    rshttp
    rsgp
    rsstats
    rsgt2
    rsnatneg
    rspeer
    rspinger
    rspt
    rsqr2
    rssake
    rssc
    rsserverbrowsing
    rsvoice2
    rswebservices
)
