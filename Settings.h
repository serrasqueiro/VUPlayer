#pragma once

#include <list>

#include "Database.h"
#include "Library.h"
#include "Playlist.h"

// MOD music fadeout flag.
static const DWORD VUPLAYER_MUSIC_FADEOUT = 0x80000000;

// Application settings
class Settings
{
public:
	// 'database' - application database.
	// 'library' - media library.
	Settings( Database& database, Library& library );

	virtual ~Settings();

	// Playlist column information.
	struct PlaylistColumn {
		int ID;			// Column ID.
		int Width;	// Column width.
	};

	// Hotkey information.
	struct Hotkey {
		int ID;						// ID.
		int Key;					// Key code.
		bool Alt;					// Alt key modifier.
		bool Ctrl;				// Control key modifier.
		bool Shift;				// Shift key modifier.
	};

	// ReplayGain mode.
	enum class ReplaygainMode {
		Disabled,
		Track,
		Album
	};

	// Notification area icon click commands.
	enum class SystrayCommand {
		None = 0,
		Play,
		Stop,
		Previous,
		Next,
		ShowHide
	};

	// A list of playlist columns.
	typedef std::list<PlaylistColumn> PlaylistColumns;

	// A list of hotkeys.
	typedef std::list<Hotkey> HotkeyList;

	// Returns the playlist control settings.
	// 'columns' - out, column settings.
	// 'font' - out, list control font.
	// 'fontColour' - out, list control font colour.
	// 'backgroundColour' - out, list control background colour.
	// 'highlightColour' - out, list control highlight colour.
	void GetPlaylistSettings( PlaylistColumns& columns, LOGFONT& font,
			COLORREF& fontColour, COLORREF& backgroundColour, COLORREF& highlightColour );

	// Sets the playlist control settings.
	// 'columns' - column settings.
	// 'font' - list control font.
	// 'fontColour' - list control font colour.
	// 'backgroundColour' - list control background colour.
	// 'highlightColour' - list control highlight colour.
	void SetPlaylistSettings( const PlaylistColumns& columns, const LOGFONT& font,
			const COLORREF& fontColour, const COLORREF& backgroundColour, const COLORREF& highlightColour );

	// Returns the tree control settings.
	// 'font' - out, tree control font.
	// 'fontColour' - out, tree control font colour.
	// 'backgroundColour' - out, tree control background colour.
	// 'highlightColour' - out, tree control highlight colour.
	// 'showAllTracks' - out, whether All Tracks is shown.
	// 'showArtists' - out, whether Artists are shown.
	// 'showAlbums' - out, whether Albums are shown.
	// 'showGenres' - out, whether Genres are shown.
	// 'showYears' - out, whether Years are shown.
	void GetTreeSettings( LOGFONT& font, COLORREF& fontColour, COLORREF& backgroundColour, COLORREF& highlightColour,
			bool& showAllTracks, bool& showArtists, bool& showAlbums, bool& showGenres, bool& showYears );

	// Sets the tree control settings.
	// 'font' - tree control font.
	// 'fontColour' - tree control font colour.
	// 'backgroundColour' - tree control background colour.
	// 'highlightColour' - tree control highlight colour.
	// 'showAllTracks' - whether All Tracks is shown.
	// 'showArtists' - whether Artists are shown.
	// 'showAlbums' - whether Albums are shown.
	// 'showGenres' - whether Genres are shown.
	// 'showYears' - whether Years are shown.
	void SetTreeSettings( const LOGFONT& font, const COLORREF& fontColour, const COLORREF& backgroundColour, const COLORREF& highlightColour,
			const bool showAllTracks, const bool showArtists, const bool showAlbums, const bool showGenres, const bool showYears );

	// Gets the playlists.
	Playlists GetPlaylists();

	// Removes a playlist from the database.
	void RemovePlaylist( const Playlist& playlist );

	// Saves a playlist to the database.
	void SavePlaylist( Playlist& playlist );

	// Gets the oscilloscope colour.
	COLORREF GetOscilloscopeColour();

	// Sets the oscilloscope colour.
	void SetOscilloscopeColour( const COLORREF colour );

	// Gets the oscilloscope background colour.
	COLORREF GetOscilloscopeBackground();

	// Sets the oscilloscope background colour.
	void SetOscilloscopeBackground( const COLORREF colour );

	// Sets the oscilloscope weight.
	float GetOscilloscopeWeight();

	// Sets the oscilloscope weight.
	void SetOscilloscopeWeight( const float weight );

	// Gets the spectrum analyser settings.
	// 'base' - out, base colour.
	// 'peak' - out, peak colour.
	// 'background - out, background colour.
	void GetSpectrumAnalyserSettings( COLORREF& base, COLORREF& peak, COLORREF& background );

	// Sets the spectrum analyser settings.
	// 'base' - base colour.
	// 'peak' - peak colour.
	// 'background - background colour.
	void SetSpectrumAnalyserSettings( const COLORREF& base, const COLORREF& peak, const COLORREF& background );

	// Gets the VUMeter decay setting.
	float GetVUMeterDecay();

	// Sets the VUMeter decay settings.
	void SetVUMeterDecay( const float decay );

	// Gets the application startup position settings.
	// 'x' - out, desktop X position.
	// 'y' - out, desktop Y position.
	// 'width' - out, main window width.
	// 'height' - out, main window height.
	// 'maximised' - out, whether the main window is maximised.
	// 'minimsed' - out, whether the main window is minimised.
	void GetStartupPosition( int& x, int& y, int& width, int& height, bool& maximised, bool& minimised );

	// Gets the application startup position settings.
	// 'x' - out, desktop X position.
	// 'y' - out, desktop Y position.
	// 'width' - out, main window width.
	// 'height' - out, main window height.
	// 'maximised' - out, whether the main window is maximised.
	// 'minimsed' - out, whether the main window is minimised.
	void SetStartupPosition( const int x, const int y, const int width, const int height, const bool maximised, const bool minimised );

	// Returns the startup visual ID.
	int GetVisualID();

	// Sets the startup visual ID.
	void SetVisualID( const int visualID );

	// Returns the startup split width.
	int GetSplitWidth();

	// Sets the startup split width.
	void SetSplitWidth( const int width );

	// Returns the startup volume level.
	float GetVolume();

	// Sets the startup volume level.
	void SetVolume( const float volume );

	// Gets the startup playlist.
	std::wstring GetStartupPlaylist();

	// Sets the startup 'playlist'.
	void SetStartupPlaylist( const std::wstring& playlist );

	// Gets the startup playlist selected index.
	int GetStartupPlaylistSelection();

	// Sets the startup playlist selected index.
	void SetStartupPlaylistSelection( const int selected );

	// Gets the counter settings.
	// 'font' - out, counter font.
	// 'fontColour' - out, counter colour.
	// 'showRemaining' - out, true to display remaining time, false for elapsed time.
	void GetCounterSettings( LOGFONT& font,	COLORREF& colour, bool& showRemaining );

	// Sets the counter settings.
	// 'font' - counter font.
	// 'fontColour' - counter colour.
	// 'showRemaining' - true to display remaining time, false for elapsed time.
	void SetCounterSettings( const LOGFONT& font,	const COLORREF colour, const bool showRemaining );

	// Returns the output device name (or an empty string for the default device).
	std::wstring GetOutputDevice();

	// Sets the output device 'name' (or an empty string for the default device).
	void SetOutputDevice( const std::wstring& name );

	// Gets default MOD music settings.
	// 'mod' - out, MOD settings.
	// 'mtm' - out, MTM settings.
	// 's3m' - out, S3M settings.
	// 'xm' - out, XM settings.
	// 'it' - out, IT settings.
	void GetDefaultMODSettings( long long& mod, long long& mtm, long long& s3m, long long& xm, long long& it );

	// Gets MOD music settings.
	// 'mod' - out, MOD settings.
	// 'mtm' - out, MTM settings.
	// 's3m' - out, S3M settings.
	// 'xm' - out, XM settings.
	// 'it' - out, IT settings.
	void GetMODSettings( long long& mod, long long& mtm, long long& s3m, long long& xm, long long& it );

	// Sets MOD music settings.
	// 'mod' - MOD settings.
	// 'mtm' - MTM settings.
	// 's3m' - S3M settings.
	// 'xm' - XM settings.
	// 'it' - IT settings.
	void SetMODSettings( const long long mod, const long long mtm, const long long s3m, const long long xm, const long long it );

	// Gets default ReplayGain settings.
	// 'mode' - out, ReplayGain mode.
	// 'preamp' - out, preamp in dB.
	// 'hardlimit' - out, true to hard limit, false to allow clipping.
	void GetDefaultReplaygainSettings( ReplaygainMode& mode, float& preamp, bool& hardlimit );

	// Gets ReplayGain settings.
	// 'mode' - out, ReplayGain mode.
	// 'preamp' - out, preamp in dB.
	// 'hardlimit' - out, true to hard limit, false to allow clipping.
	void GetReplaygainSettings( ReplaygainMode& mode, float& preamp, bool& hardlimit );

	// Sets ReplayGain settings.
	// 'mode' - ReplayGain mode.
	// 'preamp' - preamp in dB.
	// 'hardlimit' - true to hard limit, false to allow clipping.
	void SetReplaygainSettings( const ReplaygainMode mode, const float preamp, const bool hardlimit );

	// Gets notification area settings.
	// 'enable' - out, whether the notification area icon is shown.
	// 'singleClick' - out, single click action.
	// 'doubleClick' - out, double click action.
	void GetSystraySettings( bool& enable, SystrayCommand& singleClick, SystrayCommand& doubleClick );

	// Sets notification area settings.
	// 'enable' - whether the notification area icon is shown.
	// 'singleClick' - single click action.
	// 'doubleClick' - double click action.
	void SetSystraySettings( const bool enable, const SystrayCommand singleClick, const SystrayCommand doubleClick );

	// Gets playback settings.
	// 'randomPlay' - whether random playback is enabled.
	// 'repeatTrack' - whether repeat track is enabled.
	// 'repeatPlaylist' - whether repeat playlist is enabled.
	// 'crossfade' - whether crossfade is enabled.
	void GetPlaybackSettings( bool& randomPlay, bool& repeatTrack, bool& repeatPlaylist, bool& crossfade );

	// Sets playback settings.
	// 'randomPlay' - whether random playback is enabled.
	// 'repeatTrack' - whether repeat track is enabled.
	// 'repeatPlaylist' - whether repeat playlist is enabled.
	// 'crossfade' - whether crossfade is enabled.
	void SetPlaybackSettings( const bool randomPlay, const bool repeatTrack, const bool repeatPlaylist, const bool crossfade );

	// Gets hotkey settings.
	// 'enable' - out, whether hotkeys are enabled.
	// 'hotkeys' - out, hotkeys.
	void GetHotkeySettings( bool& enable, HotkeyList& hotkeys );

	// Sets hotkey settings.
	// 'enable' - whether hotkeys are enabled.
	// 'hotkeys' - hotkeys.
	void SetHotkeySettings( const bool enable, const HotkeyList& hotkeys );

private:
	// Updates the database to the current version if necessary.
	void UpdateDatabase();

	// Updates the settings table if necessary.
	void UpdateSettingsTable();

	// Updates the playlist columns table if necessary.
	void UpdatePlaylistColumnsTable();

	// Updates the playlist sources table if necessary.
	void UpdatePlaylistsTable();

	// Updates the hotkeys table if necessary.
	void UpdateHotkeysTable();

	// Updates the playlist table if necessary.
	void UpdatePlaylistTable( const std::string& table );

	// Sets the playlist files from the database.
	void ReadPlaylistFiles( Playlist& playlist );

	// Returns whether a GUID string is valid.
	static bool IsValidGUID( const std::string& guid );

	// Database.
	Database& m_Database;

	// Media library.
	Library& m_Library;
};