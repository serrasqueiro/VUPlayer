#include "WndToolbarOptions.h"

#include "resource.h"
#include "Utility.h"

// Toolbar button size.
static const int s_ButtonSize = 24;

WndToolbarOptions::WndToolbarOptions( HINSTANCE instance, HWND parent ) :
	WndToolbar( instance, parent ),
	m_ImageList( nullptr )
{
	CreateButtons();

	RECT rect = {};
	SendMessage( GetWindowHandle(), TB_GETITEMRECT, 0, reinterpret_cast<LPARAM>( &rect ) );
	const int buttonCount = static_cast<int>( SendMessage( GetWindowHandle(), TB_BUTTONCOUNT, 0, 0 ) );
	MoveWindow( GetWindowHandle(), 0 /*x*/, 0 /*y*/, ( rect.right - rect.left ) * buttonCount, rect.bottom - rect.top, TRUE /*repaint*/ );
}

WndToolbarOptions::~WndToolbarOptions()
{
	ImageList_Destroy( m_ImageList );
}

void WndToolbarOptions::Update( Output& /*output*/, const Playlist::Ptr& /*playlist*/, const Playlist::Item& /*selectedItem*/ )
{
	SetButtonEnabled( ID_FILE_OPTIONS, true );
}

void WndToolbarOptions::CreateImageList()
{
	const float dpiScale = GetDPIScaling();
	const int cx = static_cast<int>( s_ButtonSize * dpiScale );
	const int cy = static_cast<int>( s_ButtonSize * dpiScale );
	const int imageCount = 1;
	m_ImageList = ImageList_Create( cx, cy, ILC_COLOR32, 0 /*initial*/, imageCount /*grow*/ );
	HICON hIcon = static_cast<HICON>( LoadImage( GetInstanceHandle(), MAKEINTRESOURCE( IDI_OPTIONS ), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR | LR_SHARED ) );
	if ( NULL != hIcon ) {
		ImageList_ReplaceIcon( m_ImageList, -1, hIcon );
	}
}

void WndToolbarOptions::CreateButtons()
{
	CreateImageList();

	SendMessage( GetWindowHandle(), TB_BUTTONSTRUCTSIZE, sizeof( TBBUTTON ), 0 );
	SendMessage( GetWindowHandle(), TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>( m_ImageList ) );
	const int buttonCount = 1;
	TBBUTTON buttons[ buttonCount ] = {};
	buttons[ 0 ].fsStyle = TBSTYLE_BUTTON;
	buttons[ 0 ].iBitmap = 0;
	buttons[ 0 ].idCommand = ID_FILE_OPTIONS;
	SendMessage( GetWindowHandle(), TB_ADDBUTTONS, buttonCount, reinterpret_cast<LPARAM>( buttons ) );
}

UINT WndToolbarOptions::GetTooltip( const UINT /*commandID*/ ) const
{
	const UINT tooltip = IDS_OPTIONS_TITLE;
	return tooltip;
}