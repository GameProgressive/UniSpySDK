set(_GSM_ROOT_HINTS
    ${GSM_ROOT_DIR}
    ENV GSM_ROOT_DIR
)

find_path(GSM_INCLUDE_DIR NAMES gsm.h ${_GSM_ROOT_HINTS} PATH_SUFFIXES include)
find_library(GSM_LIBRARY NAMES gsm HINTS ${_GSM_ROOT_HINTS})

if(GSM_INCLUDE_DIR AND GSM_LIBRARY)
	set(GSM_FOUND TRUE)
endif()

if(GSM_FOUND)
	if (NOT Gsm_FIND_QUIETLY)
		message(STATUS "Found gsm includes:	${GSM_INCLUDE_DIR}/gsm.h")
		message(STATUS "Found gsm library: ${GSM_LIBRARY}")
	endif ()
    
    if(NOT TARGET GSM::GSM)
        add_library(GSM::GSM SHARED IMPORTED)
        set_target_properties(GSM:GSM PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${GSM_INCLUDE_DIR}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${GSM_LIBRARY}")
    endif()
    
    set(GSM_LIBRARIES GSM::GSM)
else()
	if (Gsm_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find gsm development files")
	endif ()
endif()
