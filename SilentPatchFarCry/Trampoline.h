#pragma once

#include "MemoryMgr.h"
#include <cassert>

// Trampoline class for big (>2GB) jumps
// Never needed in 32-bit processes so in those cases this does nothing but forwards to Memory functions
// NOTE: Each Trampoline class allocates a page of executable memory for trampolines and does NOT free it when going out of scope
class Trampoline
{
public:
	constexpr Trampoline() = default;

// Trampolines are useless on x86 arch
#ifdef _WIN64

	explicit Trampoline( uintptr_t preferredBaseAddr )
	{
		SYSTEM_INFO systemInfo;
		GetSystemInfo( &systemInfo );
		m_pageSize = systemInfo.dwPageSize;
		m_pageMemory = FindAndAllocateMem( preferredBaseAddr );
	}

	bool FeasibleForAddresss( uintptr_t addr ) const
	{
		return IsAddressFeasible( (uintptr_t)m_pageMemory, addr ) && ( m_pageUsed + SINGLE_TRAMPOLINE_SIZE ) <= m_pageSize;
	}

	template<typename AT, typename Func>
	void InjectHook(AT address, Func hook)
	{
		assert( FeasibleForAddresss(uintptr_t(address)) );

		union member_cast
		{
			intptr_t addr;
			Func funcPtr;
		} cast;
		static_assert( sizeof(cast.addr) == sizeof(cast.funcPtr), "member_cast failure!" );

		cast.funcPtr = hook;
		LPVOID trampoline = CreateTrampoline( cast.addr );

		// Jump to trampoline
		Memory::VP::InjectHook( address, trampoline );
	}

	template<typename AT, typename HT>
	inline void		InjectHook(AT address, HT hook, unsigned int nType)
	{
		assert( FeasibleForAddresss(uintptr_t(address)) );

		union member_cast
		{
			intptr_t addr;
			Func funcPtr;
		} cast;
		static_assert( sizeof(cast.addr) == sizeof(cast.funcPtr), "member_cast failure!" );

		cast.funcPtr = hook;
		LPVOID trampoline = CreateTrampoline( cast.addr );

		// Jump to trampoline
		Memory::VP::InjectHook( address, trampoline, nType );
	}

private:
	static constexpr size_t SINGLE_TRAMPOLINE_SIZE = 12;

	LPVOID CreateTrampoline( intptr_t addr )
	{
		LPVOID trampolineSpace = GetNewSpace();

		// Create trampoline code
		Memory::Patch( trampolineSpace, { 0x48, 0xB8 } );
		Memory::Patch<intptr_t>( static_cast<uint8_t*>(trampolineSpace) + 2, addr );
		Memory::Patch( static_cast<uint8_t*>(trampolineSpace) + 10, { 0xFF, 0xE0 } );

		return trampolineSpace;
	}

	LPVOID GetNewSpace()
	{
		m_pageUsed += SINGLE_TRAMPOLINE_SIZE;
		assert( m_pageUsed <= m_pageSize );

		LPVOID space = m_pageMemory;
		m_pageMemory = static_cast<uint8_t*>(m_pageMemory) + SINGLE_TRAMPOLINE_SIZE;
		return space;
	}

	LPVOID FindAndAllocateMem( const uintptr_t addr )
	{
		uintptr_t curAddr = addr;
		// Find the first unallocated page after 'addr' and try to allocate a page for trampolines there
		while ( true )
		{
			if ( !IsAddressFeasible( curAddr, addr ) ) break;

			MEMORY_BASIC_INFORMATION MemoryInf;
			if ( VirtualQuery( (LPCVOID)curAddr, &MemoryInf, sizeof(MemoryInf) ) == 0 ) break;
			if ( MemoryInf.State == MEM_FREE )
			{
				LPVOID mem = VirtualAlloc( (LPVOID)curAddr, m_pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
				if ( mem != nullptr )
				{
					return mem;
				}
				curAddr += m_pageSize;
			}
			curAddr += MemoryInf.RegionSize;
		}
		return nullptr;
	}

	bool IsAddressFeasible( uintptr_t trampolineOffset, uintptr_t addr ) const
	{
		const ptrdiff_t diff = trampolineOffset - addr;
		return diff >= INT32_MIN && diff <= INT32_MAX;
	}

	DWORD m_pageSize = 0;
	DWORD m_pageUsed = 0;
	LPVOID m_pageMemory = nullptr;

#else

	constexpr explicit Trampoline( uintptr_t ) { }

	constexpr bool FeasibleForAddresss( uintptr_t ) const { return true; }

	template<typename AT, typename Func>
	void InjectHook(AT address, Func hook)
	{
		Memory::VP::InjectHook( address, hook );
	}

	template<typename AT, typename HT>
	inline void		InjectHook(AT address, HT hook, unsigned int nType)
	{
		Memory::VP::InjectHook( address, hook, nType );
	}

#endif
};