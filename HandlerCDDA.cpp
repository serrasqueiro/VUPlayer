#include "HandlerCDDA.h"

#include "resource.h"

#include "DecoderCDDA.h"
#include "Utility.h"

HandlerCDDA::HandlerCDDA( const HINSTANCE instance, CDDAManager& cddaManager ) :
	m_hInst( instance ),
	m_CDDAManager( cddaManager )
{
}

HandlerCDDA::~HandlerCDDA()
{
}

std::wstring HandlerCDDA::GetDescription() const
{
	const int bufSize = 32;
	WCHAR buffer[ bufSize ] = {};
	LoadString( m_hInst, IDS_CDAUDIO, buffer, bufSize );
	const std::wstring description( buffer );
	return description;
}

std::set<std::wstring> HandlerCDDA::GetSupportedFileExtensions() const
{
	const std::set<std::wstring> filetypes = { L"cdda" };
	return filetypes;
}

bool HandlerCDDA::GetTags( const std::wstring& /*filename*/, Tags& /*tags*/ ) const
{
	return false;
}

bool HandlerCDDA::SetTags( const std::wstring& /*filename*/, const Tags& /*tags*/ ) const
{
	return false;
}

Decoder::Ptr HandlerCDDA::OpenDecoder( const std::wstring& filename ) const
{
	DecoderCDDA* decoderCDDA = nullptr;
	try {
		const CDDAManager::CDDAMediaMap mediaMap = m_CDDAManager.GetCDDADrives();
		wchar_t drive = 0;
		long track = 0;
		if ( CDDAMedia::FromMediaFilepath( filename, drive, track ) ) {
			const auto driveIter = mediaMap.find( drive );
			if ( mediaMap.end() != driveIter ) {
				const auto& media = driveIter->second;
				decoderCDDA = new DecoderCDDA( media, track );
			}
		}
	} catch ( const std::runtime_error& ) {
	}
	const Decoder::Ptr stream( decoderCDDA );
	return stream;
}

Encoder::Ptr HandlerCDDA::OpenEncoder() const
{
	return nullptr;
}

bool HandlerCDDA::IsDecoder() const
{
	return true;
}

bool HandlerCDDA::IsEncoder() const
{
	return false;
}

bool HandlerCDDA::CanConfigureEncoder() const
{
	return false;
}

bool HandlerCDDA::ConfigureEncoder( const HINSTANCE /*instance*/, const HWND /*parent*/, std::string& /*settings*/ ) const
{
	return false;
}
