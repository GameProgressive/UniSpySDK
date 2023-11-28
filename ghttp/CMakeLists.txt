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

if (UNISPY_HTTPLOG)
	target_compile_definitions(ushttp PUBLIC HTTP_LOG=1)
endif()

# Extended log for HTTP, this avoid using http errors as -1, which was done in previous SDK versions
target_compile_definitions(ushttp PUBLIC GHTTP_EXTENDEDERROR=1)

if (OPENSSL_FOUND)
    set(HTTP_OPENSSL_LIBS OpenSSL::SSL)
    target_compile_definitions(ushttp PRIVATE OPENSSL=1)
endif()

target_link_libraries(ushttp PUBLIC uscommon ${HTTP_OPENSSL_LIBS})

if(UNISPY_BUILD_TESTS)
    add_subdirectory(ghttpc)
    
    if (UNISPY_WINDOWS)
        add_subdirectory(ghttpmfc)
    endif()
endif()
