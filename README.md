# Zephyr Application for NRF52840 Sensor Module

This application is based on the Zephyr [example-application](https://github.com/zephyrproject-rtos/example-application)
Before getting started, make sure you have a proper Zephyr development
environment. Follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Building and running

To build the application, run the following command:

```shell
cd app
west build
```

config automatically will grab the board `sensor_module` from the folder `app/boards/`.

Once you have built the application, run the following command to flash it:

```shell
west flash
```

### Testing

To execute Twister integration tests, run the following command:

```shell
west twister -T tests --integration
```

### Documentation

will be added at a later point in time. 

### Open points
- clean up the folder structure and test currently unused functions
