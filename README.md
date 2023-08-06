# Environment setup:

https://developer.nordicsemi.com/nRF\_Connect\_SDK/doc/latest/nrf/getting\_started/installing.html

Replace:
west init -m https://github.com/nrfconnect/sdk-nrf --mr main
With:
west init -m https://github.com/arnulfrupp/sdk-nrf.git --mr bbtc\_main

# Clone repo:

```
mkdir tcat_demo
cd tcat_demo
git clone https://github.com/arnulfrupp/tcat_demo.git
```

# Build:
* Run ./zsetenv.sh to set environment variables

```
west build -b <board>
```

# Flashing board:

* boards containing jlink eg. Nordic nRF52840dk

```
west flash
```

* boards with usb flasher eg. Nordic nRF52840dongle
* In zflash.sh replace '/dev/ttyACM0' with your serial port

```
./zflash.sh
```