; truss.iss - Inno Setup script for Truss

[Setup]
AppName=Truss
AppVersion=latest
DefaultDirName={autopf}\Truss
DefaultGroupName=Truss
OutputDir=installer_output
OutputBaseFilename=TrussInstaller
Compression=lzma
SolidCompression=yes
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\truss.exe
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "dist\truss.dist\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Truss"; Filename: "{app}\truss.exe"
Name: "{commondesktop}\Truss"; Filename: "{app}\truss.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop icon"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\truss.exe"; Description: "Launch Truss"; Flags: nowait postinstall skipifsilent