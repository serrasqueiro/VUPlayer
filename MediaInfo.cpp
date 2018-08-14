#include "MediaInfo.h"

#include "Utility.h"

#include <tuple>

MediaInfo::MediaInfo( const std::wstring& filename ) :
	m_Filename( filename ),
	m_Filetime( 0 ),
	m_Filesize( 0 ),
	m_Duration( 0 ),
	m_SampleRate( 0 ),
	m_BitsPerSample( 0 ),
	m_Channels( 0 ),
	m_Artist(),
	m_Title(),
	m_Album(),
	m_Genre(),
	m_Year( 0 ),
	m_Comment(),
	m_Track( 0 ),
	m_Version(),
	m_GainTrack( REPLAYGAIN_NOVALUE ),
	m_GainAlbum( REPLAYGAIN_NOVALUE ),
	m_PeakTrack( REPLAYGAIN_NOVALUE ),
	m_PeakAlbum( REPLAYGAIN_NOVALUE ),
	m_ArtworkID(),
	m_Source( Source::File ),
	m_CDDB( 0 )
{
}

MediaInfo::MediaInfo( const long cddbID ) :
	MediaInfo()
{
	m_Source = Source::CDDA;
	m_CDDB = cddbID;
}

MediaInfo::~MediaInfo()
{
}

bool MediaInfo::operator<( const MediaInfo& other ) const
{
	const bool lessThan = std::tie( m_Filename, m_Filetime, m_Filesize, m_Duration, m_SampleRate, m_BitsPerSample, m_Channels, m_Artist,
		m_Title, m_Album, m_Genre, m_Year, m_Comment, m_Track, m_Version, m_GainTrack,
		m_GainAlbum, m_PeakTrack, m_PeakAlbum, m_ArtworkID, m_Source, m_CDDB ) <

		std::tie( other.m_Filename, other.m_Filetime, other.m_Filesize, other.m_Duration, other.m_SampleRate, other.m_BitsPerSample, other.m_Channels, other.m_Artist,
		other.m_Title, other.m_Album, other.m_Genre, other.m_Year, other.m_Comment, other.m_Track, other.m_Version, other.m_GainTrack,
		other.m_GainAlbum, other.m_PeakTrack, other.m_PeakAlbum, other.m_ArtworkID, other.m_Source, other.m_CDDB );

	return lessThan;
}

const std::wstring& MediaInfo::GetFilename() const
{
	return m_Filename;
}

void MediaInfo::SetFilename( const std::wstring& filename )
{
	m_Filename = filename;
}

long long MediaInfo::GetFiletime() const
{
	return m_Filetime;
}

void MediaInfo::SetFiletime( const long long filetime )
{
	m_Filetime = filetime;
}

long long MediaInfo::GetFilesize() const
{
	return m_Filesize;
}

void MediaInfo::SetFilesize( const long long filesize )
{
	m_Filesize = filesize;
}

float MediaInfo::GetDuration() const
{
	return m_Duration;
}

void MediaInfo::SetDuration( const float duration )
{
	m_Duration = duration;
}

long MediaInfo::GetSampleRate() const
{
	return m_SampleRate;
}

void MediaInfo::SetSampleRate( const long sampleRate )
{
	m_SampleRate = sampleRate;
}

long MediaInfo::GetBitsPerSample() const
{
	return m_BitsPerSample;
}

void MediaInfo::SetBitsPerSample( const long bitsPerSample )
{
	m_BitsPerSample = bitsPerSample;
}

long MediaInfo::GetChannels() const
{
	return m_Channels;
}

void MediaInfo::SetChannels( const long channels )
{
	m_Channels = channels;
}

const std::wstring& MediaInfo::GetArtist() const
{
	return m_Artist;
}

void MediaInfo::SetArtist( const std::wstring& artist )
{
	m_Artist = artist;
}

void MediaInfo::SetTitle( const std::wstring& title )
{
	m_Title = title;
}

const std::wstring& MediaInfo::GetAlbum() const
{
	return m_Album;
}

void MediaInfo::SetAlbum( const std::wstring& album )
{
	m_Album = album;
}

const std::wstring& MediaInfo::GetGenre() const
{
	return m_Genre;
}

void MediaInfo::SetGenre( const std::wstring& genre )
{
	m_Genre = genre;
}

long MediaInfo::GetYear() const
{
	return m_Year;
}

void MediaInfo::SetYear( const long year )
{
	if ( ( year >= MINYEAR ) && ( year <= MAXYEAR ) ) {
		m_Year = year;
	} else {
		m_Year = 0;
	}
}

const std::wstring& MediaInfo::GetComment() const
{
	return m_Comment;
}

void MediaInfo::SetComment( const std::wstring& comment )
{
	m_Comment = comment;
}

long MediaInfo::GetTrack() const
{
	return m_Track;
}

void MediaInfo::SetTrack( const long track )
{
	m_Track = track;
}

const std::wstring& MediaInfo::GetVersion() const
{
	return m_Version;
}

void MediaInfo::SetVersion( const std::wstring& version )
{
	m_Version = version;
}

float MediaInfo::GetGainTrack() const
{
	return m_GainTrack;
}

void MediaInfo::SetGainTrack( const float gain )
{
	m_GainTrack = gain;
}

float MediaInfo::GetGainAlbum() const
{
	return m_GainAlbum;
}

void MediaInfo::SetGainAlbum( const float gain )
{
	m_GainAlbum = gain;
}

float MediaInfo::GetPeakTrack() const
{
	return m_PeakTrack;
}

void MediaInfo::SetPeakTrack( const float peak )
{
	m_PeakTrack = peak;
}

float MediaInfo::GetPeakAlbum() const
{
	return m_PeakAlbum;
}

void MediaInfo::SetPeakAlbum( const float peak )
{
	m_PeakAlbum = peak;
}

std::wstring MediaInfo::GetTitle( const bool filenameAsTitle ) const
{
	std::wstring title = m_Title;
	if ( title.empty() && filenameAsTitle ) {
		title = m_Filename;
		size_t pos = title.rfind( '.' );
		if ( std::wstring::npos != pos ) {
			title = title.substr( 0 /*offset*/, pos /*count*/ );
		}
		pos = title.find_last_of( L"/\\" );
		if ( std::wstring::npos != pos ) {
			title = title.substr( pos + 1 /*offset*/ );
		}
	}
	return title;
}

std::wstring MediaInfo::GetType() const
{
	std::wstring type;
	const size_t pos = m_Filename.rfind( '.' );
	if ( std::wstring::npos != pos ) {
		type = WideStringToUpper( m_Filename.substr( pos + 1 /*offset*/ ) );
	}
	return type;
}

long MediaInfo::GetBitrate() const
{
	const long bitrate = ( m_Duration > 0 ) ? static_cast<long>( 0.5f + ( m_Filesize * 8 ) / ( m_Duration * 1000 ) ) : 0;
	return bitrate;
}

const std::wstring& MediaInfo::GetArtworkID() const
{
	return m_ArtworkID;
}

void MediaInfo::SetArtworkID( const std::wstring& id )
{
	m_ArtworkID = id;
}

MediaInfo::Source MediaInfo::GetSource() const
{
	return m_Source;
}

long MediaInfo::GetCDDB() const
{
	return m_CDDB;
}
