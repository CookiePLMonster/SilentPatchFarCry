#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502

#include <windows.h>

#include "Patterns.h"
#include "MemoryMgr.h"

#ifdef _WIN64
#define Is64Bit 1
#else
#define Is64Bit 0
#endif

void InstallD3D9Hook( void* func );

class CrySystem
{
public:
	inline static HMODULE (CrySystem::*orgLoadCryLibrary)(const char*, bool);
	HMODULE LoadCryLibrary( const char* name, bool flag )
	{
		return (this->*orgLoadCryLibrary)(name, flag);
	}

	HMODULE LoadCryLibrary_Hooked( const char* name, bool flag )
	{
		HMODULE module = LoadCryLibrary( name, flag );

		static bool d3d9Hooked = false;
		if ( !d3d9Hooked && strcmp( name, "XRenderD3D9.dll" ) == 0 )
		{
			// Place a D3D9 hook
#if !Is64Bit
			void* createD3D = hook::make_module_pattern( module, "6A 1F E8" ).get_first( 0x2 );
#else
			void* createD3D = hook::make_module_pattern( module, "B9 1F 00 00 00 E8" ).get_first( 0x5 );
#endif

			InstallD3D9Hook( createD3D );
			d3d9Hooked = true;
		}

		return module;
	}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);

	if ( fdwReason == DLL_PROCESS_ATTACH )
	{
		using namespace hook;
		using namespace Memory::VP;

		{
#if !Is64Bit
			void* createRenderDLL = make_module_pattern( GetModuleHandleW( L"CrySystem" ), "6A 01 8D 45 D8" ).get_first( 0x8 );
#else
			void* createRenderDLL = make_module_pattern( GetModuleHandleW( L"CrySystem" ), "48 8D 54 24 ? 41 B0 01 48 8B CB" ).get_first( 0xB );
#endif
			ReadCall( createRenderDLL, CrySystem::orgLoadCryLibrary );
			InjectHook( createRenderDLL, &CrySystem::LoadCryLibrary_Hooked );
		}
	}
	return TRUE;
}
