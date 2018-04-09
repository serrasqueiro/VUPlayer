#include "ShellMetadata.h"

#include "Utility.h"

// Maps a audio subtype GUID string to an audio format description.
typedef std::map<std::wstring,std::wstring> AudioFormatMap;

// Audio format descriptions
static AudioFormatMap sAudioFormatDescriptions;

// Maximum thumbnail size
static const int sMaxThumbnailBytes = 0x1000000;

ShellMetadata::ShellMetadata()
{
}

ShellMetadata::~ShellMetadata()
{
}

bool ShellMetadata::Get( const std::wstring& filename, Handler::Tags& tags )
{
	bool success = false;
	tags.clear();
	IShellItem2* item = nullptr;
	HRESULT hr = SHCreateItemFromParsingName( filename.c_str(), NULL /*bindContext*/, IID_PPV_ARGS( &item ) );
	if ( SUCCEEDED( hr ) ) {
		const GETPROPERTYSTOREFLAGS flags = GPS_DEFAULT;
		IPropertyStore* propStore = nullptr;
		hr = item->GetPropertyStore( flags, IID_PPV_ARGS( &propStore ) );
		if ( SUCCEEDED( hr ) ) {
			success = true;
			DWORD propCount = 0;
			hr = propStore->GetCount( &propCount );
			if ( SUCCEEDED( hr ) ) {
				for ( DWORD i = 0; i < propCount; i++ ) {
					PROPERTYKEY propKey = { 0 };
					if ( SUCCEEDED( propStore->GetAt( i, &propKey ) ) ) {
						PROPVARIANT propVar;
						if ( SUCCEEDED( propStore->GetValue( propKey, &propVar ) ) ) {
							if ( PKEY_Music_AlbumArtist == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									tags.insert( Handler::Tags::value_type( Handler::Tag::Artist, value ) );
								}
							} else if ( PKEY_Music_Artist == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									tags.erase( Handler::Tag::Artist );
									tags.insert( Handler::Tags::value_type( Handler::Tag::Artist, value ) );
								}					
							}	else if ( PKEY_Title == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									tags.insert( Handler::Tags::value_type( Handler::Tag::Title, value ) );
								}
							} else if ( PKEY_Music_Genre == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									tags.insert( Handler::Tags::value_type( Handler::Tag::Genre, value ) );
								}
							} else if ( PKEY_Music_AlbumTitle == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									tags.insert( Handler::Tags::value_type( Handler::Tag::Album, value ) );
								}
							} else if ( PKEY_Comment == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									tags.insert( Handler::Tags::value_type( Handler::Tag::Comment, value ) );
								}
							} else if ( PKEY_Audio_Format == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									const std::wstring version = ShellMetadata::GetAudioSubType( value );
									if ( !version.empty() ) {
										tags.insert( Handler::Tags::value_type( Handler::Tag::Version, version ) );
									}
								}						
							} else if ( PKEY_Media_Year == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									tags.insert( Handler::Tags::value_type( Handler::Tag::Year, value ) );
								}
							} else if ( PKEY_Music_TrackNumber == propKey ) {
								const std::wstring value = PropertyToString( propVar );
								if ( !value.empty() ) {
									tags.insert( Handler::Tags::value_type( Handler::Tag::Track, value ) );
								}
							} else if ( PKEY_ThumbnailStream == propKey ) {
								if ( VT_STREAM == propVar.vt ) {
									IStream* stream = propVar.pStream;
									if ( nullptr != stream ) {
										STATSTG stats = {};
										if ( SUCCEEDED( stream->Stat( &stats, STATFLAG_NONAME ) ) ) {
											if ( stats.cbSize.QuadPart <= sMaxThumbnailBytes ) {
												ULONG bytesRead = 0;
												BYTE* buffer = new BYTE[ static_cast<long>( stats.cbSize.QuadPart ) ];
												if ( SUCCEEDED( stream->Read( buffer, static_cast<ULONG>( stats.cbSize.QuadPart ), &bytesRead ) ) ) {
													const std::wstring encodedImage = Base64Encode( buffer, bytesRead );
													if ( !encodedImage.empty() ) {
														tags.insert( Handler::Tags::value_type( Handler::Tag::Artwork, encodedImage ) );
													}
												}
												delete [] buffer;
											}
										}
									}
								}
							}
							PropVariantClear( &propVar );
						}
					}
				}
			}
			propStore->Release();
		}
		item->Release();
	}
	return success;
}

bool ShellMetadata::Set( const std::wstring& filename, const Handler::Tags& tags )
{
	bool success = false;
	IShellItem2* item = nullptr;
	HRESULT hr = SHCreateItemFromParsingName( filename.c_str(), NULL /*bindContext*/, IID_PPV_ARGS( &item ) );
	if ( SUCCEEDED( hr ) ) {
		IStream* thumbnailStream = nullptr;
		const GETPROPERTYSTOREFLAGS flags = GPS_READWRITE;
		IPropertyStore* propStore = nullptr;
		hr = item->GetPropertyStore( flags, IID_PPV_ARGS( &propStore ) );
		if ( SUCCEEDED( hr ) ) {
			success = true;
			bool propStoreModified = false;
			for ( auto tagIter = tags.begin(); success && ( tagIter != tags.end() ); tagIter++ ) {
				const Handler::Tag tag = tagIter->first;
				const std::wstring& value = tagIter->second;
				PROPERTYKEY propKey = {};
				PROPVARIANT propVar = {};
				bool updateTag = true;
				switch ( tag ) {
					case Handler::Tag::Album : {
						propKey = PKEY_Music_AlbumTitle;
						hr = InitPropVariantFromString( value.c_str(), &propVar );
						break;
					}
					case Handler::Tag::Artist : {
						propKey = PKEY_Music_Artist;
						hr = InitPropVariantFromString( value.c_str(), &propVar );
						break;
					}
					case Handler::Tag::Comment : {
						propKey = PKEY_Comment;
						hr = InitPropVariantFromString( value.c_str(), &propVar );
						break;
					}
					case Handler::Tag::Genre : {
						propKey = PKEY_Music_Genre;
						hr = InitPropVariantFromString( value.c_str(), &propVar );
						break;
					}
					case Handler::Tag::Title : {
						propKey = PKEY_Title;
						hr = InitPropVariantFromString( value.c_str(), &propVar );
						break;
					}
					case Handler::Tag::Track : {
						propKey = PKEY_Music_TrackNumber;
						hr = InitPropVariantFromUInt32( std::stoul( value ), &propVar );
						break;
					}
					case Handler::Tag::Year : {
						propKey = PKEY_Media_Year;
						hr = InitPropVariantFromUInt32( std::stoul( value ), &propVar );
						break;
					}
					case Handler::Tag::Artwork : {
						// Note that there is no way of removing a thumbnail stream from a property store.
						updateTag = false;
						if ( nullptr == thumbnailStream ) {
							const std::vector<BYTE> imageBytes = Base64Decode( value );
							const ULONG imageSize = static_cast<UINT>( imageBytes.size() );
							if ( imageSize > 0 ) {
								propKey = PKEY_ThumbnailStream;
								hr = CreateStreamOnHGlobal( NULL /*hGlobal*/, TRUE /*deleteOnRelease*/, &thumbnailStream );
								if ( SUCCEEDED( hr ) ) {
									hr = thumbnailStream->Write( &imageBytes[ 0 ], imageSize, NULL /*bytesWritten*/ );
									if ( SUCCEEDED( hr ) ) {
										hr = thumbnailStream->Seek( { 0 }, STREAM_SEEK_SET, NULL /*newPosition*/ );
									}
									if ( SUCCEEDED( hr ) ) {
										propVar.vt = VT_STREAM;
										propVar.pStream = thumbnailStream;
										updateTag = true;
									} else {
										thumbnailStream->Release();
										thumbnailStream = nullptr;
									}
								}
							}
						}
						break;
					}
					default : {
						updateTag = false;
						break;
					}
				}
				if ( updateTag ) {
					hr = propStore->SetValue( propKey, propVar );
					success = SUCCEEDED( hr );
					propStoreModified = success;
				}
				PropVariantClear( &propVar );
			}
			if ( propStoreModified ) {
				hr = propStore->Commit();
				success = SUCCEEDED( hr );
			}
			propStore->Release();
		}
		item->Release();
		if ( nullptr != thumbnailStream ) {
			thumbnailStream->Release();
		}
	}
	return success;
}

std::wstring ShellMetadata::GetAudioSubType( const std::wstring& guid )
{
	if ( sAudioFormatDescriptions.empty() ) {
		LPOLESTR str = nullptr;
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_AAC, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"AAC" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_AMR_NB, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"AMR" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_AMR_WB, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"AMR" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_AMR_WP, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"AMR" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_Dolby_AC3, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"Dolby Digital" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_Dolby_AC3_SPDIF, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"Dolby Digital" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_Dolby_DDPlus, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"Dolby Digital Plus" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_DTS, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"DTS" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_Float, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"IEEE Float" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_MP3, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"MP3" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_MPEG, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"MPEG-1" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_MSP1, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"WMA 9 Voice" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_PCM, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"PCM" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_WMASPDIF, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"WMA 9 Pro" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_WMAudio_Lossless, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"WMA Lossless" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_WMAudioV8, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"WMA" ) );
			CoTaskMemFree( str );
		}
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_WMAudioV9, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"WMA 9 Pro" ) );
			CoTaskMemFree( str );
		}
#ifdef MFAudioFormat_ALAC
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_ALAC, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"Apple Lossless" ) );
			CoTaskMemFree( str );
		}
#endif
#ifdef MFAudioFormat_FLAC
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_FLAC, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"FLAC" ) );
			CoTaskMemFree( str );
		}
#endif
#ifdef MFAudioFormat_Opus
		if ( SUCCEEDED( StringFromCLSID( MFAudioFormat_Opus, &str ) ) ) {
			sAudioFormatDescriptions.insert( AudioFormatMap::value_type( WideStringToLower( str ), L"Opus" ) );
			CoTaskMemFree( str );
		}
#endif
	}

	std::wstring description;
	const auto iter = sAudioFormatDescriptions.find( WideStringToLower( guid ) );
	if ( iter != sAudioFormatDescriptions.end() ) {
		description = iter->second;
	}
	return description;
}

std::wstring ShellMetadata::PropertyToString( const PROPVARIANT& propVariant )
{
	std::wstring value;
	if ( VT_VECTOR & propVariant.vt ) {
		LPWSTR* strings = nullptr;
		ULONG numStrings = 0;
		if ( SUCCEEDED( PropVariantToStringVectorAlloc( propVariant, &strings, &numStrings ) ) ) {
			for ( ULONG index = 0; index < numStrings; index++ ) {
				if ( !value.empty() ) {
					value += L", ";
				}
				value += strings[ index ];
				CoTaskMemFree( strings[ index ] );
			}
			CoTaskMemFree( strings );
		}
	} else if ( VT_LPWSTR == propVariant.vt ) {
		value = propVariant.pwszVal;
	} else if ( VT_UI4 == propVariant.vt ) {
		value = std::to_wstring( propVariant.ulVal );
	}
	return value;
}