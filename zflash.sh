echo
echo Device List
echo ===========
ls -l /dev/ttyA*
echo
nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application build_dongle/zephyr/zephyr.hex --application-version 1 zephyr.zip 
nrfutil dfu usb-serial -pkg zephyr.zip -p /dev/ttyACM0
rm -f zephyr.zip