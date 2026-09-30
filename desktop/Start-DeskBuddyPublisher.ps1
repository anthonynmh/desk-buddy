# Desk Buddy Publisher for Windows PowerShell 5.1. Original app: MIT.
[CmdletBinding()]
param([string]$ConfigPath = (Join-Path $env:LOCALAPPDATA 'DeskBuddy\publisher.json'), [switch]$Check)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Runtime.WindowsRuntime
$null = [Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager, Windows.Media.Control, ContentType = WindowsRuntime]
$null = [Windows.Media.Control.GlobalSystemMediaTransportControlsSessionMediaProperties, Windows.Media.Control, ContentType = WindowsRuntime]
$script:asTask = [System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
    $_.Name -eq 'AsTask' -and $_.IsGenericMethod -and $_.GetParameters().Count -eq 1 -and
    $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1'
} | Select-Object -First 1
function Await-WinRT($Operation, [Type]$ResultType) {
    $task = $script:asTask.MakeGenericMethod($ResultType).Invoke($null, @($Operation))
    if (-not $task.Wait(2500)) { throw 'Windows media request timed out.' }
    return $task.Result
}
function Convert-MediaText([string]$Value) {
    $text = $Value.Normalize([Text.NormalizationForm]::FormD) -replace '\p{Mn}', ''
    $text = $text -replace '[^\x20-\x7e]', '?'
    return $text.Substring(0, [Math]::Min(63, $text.Length))
}
function Get-IPv4Address {
    [Net.NetworkInformation.NetworkInterface]::GetAllNetworkInterfaces() |
        Where-Object { $_.OperationalStatus -eq 'Up' } |
        ForEach-Object { $_.GetIPProperties().UnicastAddresses } |
        Where-Object { $_.Address.AddressFamily -eq [Net.Sockets.AddressFamily]::InterNetwork -and
                       -not $_.Address.ToString().StartsWith('127.') } |
        ForEach-Object { $_.Address.ToString() } | Select-Object -First 1
}
function Send-Response($Stream, [int]$Code, [string]$Body = '') {
    $reason = @{ 200 = 'OK'; 204 = 'No Content'; 400 = 'Bad Request'; 401 = 'Unauthorized'; 404 = 'Not Found'; 405 = 'Method Not Allowed' }[$Code]
    $bytes = [Text.Encoding]::ASCII.GetBytes($Body)
    $head = "HTTP/1.1 $Code $reason`r`nContent-Type: text/plain; charset=us-ascii`r`nContent-Length: $($bytes.Length)`r`nConnection: close`r`n`r`n"
    $headerBytes = [Text.Encoding]::ASCII.GetBytes($head)
    $Stream.Write($headerBytes, 0, $headerBytes.Length)
    if ($bytes.Length) { $Stream.Write($bytes, 0, $bytes.Length) }
}
function Get-MediaState {
    $sessions = @($script:manager.GetSessions() | Where-Object { $_.SourceAppUserModelId -match 'Spotify' })
    $session = $sessions | Where-Object { $_.GetPlaybackInfo().PlaybackStatus.ToString() -eq 'Playing' } | Select-Object -First 1
    if ($null -eq $session) { $session = $sessions | Select-Object -First 1 }
    $available = 0; $playing = 0; $seek = 0; $toggle = 0; $position = 0L; $duration = 0L
    $title = ''; $artist = ''; $album = ''; $identity = ''
    if ($null -ne $session) {
        $properties = Await-WinRT ($session.TryGetMediaPropertiesAsync()) ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionMediaProperties])
        $info = $session.GetPlaybackInfo(); $timeline = $session.GetTimelineProperties()
        $available = 1; $title = Convert-MediaText $properties.Title; $artist = Convert-MediaText $properties.Artist
        $album = Convert-MediaText $properties.AlbumTitle
        $identity = @($session.SourceAppUserModelId, $title, $artist, $album, $timeline.EndTime.Ticks) -join "`t"
        $playing = [int]($info.PlaybackStatus.ToString() -eq 'Playing')
        $duration = [long][Math]::Max(0, ($timeline.EndTime - $timeline.StartTime).TotalMilliseconds)
        $position = [long]($timeline.Position - $timeline.StartTime).TotalMilliseconds
        if ($playing) { $position += [long][Math]::Max(0, ([DateTimeOffset]::Now - $timeline.LastUpdatedTime).TotalMilliseconds) }
        $position = [long][Math]::Max(0, [Math]::Min($duration, $position))
        $seek = [int]($info.Controls.IsPlaybackPositionEnabled -and $duration -gt 0)
        $toggle = [int]$info.Controls.IsPlayPauseToggleEnabled
    }
    if ($identity -ne $script:identity) { $script:token = ($script:token + 1) % 2147483647; $script:identity = $identity }
    $script:session = $session; $script:duration = $duration; $script:seek = $seek; $script:toggle = $toggle
    return (@('PIP1', $available, $playing, $seek, $toggle, $script:token, $position, $duration,
              $title, $artist, $album) -join "`t") + "`n"
}
function Invoke-MediaCommand([string]$Body) {
    if ($null -eq $script:session) { return $false }
    if ($Body -match '^PIP_TOGGLE\t([0-9]{1,10})$' -and [uint32]$Matches[1] -eq $script:token -and $script:toggle) {
        return Await-WinRT ($script:session.TryTogglePlayPauseAsync()) ([bool])
    }
    if ($Body -match '^PIP_SEEK\t([0-9]{1,10})\t([0-9]{1,10})$' -and
        [uint32]$Matches[1] -eq $script:token -and $script:seek) {
        [int64]$position = 0
        if ([int64]::TryParse($Matches[2], [ref]$position) -and $position -ge 0 -and $position -le $script:duration) {
            $timeline = $script:session.GetTimelineProperties()
            return Await-WinRT ($script:session.TryChangePlaybackPositionAsync($timeline.StartTime.Ticks + $position * 10000L)) ([bool])
        }
    }
    return $false
}
function Handle-Client($Client, [string]$PairingKey, [string]$Packet) {
    try {
        $Client.ReceiveTimeout = 1000; $Client.SendTimeout = 1000
        $stream = $Client.GetStream()
        $reader = New-Object IO.StreamReader($stream, [Text.Encoding]::ASCII, $false, 8192, $true)
        $request = $reader.ReadLine()
        if ($null -eq $request -or $request.Length -gt 256 -or $request -notmatch '^([A-Z]+) ([^ ]+) HTTP/1\.[01]$') {
            Send-Response $stream 400; return
        }
        $method = $Matches[1]; $path = $Matches[2]; $headers = @{}; $lines = 0
        while ($true) {
            $line = $reader.ReadLine(); $lines++
            if ($null -eq $line -or $lines -gt 32 -or $line.Length -gt 512) { Send-Response $stream 400; return }
            if (!$line) { break }
            if ($line -match '^([^:]+):\s*(.*)$') { $headers[$Matches[1].ToLowerInvariant()] = $Matches[2] }
        }
        if ($headers['x-desk-buddy-key'] -cne $PairingKey) { Send-Response $stream 401; return }
        $length = 0
        if ($headers['content-length']) { if (![int]::TryParse($headers['content-length'], [ref]$length) -or $length -lt 0 -or $length -gt 80) { Send-Response $stream 400; return } }
        $body = if ($length) { [char[]]$reader.ReadChars($length) -join '' } else { '' }
        if ($method -eq 'GET' -and $path -eq '/v1/state') { Send-Response $stream 200 $Packet; return }
        if ($method -eq 'POST' -and $path -eq '/v1/command') {
            if (Invoke-MediaCommand $body) { Send-Response $stream 204 } else { Send-Response $stream 400 }; return
        }
        Send-Response $stream 404
    } catch { if ($stream) { try { Send-Response $stream 400 } catch {} } }
    finally { $Client.Close() }
}
if ($Check) {
    $script:manager = Await-WinRT ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager]::RequestAsync()) ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager])
    $script:token = 1; $script:identity = ''; $script:session = $null
    Write-Host (Get-MediaState).TrimEnd()
    exit
}
if (!(Test-Path -LiteralPath $ConfigPath)) { throw "Publisher config was not found: $ConfigPath. Run Install-DeskBuddyPublisher.ps1 first." }
$config = Get-Content -LiteralPath $ConfigPath -Raw | ConvertFrom-Json
if ([string]$config.PairingKey -notmatch '^[A-Z]{16,32}$' -or [int]$config.Port -lt 1024 -or [int]$config.Port -gt 65535) { throw 'Publisher config is invalid.' }
$script:manager = Await-WinRT ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager]::RequestAsync()) ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager])
$script:token = Get-Random -Minimum 1 -Maximum 2000000000; $script:identity = ''; $script:session = $null
$listener = [Net.Sockets.TcpListener]::new([Net.IPAddress]::Any, [int]$config.Port)
$listener.Start()
try {
    $ip = Get-IPv4Address
    Write-Host "Desk Buddy Publisher listening at http://${ip}:$($config.Port)"
    while ($true) {
        $packet = Get-MediaState
        while ($listener.Pending()) { Handle-Client ($listener.AcceptTcpClient()) $config.PairingKey $packet }
        Start-Sleep -Milliseconds 200
    }
} finally { $listener.Stop() }
