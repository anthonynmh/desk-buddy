# Spotify on your desk

Pip can accompany music playing in the **Spotify Windows desktop app**.
A small Windows PowerShell helper reads the local Windows media session and
sends its current state over the board's existing UART1 USB connection.
No Spotify developer account, cloud service, Wi-Fi credentials, or additional
PowerShell modules are needed. The helper runs only while its terminal is open.

## Run

1. Build and upload the firmware using [the existing board setup](setup.md).
2. Leave the data cable and physical UART switch on UART1. Close Arduino Serial
   Monitor: the helper needs exclusive access to that COM port.
3. Open **Windows PowerShell 5.1** in the repository root and run:

   ```powershell
   powershell.exe -NoProfile -File .\desktop\Start-DeskBuddy.ps1 -Port COM3
   ```

   Replace COM3 with the board's current port. If Windows blocks a downloaded
   script, review it and unblock that file using its Properties dialog.
4. Open Spotify and play a song. Pip dances while playback is active, including
   when the selected mood is Nap. Pausing returns Pip to the selected mood.
5. Tap Play/Pause or drag the timeline; seeking is sent once after release.
   Controls are enabled only when the media session reports support.
6. Stop the helper with Ctrl+C before uploading again. When the helper stops or
   USB disconnects, Pip returns to standalone operation within about four seconds.

For a read-only Windows API check without opening the serial port:

```powershell
powershell.exe -NoProfile -File .\desktop\Start-DeskBuddy.ps1 -Check
```

## Behavior and limits

- The panel shows title, artist, **album name**, progress, and playback status.
  Cover artwork and volume control are not implemented. The slider seeks.
- Dancing is a looping animation driven by playback state; it is not beat detection.
- An open Spotify process without an exposed media session shows “Waiting for
  Spotify.” Metadata and seeking depend on Spotify's Windows media integration.
  The helper filters Spotify sessions; browser playback is outside this version.
- Text is limited to 63 printable ASCII characters per field. Accented Latin
  letters are simplified; unsupported characters become `?`. Long visible text
  is shortened to fit. This follows the existing embedded font's character set.
- Metadata is sent once per second; the board interpolates progress while playing.
  Commands are requests: the next playback snapshot determines displayed state.
  Declined requests produce a warning in the helper terminal.
- Automatic reconnect handles temporary serial failures. If the COM port changes,
  restart the helper with the new port. There is no activity history or foreground
  app tracking in this first computer integration.

Windows API reference: [media sessions](https://learn.microsoft.com/en-us/uwp/api/windows.media.control.globalsystemmediatransportcontrolssession)
and [supported playback controls](https://learn.microsoft.com/en-us/uwp/api/windows.media.control.globalsystemmediatransportcontrolssessionplaybackcontrols).

## Protocol

115200 baud, newline-delimited ASCII, tab-separated fields:

```text
PIP1 available playing canSeek canToggle token positionMs durationMs title artist album
PIP_SEEK token positionMs
PIP_TOGGLE token
```

Spaces above stand for tabs. Empty metadata fields still need separators.
The helper changes the token when the session/song identity changes. A drag
captured on an earlier token is cancelled. Unsigned numeric fields are bounded
to 32 bits; boolean fields must be 0 or 1. The firmware rejects malformed or
oversized lines, handles input in bounded chunks, and expires missing heartbeats.
The helper ignores firmware logs sharing the serial link.

## Physical acceptance checks

Play/pause, seek forward/back, change song during a drag, close Spotify, stop the
helper, unplug/reconnect USB, and restart the board. Check a song without seeking
support, long/empty/non-ASCII metadata, and album availability. Check mood buttons,
tap/wave, brush/hearts, waking from Nap, and return to Nap after pausing. Watch for
touch lag, clipping, tearing, or stale text during playback. Host tests and a
firmware build do not establish these physical results.
