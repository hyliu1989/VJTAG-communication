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
#include "ftd2xx.h"

void atomic_state_trans_SR_to_SR  (BYTE *buf, int &cnt, BYTE bit_to_shift_in, bool to_read);  // change state from [Shift_DR/IR] to [Shift_DR/IR], i.e. shift one bit
void atomic_state_trans_SR_to_EX1 (BYTE *buf, int &cnt, BYTE bit_to_shift_in, bool to_read);  // change state from [Shift_DR/IR] to [Exit1_DR/IR]


// IDLE and Reset related
void atomic_state_trans_IDL_to_IDL(BYTE *buf, int &cnt);  // change state from [Run_Test/Idle] to [Run_Test/Idle]
void atomic_state_trans_RST_to_RST(BYTE *buf, int &cnt);  // change state from [Test_Logic/Reset] to [Test_Logic/Reset]
void atomic_state_trans_RST_to_IDL(BYTE *buf, int &cnt);  // change state from [Test_Logic/Reset] to [Run_Test/Idle]
void atomic_state_trans_SIS_to_RST(BYTE *buf, int &cnt);  // change state from [Select_IR_Scan] to [Test_Logic/Reset]

// Go to the DR scan and DR flow
void atomic_state_trans_IDL_to_SDS(BYTE *buf, int &cnt);  // change state from [Run_Test/Idle] to [Select_DR_Scan]
void atomic_state_trans_SDS_to_CAP(BYTE *buf, int &cnt);  // change state from [Select_DR_Scan] to [Capture_DR]
void atomic_state_trans_CAP_to_SDR(BYTE *buf, int &cnt);  // change state from [Capture_DR] to [Shift_DR]
void atomic_state_trans_CAP_to_EX1(BYTE *buf, int &cnt);  // change state from [Capture_DR/IR] to [Exit1_DR/IR]
void atomic_state_trans_EX1_to_PAU(BYTE *buf, int &cnt);  // change state from [Exit1_DR/IR] to [Pause_DR/IR]
void atomic_state_trans_EX1_to_UPD(BYTE *buf, int &cnt);  // change state from [Exit1_DR/IR] to [Update_DR/IR]
void atomic_state_trans_PAU_to_PAU(BYTE *buf, int &cnt);  // change state from [Pause_DR/IR] to [Pause_DR/IR]
void atomic_state_trans_PAU_to_EX2(BYTE *buf, int &cnt);  // change state from [Pause_DR/IR] to [Exit2_DR/IR]
void atomic_state_trans_EX2_to_SDR(BYTE *buf, int &cnt);  // change state from [Exit2_DR] to [Shift_DR]
void atomic_state_trans_EX2_to_UPD(BYTE *buf, int &cnt);  // change state from [Exit2_DR/IR] to [Update_DR/IR]
void atomic_state_trans_UPD_to_SDS(BYTE *buf, int &cnt);  // change state from [Update_DR/IR] to [Select_DR_Scan]
void atomic_state_trans_UPD_to_IDL(BYTE *buf, int &cnt);  // change state from [Update_DR/IR] to [Run_Test/Idle]

// Go to the IR scan and IR flow. The IR flow and DR flow are identical and the following functions are aliases to make
// the meaning of the script clear.
void atomic_state_trans_SDS_to_SIS(BYTE *buf, int &cnt);  // change state from [Select_DR_Scan] to [Select_IR_Scan]
void atomic_state_trans_SIS_to_CAP(BYTE *buf, int &cnt);  // change state from [Select_IR_Scan] to [Capture_IR]
void atomic_state_trans_CAP_to_SIR(BYTE *buf, int &cnt);  // change state from [Capture_IR] to [Shift_IR]
void atomic_state_trans_EX2_to_SIR(BYTE *buf, int &cnt);  // change state from [Exit2_IR] to [Shift_IR]


// Experimental Byte Shift operation
void intiate_ByteShift(BYTE *buf, int &cnt, bool to_read, unsigned nbytes);

#endif
