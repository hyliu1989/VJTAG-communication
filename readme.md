# DE0-Nano communication with a PC through VJTAG and C++

In this project, I demonstrate how to communicate with the Altera FPGA on DE0-Nano using the FTDX2 library. The FPGA side uses Virtual JTAG as a conduit to accept the data from the PC. The goal is to build a communication solution without applying SOPC of the FPGA and quartus_stp on the PC. This will make the FPGA more easier to be integrated with another app.

The Hello world example is a C++ project on the PC end. One can easily accomodate it to other language by using a wrapper around the C++ codes.


# Hello world example which sends a byte to the FPGA
## Requirements:
1. a DE0-Nano device
1. Quartus version 14.1
1. Codeblocks with MINGW compiler (you can try different C++ compilers but this one is tested and works)

## Introduction
This example uses a C++ program to communicate with the FPGA. The C++ has to handle the JTAG timing sequence such as sending 2 bytes with TCK-tag 0 and TCK-tag 1 to simulate the JTAG clocking. Fortunately, the user does not need to handle this detail as the project handles this for them. The sequence is explained in the [step guide](Example-step-guide). The user can change the data to send to the FPGA (most frequent usage) or change the VJTAG configuration (less frequent and requiring RTL change). Whenever the change has been made, the C++ program has to be recompiled. A more convenient Python project that does not require recompilation is still under construction. (See branch [reconstructure_python](https://github.com/hyliu1989/VJTAG-communication/tree/restructure_python).)

The RTL project (quartus_project) contains a module vJTAG_interface (in vJTAG_interface.v) that performs basic communication on the FPGA end. It will update its output `data_from_pc` when the VJTAG write command is completed, and it will shift the bits in `data_sent_to_pc` out to the PC when the VJTAG read command is completed (the output data is a snapshot of `data_sent_to_pc` at the beginning of data shifting).
- As you will learn later in the [following section](Example-step-guide) that we will issue `USER0` instruction and a set of dummy bits to the `USER0` DR. The snapshot happens at the "Virtual Capture DR" state, that is, the beginning of every time the user sends a set of the dummy bits.

## Example step guide
First of all, you will need to burn the RTL project to the DE0-Nano device. This project provides the compiled FPGA programming file, Blaster_Comm.sof, so that the user can skip compiling the RTL.

Next, in the cpp_project, open it in the Codeblock and hit compile and run. If a missing "ftd2xx.dll" error shows up, simply copy that from C++ project root to bin/Release/ and run again. You should see the print out on the screen. What you actually ran was the main() function in main.cpp that sends data to and reads data from the FPGA. The rest of this subsection will detail what the C++ codes do.

If you do not modify the VJTAG IP configuration in RTL, you can skip this paragraph; otherwise, you have to manually put some information from compilation report, Blaster_Comm.map.rpt, to the configuration section in main.cpp (in the followintg code block). You can find the necessary information by searching `; Virtual JTAG Settings` in Blaster_Comm.map.rpt.
```
// === Configuration copied from RTL report Blaster_Comm.map.rpt ================
const int VJTAG_INSTANCE_IR_WIDTH = 2;  // bits. The actual instruction register length for the VJTAG instance.
// The address value here already considers the bit shifting which reserves bits for the VIR command width. In the most
// common case, the VIR width is 4 which is the minimum required by VIR_CAPTURE command. Therefore addr 0x10 actually
// corresponds to 1 after removing the 4 least significant zeros.
const int VJTAG_INSTANCE_ADDR = 0x10;
const int USER1_DR_LENGTH = 5;
```

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
