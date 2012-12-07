#!/usr/bin/python2.7

"""
Raspy Juice servo, python2, python-smbus example.

This GUI program controls two servos connected to Raspy Juice.
The python-smbus module as yet only works with python2.

author: Adnan Jalaludin
last modified: December 2012
website: http://code.google.com/p/raspy-juice/w/list
"""

import smbus as smbus
from Tkinter import *

bus = smbus.SMBus(1)

def onScale1change(*args):
    val=scale1.get()
    label.config(text="Servo 1 Changing value " + str(val))
    # write to Raspy Juice Servo 1 word-register
    bus.write_word_data(0x48, 1, val)
    return    

def onScale2change(*args):
    val=scale2.get()
    label.config(text="Servo 2 Changing value " +  str(val))
    # write to Raspy Juice Servo 2 word-register
    bus.write_word_data(0x48, 2, val)
    return    

def onButton1():
    scale1.set(1500)
    onScale1change()
    return

def onButton2():
    scale2.set(1500)
    onScale2change()
    return

def onButtonResetAll():
    onButton1()
    onButton2()
    return

root = Tk()
root.geometry("320x240+10+10")

btnResetAll = Button(root, text="Reset All", command=onButtonResetAll)
btnResetAll.pack(anchor=CENTER)

pwm1 = IntVar()
scale1 = Scale(root, variable=pwm1, from_=1000, to=2000, orient='horizontal', label="Servo 1")
scale1.pack(anchor=CENTER, fill='x', padx=10)
scale1.bind('<Motion>', onScale1change)
button1 = Button(root, text="Reset", command=onButton1)
button1.pack(anchor=CENTER)

pwm2 = IntVar()
scale2 = Scale(root, variable=pwm2, from_=1000, to=2000, orient='horizontal', label="Servo 2")
scale2.pack(anchor=CENTER, fill='x', padx=10)
scale2.bind('<Motion>', onScale2change)
button2 = Button(root, text="Reset", command=onButton2)
button2.pack(anchor=CENTER)

label = Label(root, text="Label text to be changed")
label.pack()

onButtonResetAll()
root.mainloop()
