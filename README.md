# dji-mic-dictation-button

Turn the Link button on a DJI Mic into a dictation hotkey on macOS. No Karabiner, no kernel extension, no Accessibility permission.

Press the button, dictation starts. Press again, it stops. Tested with Willow Voice. Works with Raycast, Superwhisper, Wispr Flow, or anything else that binds a hotkey.

## How it works

When you press the Link button, the DJI USB receiver (Wireless Mic Rx, USB `2CA3:4011`) sends a HID Consumer Control event `0x0C/0xE9`, which macOS reads as volume up. Igor Bedesqui [found this](https://x.com/bedesqui/status/2098541184272544225).

`dji-mic-remap` is a tiny daemon that uses Apple's built-in `hidutil` to remap that one usage, on that one device, to **Right Control**. Your keyboard is untouched. The daemon runs as a LaunchAgent and re-applies the mapping at login and every time the receiver is plugged in, because `hidutil` mappings are per device instance and vanish on replug.

Right Control was chosen because MacBook keyboards do not have one, so it collides with nothing, and dictation apps accept a lone modifier as a hotkey. Willow rejects a bare F18.

## Install

Needs Xcode Command Line Tools (`xcode-select --install`).

```sh
git clone https://github.com/jonnilundy/dji-mic-dictation-button.git
cd dji-mic-dictation-button
./install.sh
```

Then, in your dictation app, add a hotkey and press the Link button while recording. It shows up as Right Control (⌃). In Willow Voice: Settings → Shortcuts → Hands-Free Mode Hotkey → Add another.

## Uninstall

```sh
./uninstall.sh
```

## Change the key

Edit the destination in `dji-mic-remap.c` and rerun `./install.sh`. The value is `0x700000000 + HID keyboard usage`. Some options:

| Key | Destination |
|---|---|
| Right Control | `0x7000000E4` |
| Right Option | `0x7000000E6` |
| Right Command | `0x7000000E7` |
| F18 | `0x70000006D` |

Different DJI receiver? Find its IDs with `hidutil list | grep -i "mic rx"` and change `VendorID`/`ProductID` in `dji-mic-remap.c`, `install.sh`, `uninstall.sh`, and the plist template.

## Files

- `dji-mic-remap.c` the daemon. ~70 lines of C, only `hidutil` and launchd's IOKit event stream.
- `com.dji-mic-remap.plist.template` LaunchAgent, rendered with your home path by `install.sh`.
- Log at `~/.local/state/log/dji-mic-remap.log`.
