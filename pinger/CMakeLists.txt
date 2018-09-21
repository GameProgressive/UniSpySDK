cmake_minimum_required(VERSION 3.0.0)

add_library(rspinger
    pingerMain.c
)

target_link_libraries(rspinger rscommon)
