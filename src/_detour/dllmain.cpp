// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "python_funcs.h"

#include "guiconsole.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		OutputDebugString("Initializing _detour");
		printf("Initializing _detour\n");
		Python_Initialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Python_Unload();
		break;
	}
	return TRUE;
}

