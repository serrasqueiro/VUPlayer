#include "WndVisual.h"

#include <VersionHelpers.h>

#include "resource.h"

#include "Artwork.h"
#include "NullVisual.h"
#include "Oscilloscope.h"
#include "PeakMeter.h"
#include "SpectrumAnalyser.h"
#include "VUMeter.h"
#include "VUPlayer.h"

// Visual control ID
static const UINT_PTR s_WndVisualID = 1678;

// Minimum swap chain width/height
static const int s_MinSwapChainSize = 4;

// Visual window class name
static const wchar_t s_VisualClass[] = L"VUVisualClass";

// Oscilloscope weights
static const std::map<UINT,float> s_OscilloscopeWeights = {
		{ ID_OSCILLOSCOPE_WEIGHT_FINE, 1.0f },
		{ ID_OSCILLOSCOPE_WEIGHT_NORMAL, 2.0f },
		{ ID_OSCILLOSCOPE_WEIGHT_BOLD, 3.0f }
};

// VUMeter decay settings
static const std::map<UINT,float> s_VUMeterDecay = {
		{ ID_VUMETER_SLOWDECAY, VUMeterDecayMinimum },
		{ ID_VUMETER_NORMALDECAY, VUMeterDecayNormal },
		{ ID_VUMETER_FASTDECAY, VUMeterDecayMaximum }
};

LRESULT CALLBACK WndVisual::VisualProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	WndVisual* wndVisual = reinterpret_cast<WndVisual*>( GetWindowLongPtr( hwnd, GWLP_USERDATA ) );
	if ( nullptr != wndVisual ) {
		switch ( message ) {
			case WM_PAINT : {
				PAINTSTRUCT ps;
				BeginPaint( hwnd, &ps );
				wndVisual->OnPaint( ps );
				EndPaint( hwnd, &ps );
				break;
			}
			case WM_WINDOWPOSCHANGING : {
				WINDOWPOS* windowPos = reinterpret_cast<WINDOWPOS*>( lParam );
				if ( nullptr != windowPos ) {
					wndVisual->Resize( windowPos );
				}
				break;
			}
			case WM_CONTEXTMENU : {
				POINT pt = {};
				pt.x = LOWORD( lParam );
				pt.y = HIWORD( lParam );
				wndVisual->OnContextMenu( pt );
				break;
			}
		}
	}
	return DefWindowProc( hwnd, message, wParam, lParam );
}

WndVisual::WndVisual( HINSTANCE instance, HWND parent, HWND rebarWnd, HWND statusWnd, Settings& settings, Output& output, Library& library ) :
	m_hInst( instance ),
	m_hWnd( NULL ),
	m_hWndParent( parent ),
	m_hWndRebar( rebarWnd ),
	m_hWndStatus( statusWnd ),
	m_Settings( settings ),
	m_Output( output ),
	m_Library( library ),
	m_D2DFactory(),
	m_D2DDeviceContext(),
	m_D2DSwapChain(),
	m_Visuals(),
	m_CurrentVisual()
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof( WNDCLASSEX );
	wc.hInstance = instance;
	wc.lpfnWndProc = VisualProc;
	wc.lpszClassName = s_VisualClass;
	wc.hCursor = LoadCursor( 0, IDC_ARROW );
	wc.hbrBackground = reinterpret_cast<HBRUSH>( COLOR_3DFACE + 1 );
	RegisterClassEx( &wc );

	const DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN;
	const DWORD exStyle = 0;
	const int x = 0;
	const int y = 0;
	const int width = 0;
	const int height = 0;
	LPVOID param = NULL;

	m_hWnd = CreateWindowEx( exStyle, s_VisualClass, NULL, style, x, y, width, height, parent, reinterpret_cast<HMENU>( s_WndVisualID ), instance, param );
	SetWindowLongPtr( m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( this ) );

	InitD2D();

	m_Visuals.insert( Visuals::value_type( ID_VISUAL_VUMETER_STEREO, new VUMeter( *this, true /*stereo*/ ) ) );
	m_Visuals.insert( Visuals::value_type( ID_VISUAL_VUMETER_MONO, new VUMeter( *this, false /*mono*/ ) ) );
	m_Visuals.insert( Visuals::value_type( ID_VISUAL_OSCILLOSCOPE, new Oscilloscope( *this ) ) );
	m_Visuals.insert( Visuals::value_type( ID_VISUAL_SPECTRUMANALYSER, new SpectrumAnalyser( *this ) ) );
	m_Visuals.insert( Visuals::value_type( ID_VISUAL_ARTWORK, new Artwork( *this ) ) );
	m_Visuals.insert( Visuals::value_type( ID_VISUAL_NONE, new NullVisual( *this ) ) );
	m_Visuals.insert( Visuals::value_type( ID_VISUAL_PEAKMETER, new PeakMeter( *this ) ) );

	int visualID = m_Settings.GetVisualID();
	switch ( visualID ) {
		case ID_VISUAL_VUMETER_STEREO :
		case ID_VISUAL_VUMETER_MONO :
		case ID_VISUAL_OSCILLOSCOPE :
		case ID_VISUAL_SPECTRUMANALYSER :
		case ID_VISUAL_ARTWORK :
		case ID_VISUAL_PEAKMETER :
		case ID_VISUAL_NONE : {
			break;
		}
		default : {
			visualID = ID_VISUAL_ARTWORK;
			break;
		}
	}
	PostMessage( GetParent( m_hWnd ), WM_COMMAND, visualID, 0 );
}

WndVisual::~WndVisual()
{
	for ( const auto& iter : m_Visuals ) {
		Visual* visual = iter.second;
		delete visual;
	}
	ReleaseD2D();

	m_Settings.SetVisualID( static_cast<int>( m_CurrentVisual ) );
}

HWND WndVisual::GetWindowHandle()
{
	return m_hWnd;
}

void WndVisual::ReleaseD2D()
{
	if ( m_D2DDeviceContext ) {
		m_D2DDeviceContext.Reset();
	}
	if ( m_D2DSwapChain ) {
		m_D2DSwapChain.Reset();
	}
	if ( m_D2DFactory ) {
		m_D2DFactory.Reset();
	}
}

void WndVisual::InitD2D()
{
	ReleaseD2D();

	// Create Direct2D factory.
	HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), &m_D2DFactory );
	if ( SUCCEEDED( hr ) ) {
		// Create Direct3D device.
		IDXGIAdapter* adapter = NULL;
		const HMODULE software = NULL;
		const UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		D3D_FEATURE_LEVEL* featureLevel = NULL;
		const UINT featureLevels = 0;
		const UINT sdkVersion = D3D11_SDK_VERSION;
		Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
		hr = D3D11CreateDevice( adapter, driverType, software, flags, featureLevel, featureLevels, sdkVersion, &d3dDevice, featureLevel, NULL /*d3dDeviceContext*/ );
		if ( FAILED( hr ) ) {
			driverType = D3D_DRIVER_TYPE_WARP;
			hr = D3D11CreateDevice( adapter, driverType, software, flags, featureLevel, featureLevels, sdkVersion, &d3dDevice, featureLevel, NULL /*d3dDeviceContext*/ );
		}

		if ( SUCCEEDED( hr ) ) {
			// Create Direct2D device context.
			Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
			hr = d3dDevice.As( &dxgiDevice );
			if ( SUCCEEDED( hr ) ) {
				Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
				hr = m_D2DFactory->CreateDevice( dxgiDevice.Get(), &d2dDevice );
				if ( SUCCEEDED( hr ) ) {
					hr = d2dDevice->CreateDeviceContext( D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_D2DDeviceContext );
					if ( SUCCEEDED( hr ) ) {
						// Create DXGI swap chain.
						DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
						swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
						swapChainDesc.SampleDesc.Count = 1;
						swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
						swapChainDesc.BufferCount = 2;
						swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

						if ( IsWindows8OrGreater() ) {
							swapChainDesc.Scaling = DXGI_SCALING_NONE;
							swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
						} else {
							swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
							swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
						}

						Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
						hr = dxgiDevice->GetAdapter( &dxgiAdapter );
						if ( SUCCEEDED( hr ) ) {
							Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
							hr = dxgiAdapter->GetParent( IID_PPV_ARGS( &dxgiFactory ) );
							if ( SUCCEEDED( hr ) ) {
								hr = dxgiFactory->CreateSwapChainForHwnd( d3dDevice.Get(), m_hWnd, &swapChainDesc, NULL /*fullscreenDesc*/, NULL /*restrictToOutput*/, &m_D2DSwapChain );
								if ( SUCCEEDED( hr ) ) {
									dxgiDevice->SetMaximumFrameLatency( 1 );
								}
							}
						}
					}
				}
			}
		}
	}

	if ( FAILED( hr ) ) {
		ReleaseD2D();
	}
}

void WndVisual::CreateD2DDeviceContext()
{
	if ( !m_D2DDeviceContext ) {
		InitD2D();
	}
}

HRESULT WndVisual::ResizeD2DSwapChain( const WINDOWPOS* windowPos )
{
	HRESULT hr = ( m_D2DFactory && m_D2DSwapChain && m_D2DDeviceContext ) ? S_OK : -1;
	if ( SUCCEEDED( hr ) ) {
		UINT clientWidth = 0;
		UINT clientHeight = 0;
		if ( nullptr != windowPos ) {
			clientWidth = ( windowPos->cx < s_MinSwapChainSize ) ? static_cast<UINT>( s_MinSwapChainSize ) : static_cast<UINT>( windowPos->cx );
			clientHeight = ( windowPos->cy < s_MinSwapChainSize ) ? static_cast<UINT>( s_MinSwapChainSize ) : static_cast<UINT>( windowPos->cy );
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			hr = m_D2DSwapChain->GetDesc1( &swapChainDesc );
			if ( SUCCEEDED( hr ) ) {
				if ( ( swapChainDesc.Width != clientWidth ) || ( swapChainDesc.Height != clientHeight ) ) {
					m_D2DDeviceContext->SetTarget( nullptr );
					hr = m_D2DSwapChain->ResizeBuffers( 0 /*bufferCount*/, clientWidth, clientHeight, DXGI_FORMAT_UNKNOWN, 0 /*flags*/ );
					if ( SUCCEEDED( hr ) ) {
						Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
						hr = m_D2DSwapChain->GetBuffer( 0, IID_PPV_ARGS( &backBuffer ) );
						if ( SUCCEEDED( hr ) ) {
							D2D1_BITMAP_PROPERTIES1 bitmapProperties = {};
							bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
							bitmapProperties.pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE };
							Microsoft::WRL::ComPtr<IDXGISurface> dxgiBackBuffer;
							hr = m_D2DSwapChain->GetBuffer( 0, IID_PPV_ARGS( &dxgiBackBuffer ) );
							if ( SUCCEEDED( hr ) ) {
								Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dBitmap;
								hr = m_D2DDeviceContext->CreateBitmapFromDxgiSurface( dxgiBackBuffer.Get(), &bitmapProperties, &d2dBitmap );
								if ( SUCCEEDED( hr ) ) {
									m_D2DDeviceContext->SetTarget( d2dBitmap.Get() );
									float dpiX = 96;
									float dpiY = 96;
									m_D2DFactory->GetDesktopDpi( &dpiX, &dpiY );
									m_D2DDeviceContext->SetDpi( dpiX, dpiY );
									Visual* visual = GetCurrentVisual();
									if ( nullptr != visual ) {
										visual->OnPaint();
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if ( FAILED( hr ) ) {
		InitD2D();
	}
	return hr;
}

Output& WndVisual::GetOutput()
{
	return m_Output;
}

Library& WndVisual::GetLibrary()
{
	return m_Library;
}

Settings& WndVisual::GetSettings()
{
	return m_Settings;
}

ID2D1DeviceContext* WndVisual::BeginDrawing()
{
	CreateD2DDeviceContext();
	if ( m_D2DDeviceContext ) {
		m_D2DDeviceContext->BeginDraw();
	}
	return m_D2DDeviceContext.Get();
}

void WndVisual::EndDrawing()
{
	if ( m_D2DDeviceContext ) {
		m_D2DDeviceContext->EndDraw();
		DXGI_PRESENT_PARAMETERS parameters = {};
		const HRESULT hr = m_D2DSwapChain->Present( 1 /*syncInterval*/, 0 /*flags*/ );
		if ( ( S_OK != hr ) && ( DXGI_STATUS_OCCLUDED != hr ) ) {
			InitD2D();
		}
	}
}

void WndVisual::OnPaint( const PAINTSTRUCT& ps )
{
	Visual* visual = GetCurrentVisual();
	if ( nullptr != visual ) {
		visual->OnPaint();
	} else {
		FillRect( ps.hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>( COLOR_3DFACE + 1 ) );
	}
}

void WndVisual::Resize( WINDOWPOS* windowPos )
{
	if ( nullptr != windowPos ) {
		Visual* visual = GetCurrentVisual();
		if ( nullptr != visual ) {
			const int requestedHeight = visual->GetHeight( windowPos->cx );
			if ( requestedHeight != windowPos->cy ) {
				windowPos->y -= ( requestedHeight - windowPos->cy );
				windowPos->cy = requestedHeight;
			}

			RECT parentRect = {};
			RECT rebarRect = {};
			RECT statusRect = {};
			if ( ( 0 != GetClientRect( m_hWndParent, &parentRect ) ) && ( 0 != GetWindowRect( m_hWndRebar, &rebarRect ) ) && ( 0 != GetWindowRect( m_hWndStatus, &statusRect ) ) ) {
				const int minY = rebarRect.bottom - rebarRect.top;
				const int maxCY = ( parentRect.bottom - parentRect.top ) - minY - ( statusRect.bottom - statusRect.top );
				if ( windowPos->y < minY ) {
					windowPos->y = minY;
				}
				if ( windowPos->cy > maxCY ) {
					windowPos->cy = maxCY;
				}
			}
			ResizeD2DSwapChain( windowPos );
		}
	}
}

Visual* WndVisual::GetCurrentVisual()
{
	Visual* visual = nullptr;
	auto visualIter = m_Visuals.find( m_CurrentVisual );
	if ( visualIter != m_Visuals.end() ) {
		visual = visualIter->second;
	}
	return visual;
}

std::set<UINT> WndVisual::GetVisualIDs() const
{
	std::set<UINT> visualIDs;
	for ( const auto& iter : m_Visuals ) {
		visualIDs.insert( iter.first );
	}
	return visualIDs;
}

void WndVisual::SelectVisual( const long id )
{
	if ( id != m_CurrentVisual ) {
		Visual* visual = GetCurrentVisual();
		if ( nullptr != visual ) {
			visual->Hide();
		}
		m_CurrentVisual = 0;

		const auto visualIter = m_Visuals.find( id );
		if ( visualIter != m_Visuals.end() ) {
			visual = visualIter->second;
			if ( nullptr != visual ) {
				visual->Show();
				m_CurrentVisual = id;
			}
		}
	}
}

long WndVisual::GetCurrentVisualID() const
{
	return m_CurrentVisual;
}

void WndVisual::OnContextMenu( const POINT& position )
{
	HMENU menu = LoadMenu( m_hInst, MAKEINTRESOURCE( IDR_MENU_VISUAL ) );
	if ( NULL != menu ) {
		HMENU visualmenu = GetSubMenu( menu, 0 /*pos*/ );
		if ( NULL != visualmenu ) {
			const UINT flags = TPM_RIGHTBUTTON;
			TrackPopupMenu( visualmenu, flags, position.x, position.y, 0 /*reserved*/, GetParent( m_hWnd ), NULL /*rect*/ );
		}
		DestroyMenu( menu );
	}
}

void WndVisual::DoRender()
{
	InvalidateRect( m_hWnd, NULL /*rect*/, FALSE /*erase*/ );
}

void WndVisual::OnOscilloscopeColour()
{
	COLORREF colour = m_Settings.GetOscilloscopeColour();
	CHOOSECOLOR chooseColor = {};
	chooseColor.lStructSize = sizeof( CHOOSECOLOR );
	chooseColor.hwndOwner = m_hWnd;
	chooseColor.rgbResult = colour;
	VUPlayer* vuplayer = VUPlayer::Get();
	if ( nullptr != vuplayer ) {
		chooseColor.lpCustColors = vuplayer->GetCustomColours();
	}
	chooseColor.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	if ( TRUE == ChooseColor( &chooseColor ) && ( colour != chooseColor.rgbResult ) ) {
		colour = chooseColor.rgbResult;
		m_Settings.SetOscilloscopeColour( colour );
		auto visual = m_Visuals.find( ID_VISUAL_OSCILLOSCOPE );
		if ( m_Visuals.end() != visual ) {
			visual->second->OnSettingsChanged();
		}
	}
}

void WndVisual::OnOscilloscopeBackground()
{
	COLORREF colour = m_Settings.GetOscilloscopeBackground();
	CHOOSECOLOR chooseColor = {};
	chooseColor.lStructSize = sizeof( CHOOSECOLOR );
	chooseColor.hwndOwner = m_hWnd;
	chooseColor.rgbResult = colour;
	VUPlayer* vuplayer = VUPlayer::Get();
	if ( nullptr != vuplayer ) {
		chooseColor.lpCustColors = vuplayer->GetCustomColours();
	}
	chooseColor.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	if ( TRUE == ChooseColor( &chooseColor ) && ( colour != chooseColor.rgbResult ) ) {
		colour = chooseColor.rgbResult;
		m_Settings.SetOscilloscopeBackground( colour );
		auto visual = m_Visuals.find( ID_VISUAL_OSCILLOSCOPE );
		if ( m_Visuals.end() != visual ) {
			visual->second->OnSettingsChanged();
		}
	}
}

void WndVisual::OnOscilloscopeWeight( const UINT commandID )
{
	const auto weightIter = s_OscilloscopeWeights.find( commandID );
	const float weight = ( s_OscilloscopeWeights.end() == weightIter ) ? s_OscilloscopeWeights.at( ID_OSCILLOSCOPE_WEIGHT_NORMAL ) : weightIter->second;
	const float currentWeight = m_Settings.GetOscilloscopeWeight();
	if ( weight != currentWeight ) {
		m_Settings.SetOscilloscopeWeight( weight );
		auto visual = m_Visuals.find( ID_VISUAL_OSCILLOSCOPE );
		if ( m_Visuals.end() != visual ) {
			visual->second->OnSettingsChanged();
		}
	}
}

void WndVisual::OnSpectrumAnalyserColour( const UINT commandID )
{
	COLORREF base = 0;
	COLORREF peak = 0;
	COLORREF background = 0;
	m_Settings.GetSpectrumAnalyserSettings( base, peak, background );
	const COLORREF initialColour = ( ID_SPECTRUMANALYSER_BASECOLOUR == commandID ) ? base : ( ( ID_SPECTRUMANALYSER_PEAKCOLOUR == commandID ) ? peak : background );
	CHOOSECOLOR chooseColor = {};
	chooseColor.lStructSize = sizeof( CHOOSECOLOR );
	chooseColor.hwndOwner = m_hWnd;
	chooseColor.rgbResult = initialColour;
	VUPlayer* vuplayer = VUPlayer::Get();
	if ( nullptr != vuplayer ) {
		chooseColor.lpCustColors = vuplayer->GetCustomColours();
	}
	chooseColor.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	if ( TRUE == ChooseColor( &chooseColor ) && ( initialColour != chooseColor.rgbResult ) ) {
		if ( ID_SPECTRUMANALYSER_BASECOLOUR == commandID ) {
			base = chooseColor.rgbResult;
		} else if ( ID_SPECTRUMANALYSER_PEAKCOLOUR == commandID ) {
			peak = chooseColor.rgbResult;
		} else {
			background = chooseColor.rgbResult;
		}
		m_Settings.SetSpectrumAnalyserSettings( base, peak, background );
		auto visual = m_Visuals.find( ID_VISUAL_SPECTRUMANALYSER );
		if ( m_Visuals.end() != visual ) {
			visual->second->OnSettingsChanged();
		}
	}
}

void WndVisual::OnPeakMeterColour( const UINT commandID )
{
	COLORREF base = 0;
	COLORREF peak = 0;
	COLORREF background = 0;
	m_Settings.GetPeakMeterSettings( base, peak, background );
	const COLORREF initialColour = ( ID_PEAKMETER_BASECOLOUR == commandID ) ? base : ( ( ID_PEAKMETER_PEAKCOLOUR == commandID ) ? peak : background );
	CHOOSECOLOR chooseColor = {};
	chooseColor.lStructSize = sizeof( CHOOSECOLOR );
	chooseColor.hwndOwner = m_hWnd;
	chooseColor.rgbResult = initialColour;
	VUPlayer* vuplayer = VUPlayer::Get();
	if ( nullptr != vuplayer ) {
		chooseColor.lpCustColors = vuplayer->GetCustomColours();
	}
	chooseColor.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	if ( TRUE == ChooseColor( &chooseColor ) && ( initialColour != chooseColor.rgbResult ) ) {
		if ( ID_PEAKMETER_BASECOLOUR == commandID ) {
			base = chooseColor.rgbResult;
		} else if ( ID_PEAKMETER_PEAKCOLOUR == commandID ) {
			peak = chooseColor.rgbResult;
		} else {
			background = chooseColor.rgbResult;
		}
		m_Settings.SetPeakMeterSettings( base, peak, background );
		auto visual = m_Visuals.find( ID_VISUAL_PEAKMETER );
		if ( m_Visuals.end() != visual ) {
			visual->second->OnSettingsChanged();
		}
	}
}

void WndVisual::OnVUMeterDecay( const UINT commandID )
{
	const auto decayIter = s_VUMeterDecay.find( commandID );
	const float decay = ( s_VUMeterDecay.end() == decayIter ) ? VUMeterDecayNormal : decayIter->second;
	const float currentDecay = m_Settings.GetVUMeterDecay();
	if ( decay != currentDecay ) {
		m_Settings.SetVUMeterDecay( decay );
		auto visual = m_Visuals.find( ID_VISUAL_VUMETER_STEREO );
		if ( m_Visuals.end() != visual ) {
			visual->second->OnSettingsChanged();
		}
		visual = m_Visuals.find( ID_VISUAL_VUMETER_MONO );
		if ( m_Visuals.end() != visual ) {
			visual->second->OnSettingsChanged();
		}
	}
}

std::map<UINT,float> WndVisual::GetOscilloscopeWeights() const
{
	return s_OscilloscopeWeights;
}

std::map<UINT,float> WndVisual::GetVUMeterDecayRates() const
{
	return s_VUMeterDecay;
}
