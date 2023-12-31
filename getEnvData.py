#!/usr/bin/env python

import time
try:
    from smbus2 import SMBus
except ImportError:
    from smbus import SMBus
from bme280 import BME280

# Initialise the BME280, we expect it on bus 3 as the thermal camera will be on bus 1
bus = SMBus(3)
bme280 = BME280(i2c_dev=bus)

temperature = bme280.get_temperature()
pressure = bme280.get_pressure()
humidity = bme280.get_humidity()
# first read is somehow incorrect, so sleep on it
time.sleep(1)

# correct read
temperature = bme280.get_temperature()
pressure = bme280.get_pressure() / 1013.25
humidity = bme280.get_humidity()

fileHandle = open("envDataOutput.txt", "w")
fileHandle.write('{:.2f} *C\n{:.3f} atm\n{:.2f} %\n '.format(temperature, pressure, humidity))
fileHandle.close()
