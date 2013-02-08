#!/usr/bin/env python

import wx
import wx.lib.buttons
import os
from arm_control import ArmControl
import time
import csv
import thread

class MainWindow(wx.Frame):
	def __init__(self, parent, title):

		self.dirname=''
		self.parent = parent
		self.recordFlag = False
		self.ResetFlag = False
		self.LightFlag = False
		self.recordedcommands = []
		self.starttime = 0	
		#create the main frame		
		wx.Frame.__init__(self, parent, title=title, size=(900,300))
		

		#create a menu
		menu = wx.Menu()
		menuOpen = menu.Append(wx.ID_OPEN,"&Open","Open a command file")
		menuSave = menu.Append(wx.ID_SAVE,"&Save","Save a command file")
		menuExit = menu.Append(wx.ID_EXIT,"E&xit","Exit")
		
		#add to the menubar
		menuBar = wx.MenuBar()
		menuBar.Append(menu,"&File")
		self.SetMenuBar(menuBar)


		#events for the menu
		self.Bind(wx.EVT_MENU, self.OnOpen, menuOpen)
		self.Bind(wx.EVT_MENU, self.OnSave, menuSave)
		self.Bind(wx.EVT_MENU, self.OnExit, menuExit)
		

		#create a panel for the buttons
		panel = wx.Panel(self,-1,style=wx.SUNKEN_BORDER)



		#create accelerator keys for keyboard control

		wxID_ACCELA = wx.NewId()		#rotate left
		wxID_ACCELS = wx.NewId()		#rotate right
		wxID_ACCELE = wx.NewId()		#shoulder up
		wxID_ACCELD = wx.NewId()		#shoulder down
		wxID_ACCELR = wx.NewId()		#elbow up
		wxID_ACCELF = wx.NewId()		#elbow down
		wxID_ACCELT = wx.NewId()		#wrist up
		wxID_ACCELG = wx.NewId()		#wrist down
		wxID_ACCELH = wx.NewId()		#grip open
		wxID_ACCELJ = wx.NewId()		#grip close
		wxID_ACCELL = wx.NewId()		#light
		wxID_ACCELSPC = wx.NewId()		#PANIC
		wxID_ACCELF1 = wx.NewId()		#set origin
		wxID_ACCELF4 = wx.NewId()		#play
		wxID_ACCELF8 = wx.NewId()		#reset



		acceltbl = wx.AcceleratorTable([(wx.ACCEL_NORMAL, ord('A'), wxID_ACCELA),
						(wx.ACCEL_NORMAL, ord('S'), wxID_ACCELS),
						(wx.ACCEL_NORMAL, ord('E'), wxID_ACCELE),
						(wx.ACCEL_NORMAL, ord('D'), wxID_ACCELD),
						(wx.ACCEL_NORMAL, ord('R'), wxID_ACCELR),
						(wx.ACCEL_NORMAL, ord('F'), wxID_ACCELF),
						(wx.ACCEL_NORMAL, ord('T'), wxID_ACCELT),
						(wx.ACCEL_NORMAL, ord('G'), wxID_ACCELG),
						(wx.ACCEL_NORMAL, ord('H'), wxID_ACCELH),
						(wx.ACCEL_NORMAL, ord('J'), wxID_ACCELJ),
						(wx.ACCEL_NORMAL, ord('L'), wxID_ACCELL),
						(wx.ACCEL_NORMAL, wx.WXK_SPACE, wxID_ACCELSPC),
						(wx.ACCEL_NORMAL, wx.WXK_F1, wxID_ACCELF1),
						(wx.ACCEL_NORMAL, wx.WXK_F4, wxID_ACCELF4),
						(wx.ACCEL_NORMAL, wx.WXK_F8, wxID_ACCELF8)
						])

		self.Bind(wx.EVT_MENU, self.OnAccelKeyA, id = wxID_ACCELA)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyS, id = wxID_ACCELS)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyE, id = wxID_ACCELE)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyD, id = wxID_ACCELD)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyR, id = wxID_ACCELR)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyF, id = wxID_ACCELF)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyT, id = wxID_ACCELT)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyG, id = wxID_ACCELG)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyH, id = wxID_ACCELH)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyJ, id = wxID_ACCELJ)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyL, id = wxID_ACCELL)
		self.Bind(wx.EVT_MENU, self.OnAccelKeySPC, id = wxID_ACCELSPC)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyF1, id = wxID_ACCELF1)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyF4, id = wxID_ACCELF4)
		self.Bind(wx.EVT_MENU, self.OnAccelKeyF8, id = wxID_ACCELF8)

		self.SetAcceleratorTable(acceltbl)

		#main sizer for control buttons
		mainsizer = wx.BoxSizer(wx.HORIZONTAL)


		#rotate buttons and text
		self.wxID_LEFTROTATE = wx.NewId()
		self.LeftRotateButton = wx.ToggleButton(panel,id=self.wxID_LEFTROTATE, label="Left")
		self.LeftRotateButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnLeftRotateToggle, id = self.wxID_LEFTROTATE)

		self.wxID_RIGHTROTATE = wx.NewId()
		self.RightRotateButton = wx.ToggleButton(panel,id=self.wxID_RIGHTROTATE, label="Right")
		self.RightRotateButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnRightRotateToggle, id = self.wxID_RIGHTROTATE)


		rbtnsizer1 = wx.BoxSizer(wx.VERTICAL)
		rbtnsizer2 = wx.BoxSizer(wx.HORIZONTAL)
		rbtnsizer3 = wx.BoxSizer(wx.HORIZONTAL)
		rbtnsizer2.Add((0,150))
		rbtnsizer2.Add(self.LeftRotateButton,1)
		rbtnsizer2.Add(self.RightRotateButton,1)
		rtxtsizer = wx.BoxSizer(wx.VERTICAL)
		rtxtsizer.Add(wx.StaticText(panel,-1,"Rotate"),0,wx.ALIGN_CENTER)
		rbtnsizer1.Add(rbtnsizer2)
		rbtnsizer1.Add(rbtnsizer3)
		rtxtsizer.Add(rbtnsizer1)
		mainsizer.Add(rtxtsizer,0,wx.ALL,10)
		
		
		#shoulder, save, load buttons and text
		wxID_SHOULDERUP = wx.NewId()
		wxID_SHOULDERDOWN = wx.NewId()
		wxID_SAVEFILE = wx.NewId()
		wxID_LOADFILE = wx.NewId()

		self.ShoulderUpButton = wx.ToggleButton(panel,id=wxID_SHOULDERUP, label="Up")
		self.ShoulderUpButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnShoulderUpToggle, id = wxID_SHOULDERUP)

		self.ShoulderDownButton = wx.ToggleButton(panel,id=wxID_SHOULDERDOWN, label="Down")
		self.ShoulderDownButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnShoulderDownToggle, id = wxID_SHOULDERDOWN)

		self.SaveFileButton = wx.Button(panel,id=wxID_SAVEFILE,label="Save")
		self.SaveFileButton.Bind(wx.EVT_BUTTON,self.OnSave,id = wxID_SAVEFILE)

		self.LoadFileButton = wx.Button(panel,id=wxID_LOADFILE,label="Load")
		self.LoadFileButton.Bind(wx.EVT_BUTTON,self.OnOpen,id = wxID_LOADFILE)
		

		sbtnsizer = wx.BoxSizer(wx.VERTICAL)
		sbtnsizer.Add(wx.StaticText(panel,-1,"Shoulder"),0,wx.ALIGN_CENTER)
		sbtnsizer.Add(self.ShoulderUpButton,1)
		sbtnsizer.Add(self.ShoulderDownButton,1)
		sbtnsizer.Add((0,75))
		sbtnsizer.Add(self.SaveFileButton,1)
		sbtnsizer.Add((0,10))
		sbtnsizer.Add(self.LoadFileButton,1)

		mainsizer.Add(sbtnsizer,0,wx.ALL,10)


		#elbow, reset, set origin buttons and text
		wxID_ELBOWUP = wx.NewId()
		wxID_ELBOWDOWN = wx.NewId()
		wxID_RESET = wx.NewId()
		wxID_ORIGIN = wx.NewId()

		self.ElbowUpButton = wx.ToggleButton(panel,id=wxID_ELBOWUP, label="Up")
		self.ElbowUpButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnElbowUpToggle, id = wxID_ELBOWUP)

		self.ElbowDownButton = wx.ToggleButton(panel,id=wxID_ELBOWDOWN, label="Down")
		self.ElbowDownButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnElbowDownToggle, id = wxID_ELBOWDOWN)

		self.ResetButton = wx.Button(panel,id=wxID_RESET, label = "Reset")
		self.ResetButton.Bind(wx.EVT_BUTTON, self.OnReset, id = wxID_RESET)

		self.OriginButton = wx.Button(panel,id=wxID_ORIGIN, label = "Set Origin")
		self.OriginButton.Bind(wx.EVT_BUTTON, self.OnOrigin, id = wxID_ORIGIN)
		
		ebtnsizer = wx.BoxSizer(wx.VERTICAL)
		ebtnsizer.Add(wx.StaticText(panel,-1,"Elbow"),0,wx.ALIGN_CENTER)
		ebtnsizer.Add(self.ElbowUpButton,1)
		ebtnsizer.Add(self.ElbowDownButton,1)
		ebtnsizer.Add((0,75))
		ebtnsizer.Add(self.ResetButton,1)
		ebtnsizer.Add((0,10))
		ebtnsizer.Add(self.OriginButton,1)
		

		mainsizer.Add(ebtnsizer,0,wx.ALL,10)

		#wrist buttons and text
		wxID_WRISTUP = wx.NewId()
		wxID_WRISTDOWN = wx.NewId()

		self.WristUpButton = wx.ToggleButton(panel,id=wxID_WRISTUP, label="Up")
		self.WristUpButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnWristUpToggle, id = wxID_WRISTUP)


		self.WristDownButton = wx.ToggleButton(panel,id=wxID_WRISTDOWN, label="Down")
		self.WristDownButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnWristDownToggle, id = wxID_WRISTDOWN)


		wbtnsizer = wx.BoxSizer(wx.VERTICAL)
		wbtnsizer.Add(wx.StaticText(panel,-1,"Wrist"),0,wx.ALIGN_CENTER)
		wbtnsizer.Add(self.WristUpButton,1)
		wbtnsizer.Add(self.WristDownButton,1)
		mainsizer.Add(wbtnsizer,0,wx.ALL,10)

		#grip buttons and text

		wxID_GRIPOPEN = wx.NewId()
		wxID_GRIPCLOSE = wx.NewId()

		self.GripOpenButton = wx.ToggleButton(panel,id=wxID_GRIPOPEN, label="Open")
		self.GripOpenButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnGripOpenToggle, id = wxID_GRIPOPEN)


		self.GripCloseButton = wx.ToggleButton(panel,id=wxID_GRIPCLOSE, label="Close")
		self.GripCloseButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnGripCloseToggle, id = wxID_GRIPCLOSE)


		gbtnsizer = wx.BoxSizer(wx.HORIZONTAL)
		gbtnsizer.Add(self.GripOpenButton,1)
		gbtnsizer.Add(self.GripCloseButton,1)
		gtxtsizer = wx.BoxSizer(wx.VERTICAL)
		gtxtsizer.Add(wx.StaticText(panel,-1,"Grip"),0,wx.ALIGN_CENTER)
		gtxtsizer.Add(gbtnsizer)
		mainsizer.Add(gtxtsizer,0,wx.ALL,10)


		#light

		wxID_LTON = wx.NewId()

		self.LightOnButton = wx.ToggleButton(panel,id=wxID_LTON, label="Toggle Light")
		self.LightOnButton.Bind(wx.EVT_TOGGLEBUTTON, self.OnLightToggle, id = wxID_LTON)

		wbtnsizer = wx.BoxSizer(wx.VERTICAL)
		wbtnsizer.Add(wx.StaticText(panel,-1,"Light"),0,wx.ALIGN_CENTER)
		wbtnsizer.Add(self.LightOnButton,1)
		mainsizer.Add(wbtnsizer,0,wx.ALL,10)

		panel.SetSizer(mainsizer)
		self.Centre()
		

		#add a panic button
		wxID_PANIC = wx.NewId()
		imageFile = "stop-sign-100x100.jpg"
		image = wx.Image(imageFile, wx.BITMAP_TYPE_ANY).ConvertToBitmap()

		self.PanicButton = wx.BitmapButton(panel,id=wxID_PANIC,bitmap=image,size=(image.GetWidth()+10,image.GetHeight()+10))
		self.PanicButton.Bind(wx.EVT_BUTTON,self.OnPanic,id=wxID_PANIC)

		pbtnsizer = wx.BoxSizer(wx.VERTICAL)
		pbtnsizer.Add((0,150))
		pbtnsizer.Add(self.PanicButton,1)
		mainsizer.Add(pbtnsizer,wx.RIGHT)
		

		#play
		wxID_PLAY = wx.NewId()
		imageFile = "play_button.gif"
		image = wx.Image(imageFile, wx.BITMAP_TYPE_ANY).ConvertToBitmap()
		self.PlayButton = wx.lib.buttons.GenBitmapToggleButton(panel,id=wxID_PLAY,bitmap=image,size=(image.GetWidth()+10,image.GetHeight()+10))
		self.PlayButton.Bind(wx.EVT_BUTTON,self.OnPlay,id=wxID_PLAY)
		rbtnsizer3.Add(self.PlayButton)


		#show the main frame
        	self.Show()

		#set origin by default
		self.SendButtonClickEvent(self.OriginButton)


	#button events

	def OnLeftRotateToggle(self,e):

		duration = time.time()
		self.SendCommand(self.LeftRotateButton,[0,0,0,0,2,0],duration,dev)

	def OnRightRotateToggle(self,e):
		duration = time.time()
		self.SendCommand(self.RightRotateButton,[0,0,0,0,1,0],duration,dev)

	def OnShoulderUpToggle(self,e):

		duration = time.time()
		self.SendCommand(self.ShoulderUpButton,[1,0,0,0,0,0],duration,dev)

	def OnShoulderDownToggle(self,e):

		duration = time.time()
		self.SendCommand(self.ShoulderDownButton,[2,0,0,0,0,0],duration,dev)


	def OnElbowUpToggle(self,e):

		duration = time.time()
		self.SendCommand(self.ElbowUpButton,[0,1,0,0,0,0],duration,dev)


	def OnElbowDownToggle(self,e):
		duration = time.time()
		self.SendCommand(self.ElbowDownButton,[0,2,0,0,0,0],duration,dev)


	def OnWristUpToggle(self,e):

		duration = time.time()
		self.SendCommand(self.WristUpButton,[0,0,1,0,0,0],duration,dev)

	def OnWristDownToggle(self,e):

		duration = time.time()
		self.SendCommand(self.WristDownButton,[0,0,2,0,0,0],duration,dev)


	def OnGripOpenToggle(self,e):

		duration = time.time()
		self.SendCommand(self.GripOpenButton,[0,0,0,2,0,0],duration,dev)


	def OnGripCloseToggle(self,e):

		duration = time.time()
		self.SendCommand(self.GripCloseButton,[0,0,0,1,0,0],duration,dev)


	def OnLightToggle(self,e):
		duration = time.time()
		if not self.LightOnButton.GetValue():
			command = arm.buildcommand(0,0,0,0,0,0)
			arm.sendcommand(dev,command)
			self.LightFlag = False
			if self.recordFlag:
				self.recordedcommands.append([0,0,0,0,0,0,duration])

		else:
			command = arm.buildcommand(0,0,0,0,0,1)
			arm.sendcommand(dev,command)
			self.LightFlag = True
			if self.recordFlag:
				self.recordedcommands.append([0,0,0,0,0,1,duration])

	def OnPanic(self,e):
		arm.sendcommand(dev,'0,0,0')	
		dev.reset()


	def OnPlay(self,e):

		thread.start_new_thread(self.Playback, ())			
			
	def Playback(self):

		for i in range(0,len(self.recordedcommands)-1):


				
			if len(self.recordedcommands[i]) == 4:
				arm.sendcommand(dev,'0,0,0')
			else:

				if len(self.recordedcommands[i+1]) == 4:
					elapsedtime = 	self.recordedcommands[i+1][3] - self.recordedcommands[i][6]
				else:
					elapsedtime = 	self.recordedcommands[i+1][6] - self.recordedcommands[i][6]
	
				command = arm.buildcommand(self.recordedcommands[i][0],self.recordedcommands[i][1],self.recordedcommands[i][2], \
				self.recordedcommands[i][3],self.recordedcommands[i][4],self.recordedcommands[i][5])
				arm.sendcommand(dev,command)
			time.sleep(elapsedtime)
		self.PlayButton.SetValue(False)
		arm.sendcommand(dev,'0,0,0')
		dev.reset()

	def OnOpen(self,e):
		#open a command file
		dlg = wx.FileDialog(self, "Select a command file", self.dirname, "", "*.dat",wx.OPEN)
        	if dlg.ShowModal() == wx.ID_OK:
			self.recordedcommands = []
            		self.filename = dlg.GetFilename()
            		self.dirname = dlg.GetDirectory()
            		f = csv.reader(open(os.path.join(self.dirname, self.filename), 'rb'),quotechar='|')
			for row in f:
				command = []
				for cmd in row[:len(row)-1]:
					command.append(int(cmd))

				command.append(float(row[len(row)-1]))
				self.recordedcommands.append(command)
            		#f.close()
        	dlg.Destroy()

	def OnSave(self,e):
		#save a command file
		dlg = wx.FileDialog(self, "Save a command file", self.dirname, "", "*.dat",wx.SAVE)
        	if dlg.ShowModal() == wx.ID_OK:
            		self.filename = dlg.GetFilename()
            		self.dirname = dlg.GetDirectory()
            		f = open(os.path.join(self.dirname, self.filename), 'wb')
			for cmdlist in self.recordedcommands:
				command = ""
				for cmd in cmdlist:
					command = command + str(cmd) + ","
				command = command[:len(command)-1]
				command = command + "\n"

				f.write(command)
            		f.close()
        	dlg.Destroy()

    	def OnExit(self,e):
        	self.Close(True)

	def OnOrigin(self,e):
		self.recordedcommands = []
		self.recordFlag = True
		self.ResetFlag = True

	def OnReset(self,e):

		if self.ResetFlag:
			resetcommands = [[0,0],[0,0],[0,0],[0,0],[0,0],[0,0]]
			for i in range(0,len(self.recordedcommands)-1):
				if len(self.recordedcommands[i]) > 4:
				

					if len(self.recordedcommands[i+1]) == 4:

						elapsedtime = 	self.recordedcommands[i+1][3] - self.recordedcommands[i][6]
					else:
						elapsedtime = 	self.recordedcommands[i+1][6] - self.recordedcommands[i][6]

					if self.recordedcommands[i][0] == 1:
						resetcommands[0][0] += elapsedtime

					if self.recordedcommands[i][0] == 2:
						resetcommands[0][1] += elapsedtime

					if self.recordedcommands[i][1] == 1:
						resetcommands[1][0] += elapsedtime

					if self.recordedcommands[i][1] == 2:
						resetcommands[1][1] += elapsedtime	

					if self.recordedcommands[i][2] == 1:
						resetcommands[2][0] += elapsedtime

					if self.recordedcommands[i][2] == 2:
						resetcommands[2][1] += elapsedtime

					if self.recordedcommands[i][3] == 1:
						resetcommands[3][0] += elapsedtime

					if self.recordedcommands[i][3] == 2:
						resetcommands[3][1] += elapsedtime	

					if self.recordedcommands[i][4] == 1:
						resetcommands[4][0] += elapsedtime

					if self.recordedcommands[i][4] == 2:
						resetcommands[4][1] += elapsedtime

					if self.recordedcommands[i][5] == 1:
						resetcommands[5][0] += elapsedtime

					if self.recordedcommands[i][5] == 2:
						resetcommands[5][1] += elapsedtime


			commands = []
			if (resetcommands[0][0] > resetcommands[0][1]):
				commands.append([2,0,0,0,0,0,resetcommands[0][0]-resetcommands[0][1]])
			elif resetcommands[0][0] <> 0 or resetcommands[0][1]<>0:
				commands.append([1,0,0,0,0,0,resetcommands[0][1]-resetcommands[0][0]])
			

			if (resetcommands[1][0] > resetcommands[1][1]):
				commands.append([0,2,0,0,0,0,resetcommands[1][0]-resetcommands[1][1]])
			elif resetcommands[1][0] <> 0 or resetcommands[1][1]<>0:
				commands.append([0,1,0,0,0,0,resetcommands[1][1]-resetcommands[1][0]])	

			if (resetcommands[2][0] > resetcommands[2][1]):
				commands.append([0,0,2,0,0,0,resetcommands[2][0]-resetcommands[2][1]])
			elif resetcommands[2][0] <> 0 or resetcommands[2][1]<>0:
				commands.append([0,0,1,0,0,0,resetcommands[2][1]-resetcommands[2][0]])

			if (resetcommands[3][0] > resetcommands[3][1]):
				commands.append([0,0,0,2,0,0,resetcommands[3][0]-resetcommands[3][1]])
			elif resetcommands[3][0] <> 0 or resetcommands[3][1]<>0:
				commands.append([0,0,0,1,0,0,resetcommands[3][1]-resetcommands[3][0]])

			if (resetcommands[4][0] > resetcommands[4][1]):
				commands.append([0,0,0,0,2,0,resetcommands[4][0]-resetcommands[4][1]])
			elif resetcommands[4][0] <> 0 or resetcommands[4][1]<>0:
				commands.append([0,0,0,0,1,0,resetcommands[4][1]-resetcommands[4][0]])

			if (resetcommands[5][0] > resetcommands[5][1]):
				commands.append([0,0,0,0,0,2,resetcommands[5][0]-resetcommands[5][1]])
			elif resetcommands[5][0] <> 0 or resetcommands[5][1]<>0:
				commands.append([0,0,0,0,0,1,resetcommands[5][1]-resetcommands[5][0]])



			ltoff = arm.buildcommand(0,0,0,0,0,0)
			arm.sendcommand(dev,ltoff)

			for cmd in commands:
				thiscmd = arm.buildcommand(cmd[0],cmd[1],cmd[2],cmd[3],cmd[4],cmd[5])
				arm.sendcommand(dev,thiscmd)
				time.sleep(cmd[6])
				arm.sendcommand(dev,'0,0,0')

			self.ResetFlag = False


	#Accelerator key handlers



	def OnAccelKeyA(self,e):
		self.SendToggleEvent(self.LeftRotateButton)

	def OnAccelKeyS(self,e):
		self.SendToggleEvent(self.RightRotateButton)

	def OnAccelKeyE(self,e):
		self.SendToggleEvent(self.ShoulderUpButton)

	def OnAccelKeyD(self,e):
		self.SendToggleEvent(self.ShoulderDownButton)

	def OnAccelKeyR(self,e):
		self.SendToggleEvent(self.ElbowUpButton)

	def OnAccelKeyF(self,e):
		self.SendToggleEvent(self.ElbowDownButton)

	def OnAccelKeyT(self,e):
		self.SendToggleEvent(self.WristUpButton)

	def OnAccelKeyG(self,e):
		self.SendToggleEvent(self.WristDownButton)

	def OnAccelKeyH(self,e):
		self.SendToggleEvent(self.GripOpenButton)

	def OnAccelKeyJ(self,e):
		self.SendToggleEvent(self.GripCloseButton)

	def OnAccelKeyL(self,e):
		self.SendToggleEvent(self.LightOnButton)

	def OnAccelKeySPC(self,e):
		self.SendButtonClickEvent(self.PanicButton)

	def OnAccelKeyF1(self,e):
		self.SendButtonClickEvent(self.OriginButton)

	def OnAccelKeyF4(self,e):
		self.SentBitmapToggleEvent(self.PlayButton)

	def OnAccelKeyF8(self,e):
		self.SendButtonClickEvent(self.ResetButton)


	#generic send event functions for different button classes

	def SendToggleEvent(self,control):
		if control.GetValue():
			control.SetValue(False)
		else:
			control.SetValue(True)

		evt = wx.CommandEvent(wx.wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,control.GetId())
		evt.SetEventObject(control)
		control.GetEventHandler().ProcessEvent(evt)


	def SendButtonClickEvent(self,control):
		evt = wx.CommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED,control.GetId())
		evt.SetEventObject(control)
		control.GetEventHandler().ProcessEvent(evt)

	
	def SentBitmapToggleEvent(self,control):
		if control.GetValue():
			control.SetValue(False)
		else:
			control.SetValue(True)

		evt = wx.CommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED,control.GetId())
		evt.SetEventObject(control)
		control.GetEventHandler().ProcessEvent(evt)


	
	def SendCommand(self,control,commands,duration,dev):
		if control.GetValue():
			cmd = arm.buildcommand(commands[0],commands[1],commands[2],commands[3],commands[4],commands[5])
			arm.sendcommand(dev,cmd)
			if self.recordFlag:
				commands.append(duration)
				self.recordedcommands.append(commands)
		else:
			arm.sendcommand(dev,'0,0,0')
			if self.recordFlag:	
				self.recordedcommands.append([0,0,0,duration])

app = wx.App(False)
frame = MainWindow(None, "Robot Arm Control")

arm = ArmControl()
dev = arm.connecttoarm()

app.MainLoop()
		
