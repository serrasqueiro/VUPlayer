#pragma once

#include "Output.h"
#include "Settings.h"

class DlgOptions
{
public:
	// 'instance' - module instance handle.
	// 'parent' - parent window handle.
	// 'settings' - application settings.
	// 'output' - output object.
	DlgOptions( HINSTANCE instance, HWND parent, Settings& settings, Output& output );

	virtual ~DlgOptions();

private:
	// Module instance handle.
	HINSTANCE m_hInst;

	// Application settings.
	Settings& m_Settings;

	// Output object.
	Output& m_Output;
};
