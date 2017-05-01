#pragma once

#include "stdafx.h"

#include <shellapi.h>

#include "Output.h"
#include "Playlist.h"
#include "Settings.h"

class WndList
{
public:
	// 'instance' - module instance handle.
	// 'parent' - parent window handle.
	// 'settings' - application settings.
	// 'output' - output object.
	WndList( HINSTANCE instance, HWND parent, Settings& settings, Output& output );

	virtual ~WndList();

	// Returns the default window procedure.
	WNDPROC GetDefaultWndProc();

	// Gets the list control handle.
	HWND GetWindowHandle();

	// Called when files are dropped onto the list.
	// 'hDrop' - handle describing the dropped files.
	void OnDropFiles( const HDROP hDrop );

	// Starts playback at the playlist 'itemID'.
	void OnPlay( const long itemID );

	// Displays the context menu at the specified 'position', in screen coordinates.
	void OnContextMenu( const POINT& position );

	// Called when a 'command' is received.
	void OnCommand( const UINT command );

	// Saves the current list control settings.
	void SaveSettings();

	// Deletes the currently selected items from the list.
	void DeleteSelectedItems();

	// Sorts the playlist by 'column' type.
	void SortPlaylist( const Playlist::Column column );

	// Sets the current playlist.
	// 'initSelection' - whether to select the first playlist item (or the currently playing item if it's in the list).
	void SetPlaylist( const Playlist::Ptr& playlist, const bool initSelection = true );

	// Called when an 'item' is added to the 'playlist' at a (0-based) 'position'.
	void OnFileAdded( Playlist* playlist, const Playlist::Item& item, const int position );

	// Adds a playlist item to the list control.
	// 'wParam' - added item information.
	void AddFileHandler( WPARAM wParam );

	// Called when an 'item' is removed from the 'playlist'.
	void OnFileRemoved( Playlist* playlist, const Playlist::Item& item );

	// Removes a playlist item from the list control.
	// 'wParam' - removed item information.
	void RemoveFileHandler( WPARAM wParam );

	// Returns the highlight colour.
	COLORREF GetHighlightColour() const;

	// Called when label editing begins on an 'item'.
	// Returns TRUE if label editing is allowed, FALSE if not.
	BOOL OnBeginLabelEdit( const LVITEM& item );

	// Called when label editing ends on an 'item'.
	void OnEndLabelEdit( const LVITEM& item );

	// Called when a column has been dragged to a different position.
	void OnEndDragColumn();

	// Ensures the dummy column is at the zero position after a column has been dragged.
	void ReorderDummyColumn();

	// Called when a list control drag operation is started on the 'itemIndex'.
	void OnBeginDrag( const int itemIndex );

	// Called when a list control drag operation finishes.
	void OnEndDrag();

	// Called when the mouse moves during a drag operation.
	// 'point' - mouse position, in client coordinates.
	void OnDrag( const POINT& point );

	// Called when the drag timer interval elapses.
	void OnDragTimer();

	// Returns the default edit control window procedure.
	WNDPROC GetEditControlWndProc();

	// Adjusts the edit control 'position' when item editing commences.
	void RepositionEditControl( WINDOWPOS* position );

	// Updates the list control media information.
	// 'mediaInfo' - updated media information
	void OnUpdatedMedia( const MediaInfo& mediaInfo );

	// Returns the playlist item ID corresponding to the list control 'itemIndex'.
	long GetPlaylistItemID( const int itemIndex );

	// Returns the current playlist associated with the list control.
	Playlist::Ptr GetPlaylist();

	// Returns the currently focused playlist item.
	Playlist::Item GetCurrentSelectedItem();

	// Returns the currently focused list control item index.
	int GetCurrentSelectedIndex();

	// Selects the previous item in the playlist.
	void SelectPreviousItem();

	// Selects the next item in the playlist.
	void SelectNextItem();

	// Ensures a playlist 'item' is visible, and optionally selected.
	void EnsureVisible( const Playlist::Item& item, const bool select = true );

	// Returns the list of selected playlist items.
	Playlist::ItemList GetSelectedPlaylistItems();

	// Returns the number of selected playlist items.
	int GetSelectedCount() const;

	// Launches a file selector dialog to add a folder to the playlist.
	void OnCommandAddFolder();

	// Launches a file selector dialog to add files to the playlist.
	void OnCommandAddFiles();

	// Called when a 'command' is received to show or hide a column.
	void OnShowColumn( const UINT command );

	// Called when a 'command' is received to sort the list.
	void OnSortPlaylist( const UINT command );

	// Gets the command IDs of the 'shown' & 'hidden' columns.
	void GetColumnVisibility( std::set<UINT>& shown, std::set<UINT>& hidden );

	// Displays the font selection dialog for changing the list control font.
	void OnSelectFont();

	// Displays the colour selection dialog for changing the list control colours.
	// 'commandID' - command ID indicating which colour to change.
	void OnSelectColour( const UINT commandID );

private:
	// Column format information.
	struct ColumnFormat {
		UINT ShowID;					// Show command ID.
		UINT SortID;					// Sort command ID.
		UINT HeaderID;				// Column header string resource ID.
		UINT Alignment;				// Column alignment.
		int Width;						// Default column width.
		bool CanEdit;					// Whether column entries can be edited.
	};

	// Added item information.
	struct AddedItem {
		Playlist* Playlist;	// Playlist pointer.
		Playlist::Item Item;	// Playlist item.
		int Position;					// Added item position (0-based).
	};

	// Maps a column type to the column format.
	typedef std::map<Playlist::Column,ColumnFormat> ColumnFormats;

	// Applies the current settings to the list.
	void ApplySettings();

	// Adds 'filename' to the list of files to add to the playlist.
	void AddFileToPlaylist( const std::wstring& filename );

	// Adds 'folder' to the list of files to add to the playlist.
	void AddFolderToPlaylist( const std::wstring& folder );

	// Inserts a 'playlistItem' into the list control.
	// 'position' item position, or -1 to append the item to the end of the list control.
	void InsertListViewItem( const Playlist::Item& playlistItem, const int position = -1 );

	// Sets the text of a list view item.
	// 'itemIndex' - list view item index.
	// 'mediaInfo' - media information.
	void SetListViewItemText( int itemIndex, const MediaInfo& mediaInfo );

	// Returns whether the 'column' is shown.
	bool IsColumnShown( const Playlist::Column& column ) const;

	// Refreshes the text of all list control items.
	void RefreshListViewItemText();

	// Updates the sort indicator on the list control header.
	void UpdateSortIndicator();

	// Gets the current list control font.
	LOGFONT GetFont();

	// Sets the current list control font.
	void SetFont( const LOGFONT& logFont );

	// Moves selected items to 'insertionIndex'.
	void MoveSelectedItems( const int insertionIndex );

	// Selects all list control items.
	void SelectAllItems();

	// Cuts or copies selected files from the playlist into the clipboard.
	// 'cut' - true to cut the files, false to copy the files.
	void OnCutCopy( const bool cut );

	// Pastes files into the playlist from the clipboard.
	void OnPaste();

	// Shows or hides a column.
	// 'column' - column type.
	// 'width' - column width.
	// 'show' - true to show, false to hide.
	void ShowColumn( const Playlist::Column column, const int width, const bool show );

	// Column format information.
	static ColumnFormats s_ColumnFormats;

	// Next available child window ID.
	static UINT_PTR s_WndListID;

	// Module instance handle.
	HINSTANCE m_hInst;

	// Window handle.
	HWND m_hWnd;

	// Default window procedure.
	WNDPROC m_DefaultWndProc;

	// Playlist.
	Playlist::Ptr m_Playlist;

	// Application settings.
	Settings& m_Settings;

	// Output object.
	Output& m_Output;

	// Highlight colour.
	COLORREF m_ColourHighlight;

	// The font resulting from the font selection dialog.
	HFONT m_ChosenFont;

	// Edit control when label editing.
	HWND m_EditControl;

	// Indicates the item index being edited.
	int m_EditItem;

	// Indicates the subitem index being edited.
	int m_EditSubItem;

	// Default edit control window procedure.
	WNDPROC m_EditControlWndProc;

	// Indicates whether list control item(s) are being dragged.
	bool m_IsDragging;

	// Image list when dragging.
	HIMAGELIST m_DragImage;

	// Cursor to set back when a drag operation finishes.
	HCURSOR m_OldCursor;
};
