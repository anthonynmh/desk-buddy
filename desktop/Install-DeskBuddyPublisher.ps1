# Installs Desk Buddy Publisher for the current Windows user. Original app: MIT.
[CmdletBinding()]
param([Parameter(Mandatory = $true)][ValidatePattern('^[A-Z]{16,32}$')][string]$PairingKey,
      [ValidateRange(1024, 65535)][int]$Port = 8791)
$ErrorActionPreference = 'Stop'
$install = Join-Path $env:LOCALAPPDATA 'DeskBuddy'
New-Item -ItemType Directory -Path $install -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'Start-DeskBuddyPublisher.ps1') -Destination $install -Force
@{ PairingKey = $PairingKey; Port = $Port } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $install 'publisher.json') -Encoding UTF8
$shell = "$env:SystemRoot\System32\WindowsPowerShell\v1.0\powershell.exe"
$action = New-ScheduledTaskAction -Execute $shell -Argument "-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File `"$install\Start-DeskBuddyPublisher.ps1`""
$trigger = New-ScheduledTaskTrigger -AtLogOn
Register-ScheduledTask -TaskName 'Desk Buddy Publisher' -Action $action -Trigger $trigger -Description 'Publishes local Spotify status to Desk Buddy.' -Force | Out-Null
Start-ScheduledTask -TaskName 'Desk Buddy Publisher'
Write-Host "Installed Desk Buddy Publisher. Set publisher IP to your computer's IPv4 address, port $Port, and pairing key $PairingKey on Pip."
