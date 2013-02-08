# Python control for Maplin Robotic Arm
# v 1.1 (c) Neil Polwart 2011


import usb.core, sys, time, optparse, pickle, csv

def buildcommand(shoulder=0, elbow=0, wrist=0, grip=0, rotate=0, light=0):
	'''	creates the code to send to USB robot arm
		usage buildcommand(shoulder,elbow,wrist,grip,rotate,light) 
		where 	shoulder,elbow,wrist,are 0,1,2
				0 = off/stop.  1=motor up.   2=motor down
			grip is 0,1,2
				0 = off/stop.  1=close.   2=open
			rotate is 0,1,2
				0 = off/stop.  1=rotate c/wise.   2=rotate c/cwise
			light is 0,1
				0 is off.   1 is on.
		values default to 0 if not set.
	'''
	#print shoulder, elbow, wrist, grip, rotate, light
	#print int(shoulder), shoulder==1, int(shoulder)==1

	# first check parameters are valid		

	if shoulder not in range(0,3): raise ValueError('Shoulder out of range')
	if elbow not in range(0,3): raise ValueError('Elbow out of range')
	if wrist not in range(0,3): raise ValueError('Wrist out of range')
	if grip not in range(0,3): raise ValueError('Grip out of range')
	if rotate not in range(0,3): raise ValueError('Rotate out of range')
	if light not in range(0,3): raise ValueError('Light out of range')

	# create the bytes 

	byte1 = (shoulder<<6)+ (elbow<<4) + (wrist<<2) + (grip)
	byte2 = (rotate)
	byte3 = (light)

	# return the bytes

	thebytes = [byte1, byte2, byte3]
	return thebytes

def connecttoarm():
	''' 	connects to the Maplin USB Robotic Arm
		returns Device not found error if unable to connect
	'''

	dev = usb.core.find(idVendor=0x1267, idProduct=0x0000)
	if dev is None:
		raise ValueError('Device not found')           # if device not found report an error
	
	# configure device
	dev.set_configuration()

	return dev

def sendcommand(device, command='0,0,0') :
	''' 	writes the command to the device (as a USB Command type)
		if no command is given three O characters are used which stops motors etc
		returns number of bytes written
	'''

	timeout=1000
	return device.ctrl_transfer(0x40, 6, 0x100, 0, command, timeout)

def zeroreset() :
	# reset all the values in resetarm.dat to 0
	
	resetdata = { 'shoulder':0,'elbow':0,'wrist':0,'grip':0,'rotate':0 }
		
	resetfile=open('resetarm.dat','wb')
	pickle.dump(resetdata, resetfile)
	resetfile.close()

	return

def parse_commandline():
	# parses the command line arguments

	usage = "usage: %prog [options] arg"

	parser = optparse.OptionParser()

	group1=optparse.OptionGroup(parser, 'Direct control from command line')
	group2=optparse.OptionGroup(parser, 'File based control options')

	group1.add_option("-s", "--shoulder", action="store", default=0, type="int", dest="shoulder", help="Value 0,1,2 for shoulder motor (off/up/down)")
	group1.add_option("-e", "--elbow", action="store", default=0,type="int", dest="elbow", help="Value 0,1,2 for elbow motor (off/up/down)")
	group1.add_option("-w", "--wrist", action="store", default=0,type="int", dest="wrist", help="Value 0,1,2 for wrist motor (off/up/down)")
	group1.add_option("-g", "--grip", action="store", default=0,type="int", dest="grip", help="Value 0,1,2 for grip (off/in/out)")
	group1.add_option("-r", "--rotate", action="store",default=0, type="int", dest="rotate" ,help="Value 0,1,2 for rotation (off/cw/ccw)")
	group1.add_option("-l", "--light", action="store",default=0,  type="int", dest="light", help="Value 0,1 for light (on/off)")
	group1.add_option("-t", "--time", action="store", default=0.1, type="float", dest="timedelay",  help="Time (s) to wait before off default: %default s")
	
	group2.add_option("-f", "--file", action="store", dest="filename",  help="path/name of a file describing a sequence arm movements")
	group2.add_option("-o", "--origin", action="store_true",dest="reset_to_origin", default='false', help="Restore arm to the 'origin' position")
	group2.add_option("-z", "--zero", action="store_true",dest="zero", default='false', help="Zero all values in resetarm file' position")
	
	parser.add_option_group(group1)
	parser.add_option_group(group2)

	(options, args) = parser.parse_args() 	# now parse them

	return options

def get_resetdata() :

	resetdata={} 				# creates a dictionary
	resetfile=open('resetarm.dat','rb') 	# opens the resetarm file to read
	
	# populate the dictionary resetdata with the contents of the file

	resetdata = pickle.load(resetfile)
	print resetdata
	resetfile.close() 			# close the file
	
	return resetdata	

def move_to_reset(device, key, direction, wait) :
	datapair={key:direction,}
	command=buildcommand(**datapair)
	if sendcommand(device,command)<>3 : print "Possible error sending data"
	time.sleep(wait)
	sendcommand(device)  			# stop motors
	
	return

def store_reset_values(thedict, resetdata, timedelay) :

	for eachkey in ['shoulder','elbow','wrist','grip','rotate']:
		if thedict[eachkey] == 1 : resetdata[eachkey]=resetdata[eachkey]+timedelay
		elif thedict[eachkey] == 2 : resetdata[eachkey]=resetdata[eachkey]-timedelay

	resetfile=open('resetarm.dat','wb')
	pickle.dump(resetdata, resetfile)
	resetfile.close()

	return resetdata


def main() :

	options=parse_commandline()
	resetdata=get_resetdata()	

	dev=connecttoarm()

	if options.filename!=None :

		filehandle=csv.reader(open(options.filename,'rb'), delimiter=',')	# opens the resetarm file to read

		for row in filehandle:
			rownumeric=[]
			for each in row[:6]: rownumeric.append(int(each))
			command=buildcommand(*rownumeric[:6])
			if sendcommand(dev,command)<>3 : print "Possible error sending data"
			rownumeric.append(float(row[6]))
			time.sleep(rownumeric[6])
			## need to increment reset file positions!
			sendcommand(dev)
			
			# create dictionary from values
			datadict=dict(zip(['shoulder','elbow','wrist','grip','rotate'],rownumeric[:6]))	
			resetdata=store_reset_values(datadict, resetdata, rownumeric[6])


	elif options.reset_to_origin==True :

		for key in resetdata : 
			if resetdata[key]<0 :
				move_to_reset(dev, key, 1, abs(resetdata[key]))
			elif resetdata[key]>0 :
				move_to_reset(dev, key, 2, abs(resetdata[key]))
		zeroreset()

	elif options.zero==True :

		zeroreset()    # zero the values in the reset file.		

	else :

		command=buildcommand(	shoulder=options.shoulder,
					elbow=options.elbow,
					wrist=options.wrist,
					grip=options.grip,
					rotate=options.rotate,
					light=options.light)
	
		if sendcommand(dev,command)<>3 : print "Possible error sending data"

		resetdata=store_reset_values(options.__dict__, resetdata, options.timedelay)

		time.sleep(options.timedelay)
	
		sendcommand(dev)  			#turn off!
		dev.reset() 				#reset USB port (no need to release?)

if __name__== '__main__':main()
