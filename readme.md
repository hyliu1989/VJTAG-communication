# DE0-Nano communication with a PC through VJTAG and C++

In this project, I demonstrate how to communicate with the Altera FPGA on DE0-Nano using the FTDX2 library. The FPGA side uses Virtual JTAG as a conduit to accept the data from the PC. The goal is to build a communication solution without applying SOPC of the FPGA and quartus_stp on the PC. This will make the FPGA more easier to be integrated with another app.

I intend to provide two examples with different complexity of the Virtual JTAG.
- One is for a simple communication; the PC-end does not handle the variation of VIR length, assuming the length is short. 
- The second one (under construction) is a Virtual JTAG with longer VIR length so that the PC-end has to handle the case. 

# Hello world example which sends a byte to the FPGA
TODO

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
