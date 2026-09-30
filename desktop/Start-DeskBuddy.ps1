# Windows PowerShell 5.1, Windows 10 1809 or later. Original app: MIT.
[CmdletBinding()]
param([string]$Port, [switch]$Check)
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
    if (-not $task.Wait(3000)) { throw 'Windows media request timed out.' }
    return $task.Result
}
function Convert-MediaText([string]$Value) {
    # The embedded font contains printable ASCII only; keep framing characters out.
    $text = $Value.Normalize([Text.NormalizationForm]::FormD) -replace '\p{Mn}', ''
    $text = $text -replace '[^\x20-\x7e]', '?'
    return $text.Substring(0, [Math]::Min(63, $text.Length))
}
if ($Check) {
    $manager = Await-WinRT ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager]::RequestAsync()) ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager])
    $sessions = @($manager.GetSessions() | Where-Object { $_.SourceAppUserModelId -match 'Spotify' })
    Write-Host "Windows media API available. Spotify sessions: $($sessions.Count)"
    foreach ($session in $sessions) {
        $properties = Await-WinRT ($session.TryGetMediaPropertiesAsync()) ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionMediaProperties])
        $info = $session.GetPlaybackInfo()
        $timeline = $session.GetTimelineProperties()
        Write-Host "Metadata readable; status=$($info.PlaybackStatus); seek=$($info.Controls.IsPlaybackPositionEnabled); toggle=$($info.Controls.IsPlayPauseToggleEnabled)"
    }
    return
}
if (-not $Port) { throw 'Specify -Port COM3 (use your board port), or -Check for a read-only API check.' }
$serial = $null
$token = Get-Random -Minimum 1 -Maximum 2000000000
$previous = ''
$lastSend = [DateTime]::MinValue
Write-Host "Pip bridge on $Port. Close Serial Monitor first. Ctrl+C to stop."
try {
    $manager = Await-WinRT ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager]::RequestAsync()) ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager])
    while ($true) {
        try {
            if ($null -eq $serial) {
                $serial = New-Object System.IO.Ports.SerialPort $Port, 115200, None, 8, One
                $serial.NewLine = "`n"
                $serial.ReadTimeout = 50
                $serial.WriteTimeout = 500
                $serial.DtrEnable = $false
                $serial.RtsEnable = $false
                $serial.Open()
                $lastSend = [DateTime]::MinValue
                Write-Host 'USB connected. Waiting for Spotify media session.'
            }
            $sessions = @($manager.GetSessions() | Where-Object { $_.SourceAppUserModelId -match 'Spotify' })
            $session = $sessions | Where-Object { $_.GetPlaybackInfo().PlaybackStatus.ToString() -eq 'Playing' } | Select-Object -First 1
            if ($null -eq $session) { $session = $sessions | Select-Object -First 1 }
            $playing = 0; $seek = 0; $toggle = 0; $position = 0L; $duration = 0L
            $title = ''; $artist = ''; $album = ''; $identity = ''
            if ($null -ne $session) {
                $properties = Await-WinRT ($session.TryGetMediaPropertiesAsync()) ([Windows.Media.Control.GlobalSystemMediaTransportControlsSessionMediaProperties])
                $info = $session.GetPlaybackInfo()
                $timeline = $session.GetTimelineProperties()
                $title = Convert-MediaText $properties.Title
                $artist = Convert-MediaText $properties.Artist
                $album = Convert-MediaText $properties.AlbumTitle
                $identity = @($session.SourceAppUserModelId, $properties.Title, $properties.Artist, $properties.AlbumTitle, $timeline.EndTime.Ticks) -join "`t"
                $playing = [int]($info.PlaybackStatus.ToString() -eq 'Playing')
                $duration = [long][Math]::Max(0, ($timeline.EndTime - $timeline.StartTime).TotalMilliseconds)
                $position = [long]($timeline.Position - $timeline.StartTime).TotalMilliseconds
                if ($playing) {
                    $position += [long][Math]::Max(0, ([DateTimeOffset]::Now - $timeline.LastUpdatedTime).TotalMilliseconds)
                }
                $position = [long][Math]::Max(0, [Math]::Min($duration, $position))
                $seek = [int]($info.Controls.IsPlaybackPositionEnabled -and $duration -gt 0)
                $toggle = [int]$info.Controls.IsPlayPauseToggleEnabled
            }
            if ($identity -ne $previous) {
                $token = ($token + 1) % 2147483647
                $previous = $identity
            }
            # Read only complete, bounded commands; ignore firmware startup/performance logs.
            $processed = 0
            while ($serial.BytesToRead -gt 0 -and $processed -lt 16) {
                try { $command = $serial.ReadLine().TrimEnd("`r") } catch [TimeoutException] { break }
                $processed++
                if ($command.Length -gt 80 -or $null -eq $session) { continue }
                $parts = $command.Split("`t")
                if ($parts.Count -lt 2 -or $parts[1] -cne [string]$token) { continue }
                $accepted = $true
                if ($parts.Count -eq 2 -and $parts[0] -eq 'PIP_TOGGLE' -and $toggle) {
                    $accepted = Await-WinRT ($session.TryTogglePlayPauseAsync()) ([bool])
                } elseif ($parts.Count -eq 3 -and $parts[0] -eq 'PIP_SEEK' -and $seek) {
                    $requested = 0L
                    if ([long]::TryParse($parts[2], [ref]$requested) -and $requested -ge 0 -and $requested -le $duration) {
                        $ticks = $timeline.StartTime.Ticks + $requested * 10000L
                        $accepted = Await-WinRT ($session.TryChangePlaybackPositionAsync($ticks)) ([bool])
                    }
                }
                if (-not $accepted) { Write-Warning 'Spotify declined the playback control request.' }
            }
            if (([DateTime]::UtcNow - $lastSend).TotalMilliseconds -ge 1000) {
                $packet = @('PIP1', [int]($null -ne $session), $playing, $seek, $toggle, $token,
                            $position, $duration, $title, $artist, $album) -join "`t"
                $serial.WriteLine($packet)
                $lastSend = [DateTime]::UtcNow
            }
            Start-Sleep -Milliseconds 200
        } catch {
            Write-Warning "Bridge reconnecting: $($_.Exception.Message)"
            if ($null -ne $serial) { $serial.Dispose(); $serial = $null }
            Start-Sleep -Seconds 2
        }
    }
} finally {
    if ($null -ne $serial) { $serial.Dispose() }
}
