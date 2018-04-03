#include "D3D9Hook.h"

#include "MemoryMgr.h"
#include "Trampoline.h"

static IDirect3D9* (WINAPI *orgDirect3DCreate9)(UINT SDKVersion);
static IDirect3D9* WINAPI FCDirect3DCreate9(UINT SDKVersion)
{
	IDirect3D9* d3dInterface = orgDirect3DCreate9( SDKVersion );
	return new FCDirect3D9(d3dInterface);
}

void InstallD3D9Hook( void* func, Trampoline& trampoline )
{
	using namespace Memory::VP;

	ReadCall( func, orgDirect3DCreate9 );
	InjectHook( func, trampoline.Jump(FCDirect3DCreate9) );
}

// ========= FCDirect3D9 =========
HRESULT FCDirect3D9::QueryInterface(REFIID riid, void ** ppvObj)
{
	if ( ppvObj == nullptr ) return E_POINTER;

	if ( riid == __uuidof(IUnknown) ||
		riid == __uuidof(IDirect3D9) )
	{
		*ppvObj = static_cast<IDirect3D9*>(this);
		AddRef();
		return S_OK;
	}

	*ppvObj = nullptr;
	return E_NOINTERFACE;
}

ULONG FCDirect3D9::AddRef(void)
{
	return _InterlockedIncrement( &m_refCount );
}

ULONG FCDirect3D9::Release(void)
{
	const LONG ref = _InterlockedDecrement( &m_refCount );
	if ( ref == 0 )
	{
		delete this;
	}
	return ref;
}

HRESULT FCDirect3D9::RegisterSoftwareDevice(void * pInitializeFunction)
{
	return m_direct3D9->RegisterSoftwareDevice(pInitializeFunction);
}

UINT FCDirect3D9::GetAdapterCount(void)
{
	return m_direct3D9->GetAdapterCount();
}

HRESULT FCDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 * pIdentifier)
{
	return m_direct3D9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT FCDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
	return m_direct3D9->GetAdapterModeCount(Adapter, Format);
}

HRESULT FCDirect3D9::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE * pMode)
{
	return m_direct3D9->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT FCDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE * pMode)
{
	return m_direct3D9->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT FCDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	return m_direct3D9->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT FCDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return m_direct3D9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT FCDirect3D9::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD * pQualityLevels)
{
	return m_direct3D9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT FCDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return m_direct3D9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT FCDirect3D9::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return m_direct3D9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT FCDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 * pCaps)
{
	return m_direct3D9->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR FCDirect3D9::GetAdapterMonitor(UINT Adapter)
{
	return m_direct3D9->GetAdapterMonitor(Adapter);
}

HRESULT FCDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS * pPresentationParameters, IDirect3DDevice9 ** ppReturnedDeviceInterface)
{
	IDirect3DDevice9* device = nullptr;

	HRESULT result = m_direct3D9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &device);
	if ( FAILED(result) )
	{
		*ppReturnedDeviceInterface = nullptr;
		return result;
	}

	*ppReturnedDeviceInterface = new FCDirect3DDevice9( device );
	return result;
}

// ========= FCDirect3DDevice9 =========

HRESULT FCDirect3DDevice9::QueryInterface(REFIID riid, void ** ppvObj)
{
	if ( ppvObj == nullptr ) return E_POINTER;

	if ( riid == __uuidof(IUnknown) ||
		riid == __uuidof(IDirect3DDevice9) )
	{
		*ppvObj = static_cast<IDirect3DDevice9*>(this);
		AddRef();
		return S_OK;
	}

	*ppvObj = nullptr;
	return E_NOINTERFACE;
}

ULONG FCDirect3DDevice9::AddRef(void)
{
	return _InterlockedIncrement( &m_refCount );
}

ULONG FCDirect3DDevice9::Release(void)
{
	const LONG ref = _InterlockedDecrement( &m_refCount );
	if ( ref == 0 )
	{
		delete this;
	}
	return ref;
}

HRESULT FCDirect3DDevice9::TestCooperativeLevel(void)
{
	return m_direct3DDevice9->TestCooperativeLevel();
}

UINT FCDirect3DDevice9::GetAvailableTextureMem(void)
{
	return m_direct3DDevice9->GetAvailableTextureMem();
}

HRESULT FCDirect3DDevice9::EvictManagedResources(void)
{
	return m_direct3DDevice9->EvictManagedResources();;
}

HRESULT FCDirect3DDevice9::GetDirect3D(IDirect3D9 ** ppD3D9)
{
	return m_direct3DDevice9->GetDirect3D(ppD3D9);
}

HRESULT FCDirect3DDevice9::GetDeviceCaps(D3DCAPS9 * pCaps)
{
	return m_direct3DDevice9->GetDeviceCaps(pCaps);
}

HRESULT FCDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE * pMode)
{
	return m_direct3DDevice9->GetDisplayMode(iSwapChain, pMode);
}

HRESULT FCDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS * pParameters)
{
	return m_direct3DDevice9->GetCreationParameters(pParameters);
}

HRESULT FCDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 * pCursorBitmap)
{
	return m_direct3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void FCDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
	m_direct3DDevice9->SetCursorPosition(X, Y, Flags);
}

BOOL FCDirect3DDevice9::ShowCursor(BOOL bShow)
{
	return m_direct3DDevice9->ShowCursor(bShow);
}

HRESULT FCDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS * pPresentationParameters, IDirect3DSwapChain9 ** pSwapChain)
{
	return m_direct3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT FCDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9 ** pSwapChain)
{
	return m_direct3DDevice9->GetSwapChain(iSwapChain, pSwapChain);
}

UINT FCDirect3DDevice9::GetNumberOfSwapChains(void)
{
	return m_direct3DDevice9->GetNumberOfSwapChains();
}

HRESULT FCDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS * pPresentationParameters)
{
	return m_direct3DDevice9->Reset(pPresentationParameters);
}

HRESULT FCDirect3DDevice9::Present(const RECT * pSourceRect, const RECT * pDestRect, HWND hDestWindowOverride, const RGNDATA * pDirtyRegion)
{
	return m_direct3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT FCDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 ** ppBackBuffer)
{
	return m_direct3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT FCDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS * pRasterStatus)
{
	return m_direct3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT FCDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
	return m_direct3DDevice9->SetDialogBoxMode(bEnableDialogs);
}

void FCDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP * pRamp)
{
	m_direct3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp);
}

void FCDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP * pRamp)
{
	m_direct3DDevice9->GetGammaRamp(iSwapChain, pRamp);
}

HRESULT FCDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9 ** ppTexture, HANDLE * pSharedHandle)
{
	return m_direct3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT FCDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9 ** ppVolumeTexture, HANDLE * pSharedHandle)
{
	return m_direct3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT FCDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT FCDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT FCDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT FCDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT FCDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT FCDirect3DDevice9::BeginStateBlock()
{
	return m_direct3DDevice9->BeginStateBlock();
}

HRESULT FCDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	return m_direct3DDevice9->CreateStateBlock(Type, ppSB);
}

HRESULT FCDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	return m_direct3DDevice9->EndStateBlock(ppSB);
}

HRESULT FCDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus)
{
	return m_direct3DDevice9->GetClipStatus(pClipStatus);
}

HRESULT FCDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	return m_direct3DDevice9->GetRenderState(State, pValue);
}

HRESULT FCDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	return m_direct3DDevice9->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT FCDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return m_direct3DDevice9->GetTransform(State, pMatrix);
}

HRESULT FCDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9 *pClipStatus)
{
	return m_direct3DDevice9->SetClipStatus(pClipStatus);
}

HRESULT FCDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	// ======= FAR CRY FIX =======
	if ( State == D3DRS_CLIPPLANEENABLE )
	{
		m_clipPlaneRenderState = Value;
	}

	return m_direct3DDevice9->SetRenderState(State, Value);
}

HRESULT FCDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	return m_direct3DDevice9->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT FCDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return m_direct3DDevice9->SetTransform(State, pMatrix);
}

HRESULT FCDirect3DDevice9::DeletePatch(UINT Handle)
{
	return m_direct3DDevice9->DeletePatch(Handle);
}

HRESULT FCDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return m_direct3DDevice9->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT FCDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return m_direct3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT FCDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	return m_direct3DDevice9->GetIndices(ppIndexData);
}

HRESULT FCDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	return m_direct3DDevice9->SetIndices(pIndexData);
}

HRESULT FCDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9 *pLight)
{
	return m_direct3DDevice9->GetLight(Index, pLight);
}

HRESULT FCDirect3DDevice9::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return m_direct3DDevice9->GetLightEnable(Index, pEnable);
}

HRESULT FCDirect3DDevice9::GetMaterial(D3DMATERIAL9 *pMaterial)
{
	return m_direct3DDevice9->GetMaterial(pMaterial);
}

HRESULT FCDirect3DDevice9::LightEnable(DWORD LightIndex, BOOL bEnable)
{
	return m_direct3DDevice9->LightEnable(LightIndex, bEnable);
}

HRESULT FCDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight)
{

	return m_direct3DDevice9->SetLight(Index, pLight);
}

HRESULT FCDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
{
	return m_direct3DDevice9->SetMaterial(pMaterial);
}

HRESULT FCDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	return m_direct3DDevice9->MultiplyTransform(State, pMatrix);
}

HRESULT FCDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	return m_direct3DDevice9->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT FCDirect3DDevice9::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	return m_direct3DDevice9->GetCurrentTexturePalette(pPaletteNumber);
}

HRESULT FCDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	return m_direct3DDevice9->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT FCDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return m_direct3DDevice9->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT FCDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
	return m_direct3DDevice9->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT FCDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	return m_direct3DDevice9->CreatePixelShader(pFunction, ppShader);
}

HRESULT FCDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	return m_direct3DDevice9->GetPixelShader(ppShader);
}

HRESULT FCDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	return m_direct3DDevice9->SetPixelShader(pShader);
}

HRESULT FCDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	// ======= FAR CRY FIX =======
	ApplyClipPlanes();

	return m_direct3DDevice9->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT FCDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	// ======= FAR CRY FIX =======
	ApplyClipPlanes();

	return m_direct3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT FCDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	// ======= FAR CRY FIX =======
	ApplyClipPlanes();

	return m_direct3DDevice9->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT FCDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	// ======= FAR CRY FIX =======
	ApplyClipPlanes();

	return m_direct3DDevice9->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT FCDirect3DDevice9::BeginScene()
{
	return m_direct3DDevice9->BeginScene();
}

HRESULT FCDirect3DDevice9::EndScene()
{
	return m_direct3DDevice9->EndScene();
}

HRESULT FCDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	return m_direct3DDevice9->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}

HRESULT FCDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	return m_direct3DDevice9->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT FCDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
	return m_direct3DDevice9->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT FCDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9 **ppTexture)
{
	return m_direct3DDevice9->GetTexture(Stage, ppTexture);
}

HRESULT FCDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	return m_direct3DDevice9->GetTextureStageState(Stage, Type, pValue);
}

HRESULT FCDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
	return m_direct3DDevice9->SetTexture(Stage, pTexture);
}

HRESULT FCDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return m_direct3DDevice9->SetTextureStageState(Stage, Type, Value);
}

HRESULT FCDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9 *pSourceTexture, IDirect3DBaseTexture9 *pDestinationTexture)
{
	return m_direct3DDevice9->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT FCDirect3DDevice9::ValidateDevice(DWORD *pNumPasses)
{
	return m_direct3DDevice9->ValidateDevice(pNumPasses);
}

HRESULT FCDirect3DDevice9::GetClipPlane(DWORD Index, float *pPlane)
{
	// ======= FAR CRY FIX =======
	if ( pPlane == nullptr || Index >= MAX_CLIP_PLANES ) return D3DERR_INVALIDCALL;

	memcpy( pPlane, m_storedClipPlanes[Index], sizeof(m_storedClipPlanes[0]) );
	return D3D_OK;
}

HRESULT FCDirect3DDevice9::SetClipPlane(DWORD Index, CONST float *pPlane)
{
	// ======= FAR CRY FIX =======
	if ( pPlane == nullptr || Index >= MAX_CLIP_PLANES ) return D3DERR_INVALIDCALL;

	memcpy( m_storedClipPlanes[Index], pPlane, sizeof(m_storedClipPlanes[0]) );
	return D3D_OK;
}

HRESULT FCDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return m_direct3DDevice9->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT FCDirect3DDevice9::GetViewport(D3DVIEWPORT9 *pViewport)
{
	return m_direct3DDevice9->GetViewport(pViewport);
}

HRESULT FCDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9 *pViewport)
{
	return m_direct3DDevice9->SetViewport(pViewport);
}

HRESULT FCDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	return m_direct3DDevice9->CreateVertexShader(pFunction, ppShader);
}

HRESULT FCDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	return m_direct3DDevice9->GetVertexShader(ppShader);
}

HRESULT FCDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	return m_direct3DDevice9->SetVertexShader(pShader);
}

HRESULT FCDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	return m_direct3DDevice9->CreateQuery(Type, ppQuery);
}

HRESULT FCDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return m_direct3DDevice9->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT FCDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return m_direct3DDevice9->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT FCDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return m_direct3DDevice9->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT FCDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return m_direct3DDevice9->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT FCDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return m_direct3DDevice9->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT FCDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return m_direct3DDevice9->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT FCDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{
	return m_direct3DDevice9->SetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT FCDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* Divider)
{
	return m_direct3DDevice9->GetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT FCDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return m_direct3DDevice9->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT FCDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return m_direct3DDevice9->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT FCDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return m_direct3DDevice9->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT FCDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return m_direct3DDevice9->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT FCDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return m_direct3DDevice9->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT FCDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return m_direct3DDevice9->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT FCDirect3DDevice9::SetFVF(DWORD FVF)
{
	return m_direct3DDevice9->SetFVF(FVF);
}

HRESULT FCDirect3DDevice9::GetFVF(DWORD* pFVF)
{
	return m_direct3DDevice9->GetFVF(pFVF);
}

HRESULT FCDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	return m_direct3DDevice9->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT FCDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	return m_direct3DDevice9->SetVertexDeclaration(pDecl);
}

HRESULT FCDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	return m_direct3DDevice9->GetVertexDeclaration(ppDecl);
}

HRESULT FCDirect3DDevice9::SetNPatchMode(float nSegments)
{
	return m_direct3DDevice9->SetNPatchMode(nSegments);
}

float FCDirect3DDevice9::GetNPatchMode(THIS)
{
	return m_direct3DDevice9->GetNPatchMode();
}

int FCDirect3DDevice9::GetSoftwareVertexProcessing(THIS)
{
	return m_direct3DDevice9->GetSoftwareVertexProcessing();
}

HRESULT FCDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return m_direct3DDevice9->SetSoftwareVertexProcessing(bSoftware);
}

HRESULT FCDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{
	return m_direct3DDevice9->SetScissorRect(pRect);
}

HRESULT FCDirect3DDevice9::GetScissorRect(RECT* pRect)
{
	return m_direct3DDevice9->GetScissorRect(pRect);
}

HRESULT FCDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	return m_direct3DDevice9->GetSamplerState(Sampler, Type, pValue);
}

HRESULT FCDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return m_direct3DDevice9->SetSamplerState(Sampler, Type, Value);
}

HRESULT FCDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	return m_direct3DDevice9->SetDepthStencilSurface(pNewZStencil);
}

HRESULT FCDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT FCDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	return m_direct3DDevice9->ColorFill(pSurface, pRect, color);
}

HRESULT FCDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	return m_direct3DDevice9->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT FCDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	return m_direct3DDevice9->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT FCDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	return m_direct3DDevice9->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT FCDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	return m_direct3DDevice9->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

// ======= FAR CRY FIX =======
void FCDirect3DDevice9::ApplyClipPlanes()
{
	DWORD index = 0;
	for( const auto clipPlane : m_storedClipPlanes )
	{
		if ( (m_clipPlaneRenderState & (1 << index)) != 0 )
		{
			m_direct3DDevice9->SetClipPlane( index, clipPlane );
		}
		index++;
	}
}