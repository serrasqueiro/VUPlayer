#pragma once

#include "Handler.h"

#include "bass.h"
#include "bassmidi.h"

// Bass handler
class HandlerBass : public Handler
{
public:
	HandlerBass();

	virtual ~HandlerBass();

	// Returns a description of the handler.
	std::wstring GetDescription() const override;

	// Returns the supported file extensions.
	std::set<std::wstring> GetSupportedFileExtensions() const override;

	// Reads 'tags' from 'filename', returning true if the tags were read.
	bool GetTags( const std::wstring& filename, Tags& tags ) const override;

	// Writes 'tags' to 'filename', returning true if the tags were written.
	bool SetTags( const std::wstring& filename, const Handler::Tags& tags ) const override;

	// Returns a decoder for 'filename', or nullptr if a decoder cannot be created.
	Decoder::Ptr OpenDecoder( const std::wstring& filename ) const override;

	// Returns an encoder, or nullptr if an encoder cannot be created.
	Encoder::Ptr OpenEncoder() const override;

	// Returns whether the handler can decode.
	bool IsDecoder() const override;

	// Returns whether the handler can encode.
	bool IsEncoder() const override;

	// Returns whether the handler supports encoder configuration.
	bool CanConfigureEncoder() const override;

	// Displays an encoder configuration dialog for the handler.
	// 'instance' - application instance handle.
	// 'parent' - application window handle.
	// 'settings' - in/out, encoder settings.
	// Returns whether the encoder has been configured.
	bool ConfigureEncoder( const HINSTANCE instance, const HWND parent, std::string& settings ) const override;

	// Called when the application 'settings' have changed.
	void SettingsChanged( Settings& settings ) override;

	// Loads the soundfont specified by the application 'settings'.
	void LoadSoundFont( Settings& settings );

private:
	// Reads Ogg tags.
	// 'oggTags' - series of null-terminated UTF-8 strings, ending with a double null.
	// 'tags' - out, tag information.
	void ReadOggTags( const char* oggTags, Tags& tags ) const;

	// Writes Ogg 'tags' to 'filename', returning true if the tags were written.
	// 'tags' - out, tag information.
	bool WriteOggTags( const std::wstring& filename, const Tags& tags ) const;

	// The supported file extensions.
	static std::set<std::wstring> s_SupportedFileExtensions;

	// Returns a temporary file name.
	std::wstring GetTemporaryFilename() const;

	// BASS midi plugin.
	HPLUGIN m_BassMidi;

	// BASS midi soundfont.
	HSOUNDFONT m_BassMidiSoundFont;

	// Currently loaded soundfont file name.
	std::wstring m_SoundFontFilename;
};