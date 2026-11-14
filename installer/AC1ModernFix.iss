; AC1 Modern Fix installer, built with Inno Setup 6 (https://jrsoftware.org/isinfo.php).
; Build: ISCC.exe installer\AC1ModernFix.iss   (after ./build.sh has made dist\AC1AffinityFix.asi)
; GitHub Actions builds it on a Windows runner; see .github/workflows/ci.yml.
;
; What it does:
;   - finds Assassin's Creed: Director's Cut (Steam library folders, GOG), or lets the user browse
;   - checks the game exe is the version EaglePatch supports
;   - downloads Ultimate ASI Loader v4.68 and EaglePatch v1.1 from their official GitHub releases
;     (SHA-256 checked; EaglePatch has no license, so it is never bundled)
;   - installs AC1AffinityFix (bundled, MIT) and the downloaded files into the game folder
;   - runs the game's own DirectX setup if xinput1_3.dll is missing
;   - shows up in Windows "Installed apps" with a normal uninstaller
; Copyright (c) 2026 Meanderpaul_gaming. MIT License.

#define AppVersion "1.2.0"

[Setup]
AppId={{8B6A4C2E-6F1D-4B7A-9E1C-AC1F1A5E0001}
AppName=AC1 Modern Fix
AppVersion={#AppVersion}
AppVerName=AC1 Modern Fix {#AppVersion}
AppPublisher=Meanderpaul_gaming
AppPublisherURL=https://github.com/meanderpaul/AC1-ModernFix
AppSupportURL=https://github.com/meanderpaul/AC1-ModernFix/blob/main/docs/TROUBLESHOOTING.md
DefaultDirName={code:GetGameDir}
AppendDefaultDirName=no
DirExistsWarning=no
DisableProgramGroupPage=yes
UsePreviousAppDir=yes
PrivilegesRequired=admin
; keep the uninstaller out of the game folder
UninstallFilesDir={commonappdata}\AC1ModernFix
UninstallDisplayName=AC1 Modern Fix
UninstallDisplayIcon={uninstallexe}
OutputDir=..\dist
OutputBaseFilename=AC1ModernFix-Setup
SetupIconFile=app.ico
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
MinVersion=10.0
VersionInfoVersion={#AppVersion}
VersionInfoCompany=Meanderpaul_gaming
VersionInfoDescription=AC1 Modern Fix Setup
VersionInfoCopyright=(c) 2026 Meanderpaul_gaming. MIT License.

[Messages]
SelectDirDesc=Where is Assassin's Creed installed?
SelectDirLabel3=Setup found the folder below. If it's wrong, click Browse and pick the folder that contains AssassinsCreed_Dx9.exe.
SelectDirBrowseLabel=To continue, click Next.

[Tasks]
Name: skipintro; Description: "Skip the intro videos"
Name: ps3controls; Description: "PS3-style controls (RB = run)"; Flags: unchecked

[Files]
Source: "..\dist\AC1AffinityFix.asi"; DestDir: "{app}\scripts"; Flags: ignoreversion
Source: "..\release\AC1AffinityFix.ini"; DestDir: "{app}\scripts"; Flags: onlyifdoesntexist
; downloaded and unzipped into {tmp} on the Ready page (see NextButtonClick)
Source: "{tmp}\asi\dinput8.dll"; DestDir: "{app}"; Flags: external ignoreversion
Source: "{tmp}\ep\EaglePatchAC1.asi"; DestDir: "{app}\scripts"; Flags: external ignoreversion
Source: "{tmp}\ep\EaglePatchAC1.ini"; DestDir: "{app}\scripts"; Flags: external onlyifdoesntexist

[UninstallDelete]
Type: dirifempty; Name: "{app}\scripts"

[Run]
Filename: "{app}\DirectX\DXSETUP.exe"; Parameters: "/silent"; StatusMsg: "Installing the DirectX controller runtime..."; Check: NeedsDirectX; Flags: waituntilterminated
Filename: "steam://rungameid/15100"; Description: "Launch Assassin's Creed"; Check: IsSteamInstall; Flags: shellexec postinstall nowait skipifsilent runasoriginaluser
Filename: "{app}\AssassinsCreed_Dx9.exe"; WorkingDir: "{app}"; Description: "Launch Assassin's Creed"; Check: not IsSteamInstall; Flags: postinstall nowait skipifsilent runasoriginaluser

[Code]
const
  GameExe = 'AssassinsCreed_Dx9.exe';
  GameExeMD5 = '8e72c3333743780e43bc2c34bbf625f9';
  AsiUrl = 'https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v4.68/Ultimate-ASI-Loader.zip';
  AsiSha256 = '7517c7d5bd8c475f18e6545b7998df379eed6e2e0a19fcd2139669152f9bcddb';
  EpUrl = 'https://github.com/Sergeanur/EaglePatch/releases/download/v1.1/EaglePatchAC1.zip';
  EpSha256 = '6dc496ac6b298b92fb70c3f90a34e323f77ac88e6ea97544f84e52435038aeea';

var
  DownloadPage: TDownloadWizardPage;

{ ---------- finding the game ---------- }

function IsGameDir(const Dir: String): Boolean;
begin
  Result := (Dir <> '') and FileExists(AddBackslash(Dir) + GameExe);
end;

function SteamLibraryGame(const Lib: String): String;
begin
  Result := AddBackslash(Lib) + 'steamapps\common\Assassins Creed';
  if not IsGameDir(Result) then
    Result := '';
end;

function FindSteamGame(): String;
var
  SteamPath, Line, Lib: String;
  Lines: TArrayOfString;
  I, P: Integer;
begin
  Result := '';
  if not RegQueryStringValue(HKCU, 'Software\Valve\Steam', 'SteamPath', SteamPath) then
    if not RegQueryStringValue(HKLM32, 'SOFTWARE\Valve\Steam', 'InstallPath', SteamPath) then
      Exit;
  StringChangeEx(SteamPath, '/', '\', True);
  Result := SteamLibraryGame(SteamPath);
  if Result <> '' then
    Exit;
  { other Steam library folders: lines like   "path"   "D:\\SteamLibrary" }
  if LoadStringsFromFile(AddBackslash(SteamPath) + 'steamapps\libraryfolders.vdf', Lines) then
    for I := 0 to GetArrayLength(Lines) - 1 do
    begin
      Line := Trim(Lines[I]);
      if Pos('"path"', Lowercase(Line)) = 1 then
      begin
        Lib := Copy(Line, 7, Length(Line));
        P := Pos('"', Lib);
        Lib := Copy(Lib, P + 1, Length(Lib));
        P := Pos('"', Lib);
        if P > 0 then
          Lib := Copy(Lib, 1, P - 1);
        StringChangeEx(Lib, '\\', '\', True);
        Result := SteamLibraryGame(Lib);
        if Result <> '' then
          Exit;
      end;
    end;
end;

function FindGogGame(): String;
var
  Names: TArrayOfString;
  I: Integer;
  Dir: String;
begin
  Result := '';
  if RegGetSubkeyNames(HKLM32, 'SOFTWARE\GOG.com\Games', Names) then
    for I := 0 to GetArrayLength(Names) - 1 do
      if RegQueryStringValue(HKLM32, 'SOFTWARE\GOG.com\Games\' + Names[I], 'path', Dir) and IsGameDir(Dir) then
      begin
        Result := Dir;
        Exit;
      end;
end;

function GetGameDir(Param: String): String;
begin
  Result := FindSteamGame();
  if Result = '' then
    Result := FindGogGame();
  if Result = '' then
    Result := ExpandConstant('{autopf32}\Steam\steamapps\common\Assassins Creed');
end;

function IsSteamInstall(): Boolean;
begin
  Result := Pos('\steamapps\common\', Lowercase(ExpandConstant('{app}'))) > 0;
end;

{ ---------- DirectX ---------- }

function NeedsDirectX(): Boolean;
var
  XInput: String;
begin
  if IsWin64 then
    XInput := ExpandConstant('{syswow64}\xinput1_3.dll')
  else
    XInput := ExpandConstant('{sys}\xinput1_3.dll');
  Result := (not FileExists(XInput)) and FileExists(ExpandConstant('{app}\DirectX\DXSETUP.exe'));
end;

{ ---------- downloads ---------- }

{ Unzips with Windows' own tar.exe (Windows 10 1803+), or PowerShell as a fallback. }
function Unzip(const Zip, Dest: String): Boolean;
var
  Tar: String;
  Code: Integer;
begin
  ForceDirectories(Dest);
  if IsWin64 then
    Tar := ExpandConstant('{sysnative}\tar.exe')
  else
    Tar := ExpandConstant('{sys}\tar.exe');
  Result := FileExists(Tar) and Exec(Tar, '-xf "' + Zip + '" -C "' + Dest + '"', '', SW_HIDE, ewWaitUntilTerminated, Code) and (Code = 0);
  if not Result then
    Result := Exec('powershell.exe', '-NoProfile -NonInteractive -Command "Expand-Archive -Force -LiteralPath ''' + Zip +
                   ''' -DestinationPath ''' + Dest + '''"', '', SW_HIDE, ewWaitUntilTerminated, Code) and (Code = 0);
end;

procedure InitializeWizard;
begin
  DownloadPage := CreateDownloadPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc), nil);
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if CurPageID = wpSelectDir then
  begin
    if not IsGameDir(WizardDirValue) then
    begin
      MsgBox('That folder doesn''t contain ' + GameExe + '.' + #13#10#13#10 +
             'Click Browse and pick your Assassin''s Creed folder. In Steam: right-click the game > Manage > Browse local files.',
             mbError, MB_OK);
      Result := False;
    end
    else if (CompareText(GetMD5OfFile(AddBackslash(WizardDirValue) + GameExe), GameExeMD5) <> 0) and
            (MsgBox('Your ' + GameExe + ' has been modified, or it''s a version EaglePatch doesn''t support ' +
                    '(for example the Ubisoft Connect version).' + #13#10#13#10 +
                    'The CPU crash fix will still work, but Xbox controller support won''t load until you restore the ' +
                    'original exe. In Steam: right-click the game > Properties > Installed Files > Verify integrity of game files.' +
                    #13#10#13#10 + 'Install anyway?', mbConfirmation, MB_YESNO) = IDNO) then
      Result := False;
  end
  else if CurPageID = wpReady then
  begin
    DownloadPage.Clear;
    DownloadPage.Add(AsiUrl, 'Ultimate-ASI-Loader.zip', AsiSha256);
    DownloadPage.Add(EpUrl, 'EaglePatchAC1.zip', EpSha256);
    DownloadPage.Show;
    try
      try
        DownloadPage.Download;
        if not (Unzip(ExpandConstant('{tmp}\Ultimate-ASI-Loader.zip'), ExpandConstant('{tmp}\asi')) and
                Unzip(ExpandConstant('{tmp}\EaglePatchAC1.zip'), ExpandConstant('{tmp}\ep')) and
                FileExists(ExpandConstant('{tmp}\asi\dinput8.dll')) and
                FileExists(ExpandConstant('{tmp}\ep\EaglePatchAC1.asi'))) then
          RaiseException('Couldn''t unzip the downloaded files.');
      except
        if DownloadPage.AbortedByUser then
          Log('Download cancelled.')
        else
          SuppressibleMsgBox('Couldn''t download EaglePatch or Ultimate ASI Loader from GitHub.' + #13#10#13#10 +
                             AddPeriod(GetExceptionMessage) + #13#10#13#10 +
                             'Check your internet connection and try again.', mbCriticalError, MB_OK, IDOK);
        Result := False;
      end;
    finally
      DownloadPage.Hide;
    end;
  end;
end;

{ ---------- install / uninstall ---------- }

procedure CurStepChanged(CurStep: TSetupStep);
var
  Dll, Ini: String;
begin
  if CurStep = ssInstall then
  begin
    { keep a different dinput8.dll the user already had (another mod), and put it back on uninstall }
    Dll := ExpandConstant('{app}\dinput8.dll');
    if FileExists(Dll) and not FileExists(Dll + '.bak') and
       (GetSHA256OfFile(Dll) <> GetSHA256OfFile(ExpandConstant('{tmp}\asi\dinput8.dll'))) then
      RenameFile(Dll, Dll + '.bak');
  end
  else if CurStep = ssPostInstall then
  begin
    Ini := ExpandConstant('{app}\scripts\EaglePatchAC1.ini');
    if WizardIsTaskSelected('skipintro') then
      SetIniString('EaglePatchAC1', 'SkipIntroVideos', '1', Ini)
    else
      SetIniString('EaglePatchAC1', 'SkipIntroVideos', '0', Ini);
    if WizardIsTaskSelected('ps3controls') then
      SetIniString('EaglePatchAC1', 'PS3Controls', '1', Ini)
    else
      SetIniString('EaglePatchAC1', 'PS3Controls', '0', Ini);
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Dll: String;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    Dll := ExpandConstant('{app}\dinput8.dll');
    if FileExists(Dll + '.bak') and not FileExists(Dll) then
      RenameFile(Dll + '.bak', Dll);
  end;
end;
