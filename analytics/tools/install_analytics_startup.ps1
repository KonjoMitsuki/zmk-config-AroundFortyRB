[CmdletBinding()]
param(
    [string]$Port,
    [string]$Directory,
    [ValidateRange(1, 86400)][int]$Interval = 10
)

$ErrorActionPreference = 'Stop'
$localRoot = Join-Path $env:LOCALAPPDATA 'AroundFortyAnalytics'
$portFile = Join-Path $localRoot 'port.txt'
if (-not $Port -and (Test-Path -LiteralPath $portFile -PathType Leaf)) {
    $Port = (Get-Content -LiteralPath $portFile -Raw).Trim()
}
if ($Port -notmatch '^COM[1-9][0-9]*$') {
    throw 'Specify the Analytics COM port, for example -Port COM7.'
}

if (-not $Directory) {
    $oneDriveRoots = @($env:OneDrive, $env:OneDriveConsumer, $env:OneDriveCommercial) |
        Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Container) } |
        Select-Object -Unique
    $existing = @($oneDriveRoots | ForEach-Object { Join-Path $_ 'AroundFortyAnalytics' } |
        Where-Object { Test-Path -LiteralPath (Join-Path $_ 'captures') -PathType Container })
    if ($existing.Count -eq 1) {
        $Directory = $existing[0]
    } elseif ($existing.Count -gt 1 -or -not $oneDriveRoots) {
        throw 'Specify the shared OneDrive folder with -Directory.'
    } else {
        $Directory = Join-Path @($oneDriveRoots)[0] 'AroundFortyAnalytics'
    }
}
$Directory = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Directory)
if (-not (Test-Path -LiteralPath (Split-Path -Parent $Directory) -PathType Container)) {
    throw "The parent directory does not exist: $Directory"
}

$launcher = (Get-Command py.exe -ErrorAction Stop).Source
$python = & $launcher -3 -c 'import serial, sys; print(sys.executable)'
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $python -PathType Leaf)) {
    throw 'Install pyserial first: py -3 -m pip install pyserial'
}

$runtime = Join-Path $localRoot 'recorder'
New-Item -ItemType Directory -Path $runtime -Force | Out-Null
foreach ($name in @('read_poc.py', 'record_recent.py')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination $runtime -Force
}

$startup = [Environment]::GetFolderPath('Startup')
$shortcutPath = Join-Path $startup 'AroundForty Analytics.lnk'
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut($shortcutPath)
$shortcut.TargetPath = $python
$recorder = Join-Path $runtime 'record_recent.py'
$shortcut.Arguments = ('-B "{0}" capture --port {1} --interval {2} --directory "{3}"' -f $recorder, $Port, $Interval, $Directory.TrimEnd('\'))
$shortcut.WorkingDirectory = $runtime
$shortcut.WindowStyle = 7
$shortcut.Description = 'Record AroundForty RB press history to OneDrive when signing in'
$shortcut.Save()

$installed = $shell.CreateShortcut($shortcutPath)
if ($installed.TargetPath -ne $python -or $installed.Arguments -ne $shortcut.Arguments) {
    throw 'The Startup shortcut could not be verified.'
}
Write-Output "Startup: $shortcutPath"
Write-Output "Recording: $Port every $Interval seconds"
Write-Output "OneDrive: $Directory"
