# DE0-Nano communication with a PC through VJTAG and C++

In this project, I demonstrate how to communicate with the Altera FPGA on DE0-Nano using the FTDX2 library. The FPGA side uses Virtual JTAG as a conduit to accept the data from the PC. The goal is to build a communication solution without applying SOPC of the FPGA and quartus_stp on the PC. This will make the FPGA more easier to be integrated with another app.

I intend to provide two examples with different complexity of the Virtual JTAG.
- One is for a simple communication; the PC-end does not handle the variation of VIR length, assuming the length is short. 
- The second one (under construction) is a Virtual JTAG with longer VIR length so that the PC-end has to handle the case. 

# Hello world example which sends a byte to the FPGA
TODO

# More detail on Virtual JTAG




Author: Hsiou-Yuan Liu
Updated: 26 June 2020
