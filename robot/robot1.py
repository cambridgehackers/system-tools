# Minimalist version of USB arm control to show how simple it could be! (c) N Polwart,  2011

import usb.core, time

dev = usb.core.find(idVendor=0x1267, idProduct=0x0000)
if dev is None:
    raise ValueError('Device not found')           # if device not found report an error

dev.set_configuration()

datapack=0x40,0,0      # change this to vary the movement
bytesout=dev.ctrl_transfer(0x40, 6, 0x100, 0, datapack, 1000)

time.sleep(1)    # waits for 1 second whilst motors move.

datapack=0,0,0
bytesout=dev.ctrl_transfer(0x40, 6, 0x100, 0, datapack, 1000)
