#include "stdafx.h"

#include "Utility.h"
#include "VUPlayer.h"

#include <fstream>
#include <sstream>

#define MAX_LOADSTRING 100

using namespace Gdiplus;

// Global Variables
HINSTANCE g_hInst = 0;
HWND g_hWnd = 0;
WCHAR g_szTitle[ MAX_LOADSTRING ] = {};
WCHAR g_szWindowClass[ MAX_LOADSTRING ] = {};

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass( HINSTANCE hInstance );
BOOL InitInstance( HINSTANCE, int );
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK About( HWND, UINT, WPARAM, LPARAM );

// ID with which to tag WM_COPYDATA messages when passing filenames to the application via the command line.
// Copied data should be an array of null terminated WCHAR strings, ending with an additional null terminator.
static const UINT VUPLAYER_COPYDATA = 0x1974;

// Command line switch to run in 'portable' mode (i.e. no persistent database).
static const TCHAR s_portableCmdLineSwitch[] = L"-portable";

// Command line switch to set the database access mode.
static const TCHAR s_databasemodeCmdLineSwitch[] = L"-mode";

// Entry point
int APIENTRY wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	// Initialize global strings
	LoadStringW( hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING );
	LoadStringW( hInstance, IDC_VUPLAYER, g_szWindowClass, MAX_LOADSTRING );

	// Parse command line
	std::list<std::wstring> cmdLineFiles;
	bool portable = false;
	std::string portableSettings;
	Database::Mode mode = Database::Mode::Temp;
	int numArgs = 0;
	LPWSTR* args = CommandLineToArgvW( GetCommandLine(), &numArgs );
	if ( nullptr != args ) {
		for ( int argc = 1; argc < numArgs; argc++ ) {
			if ( 0 == _wcsicmp( args[ argc ], s_portableCmdLineSwitch ) ) {
				// Handle the '-portable' command-line switch (and the following settings file argument).
				portable = true;
				if ( ( argc + 1 ) < numArgs ) {
					const std::wstring settingsFileName = args[ argc + 1 ];
					if ( settingsFileName.size() > 4 ) {
						const std::wstring ext = WideStringToUpper( settingsFileName.substr( settingsFileName.size() - 4 ) );
						if ( L".INI" == ext ) {
							try {
								std::ifstream filestream;
								std::ostringstream stringstream;
								filestream.open( settingsFileName );
								if ( filestream.is_open() ) {
									stringstream << filestream.rdbuf();
									portableSettings = stringstream.str();
									filestream.close();
								}
							} catch ( ... ) {
							}
						}
					}
				}
				if ( !portableSettings.empty() ) {
					++argc;
				}
			} else if ( 0 == _wcsicmp( args[ argc ], s_databasemodeCmdLineSwitch ) ) {
				// Handle the '-mode' command-line switch (and the following database access mode argument).
				if ( ( argc + 1 ) < numArgs ) {
					try {
						const int value = std::stoi( args[ argc + 1 ] );
						if ( ( value >= static_cast<int>( Database::Mode::Disk ) ) && ( value <= static_cast<int>( Database::Mode::Memory ) ) ) {
							mode = static_cast<Database::Mode>( value );
							++argc;
						}
					} catch ( ... ) {
					}
				}
			} else {
				const DWORD attributes = GetFileAttributes( args[ argc ] );
				if ( ( INVALID_FILE_ATTRIBUTES != attributes ) && !( FILE_ATTRIBUTE_DIRECTORY & attributes ) ) {
					cmdLineFiles.push_back( args[ argc ] );
				}
			}
		}
		LocalFree( args );
	}

	// Limit application to a single instance
	const HANDLE hMutex = CreateMutex( NULL /*attributes*/, FALSE /*initialOwner*/, g_szWindowClass );
	if ( ( NULL != hMutex ) && ( ERROR_ALREADY_EXISTS == GetLastError() ) ) {
		CloseHandle( hMutex );
		const HWND existingWnd = FindWindow( g_szWindowClass, NULL /*windowName*/ );
		if ( NULL != existingWnd ) {
			if ( IsIconic( existingWnd ) ) {
				ShowWindow( existingWnd, SW_RESTORE );
			}
			SetForegroundWindow( existingWnd );

			if ( !cmdLineFiles.empty() ) {
				// Send command line filenames to the application.
				COPYDATASTRUCT copyData = {};
				copyData.dwData = VUPLAYER_COPYDATA;
				for ( const auto& filename : cmdLineFiles ) {
					copyData.cbData += static_cast<DWORD>( 1 + filename.size() );
				}
				copyData.cbData += 1;
				WCHAR* buffer = new WCHAR[ copyData.cbData ]();
				copyData.cbData *= sizeof( WCHAR );
				copyData.lpData = buffer;
				WCHAR* destFilename = buffer;
				for ( const auto& srcFilename : cmdLineFiles ) {
					wcscpy_s( destFilename, 1 + srcFilename.size(), srcFilename.c_str() );
					destFilename += ( 1 + srcFilename.size() );
				}
				SendMessage( existingWnd, WM_COPYDATA, 0 /*wParam*/, reinterpret_cast<LPARAM>( &copyData ) );
				delete [] buffer;
			}
		}
		return FALSE;
	}

	// Perform application initialization
	MyRegisterClass( hInstance );
	if ( !InitInstance( hInstance, nCmdShow ) )	{
		return FALSE;
	}

	CoInitializeEx( NULL /*reserved*/, COINIT_APARTMENTTHREADED );
	INITCOMMONCONTROLSEX initCC = {};
	initCC.dwSize = sizeof( INITCOMMONCONTROLSEX );
	initCC.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES;
	InitCommonControlsEx( &initCC );

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

	VUPlayer* vuplayer = new VUPlayer( g_hInst, g_hWnd, cmdLineFiles, portable, portableSettings, mode );

	SetWindowLongPtr( g_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( vuplayer ) );
	const HACCEL hAccelTable = vuplayer ? vuplayer->GetAcceleratorTable() : nullptr;
	MSG msg;
	// Main message loop
	while ( GetMessage( &msg, nullptr, 0, 0 ) ) {
		if ( !TranslateAccelerator( g_hWnd, hAccelTable, &msg ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	delete vuplayer;

	GdiplusShutdown( gdiplusToken );

	sqlite3_shutdown();
	CoUninitialize();

	CloseHandle( hMutex );

	return (int)msg.wParam;
}

//Registers the window class
ATOM MyRegisterClass( HINSTANCE hInstance )
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof( WNDCLASSEX );

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_VUPLAYER ) );
    wcex.hCursor        = LoadCursor( nullptr, IDC_ARROW );
    wcex.lpszMenuName   = MAKEINTRESOURCEW( IDC_VUPLAYER );
    wcex.lpszClassName  = g_szWindowClass;
		wcex.hbrBackground	= reinterpret_cast<HBRUSH>( COLOR_3DFACE + 1 );

		LoadIconMetric( hInstance, MAKEINTRESOURCE( IDI_VUPLAYER ), LIM_SMALL, &wcex.hIconSm );

    return RegisterClassExW( &wcex );
}

// Saves instance handle and creates main window
BOOL InitInstance( HINSTANCE hInstance, int /*nCmdShow*/ )
{
	g_hInst = hInstance; // Store instance handle in our global variable

	const DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
	const int x = 0;
	const int y = 0;
	const int width = static_cast<int>( 900 * GetDPIScaling() );
	const int height = static_cast<int>( 600 * GetDPIScaling() );
	g_hWnd = CreateWindowW( g_szWindowClass, g_szTitle, style, x, y, width, height, nullptr, nullptr, hInstance, nullptr );
	const BOOL success = ( NULL != g_hWnd );

	return success;
}

// Message handler for main window
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	VUPlayer* vuplayer = reinterpret_cast<VUPlayer*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
	switch ( message )
	{
		case WM_COMMAND : {
			const int wmId = LOWORD( wParam );
			// Parse the menu selections:
			switch ( wmId ) {
				case IDM_ABOUT : {
					DialogBoxParam( g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About, reinterpret_cast<LPARAM>( vuplayer ) );
					break;
				}
				case IDM_EXIT : {
					DestroyWindow( hWnd );
					break;
				}
				default: {
					if ( nullptr != vuplayer ) {
						vuplayer->OnCommand( wmId );
					}
					break;
				}
			}
			break;
		}
		case WM_PAINT : {
			PAINTSTRUCT ps;
			BeginPaint( hWnd, &ps );
			EndPaint( hWnd, &ps );
			break;
		}
		case WM_DESTROY : {
			if ( nullptr != vuplayer ) {
				vuplayer->OnDestroy();
			}
			PostQuitMessage( 0 );
			break;
		}
		case WM_SIZE : {
			if ( nullptr != vuplayer ) {
				vuplayer->OnSize( wParam, lParam );
			}
			break;
		}
		case WM_NOTIFY : {
			if ( nullptr != vuplayer ) {
				LRESULT result = 0;
				if ( vuplayer->OnNotify( wParam, lParam, result ) ) {
					return result;
				}
			}
			break;
		}
		case WM_TIMER : {
			if ( ( nullptr != vuplayer ) && vuplayer->OnTimer( wParam ) ) {
				return 0;
			}
			break;
		}
		case WM_GETMINMAXINFO : {
			if ( nullptr != vuplayer ) {
				LPMINMAXINFO minMaxInfo = reinterpret_cast<LPMINMAXINFO>( lParam );
				vuplayer->OnMinMaxInfo( minMaxInfo );
			}
			break;
		}
		case WM_INITMENU : {
			if ( nullptr != vuplayer ) {
				const HMENU menu = reinterpret_cast<HMENU>( wParam );
				vuplayer->OnInitMenu( menu );
			}
			break;
		}
		case WM_HOTKEY : {
			if ( nullptr != vuplayer ) {
				vuplayer->OnHotkey( wParam );
			}
			break;
		}
		case WM_DEVICECHANGE : {
			if ( nullptr != vuplayer ) {
				vuplayer->OnDeviceChange( wParam, lParam );
			}
			break;
		}
		case WM_COPYDATA : {
			if ( nullptr != vuplayer ) {
				COPYDATASTRUCT* copyData = reinterpret_cast<COPYDATASTRUCT*>( lParam );
				if ( ( nullptr != copyData ) && ( VUPLAYER_COPYDATA == copyData->dwData ) && ( 0 != copyData->cbData ) && ( nullptr != copyData->lpData ) ) {
					// Ensure that the string array ends with a double null-terminator.
					const WCHAR* srcBuffer = reinterpret_cast<WCHAR*>( copyData->lpData );
					const int srcBufferSize = copyData->cbData / sizeof( WCHAR );
					WCHAR* destBuffer = new WCHAR[ 2 + srcBufferSize ]();
					wmemcpy_s( destBuffer, srcBufferSize, srcBuffer, srcBufferSize );
					std::list<std::wstring> filenames;
					WCHAR* filename = destBuffer;
					while ( 0 != *filename ) {
						const DWORD attributes = GetFileAttributes( filename );
						if ( ( INVALID_FILE_ATTRIBUTES != attributes ) && !( FILE_ATTRIBUTE_DIRECTORY & attributes ) ) {
							filenames.push_back( filename );
						}
						filename += ( 1 + wcslen( filename ) );
					}
					delete [] destBuffer;
					vuplayer->OnCommandLineFiles( filenames );
				}
			}
			break;
		}
		case MSG_RESTARTPLAYBACK : {
			if ( nullptr != vuplayer ) {
				const long itemID = static_cast<long>( wParam );
				vuplayer->OnRestartPlayback( itemID );
			}
			break;
		}
		case MSG_MEDIAUPDATED : {
			if ( nullptr != vuplayer ) {
				const MediaInfo* previousMediaInfo = reinterpret_cast<const MediaInfo*>( wParam );
				const MediaInfo* updatedMediaInfo = reinterpret_cast<const MediaInfo*>( lParam );
				vuplayer->OnHandleMediaUpdate( previousMediaInfo, updatedMediaInfo );
				delete previousMediaInfo;
				previousMediaInfo = nullptr;
				delete updatedMediaInfo;
				updatedMediaInfo = nullptr;
			}
			break;
		}
		case MSG_LIBRARYREFRESHED : {
			if ( nullptr != vuplayer ) {
				vuplayer->OnHandleLibraryRefreshed();
			}
			break;
		}
		case MSG_CDDAREFRESHED : {
			if ( nullptr != vuplayer ) {
				vuplayer->OnHandleCDDARefreshed();
			}
			break;
		}
		case MSG_GRACENOTEQUERYRESULT : {
			if ( nullptr != vuplayer ) {
				const Gracenote::Result* result = reinterpret_cast<const Gracenote::Result*>( wParam );
				if ( nullptr != result ) {
					vuplayer->OnGracenoteResult( *result, lParam );
					delete result;
					result = nullptr;
				}
			}
			break;
		}
		case MSG_TRAYNOTIFY : {
			if ( nullptr != vuplayer ) {
				vuplayer->OnTrayNotify( wParam, lParam );
			}
			break;
		}
		default : {
			break;
		}
	}
	return DefWindowProc( hWnd, message, wParam, lParam );
}

// Message handler for about box
INT_PTR CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER( lParam );
	switch ( message ) {
		case WM_INITDIALOG: {
			CentreDialog( hDlg );
			VUPlayer* vuplayer = reinterpret_cast<VUPlayer*>( lParam );
			if ( nullptr != vuplayer ) {
				const std::wstring bassVersion = vuplayer->GetBassVersion();
				SetDlgItemText( hDlg, IDC_ABOUT_BASSVERSION, bassVersion.c_str() );

				const HWND wndLogo = GetDlgItem( hDlg, IDC_ABOUT_GRACENOTE );
				if ( nullptr != wndLogo ) {
					RECT clientRect = {};
					GetClientRect( wndLogo, &clientRect );
					const HBITMAP hBitmap = vuplayer->GetGracenoteLogo( clientRect );
					if ( nullptr != hBitmap ) {
						SendDlgItemMessage( hDlg, IDC_ABOUT_GRACENOTE, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>( hBitmap ) );
						DeleteObject( hBitmap );
					}
				}
			}
			return (INT_PTR)TRUE;
		}
		case WM_NOTIFY : {
			PNMLINK nmlink = reinterpret_cast<PNMLINK>( lParam );
			if ( nullptr != nmlink ) {
				if ( NM_CLICK == nmlink->hdr.code ) {
					switch ( nmlink->hdr.idFrom ) {
						case IDC_SYSLINK_VUPLAYER : {
							ShellExecute( NULL, L"open", L"http://www.vuplayer.com", NULL, NULL, SW_SHOWNORMAL );
							break;
						}
						case IDC_SYSLINK_BASS : {
							ShellExecute( NULL, L"open", L"http://www.un4seen.com", NULL, NULL, SW_SHOWNORMAL );
							break;
						}
						default : {
							break;
						}
					}
				}
			}
			break;
		}
		case WM_COMMAND: {
			if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) {
				EndDialog( hDlg, LOWORD( wParam ) );
				return (INT_PTR)TRUE;
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}
