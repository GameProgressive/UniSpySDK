cmake_minimum_required(VERSION 3.5)

project(gsm VERSION 1.0.0 LANGUAGES C)

set(GSM_HEADERS
    inc/gsm.h
)

set(GSM_SOURCES 
    src/add.c
    src/code.c
    src/debug.c
    src/decode.c
    src/long_term.c
    src/lpc.c
    src/preprocess.c
    src/rpe.c
    src/gsm_destroy.c
    src/gsm_decode.c
    src/gsm_encode.c
    src/gsm_explode.c
    src/gsm_implode.c
    src/gsm_create.c
    src/gsm_print.c
    src/gsm_option.c
    src/short_term.c
    src/table.c
)

add_library(gsm STATIC ${GSM_HEADERS} ${GSM_SOURCES})
target_compile_definitions(gsm PRIVATE -DSASR -DNeedFunctionPrototypes=1)
target_include_directories(gsm
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)
