#pragma once

#include "stdafx.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

// Converts UTF-8 'text' to a wide string.
std::wstring UTF8ToWideString( const std::string& text );

// Converts wide 'text' to UTF-8.
std::string WideStringToUTF8( const std::wstring& text );

// Converts default Windows ANSI code page 'text' to a wide string.
std::wstring AnsiCodePageToWideString( const std::string& text );

// Converts wide 'text' to the default Windows ANSI code page.
std::string WideStringToAnsiCodePage( const std::wstring& text );

// Converts 'text' to lowercase.
std::wstring WideStringToLower( const std::wstring& text );

// Converts 'text' to uppercase.
std::wstring WideStringToUpper( const std::wstring& text );

// Converts 'text' to lowercase.
std::string StringToLower( const std::string& text );

// Converts 'text' to uppercase.
std::string StringToUpper( const std::string& text );

// Converts a file size to a string.
// 'instance' - module instance handle.
// 'filesize' - file size to convert.
std::wstring FilesizeToString( const HINSTANCE instance, const long long filesize );

// Converts a duration to a string.
// 'instance' - module instance handle.
// 'duration' - duration to convert.
// 'colonDelimited' - true to delimit with colons, false to delimit with resource strings.
std::wstring DurationToString( const HINSTANCE instance, const float duration, const bool colonDelimited );

// Converts a byte array to a base64 encoded string.
// 'bytes' - byte array.
// 'byteCount' - number of bytes.
std::wstring Base64Encode( const BYTE* bytes, const int byteCount );

// Converts a base64 encoded 'text' to a byte array.
std::vector<BYTE> Base64Decode( const std::wstring& text );

// Gets image information.
// 'image' - base64 encoded image.
// 'mimeType' - out, MIME type.
// 'width' - out, width in pixels.
// 'height' - out, height in pixels.
// 'depth' - out, bits per pixel.
// 'colours' - out, number of colours (for palette indexed images).
void GetImageInformation( const std::wstring& image, std::string& mimeType, int& width, int& height, int& depth, int& colours );

// Converts 'image' to a base64 encoded form, converting non-PNG/JPG/GIF images to PNG format.
std::wstring ConvertImage( const std::vector<BYTE>& image );

// Generates a GUID.
GUID GenerateGUID();

// Generates a GUID string.
std::string GenerateGUIDString();

// Replaces all occurences of the 'original' substring with 'replacement' in 'text'.
void WideStringReplace( std::wstring& text, const std::wstring& original, const std::wstring& replacement );

// Splits 'text' on the 'delimiter', returning the string parts (or the original string as a single part if it is not delimited).
std::list<std::wstring> WideStringSplit( const std::wstring& text, const wchar_t delimiter );

// Joins 'parts' using the 'delimiter', returning the delimited string.
std::wstring WideStringJoin( const std::list<std::wstring>& parts, const wchar_t delimiter );

// Gets the system DPI scaling factor.
float GetDPIScaling();