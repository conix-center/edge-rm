## Zephyr agent application

This is the folder that contains the zephyr application code

In this folder you can build and flash the application

```shell
west build -b nrf52840dk_nrf52840 . -p always -- -DCONF_FILE="prf_nrf52840_dk.conf  overlay-ot.conf" -DWAMR_BUILD_TARGET=THUMB -DWAMR_BUILD_AOT=0
west flash
```

## Structure of App

This app

1) Connects to an openthread border router
2) Prepares a WASM execution environment
3) Advertises its resources to the edge-rm master
4) Waits for RunTask requests of WASM come
5) Executes the WASM tasks
