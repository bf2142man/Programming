#!/usr/bin/env python3
print ("Started")
import RPi.GPIO as GPIO
from time import sleep


GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)

red = 13
blue = 15
green = 16

GPIO.setup(red, GPIO.OUT)
GPIO.setup(blue, GPIO.OUT)
GPIO.setup(green, GPIO.OUT)

GPIO.output(red, 0)
GPIO.output(blue, 0)
GPIO.output(green, 0)

howlong = 0.05

for i in range (1,5000):

	sleep(howlong)
	GPIO.output(red, 0)
	GPIO.output(blue, 1)
	GPIO.output(green, 0)
	sleep(howlong)
	GPIO.output(red, 1)
	GPIO.output(blue, 0)
	GPIO.output(green, 0)
	sleep(howlong)
	GPIO.output(red, 0)
	GPIO.output(blue, 0)
	GPIO.output(green, 1)
	i = i + 1

GPIO.cleanup()
print ("Finished")
