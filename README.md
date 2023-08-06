# Environment setup:

[https://developer.nordicsemi.com/nRF\_Connect\_SDK/doc/2.4.1/nrf/getting\_started/installing.html](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.1/nrf/getting_started/installing.html) <br>
Replace: <br>
west init -m https://github.com/nrfconnect/sdk-nrf --mr main <br>
With: <br>
west init -m https://github.com/arnulfrupp/sdk-nrf.git --mr bbtc\_main <br>

# Clone repo:

```
git clone https://github.com/arnulfrupp/tcat_demo.git
cd tcat_demo
```

# Build:

* Run 'source \<ncs-directory>/zephyr/zephyr-env.sh' to set environment variables

```
west build -b <board>
```

# Flashing board:

* boards containing jlink eg. Nordic nRF52840dk

```
west flash
```

* boards with usb flasher eg. Nordic nRF52840dongle
* In zflash.sh replace '/dev/ttyACM0' with your serial port and 'build\_dongle/zephyr/zephyr.hex' with the path to your hex file.

```
./zflash.sh
```