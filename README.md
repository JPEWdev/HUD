# HUD
A vehicular Heads Up Display based on an Atmel

## Dependencies
This software depends uses avr-gcc and avr-libc to compile. To flash the sofware onto the device, you will need avrdude installed and an ISP programmer. You may need to tweak the make file if you use a programmer other than the AVRISP MkII.

## Compiling
To compile the software, simply run `make`

## Installing
To install the software onto the device, run `make writeflash`.

If you have a fresh part that has never been programmed before, you will need to program the fuse bits before you can program the main software. To do this, run `make writefuses`. This command is clocked at a low speed to ensure that it will work when the AVR is running off of either its internal oscialltor or the external crystal. This is fine, because the command is only writing the fuse bits, which doesn't take long

## Assembly
The HUD.brd file in the respository can be used to fabricate the main board. You can order all of the parts except for the ELM329 chip from [Digikey](www.digikey.com) using the [BOM.txt](./misc/BOM.txt) file (you can directly import this on the [DigiKey website](https://www.digikey.com/classic/ordering/addpart.aspx), use the "Upload to Cart" option). You can order the ELM329 from [ELM's website](https://www.elmelectronics.com/ic/elm329/)

Total cost for PCB fabrication and all components should be less than $200 USD.

## Display
The display that should be connected to the main board is the Noritake GU140X16G-7003 Vacuum Fluorescent Display. You will need to solder the jumper on the back to put the display into Synchronus Serial Mode (Short jumper J2 on the back of the display). Pins 1-6 on the 6 pin header should be wired to pins 1-6 on the display header of the main board in the same order (i.e. pin 1 to pin 1). Pin 1 on the display board header is the closest to the bottom of the board. Pin 7 on the main board is not used with the VFD display.
