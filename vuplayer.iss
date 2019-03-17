; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "VUPlayer"
#define MyAppVersion "4.6"
#define MyAppFullVersion "4.6.0.0"
#define MyAppMainVersion "4"
#define MyAppDefaultDir "VUPlayer 4"
#define MyAppPublisher "James Chapman"
#define MyAppURL "http://www.vuplayer.com/"
#define MyAppExeName "VUPlayer.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{254B76AA-3DED-4BA3-9AB3-EB3FAA46136D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppMainVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
VersionInfoVersion={#MyAppFullVersion}
DefaultDirName={pf}\{#MyAppDefaultDir}
UninstallDisplayIcon={app}\VUPlayer.exe
DisableProgramGroupPage=yes
OutputBaseFilename=vuplayersetup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "x64\Release\VUPlayer.exe"; DestDir: "{app}\"; Check: Is64BitInstallMode; Flags: ignoreversion
Source: "x64\Release\bass.dll"; DestDir: "{app}"; Check: Is64BitInstallMode; Flags: ignoreversion
Source: "x64\Release\bassmidi.dll"; DestDir: "{app}"; Check: Is64BitInstallMode; Flags: ignoreversion
Source: "x64\Release\bassdsd.dll"; DestDir: "{app}"; Check: Is64BitInstallMode; Flags: ignoreversion
Source: "x64\Release\gnsdk_manager.dll"; DestDir: "{app}"; Check: Is64BitInstallMode; Flags: ignoreversion
Source: "x64\Release\gnsdk_musicid.dll"; DestDir: "{app}"; Check: Is64BitInstallMode; Flags: ignoreversion
Source: "x64\Release\gnsdk_storage_sqlite.dll"; DestDir: "{app}"; Check: Is64BitInstallMode; Flags: ignoreversion
Source: "x64\Release\gnsdk_vuplayer.dll"; DestDir: "{app}"; Check: Is64BitInstallMode; Flags: ignoreversion
Source: "Release\VUPlayer.exe"; DestDir: "{app}"; Check: not Is64BitInstallMode; Flags: solidbreak ignoreversion
Source: "Release\bass.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode; Flags: ignoreversion
Source: "Release\bassmidi.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode; Flags: ignoreversion
Source: "Release\bassdsd.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode; Flags: ignoreversion
Source: "Release\gnsdk_manager.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode; Flags: solidbreak ignoreversion
Source: "Release\gnsdk_musicid.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode; Flags: ignoreversion
Source: "Release\gnsdk_storage_sqlite.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode; Flags: solidbreak ignoreversion
Source: "Release\gnsdk_vuplayer.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode; Flags: ignoreversion
Source: "readme.txt"; DestDir: "{app}"; Flags: solidbreak

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

