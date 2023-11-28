add_library(UniSpySDK dllmain.c)

target_link_libraries(UniSpySDK SHARED
    #usbrigades
    uscommon
    uschat
    uscdkey
    ushttp
    usgp
    usstats
    usgt2
    usnatneg
    uspeer
    uspinger
    uspt
    usqr2
    ussake
    ussc
    usserverbrowsing
    usvoice2
    uswebservices
    usd2g
)
