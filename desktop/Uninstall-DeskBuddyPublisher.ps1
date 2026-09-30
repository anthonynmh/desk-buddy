# Removes Desk Buddy Publisher for the current Windows user. Original app: MIT.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Unregister-ScheduledTask -TaskName 'Desk Buddy Publisher' -Confirm:$false -ErrorAction SilentlyContinue
$install = Join-Path $env:LOCALAPPDATA 'DeskBuddy'
if (Test-Path -LiteralPath $install) { Remove-Item -LiteralPath $install -Recurse -Force }
Write-Host 'Desk Buddy Publisher was removed.'
