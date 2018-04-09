#pragma once
#include "Decoder.h"

#include <string>

#include "Opusfile.h"

// Opus decoder
class DecoderOpus : public Decoder
{
public:
	// 'filename' - file name.
	// Throws a std::runtime_error exception if the file could not be loaded.
	DecoderOpus( const std::wstring& filename );

	virtual ~DecoderOpus();

	// Reads sample data.
	// 'buffer' - output buffer (floating point format scaled to +/-1.0f).
	// 'sampleCount' - number of samples to read.
	// Returns the number of samples read, or zero if the stream has ended.
	virtual long Read( float* buffer, const long sampleCount );

	// Seeks to a 'position' in the stream, in seconds.
	// Returns the new position in seconds.
	virtual float Seek( const float position );

private:
	// Opus file
	OggOpusFile* m_OpusFile;
};

