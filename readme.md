# DE0-Nano communication with a PC through VJTAG and C++

In this project, I demonstrate how to communicate with the Altera FPGA on DE0-Nano using the FTDX2 library. The FPGA side uses Virtual JTAG as a conduit to accept the data from the PC. The goal is to build a communication solution without applying SOPC of the FPGA and quartus_stp on the PC. This will make the FPGA more easier to be integrated with another app.

I intend to provide two examples with different complexity of the Virtual JTAG.
- One is for a simple communication; the PC-end does not handle the variation of VIR length, assuming the length is short. 
- The second one (under construction) is a Virtual JTAG with longer VIR length so that the PC-end has to handle the case. 

# Hello world example which sends a byte to the FPGA
First of all, you will need a DE0-Nano device and compile and burn the RTL project to the device.

Next, in the cpp_project, you can find the main() function in main.cpp that sends data to and reads data from the FPGA. If you do not modify the VJTAG IP configuration in RTL, you can go to the next paragraph; otherwise, you have to manually put some information from compilation report, Blaster_Comm.map.rpt, to the configuration section in main.cpp:
```
// === Configuration copied from RTL report Blaster_Comm.map.rpt ================
const int VJTAG_INSTANCE_IR_WIDTH = 2;  // bits. The actual instruction register length for the VJTAG instance.
// The address value here already considers the bit shifting which reserves bits for the VIR command width. In the most
// common case, the VIR width is 4 which is the minimum required by VIR_CAPTURE command. Therefore addr 0x10 actually
// corresponds to 1 after removing the 4 least significant zeros.
const int VJTAG_INSTANCE_ADDR = 0x10;
const int USER1_DR_LENGTH = 5;
```
You can find the necessary information by searching `; Virtual JTAG Settings` in Blaster_Comm.map.rpt.

Inside main(), the JTAG device is first opened. The byte buffer that contains the instructions to the JTAG device is prepared by SendBufOperation_BitBangBasic() where the complicated JTAG operations are handled and abstracted. The flow in SendBufOperation_BitBangBasic() is listed as follows:
1. Synchronized the JTAG device to `IDLE` state
1. Update the IR to indicate that we will be sending data to the `USER1` DR. `USER1` DR holds the virtual instructions.
1. Send the first virtual instruction: `VIR_CAPTURE`, required by VJTAG IP to let the IP know that the next virtual instruction should be captured.
1. Send the second virtual instruction: `0b01`, defined by this project and means to send data from the PC to the FPGA. The data will be displayed on the LED pattern.
1. Update the IR to indicate that we will be sending data to the `USER0` DR. `USER0` DR holds the data.
1. Send the first set of 8-bit data
1. Send the second set of 8-bit data. This overwritten the first set and the LED will display the second set. However, because we enable reading the `TDO` when sending the second set, we will see the first set to be shown on the PC screen (shown in the first row in the terminal).
1. (Until now, we perform the write operation. The following will be the read operation)
1. Update the IR to indicate that we will be sending data to the USER1 DR.
1. Send the first virtual instruction: `VIR_CAPTURE`
1. Send the second virtual instruction: `0b10`, defined by this project and means to read data from the PC to the FPGA. The data is defined as `{SW[3:0], SW[3:0]}` which repeats the 4-bit switch pattern on DE0-Nano.
1. Update the IR to indicate that we will be sending data to the `USER0` DR.
1. Send 8-bit data where the data content does not matter. We enable reading the `TDO` when sending the data and you will see the repeated switch pattern (shown in the second row in the terminal).

The ByteShift mode also works! It was previously considered not working in 2013 when this project was spun off. The command `0b10` (read) part is not implemented in ByteShift mode and is left as a TODO.


# JTAG basic description
The main method of communication, Virtual JTAG, mimicks the JTAG structure and pins. Therefore, having basic understanding of what the signal pins and the state transition of JTAG tap controller will help understanding how Virtual JTAG works.
- Test data in (TDI), used to shift data into the IR and DR shift register chains. JTAG chip captures it at rising edge of TCK in "shift" states.
- Test data out (TDO), used to shift data out of the IR and DR shift register chains. JTAGs chip changes it at falling edge of TCK in "shift" states so that the next device in the chain can have a proper TDI at the rising edges.
- Test mode select (TMS), used as an input into the TAP controller
- TCK, used as the clock source for the JTAG circuitry
- TRST resets the TAP controller. This is an optional input pin defined by the 1149.1 standard. (The TRST pin is not present in the Cyclone device family.)

Ref: [Altera Virtual JTAG (altera_virtual_jtag) IP Core User Guide](https://www.intel.com/content/dam/www/programmable/us/en/pdfs/literature/ug/ug_virtualjtag.pdf) (2020), Section "Design Example: TAP Controller State Machine".


Author: Hsiou-Yuan Liu
Updated: 28 June 2020
