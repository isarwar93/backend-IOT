##  To turn on debugging mode

To read the logs about bluetooth:

```
sudo btmon 
```
## To restart bluetooth:

sudo systemctl restart bluetooth

## To test the bluetooth peripheral from the terminal:

```
bluetoothctl

Then inside the app:
```
scan on
```
Find the ble peripheral, copy the mac

```
scan off
connect [MAC address]
menu gatt
list-attributes
```

Find the correct attribute and then run following:

```
select-attribute /org/bluez/hci0/dev_A0_DD_6C_AF_73_9E/service0028/char0029
notify on
```