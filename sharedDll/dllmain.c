///////////////////////////////////////////////////////////////////////////////
// File:	dllmain.c
// SDK:		UniSpy SDK Shared Library
//

#include "../common/gsPlatform.h"

// TODO: Verify if Xbox and Xbox360 have shared libraries
#if defined(_WIN32) && !defined(_XBOX) && !defined(_X360)
BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved
)

{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_DETACH:
		break;

	default:
		break;
	}

	return TRUE;
}
#endif
