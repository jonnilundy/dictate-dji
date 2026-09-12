#!/bin/zsh
# Remove the daemon and clear the device remap. The Link button goes back to volume up.
LABEL=com.dji-mic-remap
launchctl bootout gui/$(id -u)/$LABEL 2>/dev/null || true
rm -f ~/Library/LaunchAgents/$LABEL.plist ~/.local/libexec/dji-mic-remap
hidutil property --matching '{"VendorID":11427,"ProductID":16401}' --set '{"UserKeyMapping":[]}' >/dev/null 2>&1 || true
echo "removed."
