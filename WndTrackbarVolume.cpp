#include "WndTrackbarVolume.h"

#include "resource.h"
#include "VUPlayer.h"

#include <iomanip>
#include <sstream>

WndTrackbarVolume::WndTrackbarVolume( HINSTANCE instance, HWND parent, Output& output, Settings& settings ) :
	WndTrackbar( instance, parent, output, 0 /*minValue*/, 200 /*maxValue*/, GetControlType( settings ) ),
	m_Tooltip(),
	m_Settings( settings )
{
	Update();
}

WndTrackbarVolume::~WndTrackbarVolume()
{
}

const std::wstring& WndTrackbarVolume::GetTooltipText()
{
	m_Tooltip.clear();

	const Type type = GetType();
	switch ( type ) {
		case Type::Volume : {
			const int value = static_cast<int>( GetOutput().GetVolume() * 100 );
			const int bufSize = 32;
			WCHAR buf[ bufSize ] = {};
			LoadString( GetInstanceHandle(), IDS_VOLUME, buf, bufSize );
			std::wstringstream ss;
			ss << buf << L": " << value << L"%";
			m_Tooltip = ss.str();
			break;
		}
		case Type::Pitch : {
			const float value = ( GetOutput().GetPitch() - 1.0f ) * 100;
			const int bufSize = 32;
			WCHAR buf[ bufSize ] = {};
			LoadString( GetInstanceHandle(), IDS_PITCH, buf, bufSize );
			std::wstringstream ss;
			ss << buf << L": " << std::fixed << std::setprecision( 1 ) << std::showpos << value << L"%";
			m_Tooltip = ss.str();
			break;
		}
		default : {
			break;
		}
	}

	return m_Tooltip;
}

void WndTrackbarVolume::OnPositionChanged( const int position )
{
	const Type type = GetType();
	switch ( type ) {
		case Type::Volume : {
			UpdateVolume( position );
			break;
		}
		case Type::Pitch : {
			UpdatePitch( position );
			break;
		}
		default : {
			break;
		}
	}
}

void WndTrackbarVolume::OnDrag( const int position )
{
	const Type type = GetType();
	switch ( type ) {
		case Type::Volume : {
			UpdateVolume( position );
			break;
		}
		case Type::Pitch : {
			UpdatePitch( position );
			break;
		}
		default : {
			break;
		}
	}
}

void WndTrackbarVolume::Update()
{
	const int previousPosition = GetPosition();
	int position = previousPosition;
	const Type type = GetType();
	switch ( type ) {
		case Type::Volume : {
			position = GetPositionFromVolume( GetOutput().GetVolume() );
			break;
		}
		case Type::Pitch : {
			position = GetPositionFromPitch( GetOutput().GetPitch() );
			break;
		}
		default : {
			break;
		}
	}
	if ( previousPosition != position ) {
		SetPosition( position );
	}
}

void WndTrackbarVolume::UpdateVolume( const int position )
{
	const float volume = GetVolumeFromPosition( position );
	GetOutput().SetVolume( volume );
}

float WndTrackbarVolume::GetVolumeFromPosition( const int position ) const
{
	const int range = GetRange();
	const float volume = static_cast<float>( position ) / range;
	return volume;
}

int WndTrackbarVolume::GetPositionFromVolume( const float volume ) const
{
	const int range = GetRange();
	const int position = static_cast<int>( volume * range );
	return position;
}

void WndTrackbarVolume::UpdatePitch( const int position )
{
	const float pitch = GetPitchFromPosition( position );
	GetOutput().SetPitch( pitch );
}

float WndTrackbarVolume::GetPitchFromPosition( const int position ) const
{
	float pitch = 1.0f;
	const float pitchRange = GetOutput().GetPitchRange();
	const int controlRange = GetRange();
	if ( controlRange > 0 ) {
		const int midpoint = controlRange / 2;
		const float adjustment = static_cast<float>( position ) / midpoint - 1;
		pitch += pitchRange * adjustment;
	}
	return pitch;
}

int WndTrackbarVolume::GetPositionFromPitch( const float pitch ) const
{
	int position = 0;
	const float pitchRange = GetOutput().GetPitchRange();
	if ( pitchRange > 0 ) {
		const int controlRange = GetRange();
		const int midpoint = controlRange / 2;
		position = static_cast<int>( midpoint + midpoint * ( pitch - 1.0f ) / pitchRange );
	}
	return position;
}

void WndTrackbarVolume::SetType( const Type type )
{
	if ( GetType() != type ) {
		WndTrackbar::SetType( type );
		Update();
	}
}

void WndTrackbarVolume::OnContextMenu( const POINT& position )
{
	HMENU menu = LoadMenu( GetInstanceHandle(), MAKEINTRESOURCE( IDR_MENU_TRACKBAR ) );
	if ( NULL != menu ) {
		HMENU trackbarMenu = GetSubMenu( menu, 0 /*pos*/ );
		if ( NULL != trackbarMenu ) {
			const Type type = GetType();

			CheckMenuItem( trackbarMenu, ID_VIEW_TRACKBAR_VOLUME, ( Type::Volume == type ) ? MF_CHECKED : MF_UNCHECKED );
			CheckMenuItem( trackbarMenu, ID_VIEW_TRACKBAR_PITCH, ( Type::Pitch == type ) ? MF_CHECKED : MF_UNCHECKED );
			EnableMenuItem( trackbarMenu, ID_CONTROL_PITCHRESET, ( GetOutput().GetPitch() != 1.0f ) ? MF_ENABLED : MF_DISABLED );

			const Settings::PitchRange pitchRange = m_Settings.GetPitchRange();
			CheckMenuItem( menu, ID_CONTROL_PITCHRANGE_SMALL, ( Settings::PitchRange::Small == pitchRange ) ? MF_CHECKED : MF_UNCHECKED );
			CheckMenuItem( menu, ID_CONTROL_PITCHRANGE_MEDIUM, ( Settings::PitchRange::Medium == pitchRange ) ? MF_CHECKED : MF_UNCHECKED );
			CheckMenuItem( menu, ID_CONTROL_PITCHRANGE_LARGE, ( Settings::PitchRange::Large == pitchRange ) ? MF_CHECKED : MF_UNCHECKED );

			const UINT flags = TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD;
			const UINT command = TrackPopupMenu( trackbarMenu, flags, position.x, position.y, 0 /*reserved*/, GetWindowHandle(), NULL /*rect*/ );
			switch ( command ) {
				case ID_VIEW_TRACKBAR_VOLUME : {
					SetType( Type::Volume );
					break;
				}
				case ID_VIEW_TRACKBAR_PITCH : {
					SetType( Type::Pitch );
					break;
				}
				case ID_CONTROL_PITCHRESET :
				case ID_CONTROL_PITCHRANGE_SMALL :
				case ID_CONTROL_PITCHRANGE_MEDIUM :
				case ID_CONTROL_PITCHRANGE_LARGE : {
					VUPlayer* vuplayer = VUPlayer::Get();
					if ( nullptr != vuplayer ) {
						vuplayer->OnCommand( command );
					}	
					break;
				}
				default : {
					break;
				}
			}
		}
		DestroyMenu( menu );
	}
}

WndTrackbar::Type WndTrackbarVolume::GetControlType( Settings& settings )
{
	Type type = Type::Volume;
	if ( settings.GetOutputControlType() == static_cast<int>( Type::Pitch ) ) {
		type = Type::Pitch;
	}
	return type;
}
