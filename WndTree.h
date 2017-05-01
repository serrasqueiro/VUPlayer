#pragma once

#include "stdafx.h"

#include "Library.h"
#include "Settings.h"

class WndTree
{
public:
	// 'instance' - module instance handle.
	// 'parent' - parent window handle.
	// 'library' - media library.
	// 'settings' - application settings.
	WndTree( HINSTANCE instance, HWND parent, Library& library, Settings& settings );

	virtual ~WndTree();

	// Returns the default window procedure.
	WNDPROC GetDefaultWndProc();

	// Gets the tree control handle.
	HWND GetWindowHandle();

	// Returns the playlist associated with a tree 'node', or a null playlist if it's not a playlist node.
	Playlist::Ptr GetPlaylist( const HTREEITEM node );

	// Returns the currently selected playlist, or a null playlist if one isn't selected.
	Playlist::Ptr GetSelectedPlaylist();

	// Called when a 'command' is received.
	void OnCommand( const UINT command );

	// Displays the context menu at the specified 'position', in screen coordinates.
	void OnContextMenu( const POINT& position );

	// Called when item label editing begins.
	// Returns TRUE if label editing is to be prevented.
	LPARAM OnBeginLabelEdit( WPARAM wParam, LPARAM lParam );

	// Called when item label editing ends.
	void OnEndLabelEdit( WPARAM wParam, LPARAM lParam );

	// Called when the window is destroyed.
	void OnDestroy();

	// Gets the playlists.
	Playlists GetPlaylists();

	// Adds a 'playlist'.
	void AddPlaylist( const Playlist::Ptr& playlist );

	// Creates and returns a new playlist.
	Playlist::Ptr NewPlaylist();

	// Deletes the currently selected playlist.
	void DeleteSelectedPlaylist();

	// Launches a file selector dialog to import a playlist.
	void ImportPlaylist();

	// Launches a file selector dialog to export the currently selected playlist.
	void ExportSelectedPlaylist();

	// Selects a 'playlist'.
	void SelectPlaylist( const Playlist::Ptr& playlist );

	// Displays the font selection dialog for changing the tree control font.
	void OnSelectFont();

	// Displays the colour selection dialog for changing the tree control colours.
	// 'commandID' - command ID indicating which colour to change.
	void OnSelectColour( const UINT commandID );

	// Initialises the tree control playlist structure.
	void Initialise();

	// Called when media information has been updated.
	// 'previousMediaInfo' - previous media information.
	// 'updatedMediaInfo' - updated media information.
	// Returns the playlists that have been updated.
	Playlist::Set OnUpdatedMedia( const MediaInfo& previousMediaInfo, const MediaInfo& updatedMediaInfo );

	// Toggles 'All Tracks' on the tree control.
	void OnAllTracks();

	// Toggles artists on the tree control.
	void OnArtists();

	// Toggles albums on the tree control.
	void OnAlbums();

	// Toggles genres on the tree control.
	void OnGenres();

	// Toggles years on the tree control.
	void OnYears();

	// Returns whether the tree control node represented by 'commandID' is shown.
	bool IsShown( const UINT commandID ) const;

	// Saves the startup 'playlist' to application settings.
	void SaveStartupPlaylist( const Playlist::Ptr& playlist );

	// Returns the highlight colour.
	COLORREF GetHighlightColour() const;

private:
	// Maps a tree item to a playlist.
	typedef std::map<HTREEITEM,Playlist::Ptr> PlaylistMap;

	// Loads playlists
	void LoadPlaylists();

	// Imports a VPL playlist from 'filename', returning the playlist.
	Playlist::Ptr ImportPlaylistVPL( const std::wstring& filename );
	
	// Imports an M3U playlist from 'filename', returning the playlist.
	Playlist::Ptr ImportPlaylistM3U( const std::wstring& filename );

	// Imports a PLS playlist from 'filename', returning the playlist.
	Playlist::Ptr ImportPlaylistPLS( const std::wstring& filename );

	// Gets the current tree control font.
	LOGFONT GetFont();

	// Sets the current tree control font.
	void SetFont( const LOGFONT& logFont );

	// Initialises the tree control from application settings.
	void ApplySettings();

	// Saves the tree control settings to application settings.
	void SaveSettings();

	// Adds 'All Tracks' to the tree control.
	void AddAllTracks();

	// Adds artists to the tree control.
	void AddArtists();

	// Adds albums to the tree control.
	void AddAlbums();

	// Adds genres to the tree control.
	void AddGenres();

	// Adds years to the tree control.
	void AddYears();

	// Removes 'All Tracks' from the tree control.
	void RemoveAllTracks();

	// Removes artists from the tree control.
	void RemoveArtists();

	// Removes albums from the tree control.
	void RemoveAlbums();

	// Removes genres from the tree control.
	void RemoveGenres();

	// Removes years from the tree control.
	void RemoveYears();

	// Adds an item to the tree control.
	// 'parentItem' - parent tree item.
	// 'label' - item label.
	// 'type' - item type.
	// Returns the added item.
	HTREEITEM AddItem( const HTREEITEM parentItem, const std::wstring& label, const Playlist::Type type );

	// Removes an item from the tree control.
	// 'item' - item to remove.
	void RemoveItem( const HTREEITEM item );

	// Gets the tree 'item' type.
	Playlist::Type GetItemType( const HTREEITEM item ) const;

	// Gets the tree 'item' label.
	std::wstring GetItemLabel( const HTREEITEM item ) const;

	// Returns all the playlist names.
	std::set<std::wstring> GetPlaylistNames() const;

	// Processes any playlists with pending files.
	void ProcessPendingPlaylists();

	// Updates artists when media information has been updated.
	// 'previousMediaInfo' - previous media information.
	// 'updatedMediaInfo' - updated media information.
	// 'updatedPlaylists' - in/out, the playlists that have been updated.
	void UpdateArtists( const MediaInfo& previousMediaInfo, const MediaInfo& updatedMediaInfo, Playlist::Set& updatedPlaylists );

	// Updates albums when media information has been updated.
	// 'parentItem' - the parent tree item containing the albums.
	// 'previousMediaInfo' - previous media information
	// 'updatedMediaInfo' - updated media information
	// 'updatedPlaylists' - in/out, the playlists that have been updated.
	void UpdateAlbums( const HTREEITEM parentItem, const MediaInfo& previousMediaInfo, const MediaInfo& updatedMediaInfo, Playlist::Set& updatedPlaylists );

	// Updates genres when media information has been updated.
	// 'previousMediaInfo' - previous media information
	// 'updatedMediaInfo' - updated media information
	// 'updatedPlaylists' - in/out, the playlists that have been updated.
	void UpdateGenres( const MediaInfo& previousMediaInfo, const MediaInfo& updatedMediaInfo, Playlist::Set& updatedPlaylists );

	// Updates years when media information has been updated.
	// 'previousMediaInfo' - previous media information
	// 'updatedMediaInfo' - updated media information
	// 'updatedPlaylists' - in/out, the playlists that have been updated.
	void UpdateYears( const MediaInfo& previousMediaInfo, const MediaInfo& updatedMediaInfo, Playlist::Set& updatedPlaylists );

	// Updates playlists when media information has been updated.
	// 'updatedMediaInfo' - updated media information.
	// 'updatedPlaylists' - in/out, the playlists that have been updated.
	void UpdatePlaylists( const MediaInfo& updatedMediaInfo, Playlist::Set& updatedPlaylists );

	// Returns the initial selected tree item from application settings.
	HTREEITEM GetStartupItem();

	// Module instance handle.
	HINSTANCE m_hInst;

	// Window handle.
	HWND m_hWnd;

	// Default window procedure.
	WNDPROC m_DefaultWndProc;

	// Playlists node.
	HTREEITEM m_NodePlaylists;

	// Artists node.
	HTREEITEM m_NodeArtists;

	// Albums node.
	HTREEITEM m_NodeAlbums;

	// Genres node.
	HTREEITEM m_NodeGenres;

	// Years node.
	HTREEITEM m_NodeYears;

	// All tracks node.
	HTREEITEM m_NodeAll;

	// Media library.
	Library& m_Library;

	// Application settings.
	Settings& m_Settings;

	// Playlists.
	PlaylistMap m_PlaylistMap;

	// Artists.
	PlaylistMap m_ArtistMap;

	// Albums.
	PlaylistMap m_AlbumMap;

	// Genres.
	PlaylistMap m_GenreMap;

	// Years.
	PlaylistMap m_YearMap;

	// All Tracks playlist.
	Playlist::Ptr m_PlaylistAll;

	// The font resulting from the font selection dialog.
	HFONT m_ChosenFont;

	// Highlight colour.
	COLORREF m_ColourHighlight;
};

