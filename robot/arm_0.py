	# Import the required libraries

import usb.core
import usb.util
import sys
import time

	# This program is intended to control a robotic arm via USB from Linux
	# The code is written in Python by Neil Polwart (c) 2011
	# It is a work in progress and will improved!

	# locate the device device

dev = usb.core.find(idVendor=0x1267, idProduct=0x0000)

	# assigns the device to the handle "dev"
	# can check the device is visible to Linux with command line command lsusb
	# which should report a device with the above vendor and id codes.

	# was it found?

if dev is None:
	raise ValueError('Device not found')           # if device not found report an error


	# set the active configuration

dev.set_configuration()

	# as no arguments, the first configuration will be the active one
	# note as commands are sent to device as commands not data streams 
	# no need to define the endpoint

	# defines the command packet to send

datapack=0x80,0,0

	# change this packet to make different moves.  
	# first byte defines most of the movements, second byte shoulder rotation, third byte light
	# command structure in more detail:
	# http://notbrainsurgery.livejournal.com/38622.html?view=93150#t93150

print "requested move",datapack    # reports the requested movement to the user

	# send the command

bytesout=dev.ctrl_transfer(0x40, 6, 0x100, 0, datapack, 1000)

	# outputs the command to the USB device, using the ctrl_transfer method
	# 0x40, 6, 0x100, 0 defines the details of the write - bRequestType, bRequest, wValue, wIndex
	# datapack is our command (3 bytes)
	# the final value is a timeout (in ms) which is optional
	# bytesout = the number of bytes written (i.e. 3 if successful)

print "Written :",bytesout,"bytes" 	# confirm to user that data was sent OK

	# wait for a defined period

time.sleep(1)    # waits for 1 second whilst motors move.

	# now STOP the motors
	
datapack=0,0,0

bytesout=dev.ctrl_transfer(0x40, 6, 0x100, 0, datapack, 1000)

if bytesout == 3: print "Motors stopped"
