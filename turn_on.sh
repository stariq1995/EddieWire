rfkill unblock bluetooth
systemctl start connman
connmanctl enable bluetooth
connmanctl enable wifi
echo "discoverable on" | bluetoothctl
