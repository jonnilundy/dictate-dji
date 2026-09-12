#!/bin/zsh
# Build, install, and (re)load the DJI mic remap daemon. Idempotent. Needs Xcode Command Line Tools (cc).
set -euo pipefail
cd "$(dirname "$0")"
LABEL=com.dji-mic-remap
mkdir -p ~/.local/libexec ~/.local/state/log
cc -std=c11 -fblocks -Wall -Wextra -Werror dji-mic-remap.c -o /tmp/dji-mic-remap.$$
install -m 0755 /tmp/dji-mic-remap.$$ ~/.local/libexec/dji-mic-remap; rm -f /tmp/dji-mic-remap.$$
sed "s#__HOME__#$HOME#g" $LABEL.plist.template > ~/Library/LaunchAgents/$LABEL.plist
launchctl bootout gui/$(id -u)/$LABEL 2>/dev/null || true
launchctl bootstrap gui/$(id -u) ~/Library/LaunchAgents/$LABEL.plist
launchctl kickstart -k gui/$(id -u)/$LABEL
sleep 1
echo "--- agent"; launchctl print gui/$(id -u)/$LABEL | grep -E "state" | head -1
echo "--- mapping on receiver (empty if not plugged in)"; hidutil property --matching '{"VendorID":11427,"ProductID":16401}' --get UserKeyMapping
echo "--- log"; tail -3 ~/.local/state/log/dji-mic-remap.log
