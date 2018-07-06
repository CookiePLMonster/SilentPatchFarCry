#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502

#include <windows.h>

#include "Patterns.h"
#include "MemoryMgr.h"
#include "Trampoline.h"

#ifdef _WIN64
#define Is64Bit 1
#else
#define Is64Bit 0
#endif

TrampolineMgr trampolines;

void InstallD3D9Hook( void* func, Trampoline& trampoline );
extern const int* r_VSync;

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
		static bool gameDllHooked = false;
		if ( !d3d9Hooked && strcmp( name, "XRenderD3D9.dll" ) == 0 )
		{
			// Place a D3D9 hook
#if !Is64Bit
			void* createD3D = hook::make_module_pattern( module, "6A 1F E8" ).get_first( 0x2 );
#else
			void* createD3D = hook::make_module_pattern( module, "B9 1F 00 00 00 E8" ).get_first( 0x5 );
#endif

			InstallD3D9Hook( createD3D, trampolines.MakeTrampoline( module ) );
			
			// VSync hook
#if !Is64Bit
			r_VSync = *hook::make_module_pattern( module, "A1 ? ? ? ? 3B 86 E8 67 01 00" ).get_first<int*>( 0x1 );
#else
			uintptr_t addr = reinterpret_cast<uintptr_t>(hook::make_module_pattern( module, "8B 05 ? ? ? ? 41 3B 85 24 76 01 00" ).get_first());
			Memory::ReadOffsetValue( addr + 2, r_VSync );
#endif

			d3d9Hooked = true;
		}
#if Is64Bit
		else if ( !gameDllHooked && strcmp( name, "CryGame.dll" ) == 0 )
		{
			// Skip atexit for this (whatever it is), since it crashes
			void* crashyAtexit = hook::make_module_pattern( module, "33 F6 8B FE" ).get_first( -0x1C );
			Memory::VP::Patch<uint8_t>( crashyAtexit, 0xC3 );

			gameDllHooked = true;
		}
#endif

		return module;
	}
};


#if !Is64Bit
#include <shellapi.h>
#include <algorithm>
#endif
void Parse64BitCmdArgument()
{
#if !Is64Bit
	int argc = 0;
	if ( LPWSTR* argv = CommandLineToArgvW( GetCommandLineW(), &argc ) )
	{
		for ( ptrdiff_t i = 1; i < argc; i++ )
		{
			if ( wcscmp( argv[i], L"-64bit" ) == 0 )
			{
				std::wstring applicationName = std::wstring(argv[0]);
				std::transform( applicationName.begin(), applicationName.end(), applicationName.begin(), []( auto ch ) { return ::towlower( ch ); } );

				auto bin32Pos = applicationName.rfind( L"\\bin32\\" );
				if ( bin32Pos != std::string::npos )
				{
					// Replace 32 with 64
					applicationName[ bin32Pos + 4 ] = '6';
					applicationName[ bin32Pos + 5 ] = '4';

					std::wstring commandLine;
					for ( ptrdiff_t arg = 1; arg < argc; arg++ )
					{
						if ( arg != 1 )
						{
							commandLine += L' ';
						}
						commandLine.append( argv[arg] );
					}

					STARTUPINFOW si = { sizeof(si) };
					PROCESS_INFORMATION pi;
					BOOL result = CreateProcessW( applicationName.c_str(), commandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi );
					if ( result != FALSE )
					{
						CloseHandle( pi.hThread );
						CloseHandle( pi.hProcess );
						ExitProcess(0);
					}
				}

				break;
			}
		}

		LocalFree( argv );
	}
#endif
}

void InjectCrySystemPatches( HMODULE crySystem )
{
	using namespace hook;
	using namespace Memory::VP;

	{
		Trampoline& trampoline = trampolines.MakeTrampoline( crySystem );

#if !Is64Bit
		void* createRenderDLL = make_module_pattern( crySystem, "6A 01 8D 45 D8" ).get_first( 0x8 );
#else
		void* createRenderDLL = make_module_pattern( crySystem, "48 8D 54 24 ? 41 B0 01 48 8B CB" ).get_first( 0xB );
#endif
		ReadCall( createRenderDLL, CrySystem::orgLoadCryLibrary );
		InjectHook( createRenderDLL, trampoline.Jump(&CrySystem::LoadCryLibrary_Hooked) );

#if Is64Bit
		// 64-bit needs to be able to intercept CryGame.dll loading to grab a handle to it (so FreeLibrary does not unload anything and doesn't crash)
		void* loadGameDLL1 = make_module_pattern( crySystem, "E8 ? ? ? ? 48 85 C0 48 89 86 ? ? ? ? 75 4C" ).get_first( );
		void* loadGameDLL2 = make_module_pattern( crySystem, "E8 ? ? ? ? 48 89 86 ? ? ? ? EB 17" ).get_first( );
		void* loadGameDLL3 = make_module_pattern( crySystem, "E8 ? ? ? ? 48 89 86 ? ? ? ? 48 8B 8E" ).get_first( );

		InjectHook( loadGameDLL1, trampoline.Jump(&CrySystem::LoadCryLibrary_Hooked) );
		InjectHook( loadGameDLL2, trampoline.Jump(&CrySystem::LoadCryLibrary_Hooked) );
		InjectHook( loadGameDLL3, trampoline.Jump(&CrySystem::LoadCryLibrary_Hooked) );
#endif
	}
}

static FARPROC (WINAPI **orgGetProcAddress)(HMODULE hModule, LPCSTR lpProcName);
FARPROC WINAPI GetProcAddress_CrySystemInject(HMODULE hModule, LPCSTR lpProcName)
{
	const HMODULE crySystem = GetModuleHandleW( L"CrySystem" );
	if ( crySystem != nullptr )
	{
		InjectCrySystemPatches( crySystem );
	}

	return (*orgGetProcAddress)(hModule, lpProcName);
}
static auto* const pGetProcAddress_CrySystemInject = &GetProcAddress_CrySystemInject;

void InjectFarCryExePatches()
{
	using namespace hook;
	using namespace Memory::VP;

	const HMODULE crySystem = GetModuleHandleW( L"CrySystem" );
	if ( crySystem != nullptr )
	{
		// We are loaded late enough, just patch
		InjectCrySystemPatches( crySystem );
	}
	else
	{
		// Too early, patch into the place we SHOULD have been loaded from
#if !Is64Bit
		auto getProcAddress = pattern( "50 FF 15 ? ? ? ? 8B 0D" ).count_hint(1);
		if ( getProcAddress.size() == 1 )
		{
			auto addr = getProcAddress.get(0).get<decltype(GetProcAddress)**>( 1 + 2 );

			orgGetProcAddress = *addr;
			Patch( addr, &pGetProcAddress_CrySystemInject );

		}
#else

#endif
	}

	{
		// Fixed crash when scrolling mouse wheel on loading screen (32-bit only? 1.4 only?)
#if !Is64Bit
		auto jump = pattern( "2D 06 01 00 00" ).count_hint(1);
		if ( jump.size() == 1 )
		{
			Nop( jump.get(0).get<void>( 5 ), 2 );
		}
#endif
	}
}

void InitASI()
{
	Parse64BitCmdArgument();

	InjectFarCryExePatches();
}

extern "C"
{
	static LONG InitCount = 0;
	__declspec(dllexport) void InitializeASI()
	{
		if ( _InterlockedCompareExchange( &InitCount, 1, 0 ) != 0 ) return;
		InitASI();
	}
}
