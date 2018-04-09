#pragma once

#include "stdafx.h"

#include "resource.h"

#include "Database.h"
#include "Hotkeys.h"
#include "Library.h"
#include "LibraryMaintainer.h"
#include "Output.h"
#include "ReplayGain.h"
#include "Settings.h"

#include "WndCounter.h"
#include "WndList.h"
#include "WndRebar.h"
#include "WndSplit.h"
#include "WndStatus.h"
#include "WndToolbarCrossfade.h"
#include "WndToolbarFile.h"
#include "WndToolbarFlow.h"
#include "WndToolbarInfo.h"
#include "WndToolbarOptions.h"
#include "WndToolbarPlayback.h"
#include "WndToolbarPlaylist.h"
#include "WndTrackbarSeek.h"
#include "WndTrackbarVolume.h"
#include "WndTray.h"
#include "WndTree.h"
#include "WndVisual.h"

// Message ID for signalling that media information has been updated.
// 'wParam' : pointer to previous MediaInfo, to be deleted by the message handler.
// 'lParam' : pointer to updated MediaInfo, to be deleted by the message handler.
static const UINT MSG_MEDIAUPDATED = WM_APP + 77;

// Message ID for signalling that the media library has been refreshed.
// 'wParam' : unused.
// 'lParam' : unused.
static const UINT MSG_LIBRARYREFRESHED = WM_APP + 78;

// Main application singleton
class VUPlayer
{
public:
	// Gets the main application instance.
	static VUPlayer* Get( const HINSTANCE instance = NULL, const HWND hwnd = NULL );

	// Releases the main application instance.
	static void Release();

	// Returns (and creates if necessary) the VUPlayer documents folder (with a trailing slash).
	static std::wstring DocumentsFolder();

	// Called when the application window is resized.
	void OnSize( WPARAM wParam, LPARAM lParam );

	// Called when a notification message is received.
	// Returns true if the notification message is handled, in which case the 'result' parameter is valid.
	bool OnNotify( WPARAM wParam, LPARAM lParam, LRESULT& result );

	// Called when a timer message is received.
	// 'timerID' - timer identifier.
	// Returns true if the timer message was handled.
	bool OnTimer( const UINT_PTR timerID );

	// Called when the application window is about to be resized.
	void OnMinMaxInfo( LPMINMAXINFO minMaxInfo );

	// Called when the application window is about to be destroyed.
	void OnDestroy();

	// Called when the application window receives a command.
	void OnCommand( const int commandID );

	// Called when a 'menu' is about to be shown.
	void OnInitMenu( const HMENU menu );

	// Called when an 'item' is added to the 'playlist' at a (0-based) 'position'.
	void OnPlaylistItemAdded( Playlist* playlist, const Playlist::Item& item, const int position );

	// Called when an 'item' is removed from the 'playlist'.
	void OnPlaylistItemRemoved( Playlist* playlist, const Playlist::Item& item );

	// Called when information in the media database is updated.
	// 'previousMediaInfo' - the previous media information.
	// 'updatedMediaInfo' - the updated media information.
	void OnMediaUpdated( const MediaInfo& previousMediaInfo, const MediaInfo& updatedMediaInfo );

	// Called when the media library has been refreshed.
	void OnLibraryRefreshed();

	// Handles the update of 'previousMediaInfo' to 'updatedMediaInfo', from the main thread.
	void OnHandleMediaUpdate( const MediaInfo* previousMediaInfo, const MediaInfo* updatedMediaInfo );

	// Handles a media library refresh, from the main thread.
	void OnHandleLibraryRefreshed();

	// Restarts playback from a playlist 'itemID'.
	void OnRestartPlayback( const long itemID );

	// Shows the track information dialog.
	void OnTrackInformation();

	// Returns the custom colours to use for colour selection dialogs.
	COLORREF* GetCustomColours();

	// Returns the placeholder image.
	std::shared_ptr<Gdiplus::Bitmap> GetPlaceholderImage();

	// Returns the application settings.
	Settings& GetApplicationSettings();

	// Called when a notification area icon message is received.
	void OnTrayNotify( WPARAM wParam, LPARAM lParam );

	// Called when a hotkey message is received.
	void OnHotkey( WPARAM wParam );

	// Called when the Calculate ReplayGain command is received.
	void OnCalculateReplayGain();

	// Creates and returns a new playlist.
	Playlist::Ptr NewPlaylist();

	// Returns the BASS library version.
	std::wstring GetBassVersion() const;

private:
	// 'instance' - module instance handle.
	// 'hwnd' - main window handle.
	VUPlayer( const HINSTANCE instance, const HWND hwnd );

	// Reads and applies windows position from settings.
	void ReadWindowSettings();

	// Writes windows position to settings.
	void WriteWindowSettings();

	// Handles a change in the current output 'item'.
	void OnOutputChanged( const Output::Item& item );

	// Called when the selection changes on the list control.
	void OnListSelectionChanged();

	// Displays the options dialog.
	void OnOptions();

	// Returns whether to allow a skip backwards or forwards.
	bool AllowSkip() const;

	// Resets the last skip count to the current performance count.
	void ResetLastSkipCount();

	virtual ~VUPlayer();

	// Main application instance.
	static VUPlayer* s_VUPlayer;

	// Module instance handle.
	HINSTANCE m_hInst;

	// Application window handle.
	HWND m_hWnd;

	// Audio format handlers.
	Handlers m_Handlers;

	// Database.
	Database m_Database;

	// Media library.
	Library m_Library;

	// Media library maintainer.
	LibraryMaintainer m_Maintainer;

	// Application settings.
	Settings m_Settings;

	// Output.
	Output m_Output;

	// ReplayGain calculator.
	ReplayGain m_ReplayGain;

	// Rebar control.
	WndRebar m_Rebar;

	// Status bar control.
	WndStatus m_Status;

	// Tree control.
	WndTree m_Tree;

	// Visual control.
	WndVisual m_Visual;

	// List control.
	WndList m_List;

	// Seek control.
	WndTrackbarSeek m_SeekControl;

	// Volume control.
	WndTrackbarVolume m_VolumeControl;

	// Toolbar (crossfade).
	WndToolbarCrossfade m_ToolbarCrossfade;

	// Toolbar (file).
	WndToolbarFile m_ToolbarFile;

	// Toolbar (random/repeat).
	WndToolbarFlow m_ToolbarFlow;

	// Toolbar (track information).
	WndToolbarInfo m_ToolbarInfo;

	// Toolbar (options).
	WndToolbarOptions m_ToolbarOptions;

	// Toolbar (playback).
	WndToolbarPlayback m_ToolbarPlayback;

	// Toolbar (playlist).
	WndToolbarPlaylist m_ToolbarPlaylist;

	// Counter control.
	WndCounter m_Counter;

	// Splitter control.
	WndSplit m_Splitter;

	// Notification area control.
	WndTray m_Tray;

	// The current output item.
	Output::Item m_CurrentOutput;

	// Custom colours for colour selection dialogs.
	COLORREF m_CustomColours[ 16 ];

	// Hotkeys.
	Hotkeys m_Hotkeys;

	// Performance count at which the last skip backwards or forwards occured.
	LARGE_INTEGER m_LastSkipCount;
};