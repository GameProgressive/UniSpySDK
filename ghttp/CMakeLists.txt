if (UNISPY_OPENSSL)
	find_package(OpenSSL COMPONENTS SSL)

    if (NOT OPENSSL_FOUND)
        message(STATUS "Failed to find OpenSSL libraries, falling back to built in SSL.")
    endif()
endif()

add_library(ushttp
    ghttpBuffer.c
    ghttpCallbacks.c
    ghttpCommon.c
    ghttpConnection.c
    ghttpEncryption.c
    ghttpMain.c
    ghttpPost.c
    ghttpProcess.c
    ghttp.h
    ghttpASCII.h
    ghttpBuffer.h
    ghttpCallbacks.h
    ghttpCommon.h
    ghttpConnection.h
    ghttpEncryption.h
    ghttpMain.h
    ghttpPost.h
    ghttpProcess.h
    ghttpSoap.h
    ghttpSoap.c
)

target_link_libraries(ushttp PUBLIC uscommon)

if (UNISPY_HTTPLOG)
	target_compile_definitions(ushttp PUBLIC HTTP_LOG=1)
endif()

# Extended log for HTTP
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
	target_compile_definitions(ushttp PUBLIC GHTTP_EXTENDEDERROR=1)
endif()

if (OPENSSL_FOUND)
    target_link_libraries(ushttp OpenSSL::SSL)
    target_compile_definitions(ushttp PRIVATE OPENSSL=1)
endif()

if(UNISPY_BUILD_TESTS)
    add_subdirectory(ghttpc)
    
    if (UNISPY_WINDOWS)
        add_subdirectory(ghttpmfc)
    endif()
endif()
