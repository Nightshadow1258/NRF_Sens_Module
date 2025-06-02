# Notes

## check if there are buffer issues again if they can be increases via
the kconfig menu. build with:
```
west build -t menuconfig
```
and navigate to location:
```
(Top) > Subsystems and OS Services > Bluetooth > Bluetooth buffer configuration
```
found this due to the depracation of `CONFIG_BT_BUF_ACL_RX_COUNT`
see for more infos: https://docs.zephyrproject.org/latest/releases/migration-guide-4.1.html