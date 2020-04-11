## GBA as Bluetooth Gamepad ##
Turn your Game Boy Advance into a Bluetooth Controller :)
![Gameboy](images/DSC_0710.jpg?raw=true "GBA")

### How to use it: ###
After launching the rom the bluetooth module enters discoverable mode. This means you can pair to it from whatever device you are going to use it with (PC, phone, Android TV...). After pairing the program will start sending keys to the device. The device address is stored so next time you launch you just have to press A to automatically connect to that device.

### How it works: ###
The project is basically an ESP32 connected to the GBA through the link port. With the device connected and without any cartridge inserted in the GBA, once the GBA turns on the ESP32 sends a small rom to be loaded in the GBA. This rom is a program made to enable communication between the ESP32 and GBA for both handling bluetooth conneciton and sending the user input to the ESP32 when it is connected to a bluetooth host and act as a gamepad. Unfortunatelly it only works with traditional GBA as I couln't make it work with GBA SP. I think GBA SP just doesn't give enough current.

When turned on the ESP32 (using the code found in [esp32](esp32) peforms a multiboot sequence through the SPI to the GBA sending a rom that the ESP32 has stored in the flash memory. The code for this rom can be found in [gba](gba). Once loaded the ESP32 enables the UART port in the same pins and the rom communicates with the ESP32 using UART through the link port.

The ESP32 is powered by the 3.3V the GBA gives through the port.
Once the ESP32 is programmed the male link port connector should be wired to it like the following diagram:
![ESP32Diagram](images/ESP32-diagram.png?raw=true "Diagram")

The first prototype was made with an Atmega and HC-05:
### Old version prototype: ###
![Gameboy](images/DSC_0244.JPG?raw=true "GBA")
The [gba](gba) contains the code for a GBA program that uses the link port to communicate with a HC-05 bluetooth module using UART (HC-05 flashed with RN-42 firmware, find how to do it in the links at the bottom).

This rom is stored in a W25Q32 flash chip. An Atmega328P performs a multiboot sequence reading from this chip and sending to the GBA through the link port. You can find the Atmega code [here](arduino) (It is actually written for arduino so you'll need to burn arduino's bootloader if you want to use it).

Once multiboot ends, the gba program runs and talks to the HC-05 to handle bluetooth connection, and key presses.

Additional 74XX157 quad 2-input multiplexer makes possible to multiplex SPI to read from the flash chip and send the multiboot, it alternates byte reads from the flash chip and send them to the GBA. Then using hcf4066 switch link port pins are switched from the spi to the HC-05 uart pins.

The whole circuit runs at 3.3V provided by GBA through the link port.

Final version of the circuit includes a 6 pin port that allows to reprogram the atmega if necessary. Also it lets to turn the atmega into a special write mode to reprogram the flash memory. You can find the code for the memory programmer [here](gba-hid-fw-flasher) it needs to run in a separate board (not arduino due to the large size of the rom to program, I used a Stellaris Launchpad)

There's a KiCad project [here](circuit) that includes this diagram:
![Diagram](images/Diagram.png?raw=true "GBA")

![Gameboy](images/DSC_0245.jpeg?raw=true "GBA")

### Links that helped me to get this done: ###
ESP32 as a bluetooth Gamepad:
Thanks to the Bluecubemod from Nathan Reeves I learned how to turn ESP32 into a bluetooth gamepad
https://github.com/NathanReeves/BlueCubeMod

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
