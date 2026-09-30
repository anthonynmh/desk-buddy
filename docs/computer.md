# Desk Buddy on Wi-Fi

Pip connects to your **2.4 GHz Wi-Fi** and polls a small Windows companion app
on your local network. The app, **Desk Buddy Publisher**, reads Spotify's
Windows media session and accepts Pip's play/pause and seek requests. Pip can
then stay powered from any USB supply; UART1 and a COM port are only needed for
firmware upload and diagnostics.

The publisher is necessary. An ESP32 cannot see which Windows apps are running
or read Spotify playback by itself. This first version publishes Spotify state,
metadata, a connection heartbeat, and playback controls. It is the foundation
for later computer and application events.

## One-time setup

1. Upload the Wi-Fi firmware using [the existing board setup](setup.md). You do
   not need to leave the UART1 cable connected afterward.
2. Choose a pairing key of 16–32 uppercase letters. For example,
   `MINTYDESKBUDDYPALS`. Keep it private: Pip and the publisher must use the
   exact same value.
3. In Windows PowerShell, from the repository root, install the publisher for
   your current user:

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\desktop\Install-DeskBuddyPublisher.ps1 -PairingKey MINTYDESKBUDDYPALS
   ```

   The installer copies the publisher to `%LOCALAPPDATA%\DeskBuddy`, stores the
   pairing key there, starts it, and registers the **Desk Buddy Publisher** task
   to launch it when you sign in. It does not need administrator access.
4. Find your Windows computer's **IPv4** address on the same Wi-Fi network:

   ```powershell
   ipconfig
   ```

   Use the IPv4 address under your connected Wi-Fi or Ethernet adapter, such as
   `192.168.1.25`. Do not use a Bluetooth, VPN, `127.0.0.1`, or IPv6 address.
5. On Pip, tap **SET UP WI-FI** in the top-right corner. Pip scans automatically.
   Tap **Nearby Wi-Fi**, select your network, enter its password, and tap
   **Connect**. Open networks do not need a password. **Refresh** scans again;
   **Previous/Next** page through up to 12 networks, strongest first. Hidden
   networks are not listed. Wi-Fi connects without publisher details.
   Then open **Computer pairing**, enter the publisher IPv4 address, pairing
   key and port `8791`, and tap **Save pairing**. **Back** closes the dropdown,
   returns from pairing to Wi-Fi, or returns from Wi-Fi to Pip. Unsaved edits
   do not change the active connection. The keyboard cycles
   `ABC` → `123` → `abc` → `#+=`; it also has space, period, dash, and delete.
6. If Windows Defender Firewall asks, allow the publisher on **Private**
   networks. If no prompt appears, run the installed publisher once in a visible
   PowerShell window, then allow it:

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File $env:LOCALAPPDATA\DeskBuddy\Start-DeskBuddyPublisher.ps1
   ```

   Leave that window open only for troubleshooting; the installed sign-in task
   normally keeps the publisher running in the background.
7. Start Spotify desktop playback. The music panel should show the song,
   artist, album, progress, and active controls; Pip dances while Spotify plays.

If Pip says it cannot connect, confirm the board and computer are on the same
LAN, the router does not isolate wireless clients, the IP/key match, and the
publisher task is running. Reopen Wi‑Fi settings to correct any value or tap
**Forget** to erase the saved settings from Pip.

## Controls and privacy

- The slider seeks after you release it. Play/Pause sends a request only when
  Spotify reports that control as supported.
- The publisher exposes `GET /v1/state` and `POST /v1/command` on port 8791.
  Every request needs the pairing key in `X-Desk-Buddy-Key`; malformed and
  oversized requests are rejected. The key provides local access control, but
  the traffic is plain HTTP. Use a trusted private LAN, not a public network.
- Wi-Fi name, password, publisher IP, key, and port are stored in the ESP32's
  NVS storage. The publisher's key and port are stored in the current user's
  `%LOCALAPPDATA%\DeskBuddy\publisher.json`.
- The current publisher intentionally filters to the Spotify Windows media
  session. It has no cloud account, Spotify developer token, telemetry, remote
  service, or activity history. Browser Spotify and album artwork are outside
  this release.
- The publisher starts when you sign in. To remove it, run:

  ```powershell
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\desktop\Uninstall-DeskBuddyPublisher.ps1
  ```

## Physical acceptance checks

Test initial Wi-Fi setup, wrong password, a changed IP/key, Wi-Fi loss and
recovery, stopping/restarting the Windows publisher, Spotify paused/playing,
seek forward/back, a song change during a drag, and a song where seeking is not
available. Also check tap/wave, brushing, mood buttons, waking from Nap, and
returning to the selected mood after playback pauses. Host tests and compilation
cannot establish radio performance, firewall behavior, or touchscreen behavior
on your hardware.
