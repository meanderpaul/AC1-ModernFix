; ManyCoreFix installer, built with Inno Setup 6 (https://jrsoftware.org/isinfo.php).
; Build: ISCC.exe installer\ManyCoreFix.iss   (after ./build.sh has made dist\ManyCoreFix*.asi)
;
; What it does:
;   - the user picks the game's .exe; the installer reads whether it's a 32-bit or 64-bit game
;   - downloads the matching Ultimate ASI Loader v4.68 from its official GitHub release (SHA-256 checked)
;   - installs it under the DLL name the user picks (dinput8.dll by default), plus ManyCoreFix(64).asi
;     and its ini in the game's scripts folder, with the MaxCPUs the user picks
;   - uninstalls from Windows "Installed apps" (removes it from every game it was installed to)
; Copyright (c) 2026 Meanderpaul_gaming. MIT License.

#define AppVersion "1.0.0"

[Setup]
AppId={{3D7F2B91-5C4E-4A86-B1D2-0C0FEC0FE001}
AppName=ManyCoreFix
AppVersion={#AppVersion}
AppVerName=ManyCoreFix {#AppVersion} (beta)
AppPublisher=Meanderpaul_gaming
AppPublisherURL=https://github.com/meanderpaul/AC1-ModernFix
AppSupportURL=https://github.com/meanderpaul/AC1-ModernFix/blob/main/docs/MANYCOREFIX.md
; the real folder comes from the game exe the user picks (see NextButtonClick)
DefaultDirName={autopf}\ManyCoreFix
DisableDirPage=yes
UsePreviousAppDir=no
DisableProgramGroupPage=yes
PrivilegesRequired=admin
UninstallFilesDir={commonappdata}\ManyCoreFix
UninstallDisplayName=ManyCoreFix
UninstallDisplayIcon={uninstallexe}
OutputDir=..\dist
OutputBaseFilename=ManyCoreFix-Setup
SetupIconFile=app.ico
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
MinVersion=10.0
VersionInfoVersion={#AppVersion}
VersionInfoCompany=Meanderpaul_gaming
VersionInfoDescription=ManyCoreFix Setup
VersionInfoCopyright=(c) 2026 Meanderpaul_gaming. MIT License.

[Files]
Source: "..\dist\ManyCoreFix.asi"; DestDir: "{app}\scripts"; Check: not Is64Game; Flags: ignoreversion
Source: "..\dist\ManyCoreFix64.asi"; DestDir: "{app}\scripts"; Check: Is64Game; Flags: ignoreversion
Source: "..\release\ManyCoreFix.ini"; DestDir: "{app}\scripts"; Check: not Is64Game; Flags: onlyifdoesntexist
Source: "..\release\ManyCoreFix.ini"; DestDir: "{app}\scripts"; DestName: "ManyCoreFix64.ini"; Check: Is64Game; Flags: onlyifdoesntexist
; Ultimate ASI Loader, downloaded and unzipped into {tmp}\asi on the Ready page, installed under the chosen name
Source: "{tmp}\asi\dinput8.dll"; DestDir: "{app}"; DestName: "dinput8.dll"; Check: ProxyIs(0); Flags: external ignoreversion
Source: "{tmp}\asi\dinput8.dll"; DestDir: "{app}"; DestName: "version.dll"; Check: ProxyIs(1); Flags: external ignoreversion
Source: "{tmp}\asi\dinput8.dll"; DestDir: "{app}"; DestName: "winmm.dll"; Check: ProxyIs(2); Flags: external ignoreversion
Source: "{tmp}\asi\dinput8.dll"; DestDir: "{app}"; DestName: "d3d9.dll"; Check: ProxyIs(3); Flags: external ignoreversion

[UninstallDelete]
Type: dirifempty; Name: "{app}\scripts"

[Run]
Filename: "{code:GameExePath}"; WorkingDir: "{app}"; Description: "Start the game now"; Flags: postinstall nowait skipifsilent runasoriginaluser unchecked

[Code]
const
  Asi32Url = 'https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v4.68/Ultimate-ASI-Loader.zip';
  Asi32Sha256 = '7517c7d5bd8c475f18e6545b7998df379eed6e2e0a19fcd2139669152f9bcddb';
  Asi64Url = 'https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v4.68/Ultimate-ASI-Loader_x64.zip';
  Asi64Sha256 = 'b4d08398c60498cb8772763aefc8db100d0309bed267850731a0dabc9f87e171';
  ProxyNames = 'dinput8.dll,version.dll,winmm.dll,d3d9.dll';

var
  ExePage: TInputFileWizardPage;
  CpuPage: TInputQueryWizardPage;
  ProxyPage: TInputOptionWizardPage;
  DownloadPage: TDownloadWizardPage;
  Game64: Boolean;

{ ---------- the game ---------- }

{ Reads the PE header: 0 = not a Windows exe, 32 or 64. }
function ExeBits(const FileName: String): Integer;
var
  S: TFileStream;
  Buf: AnsiString;
  PeOff, Machine: Integer;
begin
  Result := 0;
  try
    S := TFileStream.Create(FileName, fmOpenRead or fmShareDenyNone);
    try
      SetLength(Buf, 64);
      S.ReadBuffer(Buf, 64);
      if Copy(Buf, 1, 2) <> 'MZ' then
        Exit;
      PeOff := Ord(Buf[61]) + Ord(Buf[62]) * 256 + Ord(Buf[63]) * 65536;
      S.Seek(PeOff, soFromBeginning);
      SetLength(Buf, 6);
      S.ReadBuffer(Buf, 6);
      if Copy(Buf, 1, 4) <> ('PE' + #0#0) then
        Exit;
      Machine := Ord(Buf[5]) + Ord(Buf[6]) * 256;
      if Machine = $14C then
        Result := 32
      else if Machine = $8664 then
        Result := 64;
    finally
      S.Free;
    end;
  except
    Result := 0;
  end;
end;

function Is64Game(): Boolean;
begin
  Result := Game64;
end;

function GameExePath(Param: String): String;
begin
  Result := ExePage.Values[0];
end;

function ProxyIs(Index: Integer): Boolean;
begin
  Result := ProxyPage.SelectedValueIndex = Index;
end;

function ProxyName(): String;
var
  Names: TArrayOfString;
begin
  Names := StringSplit(ProxyNames, [','], stAll);
  Result := Names[ProxyPage.SelectedValueIndex];
end;

{ ---------- wizard ---------- }

procedure InitializeWizard;
begin
  ExePage := CreateInputFilePage(wpWelcome, 'Choose the game',
    'Which game keeps crashing on your many-core CPU?',
    'Pick the game''s main .exe file (the one in the game''s install folder, not a launcher). ' +
    'In Steam: right-click the game > Manage > Browse local files.');
  ExePage.Add('Game .exe:', 'Programs (*.exe)|*.exe', '.exe');

  CpuPage := CreateInputQueryPage(ExePage.ID, 'How many CPU threads?',
    'The game will use, and see, at most this many CPU threads.',
    'Start with 8. If the game still crashes, run this installer again with 4, then 2.' + #13#10#13#10 +
    'Suggested starting points: Prototype 4, Far Cry 2 and 3 8, Rocksmith 2014 8, ' +
    'Medal of Honor: Airborne 16, Child of Light 8, Fallout 3 2.');
  CpuPage.Add('MaxCPUs (1-64):', False);
  CpuPage.Values[0] := '8';

  ProxyPage := CreateInputOptionPage(CpuPage.ID, 'Loader file name',
    'How should the plugin loader be named?',
    'Leave this on dinput8.dll unless the game never writes ManyCoreFix.log in its scripts folder. ' +
    'Then run the installer again and try the next name.', True, False);
  ProxyPage.Add('dinput8.dll (works for most games)');
  ProxyPage.Add('version.dll');
  ProxyPage.Add('winmm.dll');
  ProxyPage.Add('d3d9.dll (DirectX 9 games only)');
  ProxyPage.SelectedValueIndex := 0;

  DownloadPage := CreateDownloadPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc), nil);
end;

{ Unzips with Windows' own tar.exe, or PowerShell as a fallback. }
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

function NextButtonClick(CurPageID: Integer): Boolean;
var
  Bits, N: Integer;
begin
  Result := True;
  if CurPageID = ExePage.ID then
  begin
    Bits := ExeBits(ExePage.Values[0]);
    if Bits = 0 then
    begin
      MsgBox('That doesn''t look like a Windows game .exe. Pick the game''s main .exe file.', mbError, MB_OK);
      Result := False;
    end
    else
    begin
      Game64 := Bits = 64;
      WizardForm.DirEdit.Text := ExtractFileDir(ExePage.Values[0]);
    end;
  end
  else if CurPageID = CpuPage.ID then
  begin
    N := StrToIntDef(Trim(CpuPage.Values[0]), 0);
    if (N < 1) or (N > 64) then
    begin
      MsgBox('Enter a number from 1 to 64.', mbError, MB_OK);
      Result := False;
    end;
  end
  else if CurPageID = wpReady then
  begin
    DownloadPage.Clear;
    if Game64 then
      DownloadPage.Add(Asi64Url, 'asi.zip', Asi64Sha256)
    else
      DownloadPage.Add(Asi32Url, 'asi.zip', Asi32Sha256);
    DownloadPage.Show;
    try
      try
        DownloadPage.Download;
        if not (Unzip(ExpandConstant('{tmp}\asi.zip'), ExpandConstant('{tmp}\asi')) and
                FileExists(ExpandConstant('{tmp}\asi\dinput8.dll'))) then
          RaiseException('Couldn''t unzip the downloaded file.');
      except
        if DownloadPage.AbortedByUser then
          Log('Download cancelled.')
        else
          SuppressibleMsgBox('Couldn''t download Ultimate ASI Loader from GitHub.' + #13#10#13#10 +
                             AddPeriod(GetExceptionMessage) + #13#10#13#10 +
                             'Check your internet connection and try again.', mbCriticalError, MB_OK, IDOK);
        Result := False;
      end;
    finally
      DownloadPage.Hide;
    end;
  end;
end;

function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo,
  MemoGroupInfo, MemoTasksInfo: String): String;
var
  Bits: String;
begin
  if Game64 then Bits := '64-bit' else Bits := '32-bit';
  Result := 'Game:' + NewLine + Space + ExePage.Values[0] + ' (' + Bits + ')' + NewLine + NewLine +
            'MaxCPUs:' + NewLine + Space + Trim(CpuPage.Values[0]) + NewLine + NewLine +
            'Loader file:' + NewLine + Space + ProxyName() + NewLine + NewLine +
            'Setup will download Ultimate ASI Loader from GitHub, then install it with ManyCoreFix ' +
            'into the game folder.';
end;

{ ---------- install / uninstall ---------- }

procedure CurStepChanged(CurStep: TSetupStep);
var
  Dll, Ini: String;
begin
  if CurStep = ssInstall then
  begin
    { keep a different DLL of the same name the game already had, and put it back on uninstall }
    Dll := ExpandConstant('{app}\') + ProxyName();
    if FileExists(Dll) and not FileExists(Dll + '.bak') and
       (GetSHA256OfFile(Dll) <> GetSHA256OfFile(ExpandConstant('{tmp}\asi\dinput8.dll'))) then
      RenameFile(Dll, Dll + '.bak');
  end
  else if CurStep = ssPostInstall then
  begin
    if Game64 then
      Ini := ExpandConstant('{app}\scripts\ManyCoreFix64.ini')
    else
      Ini := ExpandConstant('{app}\scripts\ManyCoreFix.ini');
    SetIniString('ManyCoreFix', 'MaxCPUs', Trim(CpuPage.Values[0]), Ini);
    SetIniString('ManyCoreFix', 'Enabled', '1', Ini);
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Names: TArrayOfString;
  I: Integer;
  Dll: String;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    Names := StringSplit(ProxyNames, [','], stAll);
    for I := 0 to GetArrayLength(Names) - 1 do
    begin
      Dll := ExpandConstant('{app}\') + Names[I];
      if FileExists(Dll + '.bak') and not FileExists(Dll) then
        RenameFile(Dll + '.bak', Dll);
    end;
  end;
end;
