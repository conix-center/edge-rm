## Zephyr agent application

This is the folder that contains the zephyr application code

In this folder you can build and flash the application

```shell
west build -b nrf52840dk_nrf52840 . -p always -- -DCONF_FILE="prf_nrf52840_dk.conf  overlay-ot.conf" -DWAMR_BUILD_TARGET=THUMB -DWAMR_BUILD_AOT=0
west flash
```
