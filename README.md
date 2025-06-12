# FTMS project ESP32
This is a quick, and a really *dirty* Arduino ESP32 project(s), that attempts to reverse engineer the Sole F85 BLE protocol. In the end it will be used to capture my exercise progress and also potentially I will FTMS enable my old retro fitness bike

# How to use it
- Change the MAC-address to your Sole F85
- Compile on Ardunio

# Dependencies:
- ESP32 BLE, I'm using 3.2.0
- SD-card, SPI, FS

# Compile for your ESP32
- Should not be any concerns here.
- I have been using ESP32 WROOM

# Usage
- The usage is quite stupid
When you send a command, the device will listen for a response, this is not fully functional, since you often get error, as it won't wait for the "right" command.
If you pase a command in the serial, it will send that, and handle the response accordingly
- If it receives a response (basically a request/indication from the treadmill, it will try to respond to this according to the best knowledge on how the protocol should work.





