#include "WndRebar.h"
#include <commctrl.h>
#include <windowsx.h>

#include "Utility.h"

// Rebar control ID
static const UINT_PTR s_WndRebarID = 1200;

// Rebar band ID
UINT WndRebar::s_BandID = 13;

// Rebar image size
static const int s_ImageSize = 24;

// Window procedure
static LRESULT CALLBACK WndRebarProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	WndRebar* wndRebar = reinterpret_cast<WndRebar*>( GetWindowLongPtr( hwnd, GWLP_USERDATA ) );
	if ( nullptr != wndRebar ) {
		switch ( message ) {
			case WM_HSCROLL : {
				SendMessage( reinterpret_cast<HWND>( lParam ) /*trackbarWnd*/, message, wParam, lParam );
				break;
			}
			case WM_LBUTTONDOWN : {
				const POINT pt = { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };
				RBHITTESTINFO hitTestInfo = {};
				hitTestInfo.pt = pt;
				const int bandIndex = static_cast<int>( SendMessage( hwnd, RB_HITTEST, 0, reinterpret_cast<LPARAM>( &hitTestInfo ) ) );
				if ( ( bandIndex >= 0 ) && ( RBHT_CAPTION == hitTestInfo.flags ) ) {
					REBARBANDINFO bandInfo = {};
					bandInfo.cbSize = sizeof( REBARBANDINFO );
					bandInfo.fMask = RBBIM_ID;
					SendMessage( hwnd, RB_GETBANDINFO, bandIndex, reinterpret_cast<LPARAM>( &bandInfo ) );
					wndRebar->OnClickCaption( bandInfo.wID );
				}
				break;
			}
		}
	}
	return CallWindowProc( wndRebar->GetDefaultWndProc(), hwnd, message, wParam, lParam );
}

WndRebar::WndRebar( HINSTANCE instance, HWND parent ) :
	m_hInst( instance ),
	m_hWnd( NULL ),
	m_DefaultWndProc( NULL ),
	m_ImageList( NULL ),
	m_IconBands(),
	m_ImageListMap(),
	m_IconCallbackMap(),
	m_ClickCallbackMap()
{
	const DWORD style = WS_CHILD | WS_VISIBLE | /*WS_CLIPCHILDREN |*/ CCS_NODIVIDER | RBS_FIXEDORDER | RBS_BANDBORDERS;
	const int x = 0;
	const int y = 0;
	const int width = 0;
	const int height = 30;
	LPVOID param = NULL;
	m_hWnd = CreateWindowEx( WS_EX_TOOLWINDOW, REBARCLASSNAME, 0, style, x, y, width, height, parent, reinterpret_cast<HMENU>( s_WndRebarID ), instance, param );
	SetWindowLongPtr( m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( this ) );
	m_DefaultWndProc = reinterpret_cast<WNDPROC>( SetWindowLongPtr( m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( WndRebarProc ) ) );

	CreateImageList();
}

WndRebar::~WndRebar()
{
	SetWindowLongPtr( m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( m_DefaultWndProc ) );
	ImageList_Destroy( m_ImageList );
}

WNDPROC WndRebar::GetDefaultWndProc()
{
	return m_DefaultWndProc;
}

HWND WndRebar::GetWindowHandle()
{
	return m_hWnd;
}

void WndRebar::AddControl( HWND hwnd )
{
	REBARBANDINFO info = {};
	info.cbSize = sizeof( REBARBANDINFO );
	info.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDSIZE;
	info.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_HEADERSIZE;
	info.wID = s_BandID++;
	info.cxHeader = 1;
	RECT rect = {};
	GetWindowRect( hwnd, &rect );
	info.hwndChild = hwnd;
	info.cx = rect.right - rect.left;
	info.cxMinChild = info.cx;
	info.cyMinChild = rect.bottom - rect.top;
	SendMessage( m_hWnd, RB_INSERTBAND, static_cast<WPARAM>( -1 ), reinterpret_cast<LPARAM>( &info ) );
}

void WndRebar::AddControl( HWND hwnd, const std::set<UINT>& icons, IconCallback iconCallback, ClickCallback clickCallback )
{
	REBARBANDINFO info = {};
	info.cbSize = sizeof( REBARBANDINFO );
	info.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDSIZE;
	info.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_IMAGE;
	info.wID = s_BandID++;

	const float dpiScale = GetDPIScaling();
	const int cx = static_cast<int>( s_ImageSize * dpiScale );
	const int cy = static_cast<int>( s_ImageSize * dpiScale );

	for ( const auto& iconID : icons ) {
		HICON hIcon = static_cast<HICON>( LoadImage( m_hInst, MAKEINTRESOURCE( iconID ), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR | LR_SHARED ) );
		if ( NULL != hIcon ) {
			const int imageListIndex = ImageList_ReplaceIcon( m_ImageList, -1, hIcon );
			m_ImageListMap.insert( ImageListMap::value_type( iconID, imageListIndex ) );
		}	
	}
	m_IconCallbackMap.insert( IconCallbackMap::value_type( info.wID, iconCallback ) );
	m_ClickCallbackMap.insert( ClickCallbackMap::value_type( info.wID, clickCallback ) );
	info.iImage = GetImageListIndex( info.wID );
	m_IconBands.insert( info.wID );

	RECT rect = {};
	GetWindowRect( hwnd, &rect );
	info.hwndChild = hwnd;
	info.cx = rect.right - rect.left;
	info.cxMinChild = info.cx;
	info.cyMinChild = rect.bottom - rect.top;
	SendMessage( m_hWnd, RB_INSERTBAND, static_cast<WPARAM>( -1 ), reinterpret_cast<LPARAM>( &info ) );
}

int WndRebar::GetPosition( const HWND hwnd ) const
{
	REBARBANDINFO info = {};
	info.cbSize = sizeof( REBARBANDINFO );
	info.fMask = RBBIM_CHILD;
	int position = -1;
	const int count = static_cast<int>( SendMessage( m_hWnd, RB_GETBANDCOUNT, 0, 0 ) );
	for ( int index = 0; index < count; index++ ) {
		SendMessage( m_hWnd, RB_GETBANDINFO, index, reinterpret_cast<LPARAM>( &info ) );
		if ( info.hwndChild == hwnd ) {
			position = static_cast<int>( index );
			break;
		}
	}
	return position;
}

void WndRebar::MoveToStart( const int currentPosition )
{
	SendMessage( m_hWnd, RB_MOVEBAND, currentPosition, 0 /*newPosition*/ );
}

void WndRebar::CreateImageList()
{
	const float dpiScale = GetDPIScaling();
	const int cx = static_cast<int>( s_ImageSize * dpiScale );
	const int cy = static_cast<int>( s_ImageSize * dpiScale );
	m_ImageList = ImageList_Create( cx, cy, ILC_COLOR32, 0 /*initial*/, 1 /*grow*/ );

	REBARINFO rebarInfo = {};
	rebarInfo.cbSize = sizeof( REBARINFO );
	rebarInfo.fMask = RBIM_IMAGELIST;
	rebarInfo.himl = m_ImageList;
	SendMessage( m_hWnd, RB_SETBARINFO, 0, reinterpret_cast<LPARAM>( &rebarInfo ) );
}

int WndRebar::GetImageListIndex( const UINT bandID ) const
{
	int imageListIndex = -1;
	const auto callbackIter = m_IconCallbackMap.find( bandID );
	if ( m_IconCallbackMap.end() != callbackIter ) {
		IconCallback callback = callbackIter->second;
		if ( nullptr != callback ) {
			const UINT iconID = callback();
			const auto imageListIter = m_ImageListMap.find( iconID );
			if ( m_ImageListMap.end() != imageListIter ) {
				imageListIndex = imageListIter->second;
			}
		}
	}
	return imageListIndex;
}

void WndRebar::Update()
{
	// Update any icons.
	for ( const auto& bandID : m_IconBands ) {
		const int bandIndex = static_cast<int>( SendMessage( m_hWnd, RB_IDTOINDEX, bandID, 0 ) );
		REBARBANDINFO info = {};
		info.cbSize = sizeof( REBARBANDINFO );
		info.fMask = RBBIM_IMAGE;
		SendMessage( m_hWnd, RB_GETBANDINFO, bandIndex, reinterpret_cast<LPARAM>( &info ) );
		const int imageListIndex = GetImageListIndex( bandID );
		if ( imageListIndex != info.iImage ) {
			info.iImage = imageListIndex;
			SendMessage( m_hWnd, RB_SETBANDINFO, bandIndex, reinterpret_cast<LPARAM>( &info ) );
		}
	}
}

void WndRebar::OnClickCaption( const UINT bandID )
{
	const auto callbackIter = m_ClickCallbackMap.find( bandID );
	if ( m_ClickCallbackMap.end() != callbackIter ) {
		ClickCallback callback = callbackIter->second;
		if ( nullptr != callback ) {
			callback();
		}
	}
}
