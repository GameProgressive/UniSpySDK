#ifndef WINERRORDXGI
#define WINERRORDXGI

#include <winerror.h> // Conflicting header declaration

// Undefine shared macros

#undef DXGI_STATUS_OCCLUDED
#undef DXGI_STATUS_CLIPPED
#undef DXGI_STATUS_NO_REDIRECTION
#undef DXGI_STATUS_NO_DESKTOP_ACCESS
#undef DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_STATUS_MODE_CHANGED
#undef DXGI_STATUS_MODE_CHANGE_IN_PROGRESS
#undef DXGI_ERROR_INVALID_CALL
#undef DXGI_ERROR_NOT_FOUND
#undef DXGI_ERROR_MORE_DATA
#undef DXGI_ERROR_UNSUPPORTED
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_HUNG
#undef DXGI_ERROR_DEVICE_RESET
#undef DXGI_ERROR_WAS_STILL_DRAWING
#undef DXGI_ERROR_FRAME_STATISTICS_DISJOINT
#undef DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_ERROR_DRIVER_INTERNAL_ERROR
#undef DXGI_ERROR_NONEXCLUSIVE
#undef DXGI_ERROR_NOT_CURRENTLY_AVAILABLE
#undef DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED
#undef DXGI_ERROR_REMOTE_OUTOFMEMORY
#undef DXGI_CPU_ACCESS_NONE
#undef DXGI_CPU_ACCESS_DYNAMIC
#undef DXGI_CPU_ACCESS_READ_WRITE
#undef DXGI_CPU_ACCESS_SCRATCH
#undef DXGI_CPU_ACCESS_FIELD
#undef DXGI_USAGE_SHADER_INPUT
#undef DXGI_USAGE_RENDER_TARGET_OUTPUT
#undef DXGI_USAGE_BACK_BUFFER
#undef DXGI_USAGE_SHARED
#undef DXGI_USAGE_READ_ONLY
#undef DXGI_USAGE_DISCARD_ON_PRESENT
#undef DXGI_USAGE_UNORDERED_ACCESS
#undef D3DERR_WRONG_STATE
#undef DWRITE_E_FILEFORMAT
#undef DWRITE_E_UNEXPECTED
#undef DWRITE_E_NOFONT
#undef DWRITE_E_FILENOTFOUND
#undef DWRITE_E_FILEACCESS
#undef DWRITE_E_FONTCOLLECTIONOBSOLETE
#undef DWRITE_E_ALREADYREGISTERED
#undef D2DERR_WRONG_STATE
#undef D2DERR_NOT_INITIALIZED
#undef D2DERR_UNSUPPORTED_OPERATION
#undef D2DERR_SCANNER_FAILED
#undef D2DERR_SCREEN_ACCESS_DENIED
#undef D2DERR_DISPLAY_STATE_INVALID
#undef D2DERR_ZERO_VECTOR
#undef D2DERR_INTERNAL_ERROR
#undef D2DERR_DISPLAY_FORMAT_NOT_SUPPORTED
#undef D2DERR_INVALID_CALL
#undef D2DERR_NO_HARDWARE_DEVICE
#undef D2DERR_RECREATE_TARGET
#undef D2DERR_TOO_MANY_SHADER_ELEMENTS
#undef D2DERR_SHADER_COMPILE_FAILED
#undef D2DERR_MAX_TEXTURE_SIZE_EXCEEDED
#undef D2DERR_UNSUPPORTED_VERSION
#undef D2DERR_BAD_NUMBER
#undef D2DERR_WRONG_FACTORY
#undef D2DERR_LAYER_ALREADY_IN_USE
#undef D2DERR_POP_CALL_DID_NOT_MATCH_PUSH
#undef D2DERR_WRONG_RESOURCE_DOMAIN
#undef D2DERR_PUSH_POP_UNBALANCED
#undef D2DERR_RENDER_TARGET_HAS_LAYER_OR_CLIPRECT
#undef D2DERR_INCOMPATIBLE_BRUSH_TYPES
#undef D2DERR_WIN32_ERROR
#undef D2DERR_TARGET_NOT_GDI_COMPATIBLE
#undef D2DERR_TEXT_EFFECT_IS_WRONG_TYPE
#undef D2DERR_TEXT_RENDERER_NOT_RELEASED 
#undef D2DERR_EXCEEDS_MAX_BITMAP_SIZE 
#undef D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D10_ERROR_FILE_NOT_FOUND
#undef D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D11_ERROR_FILE_NOT_FOUND
#undef D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS
#undef D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD
#undef D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD

// Windows macro for DXGIType/ new DXGI defines
#define BYTE unsigned char
#define BOOL int
#define UINT unsigned int

#include <dxgitype.h> // June 2010 inclusion

typedef struct DXGI_JPEG_AC_HUFFMAN_TABLE {
	BYTE CodeCounts[16];
	BYTE CodeValues[162];
} DXGI_JPEG_AC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_DC_HUFFMAN_TABLE {
	BYTE CodeCounts[12];
	BYTE CodeValues[12];
} DXGI_JPEG_DC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_QUANTIZATION_TABLE {
	BYTE Elements[64];
} DXGI_JPEG_QUANTIZATION_TABLE;

// Undefine precedent macros (keep compatibility with windef.h)
#undef BYTE
#undef BOOL
#undef UINT

#endif
