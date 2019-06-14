## GBA Bluetooth HID ##
Turn your Game Boy Advance into a Bluetooth Controller :)

![Gameboy](images/DSC_0244.JPG?raw=true "GBA")
### How to use it: ###
After launching the rom the bluetooth module enters discoverable mode. This means you can pair to it from whatever device you are going to use it with (PC, phone, Android TV...). After pairing the program will start sending keys to the device. The device address is stored so next time you launch you just have to press A to automatically connect to that device.

### How it works: ###
The [gba](gba) contains the code for a GBA program that uses the link port to communicate with a HC-05 bluetooth module using UAR (HC-05 flashed with RN-42 firmware, find how to do it in the links at the bottom).

This rom is stored in a W25Q32 flash chip. An Atmega328P performs a multiboot sequence reading from this chip and sending to the GBA through the link port. You can find the Atmega code [here](gba-bt-hid-fw) (It is actually written for arduino so you'll need to burn arduino's bootloader if you want to use it).

Once multiboot ends, the gba program runs and talks to the HC-05 to handle bluetooth connection, and key presses.

Additional 74XX157 quad 2-input multiplexer makes possible to multiplex SPI to read from the flash chip and send the multiboot, it alternates byte reads from the flash chip and send them to the GBA. Then usin hcf4066 siwtch link port pins are switched from the spi to the HC-05 uart pins.

The whole circuit runs at 3.3V provided by GBA through the link port.

Final version of the circuit includes a 6 pin port that allows to reprogram the atmega if necessary. Also it lets to turn the atmega into a special write mode to reprogram the flash memory. 

There's a KiCad project [here](circuit) that includes this diagram:
![Diagram](images/Diagram.png?raw=true "GBA")

![Gameboy](images/DSC_0245.jpeg?raw=true "GBA")
![Gameboy](images/DSC_0241.jpg?raw=true "GBA")

Terrible soldering skills I know...

### Links that helped me to get this done: ###

HC-05 as bluetooth HID device:

https://www.youtube.com/watch?v=y8PcNbAA6AQ

https://www.youtube.com/watch?v=BBqsVKMYz1I

GBA Multiboot:

http://www.akkit.org/info/gbatek.htm

https://github.com/akkera102/gba_01_multiboot

https://github.com/ataulien/elm-gba-multiboot

https://github.com/cartr/MSMCcable

https://github.com/tangrs/usb-gba-multiboot

https://github.com/MerryMage/gba-multiboot
