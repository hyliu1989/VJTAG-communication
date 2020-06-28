#ifndef JTAG_TAP_H
#define JTAG_TAP_H
/*
Declares functions that handles a byte buffer which contains the information to control JTAG tap controllers.

The functions prepare the buffer whose bytes will be send to the JTAG chain through the ftd2xx library. The information
in the buffer is not written to the JTAG chain until the FT_Write function (ftd2xx library) is called.
For more information about the JTAG transitioning, look up:
1. Altera Virtual JTAG (altera_virtual_jtag) IP Core User Guide, Section "Design Example: TAP Controller State Machine",
   https://www.intel.com/content/dam/www/programmable/us/en/pdfs/literature/ug/ug_virtualjtag.pdf (2020)
2. fpga4fun JTAG tutorial, https://www.fpga4fun.com/JTAG.html (2020)

*/





#endif
