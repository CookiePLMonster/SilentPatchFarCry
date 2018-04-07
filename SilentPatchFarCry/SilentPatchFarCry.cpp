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
			r_VSync = reinterpret_cast<int*>( addr + 6 + *reinterpret_cast<int32_t*>(addr+2) );
#endif

			d3d9Hooked = true;
		}

		return module;
	}
};


#if !Is64Bit
#include <shellapi.h>
#endif
void Parse64BitCmdArgument()
{
#if !Is64Bit
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW( GetCommandLineW(), &argc );

	for ( ptrdiff_t i = 1; i < argc; i++ )
	{
		if ( wcscmp( argv[i], L"-64bit" ) == 0 )
		{
			std::wstring applicationName = std::wstring(argv[0]);
			applicationName.erase( applicationName.find_last_of( L"/\\" ) );
			applicationName.erase( applicationName.find_last_of( L"/\\" ) + 1 );
			applicationName.append( L"Bin64\\FarCry.exe" );

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

			break;
		}
	}

	LocalFree( argv );
#endif
}

void InitializeASI()
{
	using namespace hook;
	using namespace Memory::VP;

	Parse64BitCmdArgument();

	{
		const HMODULE crySystem = GetModuleHandleW( L"CrySystem" );
		Trampoline& trampoline = trampolines.MakeTrampoline( crySystem );

#if !Is64Bit
		void* createRenderDLL = make_module_pattern( crySystem, "6A 01 8D 45 D8" ).get_first( 0x8 );
#else
		void* createRenderDLL = make_module_pattern( crySystem, "48 8D 54 24 ? 41 B0 01 48 8B CB" ).get_first( 0xB );
#endif
		ReadCall( createRenderDLL, CrySystem::orgLoadCryLibrary );
		InjectHook( createRenderDLL, trampoline.Jump(&CrySystem::LoadCryLibrary_Hooked) );
	}
}
