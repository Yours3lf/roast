# Roast coffee roasting software

## Overview
It utilises a client-server model where the server runs on a Raspberry Pi (3B+).
There it gathers data from a BME280 temperature, humidity and pressure sensor and an MLX90640 thermal camera.
It sends this data over using the websockets protocol to a client that runs inside a webpage running on a PC.
This client visualises the thermal camera data and allows the user to log roasting and control events.
It also allows the user to note various properties of the coffee beans (pre/post roast weight etc.)
and take notes on the roasting process and resulting coffee flavour.
This data can then be saved to a JSON file, shared and loaded on demand.

## Hardware setup
- Behmor 2020SR Plus
  - with light bulb changed to a natural 6500k led bulb
  - hole drilled on left side for the thermal camera
  - thermal camera is mounted over the hole with an aluminium bracket
  - a coke can is modified to serve as an air funnel, taped to the side using insulating tape
  - aluminium foil over the door with a cutout for observing the coffee
  - plugged into a watt meter for monitoring
- 120mm PC fan with a variable voltage ac-dc power supply
  - pointed down the coke can towards the thermal camera
  - together with the coke can it provides positive air pressure so that the thermal camera stays clean and cool (like the Aillio IBTS)
- Raspberry Pi 3B+
  - BME280 air temperature, humidity, pressure sensor
  - MLX90640 thermal camera
- Laptop
- Hair dryer on cold setting for improved cooling phase performance

## Usage
- Create makefile using Cmake. Needs:
  - system threads lib
  - OpenSSL
  - MLX90640 API lib
  - BME280 python lib
  - Python 3
- Use make to compile Server executable
- Run the server: `./roast [ip address] [optional port] [optional thermal cam refresh rate]`
  - Default port: 50000
  - Default refresh rate: 4HZ
- Open the Client.html file in Chrome
- Click the cog icon in the top left corner to open the settings dialog
- Connect to the server by entering the websocket address and clicking the connect button
  - Websocket address: `ws://[ip address]:[port]`
  - Observe successful connection message
- Close settings dialog
- Start roasting on your Behmor
  - Select weight
  - Press start
  - Select manual heat setting
- Click the start button on the left to start recording data

## Calibrating beanFilterMask
Depending on your physical setup, ie. where your thermal camera points, you need a different beanFilterMask array than provided.
- To calibrate this crucial mask heat up about 250g of roasted (brown) coffee to about 180c. 
- Then add these hot coffee beans into the roaster basket.
- Next, start your roasting session
  - on the lowest power setting (P1 manual, ie. heating off)
  - the highest drum speed setting.
- Record the thermal camera output through the roasting client. The hot beans should be clearly distinguishable from the cold background.
- You can edit your beanFilterMask by ticking that little box by the thermal camera output.
- Select pixels with the left mouse button and deselect with the right mouse button.
- When satisfied you can print your mask using printBeanFilterMask() through the javascript console.
- Replacing the beanFilterMask in the html code with this should calibrate your mask.
- Reload your client file to inspect the results
